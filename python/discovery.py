"""
T100 openDAQ(TM) example -- Python -- Device Discovery

Uses openDAQ(TM)'s built-in mDNS discovery to find all T100 / openDAQ devices
on the local network and prints their name, connection string and server
capabilities (protocols they support).

NOTE: mDNS discovery relies on UDP multicast on the local subnet. Corporate
proxies / SASE clients such as Zscaler frequently block multicast traffic,
in which case this example will find no devices even though the T100 is
reachable. In that situation, connect directly by IP address instead
(see torque_speed_reader.py) using a connection string such as
"daq.lt://<device-ip>:7414" (the T100 only supports the openDAQ(TM) LT
streaming protocol, server capability ID "OpenDAQLTStreaming").
"""

import opendaq as daq


def main():
    instance = daq.Instance()

    print("Scanning the network for openDAQ(TM) devices (mDNS discovery)...")
    available_devices = instance.available_devices

    if not available_devices:
        print("No devices found.")
        print("If you know the device's IP address (e.g. behind a proxy/VPN such as")
        print("Zscaler that blocks mDNS), connect directly instead, e.g.:")
        print('    instance.add_device("daq.lt://172.19.211.82:7414")')
        return

    for info in available_devices:
        print("-----------------------------------")
        print(f"Name:              {info.name}")
        print(f"Connection string: {info.connection_string}")
        print(f"Manufacturer:      {info.manufacturer}")
        print(f"Serial number:     {info.serial_number}")

        for capability in info.server_capabilities:
            print(f"  Protocol: {capability.protocol_name}  ->  {capability.connection_string}")

    print("-----------------------------------")


if __name__ == "__main__":
    main()
