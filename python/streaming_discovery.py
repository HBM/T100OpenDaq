"""
T100 openDAQ(TM) example -- Python -- Discovery streaming

Uses openDAQ(TM)'s built-in mDNS discovery to automatically find a T100
device on the local network (server capability "OpenDAQLTStreaming"),
connects to it and streams the two measurement signals "torque-1" and
"speed-1".

NOTE: mDNS discovery relies on UDP multicast on the local subnet. Some
network setups (proxies, VPN/SASE clients, segmented networks) block this
traffic. If no device is found, use streaming_manual.py instead and provide
the device's connection string directly, e.g.:

    python streaming_manual.py daq.lt://<device-ip>:7414

Usage:
    python streaming_discovery.py
"""

import sys
import time

import opendaq as daq


def main():
    instance = daq.Instance()

    print("Scanning the network for openDAQ(TM) LT devices (mDNS discovery)...")
    available_devices = instance.available_devices

    # Find the first device that advertises the "OpenDAQLTStreaming" capability (the T100's protocol)
    connection_string = None
    device_name = None
    for info in available_devices:
        for capability in info.server_capabilities:
            if capability.protocol_name == "OpenDAQLTStreaming":
                connection_string = capability.connection_string
                device_name = info.name
                break
        if connection_string:
            break

    if not connection_string:
        print("No openDAQ(TM) LT device found via discovery.", file=sys.stderr)
        print("If discovery is blocked in your network, use streaming_manual.py instead", file=sys.stderr)
        print("and pass the device's connection string directly, e.g.:", file=sys.stderr)
        print("    python streaming_manual.py daq.lt://<device-ip>:7414", file=sys.stderr)
        sys.exit(1)

    print(f"Found device: {device_name} -> {connection_string}")

    try:
        device = instance.add_device(connection_string)
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
