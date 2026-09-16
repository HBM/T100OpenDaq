"""
T100 openDAQ(TM) example -- Python -- Manual connection string streaming

Connects to a T100 device using an explicitly given openDAQ(TM) connection
string (no mDNS/network discovery involved) and streams the two measurement
signals "torque-1" and "speed-1".

The T100 only supports the openDAQ(TM) LT streaming protocol
(server capability ID "OpenDAQLTStreaming"), so the connection string must
use the "daq.lt://" prefix, e.g.:

    python streaming_manual.py daq.lt://192.168.0.10/stream

Usage:
    python streaming_manual.py <connection-string>

Install dependencies:
    pip install -r requirements.txt
"""

import sys
import time

import opendaq as daq


def main():
    if len(sys.argv) != 2:
        print(f"Usage: python {sys.argv[0]} <connection-string>", file=sys.stderr)
        print(f"Example: python {sys.argv[0]} daq.lt://192.168.0.10/stream", file=sys.stderr)
        sys.exit(1)

    connection_string = sys.argv[1]

    # Create a fresh openDAQ(TM) instance
    instance = daq.Instance()

    print(f"Connecting to {connection_string} ...")
    try:
        device = instance.add_device(connection_string)
    except Exception as e:
        print(f"Failed to connect to device: {e}", file=sys.stderr)
        sys.exit(1)

    print(f"Connected to: {device.name}")
    speedChannelName = "speed-1"
    torqueChannelName = "torque-1"

    # List all available signals on the device for debugging/reference
    all_signals = device.get_signals_recursive()
    print(f"Available signals ({len(all_signals)}):")
    for signal in all_signals:
        print(f"  - name={signal.name}, local_id={signal.local_id}")

    # Find the "torque-1" and "speed-1" signals anywhere in the device's component tree
    torque_signals = device.get_signals_recursive(daq.LocalIdSearchFilter(torqueChannelName))
    speed_signals = device.get_signals_recursive(daq.LocalIdSearchFilter(speedChannelName))

    has_torque = len(torque_signals) > 0
    has_speed = len(speed_signals) > 0

    if not has_torque and not has_speed:
        print(f"Could not find signals '{torqueChannelName}' and/or '{speedChannelName}' on the device.", file=sys.stderr)
        sys.exit(1)

    found = []
    if has_torque:
        torque_signal = torque_signals[0]
        found.append(torque_signal.name)
    if has_speed:
        speed_signal = speed_signals[0]
        found.append(speed_signal.name)

    print(f"Found signals: {', '.join(found)}")

    # Create stream readers only for the signals that are present
    # Wrap with a time reader to get human-readable timestamps for the domain values
    torque_time_reader = daq.TimeStreamReader(daq.StreamReader(torque_signal)) if has_torque else None
    speed_time_reader = daq.TimeStreamReader(daq.StreamReader(speed_signal)) if has_speed else None

    print("Reading torque-1 / speed-1 ... (Ctrl+C to stop)")

    try:
        while True:
            time.sleep(0.1)

            if has_torque:
                torque_values, torque_timestamps = torque_time_reader.read_with_timestamps(100)
                for value, timestamp in zip(torque_values, torque_timestamps):
                    print(f"{torqueChannelName}: {float(value):.4f} Nm @ {timestamp}")

            if has_speed:
                speed_values, speed_timestamps = speed_time_reader.read_with_timestamps(100)
                for value, timestamp in zip(speed_values, speed_timestamps):
                    print(f"{speedChannelName}: {float(value):.4f} rpm @ {timestamp}")
    except KeyboardInterrupt:
        print("\nStopped by user.")


if __name__ == "__main__":
    main()
