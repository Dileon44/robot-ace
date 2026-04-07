#!/usr/bin/env python3
"""
Encoder angle data receiver and plotter.

Reads lines in format "%u;%u\r\n" (angle;timestamp_ms) from a serial port
for N seconds, then displays a plot of raw angle vs time.

Usage:
    python3 receive.py <port> <baudrate> <duration>

Example:
    python3 receive.py /dev/ttyACM0 115200 5
    python3 receive.py /dev/ttyUSB0 921600 10
"""

import argparse
import sys
import time

import matplotlib.pyplot as plt
import mplcursors
import serial

SUPPORTED_BAUDRATES = [115200, 230400, 460800, 921600]


def parse_args():
    parser = argparse.ArgumentParser(
        description="Receive encoder angle data over serial and plot it."
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
    parser.add_argument(
        "duration",
        type=float,
        help="Collection duration in seconds (e.g. 5, 10.5)",
    )
    return parser.parse_args()


def receive_data(port: str, baudrate: int, duration: float):
    """Open serial port and collect (angle, time_s) pairs for `duration` seconds."""
    angles = []
    timestamps = []

    print(f"Opening {port} at {baudrate} baud...")
    with serial.Serial(port, baudrate, timeout=1.0) as ser:
        # Flush any stale data in the buffer.
        # Wait briefly so any in-flight UART bytes arrive, then flush again.
        ser.reset_input_buffer()
        time.sleep(0.1)
        ser.reset_input_buffer()

        deadline = time.monotonic() + duration
        print(f"Collecting data for {duration} s  (press Ctrl+C to stop early)...")

        try:
            while time.monotonic() < deadline:
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
                    ts_ms  = int(parts[1])
                except ValueError:
                    continue

                if not (0.0 <= angle <= 4095.0):
                    continue

                angles.append(angle)
                timestamps.append(ts_ms)

        except KeyboardInterrupt:
            print("\nCollection stopped by user.")

    print(f"Collected {len(angles)} samples.")

    if len(timestamps) >= 2:
        diffs = [(timestamps[i] - timestamps[i - 1]) % (2**32) for i in range(1, len(timestamps))]
        avg_ms = sum(diffs) / len(diffs)
        print(f"Average interval between samples: {avg_ms:.2f} ms")

    return timestamps, angles


def plot_data(timestamps, angles):
    if not timestamps:
        print("No data to plot.")
        return

    plt.rcParams.update({"font.size": 20})

    fig, ax = plt.subplots(figsize=(10, 5))
    line, = ax.plot(timestamps, angles, linewidth=0.8, color="royalblue", marker="o", markersize=3)
    cursor = mplcursors.cursor(line, hover=True)
    cursor.connect("add", lambda sel: sel.annotation.set_text(
        f"t={sel.target[0]:.0f}ms\nangle={sel.target[1]:.1f}"
    ))
    ax.set_xlabel("t, ms")
    ax.set_ylabel("raw angle")
    ax.set_title("Encoder raw angle over time")
    ax.set_ylim(-50, 4145)
    ax.grid(True, linestyle="--", alpha=0.5)
    plt.tight_layout()
    plt.show()


def main():
    args = parse_args()
    timestamps, angles = receive_data(args.port, args.baudrate, args.duration)
    plot_data(timestamps, angles)


if __name__ == "__main__":
    main()
