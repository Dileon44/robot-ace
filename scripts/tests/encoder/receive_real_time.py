#!/usr/bin/env python3
"""
Encoder angle real-time plotter.

Reads lines in format "angle;timestamp_ms\r\n" from a serial port
and plots incoming data live. Only the last 10 seconds are shown.

Usage:
    python3 receive_real_time.py <port> <baudrate>

Example:
    python3 receive_real_time.py /dev/ttyACM0 230400
"""

import argparse
import sys
import threading
import time
from collections import deque

import matplotlib.animation as animation
import matplotlib.pyplot as plt
import serial

SUPPORTED_BAUDRATES = [115200, 230400, 460800, 921600]
WINDOW_MS = 10_000  # visible window width in ms (host time)


def parse_args():
    parser = argparse.ArgumentParser(
        description="Real-time encoder angle plotter over serial."
    )
    parser.add_argument(
        "port",
        help="Serial port device (e.g. /dev/ttyACM0, /dev/ttyUSB0, COM3)",
    )
    parser.add_argument(
        "baudrate",
        type=int,
        choices=SUPPORTED_BAUDRATES,
        help=f"Baud rate: {SUPPORTED_BAUDRATES}",
    )
    return parser.parse_args()


# Shared buffers (host monotonic time [s], angle)
_lock = threading.Lock()
_times: deque = deque()
_angles: deque = deque()
_running = True


def _reader_thread(port: str, baudrate: int) -> None:
    global _running
    try:
        with serial.Serial(port, baudrate, timeout=1.0) as ser:
            ser.reset_input_buffer()
            time.sleep(0.1)
            ser.reset_input_buffer()

            while _running:
                raw = ser.readline()
                if not raw:
                    continue

                try:
                    line = raw.decode("ascii", errors="ignore").strip()
                except Exception:
                    continue

                if not line:
                    continue

                parts = line.split(";")
                if len(parts) != 2:
                    continue

                try:
                    angle = float(parts[0])
                    int(parts[1])  # validate timestamp field
                except ValueError:
                    continue

                if not (0.0 <= angle <= 4095.0):
                    continue

                now = time.monotonic()
                with _lock:
                    _times.append(now)
                    _angles.append(angle)

    except serial.SerialException as e:
        print(f"\nSerial error: {e}")
        _running = False


def main():
    global _running

    args = parse_args()

    thread = threading.Thread(target=_reader_thread, args=(args.port, args.baudrate), daemon=True)
    thread.start()
    print(f"Opened {args.port} at {args.baudrate} baud. Close the window to stop.")

    plt.rcParams.update({"font.size": 20})
    fig, ax = plt.subplots(figsize=(12, 6))
    line, = ax.plot([], [], linewidth=0.8, color="royalblue", marker="o", markersize=3)
    ax.set_xlabel("t, s")
    ax.set_ylabel("angle")
    ax.set_title("Encoder angle (real-time)")
    ax.grid(True, linestyle="--", alpha=0.5)

    def update(_frame):
        with _lock:
            if not _times:
                return line,
            ts = list(_times)
            an = list(_angles)

        now = ts[-1]
        cutoff = now - WINDOW_MS / 1000.0

        # Drop samples older than the window
        start_idx = 0
        for i, t in enumerate(ts):
            if t >= cutoff:
                start_idx = i
                break

        ts = ts[start_idx:]
        an = an[start_idx:]

        t0 = ts[0]
        t_rel = [t - t0 for t in ts]

        line.set_data(t_rel, an)
        ax.set_xlim(0, WINDOW_MS / 1000.0)

        margin = (max(an) - min(an)) * 0.03 if max(an) != min(an) else 1.0
        ax.set_ylim(min(an) - margin, max(an) + margin)
        return line,

    ani = animation.FuncAnimation(fig, update, interval=50, blit=False, cache_frame_data=False)

    try:
        plt.tight_layout()
        plt.show()
    finally:
        _running = False

    sys.exit(0)


if __name__ == "__main__":
    main()
