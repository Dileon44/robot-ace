#!/usr/bin/env python3
"""
Firmware build script for robot-ace project.
Parses key=value build arguments, translates them into CMake -D flags,
and runs the appropriate CMake configure + build commands.

Usage:
    fw_builder.py build action=rebuild preset=m0r0c0 type=app tag=prd bsp=0
    fw_builder.py clean preset=m0r0c0
    fw_builder.py configure preset=m0r0c0
"""

import logging
import re
import subprocess
import sys
from pathlib import Path
from time import time

from memory_table import RE_DATA as _RE_MEMORY_DATA
from memory_table import render as _render_memory
from memory_table import render_sections as _render_sections
from tqdm import tqdm

log = logging.getLogger(__name__)

ROOT_DIR = Path(__file__).resolve().parent.parent
BUILD_BASE_DIR = ROOT_DIR / "build"

# ANSI color codes
class C:
    RED    = "\033[31m"
    YELLOW = "\033[33m"
    CYAN   = "\033[36m"
    GREEN  = "\033[32m"
    BLUE   = "\033[34m"
    WHITE  = "\033[37m"
    DIM    = "\033[2m"
    BOLD   = "\033[1m"
    RESET  = "\033[0m"

# Mapping: build-arg key -> CMake cache variable name that will be
# passed via -D and will appear as a preprocessor macro in C code.
# Values are forwarded verbatim unless a transformer function is applied.
ARG_TO_CMAKE_DEFINE: dict[str, str] = {
    "tag":      "FW_TYPE",    # prd / dev
    "bsp":      "FW_BSP",
    "target":   "FW_TARGET",  # app / boot
    "opt":      "FW_OPT",     # 0 = -O0 (debug), 1 = -O1
}

_RE_ERROR   = re.compile(r"\berror:", re.IGNORECASE)
_RE_WARNING = re.compile(r"\bwarning:", re.IGNORECASE)
_RE_NOTE    = re.compile(r"\bnote:", re.IGNORECASE)
_RE_FAILED  = re.compile(r"^FAILED:")
_RE_BUILD   = re.compile(r"^\[[\d ]+/[\d ]+\] Building")
_RE_LINK    = re.compile(r"^\[[\d ]+/[\d ]+\] Linking")
_RE_PROGRESS= re.compile(r"^\[[\d ]+/[\d ]+\]")


def colorize_line(line: str) -> str:
    if _RE_FAILED.match(line):
        return C.BOLD + C.RED + line + C.RESET
    if _RE_ERROR.search(line):
        return C.RED + line + C.RESET
    if _RE_WARNING.search(line):
        return C.YELLOW + line + C.RESET
    if _RE_NOTE.search(line):
        return C.DIM + line + C.RESET
    if _RE_LINK.match(line):
        return C.BOLD + C.BLUE + line + C.RESET
    if _RE_BUILD.match(line):
        return C.DIM + line + C.RESET
    if _RE_PROGRESS.match(line):
        return C.CYAN + line + C.RESET
    return line


class _ColoredFormatter(logging.Formatter):
    _COLORS = {
        logging.DEBUG:    C.DIM,
        logging.INFO:     C.CYAN,
        logging.WARNING:  C.YELLOW,
        logging.ERROR:    C.RED,
        logging.CRITICAL: C.BOLD + C.RED,
    }

    def format(self, record: logging.LogRecord) -> str:
        color = self._COLORS.get(record.levelno, "")
        msg = super().format(record)
        return color + msg + C.RESET


def setup_logging() -> None:
    fmt = "%(asctime)s [%(levelname)-7s] %(message)s"
    handler = logging.StreamHandler(sys.stdout)
    handler.setFormatter(_ColoredFormatter(fmt))
    root = logging.getLogger()
    root.setLevel(logging.DEBUG)
    root.addHandler(handler)


def parse_args(raw: list[str]) -> dict[str, str]:
    """Parse a list of 'key=value' strings into a dict."""
    result: dict[str, str] = {}
    for item in raw:
        if "=" in item:
            k, v = item.split("=", 1)
            result[k.strip()] = v.strip()
        else:
            log.warning(f"Ignoring argument without '=': {item!r}")
    return result


_RE_LD_ORIGIN = re.compile(
    r"^\s*(\w+)\s*(?:\([^)]*\)\s*)?:\s*ORIGIN\s*=\s*(0x[0-9a-fA-F]+)",
    re.IGNORECASE,
)


def _read_linker_origins(ld: Path) -> dict[str, str] | None:
    """Parse MEMORY block in a linker script, return region_name → hex_origin."""
    try:
        origins: dict[str, str] = {}
        with ld.open() as f:
            for line in f:
                m = _RE_LD_ORIGIN.match(line)
                if m:
                    origins[m.group(1)] = m.group(2)
        return origins or None
    except Exception:
        return None


def _read_elf_sections(elf: Path) -> dict[str, int] | None:
    """Run arm-none-eabi-size on *elf* and return text/data/bss sizes in bytes."""
    try:
        out = subprocess.check_output(
            ["arm-none-eabi-size", str(elf)],
            stderr=subprocess.DEVNULL,
            text=True,
        )
        # Output format:  text   data    bss    dec    hex  filename
        for line in out.splitlines()[1:]:
            parts = line.split()
            if len(parts) >= 3:
                return {"text": int(parts[0]), "data": int(parts[1]), "bss": int(parts[2])}
    except Exception:
        pass
    return None


def run(cmd: list[str], cwd: Path, show_progress: bool = False,
        elf_path: Path | None = None,
        ld_path: Path | None = None) -> int:
    """Run a subprocess command, stream and colorize output, return exit code."""
    log.info("Run: " + " ".join(cmd))
    process = subprocess.Popen(
        cmd,
        cwd=cwd,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        errors="replace",
    )

    pbar: tqdm | None = None
    memory_lines: list[str] = []
    in_memory_section = False

    for line in process.stdout:
        line = line.rstrip()

        # Intercept memory-region output for table rendering after build
        if line.lstrip().startswith("Memory region"):
            in_memory_section = True
            continue
        if in_memory_section:
            if _RE_MEMORY_DATA.match(line):
                memory_lines.append(line)
                continue
            in_memory_section = False

        m = re.match(r"^\[(\s*\d+)/(\s*\d+)\]", line)
        if m and show_progress:
            current, t = int(m.group(1)), int(m.group(2))

            # [0/N] is ninja's internal status step (e.g. "Re-checking globbed
            # directories") — not a real compilation unit; skip from bar tracking
            if current == 0:
                stripped = re.sub(r"^\[\s*\d+/\s*\d+\]\s*", "", line)
                (pbar.write if pbar is not None else print)(colorize_line(stripped))
                continue

            if pbar is None:
                bar_fmt = "{l_bar}{bar:60}{r_bar}"
                pbar = tqdm(
                    total=t,
                    bar_format=bar_fmt,
                    colour="cyan",
                    file=sys.stdout,
                    dynamic_ncols=True,
                )
                pbar.n = current - 1
                pbar.refresh()
            elif pbar.total != t:
                pbar.total = t
                pbar.refresh()

            # Building lines → update bar silently
            if _RE_BUILD.match(line):
                fname = line.split("/")[-1] if "/" in line else line
                pbar.set_postfix_str(fname, refresh=False)
                pbar.update(1)
                continue

            # Linking / other [X/Y] → update bar, print above it (strip prefix)
            pbar.update(1)
            pbar.write(colorize_line(re.sub(r"^\[\s*\d+/\s*\d+\]\s*", "", line)))
            continue

        # Non-progress line: print above bar (if open) or directly
        stripped = re.sub(r"^\[\s*\d+/\s*\d+\]\s*", "", line)
        if pbar is not None:
            pbar.write(colorize_line(stripped))
        else:
            print(colorize_line(stripped))

    if pbar is not None:
        pbar.close()

    if memory_lines:
        origins = _read_linker_origins(ld_path) if ld_path else None
        _render_memory(memory_lines, origins)
        sections = _read_elf_sections(elf_path) if elf_path else None
        if sections:
            _render_sections(sections)

    process.wait()
    return process.returncode


def build_cmake_defines(args: dict[str, str]) -> list[str]:
    """
    Translate build arguments into CMake -DKEY=VALUE flags.
    These become compile-time macros visible in C code via add_compile_definitions
    in CMakeLists.txt.
    """
    defines: list[str] = []
    for arg_key, cmake_var in ARG_TO_CMAKE_DEFINE.items():
        val = args.get(arg_key)
        if val is not None:
            defines.append(f"-D{cmake_var}={val}")
    return defines


def get_preset(args: dict[str, str]) -> str:
    preset = args.get("preset")
    if not preset:
        log.error("'preset' argument is required (e.g. preset=m0r0c0)")
        sys.exit(1)
    return preset


def cmd_configure(args: dict[str, str]) -> int:
    preset = get_preset(args)
    extra_defines = build_cmake_defines(args)
    cmake_args = ["cmake", "--preset", preset] + extra_defines
    return run(cmake_args, ROOT_DIR)


def cmd_build(args: dict[str, str]) -> int:
    preset = get_preset(args)
    build_dir = BUILD_BASE_DIR / preset
    target_name = args.get("target", "app")
    cmake_target = target_name

    if not (build_dir / "CMakeCache.txt").exists():
        log.info("CMakeCache.txt not found — running configure first")
        ret = cmd_configure(args)
        if ret != 0:
            return ret

    elf_path: Path | None = None
    build_dir_root = ROOT_DIR / "build"
    if target_name == "boot":
        candidates = sorted(build_dir_root.glob("*.boot.*.elf"))
    else:
        candidates = sorted(build_dir_root.glob("*.app.*.elf"))
    if candidates:
        elf_path = candidates[-1]

    ld_path = BUILD_BASE_DIR / f"{preset}_{target_name}_linker_script.ld"

    cmake_args = ["cmake", "--build", str(build_dir), "--target", cmake_target]
    return run(cmake_args, ROOT_DIR, show_progress=True, elf_path=elf_path, ld_path=ld_path)


def cmd_clean(args: dict[str, str]) -> int:
    preset = get_preset(args)
    build_dir = BUILD_BASE_DIR / preset

    if not (build_dir / "CMakeCache.txt").exists():
        log.warning(f"Nothing to clean: {build_dir} has no CMakeCache.txt")
        return 0

    cmake_args = ["cmake", "--build", str(build_dir), "--target", "clean"]
    ret = run(cmake_args, ROOT_DIR)

    # Remove versioned artifacts copied to build/
    for pattern in ("*.elf", "*.hex", "*.bin", "*.map"):
        for f in BUILD_BASE_DIR.glob(pattern):
            f.unlink()
            log.info(f"Removed: {f}")

    # Remove legacy/accumulated versioned ELF files inside preset subdirs
    for target_subdir in ("app", "boot"):
        for f in (build_dir / target_subdir).glob("*.elf*"):
            f.unlink()
            log.info(f"Removed: {f}")

    return ret


def cmd_rebuild(args: dict[str, str]) -> int:
    ret = cmd_configure(args)
    if ret != 0:
        return ret
    ret = cmd_clean(args)
    if ret != 0:
        return ret
    return cmd_build(args)


def main() -> int:
    setup_logging()

    if len(sys.argv) < 2:
        log.error("Usage: fw_builder.py <command> [key=value ...]")
        log.error("Commands: configure | build | clean | rebuild")
        return 1

    command = sys.argv[1]
    args = parse_args(sys.argv[2:])

    log.info(f"Command : {command}")
    log.info(f"Args    : {args}")

    start = time()

    match command:
        case "configure":
            ret = cmd_configure(args)
        case "build":
            ret = cmd_build(args)
        case "clean":
            ret = cmd_clean(args)
        case "rebuild":
            ret = cmd_rebuild(args)
        case _:
            log.error(f"Unknown command: {command!r}. Use: configure | build | clean | rebuild")
            return 1

    elapsed = time() - start
    if ret != 0:
        print(C.BOLD + C.RED + f"\n❌ Build failed ({elapsed:.1f}s)" + C.RESET)

    return ret


if __name__ == "__main__":
    sys.exit(main())

