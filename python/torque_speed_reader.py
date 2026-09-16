"""
T100 openDAQ(TM) example -- Python

Connects directly to a T100 device via its IP address (bypassing mDNS/discovery,
e.g. when the network scan is blocked by a proxy such as Zscaler) and streams
the two measurement signals "torque-1" and "speed-1".

Adjust DEVICE_IP / DEVICE_CONNECTION_STRING to match your T100 device.

Install dependencies:
    pip install -r requirements.txt
"""

import sys
import time

import opendaq as daq

# IP address of the T100 device (from the customer's network)
DEVICE_IP = "172.19.211.82"

# The T100 only supports the openDAQ(TM) LT streaming protocol (server capability ID
# "OpenDAQLTStreaming"), not the full OPC UA configuration protocol. LT devices expose
# a flat list of signals directly (no full component tree). The "daq.lt://" prefix
# selects this protocol explicitly; the default LT port is 7414 and can be omitted.
DEVICE_CONNECTION_STRING = f"daq.lt://{DEVICE_IP}:7414"


def main():
    # Create a fresh openDAQ(TM) instance
    instance = daq.Instance()

    print(f"Connecting to T100 at {DEVICE_CONNECTION_STRING} ...")
    try:
        device = instance.add_device(DEVICE_CONNECTION_STRING)
    except Exception as e:
        print(f"Failed to connect to device: {e}", file=sys.stderr)
        sys.exit(1)

    print(f"Connected to: {device.name}")

    # Find the "torque-1" and "speed-1" signals anywhere in the device's component tree
    torque_signals = device.get_signals_recursive(daq.LocalIdSearchFilter("torque-1"))
    speed_signals = device.get_signals_recursive(daq.LocalIdSearchFilter("speed-1"))

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
            if len(torque_values) > 0:
                print(f"torque-1: {torque_values[-1]:.4f} Nm  @ {torque_timestamps[-1]}")

            speed_values, speed_timestamps = speed_time_reader.read_with_timestamps(100)
            if len(speed_values) > 0:
                print(f"speed-1:  {speed_values[-1]:.4f} rpm @ {speed_timestamps[-1]}")
    except KeyboardInterrupt:
        print("\nStopped by user.")


if __name__ == "__main__":
    main()
