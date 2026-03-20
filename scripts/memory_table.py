#!/usr/bin/env python3
"""Pretty-print CMake linker memory-usage output as a table."""

import re

_RESET  = "\033[0m"
_BOLD   = "\033[1m"
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


def render(raw_lines: list[str]) -> None:
    """Parse raw memory-region lines and print a formatted table."""
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

    used_strs  = [f"{e[1]} {e[2]}" for e in entries]
    total_strs = [f"{e[3]} {e[4]}" for e in entries]
    pct_strs   = [f"{e[5]:.1f}%"   for e in entries]

    w_r = max(len("Region"), *(len(e[0]) for e in entries))
    w_u = max(len("Used"),   *(len(s)    for s in used_strs))
    w_t = max(len("Total"),  *(len(s)    for s in total_strs))
    w_p = max(len("Usage"),  *(len(s)    for s in pct_strs))
    # last column visual content: bar + space + pct
    iw_bp = _BAR_WIDTH + 1 + w_p

    def hline(l: str, mid: str, r: str, fill: str = "─") -> str:
        return (
            l + fill * (w_r + 2) + mid +
            fill * (w_u + 2) + mid +
            fill * (w_t + 2) + mid +
            fill * (iw_bp + 2) + r
        )

    top = hline("┌", "┬", "┐")
    sep = hline("├", "┼", "┤")
    bot = hline("└", "┴", "┘")

    usage_label = f"{'Usage':^{iw_bp}}"
    header = (
        f"│ {'Region':<{w_r}} │ {'Used':>{w_u}} │ {'Total':>{w_t}} │ {usage_label} │"
    )

    print(_BOLD + _CYAN + top    + _RESET)
    print(_BOLD + _CYAN + header + _RESET)
    print(_BOLD + _CYAN + sep    + _RESET)

    for (region, _, _, _, _, pct), us, ts, ps in zip(entries, used_strs, total_strs, pct_strs):
        color = _pct_color(pct)
        bar   = _bar(pct)
        print(
            f"│ {_BOLD}{region:<{w_r}}{_RESET} "
            f"│ {us:>{w_u}} "
            f"│ {ts:>{w_t}} "
            f"│ {color}{bar}{_RESET} {color}{ps:>{w_p}}{_RESET} │"
        )

    print(_BOLD + _CYAN + bot + _RESET)
