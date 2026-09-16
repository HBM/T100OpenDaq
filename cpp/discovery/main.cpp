/**
 * T100 openDAQ(TM) example — C++ — Device Discovery
 *
 * Uses openDAQ(TM)'s built-in mDNS discovery to find all T100 / openDAQ devices
 * on the local network and prints their name, connection string and server
 * capabilities (protocols they support).
 *
 * NOTE: mDNS discovery relies on UDP multicast on the local subnet. Corporate
 * proxies / SASE clients such as Zscaler frequently block multicast traffic,
 * in which case this example will find no devices even though the T100 is
 * reachable. In that situation, connect directly by IP address instead
 * (see the "torque_speed_reader" example) using a connection string such as
 * "daq.lt://<device-ip>:7414" (the T100 only supports the openDAQ(TM) LT
 * streaming protocol, server capability ID "OpenDAQLTStreaming").
 */

#include <opendaq/opendaq.h>
#include <iostream>

using namespace daq;

int main()
{
    InstancePtr instance = Instance();

    std::cout << "Scanning the network for openDAQ(TM) devices (mDNS discovery)..." << std::endl;

    ListPtr<IDeviceInfo> availableDevices = instance.getAvailableDevices();

    if (availableDevices.getCount() == 0)
    {
        std::cout << "No devices found.\n"
                  << "If you know the device's IP address (e.g. behind a proxy/VPN such as Zscaler\n"
                  << "that blocks mDNS), connect directly instead, e.g.:\n"
                  << "    instance.addDevice(\"daq.lt://172.19.211.82:7414\");" << std::endl;
        return 0;
    }

    for (const auto& info : availableDevices)
    {
        std::cout << "-----------------------------------" << std::endl;
        std::cout << "Name:              " << info.getName() << std::endl;
        std::cout << "Connection string: " << info.getConnectionString() << std::endl;
        std::cout << "Manufacturer:      " << info.getManufacturer() << std::endl;
        std::cout << "Serial number:     " << info.getSerialNumber() << std::endl;

        for (const auto& capability : info.getServerCapabilities())
        {
            std::cout << "  Protocol: " << capability.getProtocolName()
                      << "  ->  " << capability.getConnectionString() << std::endl;
        }
    }
    std::cout << "-----------------------------------" << std::endl;

    return 0;
}
