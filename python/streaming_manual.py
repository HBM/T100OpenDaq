"""
T100 openDAQ(TM) example -- Python -- Manual connection string streaming

Connects to a T100 device using an explicitly given openDAQ(TM) connection
string (no mDNS/network discovery involved) and streams the two measurement
signals "torque-1" and "speed-1".

The T100 only supports the openDAQ(TM) LT streaming protocol
(server capability ID "OpenDAQLTStreaming"), so the connection string must
use the "daq.lt://" prefix, e.g.:

    python streaming_manual.py daq.lt://192.168.0.10:7414

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
        print(f"Example: python {sys.argv[0]} daq.lt://192.168.0.10:7414", file=sys.stderr)
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
    

    if not torque_signals or not speed_signals:
        print("Could not find signals 'torque-1' and/or 'speed-1' on the device.", file=sys.stderr)
        sys.exit(1)

    torque_signal = torque_signals[0]
    speed_signal = speed_signals[0]

    print(f"Found signals: {torque_signal.name}, {speed_signal.name}")

    # Create stream readers for both signals
    torque_reader = daq.StreamReader(torque_signal)
    speed_reader = daq.StreamReader(speed_signal)

    # Wrap with a time reader to get human-readable timestamps for the domain values
    torque_time_reader = daq.TimeStreamReader(torque_reader)
    speed_time_reader = daq.TimeStreamReader(speed_reader)

    print("Reading torque-1 / speed-1 ... (Ctrl+C to stop)")

    try:
        while True:
            time.sleep(0.1)

            torque_values, torque_timestamps = torque_time_reader.read_with_timestamps(100)
            speed_values, speed_timestamps = speed_time_reader.read_with_timestamps(100)

            for (torque_value, torque_timestamp), (speed_value, speed_timestamp) in zip(
                zip(torque_values, torque_timestamps), zip(speed_values, speed_timestamps)
            ):
                print(
                    f"torque-1: {float(torque_value):.4f} Nm @ {torque_timestamp}  |  "
                    f"speed-1: {float(speed_value):.4f} rpm @ {speed_timestamp}"
                )
    except KeyboardInterrupt:
        print("\nStopped by user.")


if __name__ == "__main__":
    main()
