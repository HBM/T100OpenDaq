"""
T100 openDAQ(TM) example -- Python -- Discovery streaming

Uses openDAQ(TM)'s built-in mDNS discovery to automatically find a T100
device on the local network (device family "T100", server capability
"OpenDAQLTStreaming"), connects to it and streams the two measurement
signals "torque-1" and "speed-1".

An optional UUID argument can be passed to pick a specific T100 out of
several devices found by the scan (matched against the device's
system UUID). Without it, the first matching T100 is used.

NOTE: mDNS discovery relies on UDP multicast on the local subnet. Some
network setups (proxies, VPN/SASE clients, segmented networks) block this
traffic. If no device is found, use streaming_manual.py instead and provide
the device's connection string directly, e.g.:

    python streaming_manual.py daq.lt://<device-ip>/stream

Usage:
    python streaming_discovery.py [uuid]
"""

import sys
import time

import opendaq as daq

T100_FAMILY = "t100"


def is_t100_device(info) -> bool:
    return T100_FAMILY in info.model.lower() or T100_FAMILY in info.name.lower()


def main():
    uuid_filter = sys.argv[1] if len(sys.argv) > 1 else None

    instance = daq.Instance()

    print("Scanning the network for openDAQ(TM) LT devices (mDNS discovery)...")
    available_devices = instance.available_devices

    # Only consider devices of the T100 family that advertise the "OpenDAQLTStreaming" capability
    candidates = []
    for info in available_devices:
        if not is_t100_device(info):
            continue
        for capability in info.server_capabilities:
            if capability.protocol_name == "OpenDAQLTStreaming":
                candidates.append((info, capability.connection_string))
                break

    if not candidates:
        print("No T100 openDAQ(TM) LT device found via discovery.", file=sys.stderr)
        print("If discovery is blocked in your network, use streaming_manual.py instead", file=sys.stderr)
        print("and pass the device's connection string directly, e.g.:", file=sys.stderr)
        print("    python streaming_manual.py daq.lt://<device-ip>/stream", file=sys.stderr)
        sys.exit(1)

    if uuid_filter:
        matching = [c for c in candidates if c[0].system_uuid.lower() == uuid_filter.lower()]
        if not matching:
            found_uuids = ", ".join(info.system_uuid for info, _ in candidates) or "(none reported a UUID)"
            print(f"No T100 device with UUID '{uuid_filter}' found in scan results.", file=sys.stderr)
            print(f"UUIDs found: {found_uuids}", file=sys.stderr)
            sys.exit(1)
        info, connection_string = matching[0]
    else:
        info, connection_string = candidates[0]

    device_name = info.name
    print(f"Found device: {device_name} (uuid={info.system_uuid}) -> {connection_string}")

    try:
        device = instance.add_device(connection_string)
    except Exception as e:
        print(f"Failed to connect to device: {e}", file=sys.stderr)
        sys.exit(1)

    print(f"Connected to: {device.name}")

    # Find the "torque-1" and "speed-1" signals anywhere in the device's component tree
    torque_signals = device.get_signals_recursive(daq.LocalIdSearchFilter("torque-1"))
    speed_signals = device.get_signals_recursive(daq.LocalIdSearchFilter("speed-1"))

    has_torque = len(torque_signals) > 0
    has_speed = len(speed_signals) > 0

    if not has_torque and not has_speed:
        print("Could not find signals 'torque-1' and/or 'speed-1' on the device.", file=sys.stderr)
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
                if len(torque_values) > 0:
                    print(f"torque-1: {torque_values[-1]:.4f} Nm  @ {torque_timestamps[-1]}")

            if has_speed:
                speed_values, speed_timestamps = speed_time_reader.read_with_timestamps(100)
                if len(speed_values) > 0:
                    print(f"speed-1:  {speed_values[-1]:.4f} rpm @ {speed_timestamps[-1]}")
    except KeyboardInterrupt:
        print("\nStopped by user.")


if __name__ == "__main__":
    main()
