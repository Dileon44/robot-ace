#!/usr/bin/env python3
"""Pretty-print CMake linker memory-usage output as a table."""

import re

_RESET  = "\033[0m"
_BOLD   = "\033[1m"
_DIM    = "\033[2m"
_CYAN   = "\033[36m"
_GREEN  = "\033[32m"
_YELLOW = "\033[33m"
_RED    = "\033[31m"

# Matches lines like:  "             RAM:        7360 B        32 KB     22.46%"
RE_DATA = re.compile(r"^\s*(\w+):\s+(\d+)\s+(\w+)\s+(\d+)\s+(\w+)\s+([\d.]+)%")

_BAR_WIDTH = 20


def _pct_color(pct: float) -> str:
    if pct < 60.0:
        return _GREEN
    if pct < 80.0:
        return _YELLOW
    return _RED


def _bar(pct: float) -> str:
    filled = round(pct / 100.0 * _BAR_WIDTH)
    return "█" * filled + "░" * (_BAR_WIDTH - filled)


def render(raw_lines: list[str], origins: dict[str, str] | None = None) -> None:
    """Parse raw memory-region lines and print a formatted table.

    If *origins* is provided (region_name -> hex_address string), an Origin
    column is inserted between Region and Used.
    """
    entries: list[tuple] = []
    for line in raw_lines:
        m = RE_DATA.match(line)
        if m:
            entries.append((
                m.group(1),         # region name
                int(m.group(2)),    # used size
                m.group(3),         # used unit
                int(m.group(4)),    # total size
                m.group(5),         # total unit
                float(m.group(6)),  # percent
            ))
    if not entries:
        return

    orig_strs  = [origins.get(e[0], "—") for e in entries] if origins else None
    used_strs  = [f"{e[1]} {e[2]}" for e in entries]
    total_strs = [f"{e[3]} {e[4]}" for e in entries]
    pct_strs   = [f"{e[5]:.1f}%"   for e in entries]

    w_r = max(len("Region"), *(len(e[0]) for e in entries))
    w_o = max(len("Origin"), *(len(s) for s in orig_strs)) if orig_strs else 0
    w_u = max(len("Used"),   *(len(s) for s in used_strs))
    w_t = max(len("Total"),  *(len(s) for s in total_strs))
    w_p = max(len("Usage"),  *(len(s) for s in pct_strs))
    iw_bp = _BAR_WIDTH + 1 + w_p

    def hline(l: str, mid: str, r: str, fill: str = "─") -> str:
        s = l + fill * (w_r + 2) + mid
        if orig_strs:
            s += fill * (w_o + 2) + mid
        return (
            s +
            fill * (w_u + 2) + mid +
            fill * (w_t + 2) + mid +
            fill * (iw_bp + 2) + r
        )

    top = hline("┌", "┬", "┐")
    sep = hline("├", "┼", "┤")
    bot = hline("└", "┴", "┘")

    usage_label = f"{'Usage':^{iw_bp}}"
    orig_hdr = f" {'Origin':>{w_o}} │" if orig_strs else ""
    header = (
        f"│ {'Region':<{w_r}} │{orig_hdr} {'Used':>{w_u}} │ {'Total':>{w_t}} │ {usage_label} │"
    )

    print(_BOLD + _CYAN + top    + _RESET)
    print(_BOLD + _CYAN + header + _RESET)
    print(_BOLD + _CYAN + sep    + _RESET)

    sentinel = [""] * len(entries)
    for (region, _, _, _, _, pct), us, ts, ps, os in zip(
        entries, used_strs, total_strs, pct_strs, orig_strs if orig_strs else sentinel
    ):
        color = _pct_color(pct)
        bar   = _bar(pct)
        orig_cell = f" {os:>{w_o}}{_RESET} {_CYAN}│{_RESET}" if orig_strs else ""
        print(
            f"{_CYAN}│{_RESET} {_BOLD}{region:<{w_r}}{_RESET} {_CYAN}│{_RESET}{orig_cell}"
            f" {us:>{w_u}} "
            f"{_CYAN}│{_RESET} {ts:>{w_t}} "
            f"{_CYAN}│{_RESET} {color}{bar}{_RESET} {color}{ps:>{w_p}}{_RESET} {_CYAN}│{_RESET}"
        )

    print(_BOLD + _CYAN + bot + _RESET)


def render_sections(sections: dict[str, int]) -> None:
    """Print a compact ELF-section breakdown line below the memory table."""
    parts = [
        (".text", sections.get("text", 0)),
        (".data", sections.get("data", 0)),
        (".bss",  sections.get("bss",  0)),
    ]
    w = max(len(f"{v} B") for _, v in parts)
    items = [f"{_DIM}{lbl}{_RESET}  {f'{v} B':>{w}}" for lbl, v in parts]
    sep = f"  {_DIM}·{_RESET}  "
    print("  " + sep.join(items))
