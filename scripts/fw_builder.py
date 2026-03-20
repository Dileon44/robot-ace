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
import os
import subprocess
import sys
from pathlib import Path

log = logging.getLogger(__name__)

ROOT_DIR = Path(__file__).resolve().parent.parent
BUILD_BASE_DIR = ROOT_DIR / "build"

# Mapping: build-arg key -> CMake cache variable name that will be
# passed via -D and will appear as a preprocessor macro in C code.
# Values are forwarded verbatim unless a transformer function is applied.
ARG_TO_CMAKE_DEFINE: dict[str, str] = {
    "tag":      "FW_TYPE",    # prd / dev
    "bsp":      "FW_BSP",
    "target":   "FW_TARGET",  # app / boot
    "opt":      "FW_OPT",     # 0 = -O0 (debug), 1 = -O1
}


def setup_logging() -> None:
    fmt = "%(asctime)s [%(levelname)-7s] %(message)s"
    logging.basicConfig(level=logging.DEBUG, format=fmt, stream=sys.stdout)


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


def run(cmd: list[str], cwd: Path) -> int:
    """Run a subprocess command, stream output, return exit code."""
    log.info("Run: " + " ".join(cmd))
    result = subprocess.run(cmd, cwd=cwd)
    return result.returncode


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
    cmake_target = f"{target_name}.elf"

    # Configure first if build directory doesn't have CMakeCache.txt
    if not (build_dir / "CMakeCache.txt").exists():
        log.info("CMakeCache.txt not found — running configure first")
        ret = cmd_configure(args)
        if ret != 0:
            return ret

    cmake_args = ["cmake", "--build", str(build_dir), "--target", cmake_target]
    return run(cmake_args, ROOT_DIR)


def cmd_clean(args: dict[str, str]) -> int:
    preset = get_preset(args)
    build_dir = BUILD_BASE_DIR / preset

    if not (build_dir / "CMakeCache.txt").exists():
        log.warning(f"Nothing to clean: {build_dir} has no CMakeCache.txt")
        return 0

    cmake_args = ["cmake", "--build", str(build_dir), "--target", "clean"]
    return run(cmake_args, ROOT_DIR)


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

    match command:
        case "configure":
            return cmd_configure(args)
        case "build":
            return cmd_build(args)
        case "clean":
            return cmd_clean(args)
        case "rebuild":
            return cmd_rebuild(args)
        case _:
            log.error(f"Unknown command: {command!r}. Use: configure | build | clean | rebuild")
            return 1


if __name__ == "__main__":
    sys.exit(main())
