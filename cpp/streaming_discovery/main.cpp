/**
 * T100 openDAQ(TM) example -- C++ -- Discovery streaming
 *
 * Uses openDAQ(TM)'s built-in mDNS discovery to automatically find a T100
 * device on the local network (server capability "OpenDAQLTStreaming"),
 * connects to it and streams the two measurement signals "torque-1" and
 * "speed-1".
 *
 * NOTE: mDNS discovery relies on UDP multicast on the local subnet. Some
 * network setups (proxies, VPN/SASE clients, segmented networks) block this
 * traffic. If no device is found, use the "streaming_manual" example instead
 * and provide the device's connection string directly, e.g.:
 *
 *     ./streaming_manual daq.lt://<device-ip>/stream
 *
 * Usage:
 *     streaming_discovery
 */

#include <opendaq/opendaq.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <thread>
#include <optional>

using namespace daq;
using namespace std::chrono_literals;

// std::chrono::system_clock::time_point has no operator<< in C++17
static std::string formatTimestamp(const std::chrono::system_clock::time_point& timePoint)
{
    auto time = std::chrono::system_clock::to_time_t(timePoint);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

int main()
{
    // Create a fresh openDAQ(TM) instance
    // OPENDAQ_MODULES_DIR is defined by CMake to point at the SDK's modules folder,
    // since executables don't have modules alongside them like the Python package does
    InstancePtr instance = Instance(OPENDAQ_MODULES_DIR);

    std::cout << "Scanning the network for openDAQ(TM) LT devices (mDNS discovery)..." << std::endl;

    ListPtr<IDeviceInfo> availableDevices = instance.getAvailableDevices();

    // Find the first device that advertises the "OpenDAQLTStreaming" capability (the T100's protocol)
    StringPtr connectionString;
    StringPtr deviceName;
    for (const auto& info : availableDevices)
    {
        for (const auto& capability : info.getServerCapabilities())
        {
            if (capability.getProtocolName() == "OpenDAQLTStreaming")
            {
                connectionString = capability.getConnectionString();
                deviceName = info.getName();
                break;
            }
        }
        if (connectionString.assigned())
            break;
    }

    if (!connectionString.assigned())
    {
        std::cerr << "No openDAQ(TM) LT device found via discovery.\n"
                  << "If discovery is blocked in your network, use 'streaming_manual' instead\n"
                  << "and pass the device's connection string directly, e.g.:\n"
                  << "    ./streaming_manual daq.lt://<device-ip>/stream" << std::endl;
        return 1;
    }

    std::cout << "Found device: " << deviceName << " -> " << connectionString << std::endl;

    DevicePtr device;
    try
    {
        device = instance.addDevice(connectionString);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to connect to device: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Connected to: " << device.getInfo().getName() << std::endl;

    // Find the "torque-1" and "speed-1" signals anywhere in the device's component tree
    auto torqueSignals = device.getSignalsRecursive(search::LocalId("torque-1"));
    auto speedSignals   = device.getSignalsRecursive(search::LocalId("speed-1"));

    bool hasTorque = torqueSignals.getCount() > 0;
    bool hasSpeed = speedSignals.getCount() > 0;

    if (!hasTorque && !hasSpeed)
    {
        std::cerr << "Could not find signals 'torque-1' and/or 'speed-1' on the device." << std::endl;
        return 1;
    }

    SignalPtr torqueSignal = hasTorque ? torqueSignals[0] : nullptr;
    SignalPtr speedSignal = hasSpeed ? speedSignals[0] : nullptr;

    std::cout << "Found signals:";
    if (hasTorque)
        std::cout << " " << torqueSignal.getName();
    if (hasSpeed)
        std::cout << " " << speedSignal.getName();
    std::cout << std::endl;

    // Create stream readers for the signals that are present (value type double, domain type int64 / time)
    // Wrap with a time reader to get human-readable timestamps for the domain values
    std::optional<TimeReader<StreamReaderPtr>> torqueTimeReader;
    std::optional<TimeReader<StreamReaderPtr>> speedTimeReader;

    if (hasTorque)
        torqueTimeReader.emplace(StreamReader<double, uint64_t>(torqueSignal));
    if (hasSpeed)
        speedTimeReader.emplace(StreamReader<double, uint64_t>(speedSignal));

    double torqueSamples[100];
    double speedSamples[100];
    std::chrono::system_clock::time_point torqueTimeStamps[100];
    std::chrono::system_clock::time_point speedTimeStamps[100];

    std::cout << "Reading torque-1 / speed-1 ... (Ctrl+C to stop)" << std::endl;

    while (true)
    {
        std::this_thread::sleep_for(100ms);

        if (hasTorque)
        {
            SizeT torqueCount = 100;
            torqueTimeReader->readWithDomain(torqueSamples, torqueTimeStamps, &torqueCount);
            if (torqueCount > 0)
                std::cout << "torque-1: " << torqueSamples[torqueCount - 1]
                           << " Nm  @ " << formatTimestamp(torqueTimeStamps[torqueCount - 1]) << std::endl;
        }

        if (hasSpeed)
        {
            SizeT speedCount = 100;
            speedTimeReader->readWithDomain(speedSamples, speedTimeStamps, &speedCount);
            if (speedCount > 0)
                std::cout << "speed-1:  " << speedSamples[speedCount - 1]
                           << " rpm @ " << formatTimestamp(speedTimeStamps[speedCount - 1]) << std::endl;
        }
    }

    return 0;
}
