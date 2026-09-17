/**
 * T100 openDAQ(TM) example -- C++ -- Discovery streaming
 *
 * Uses openDAQ(TM)'s built-in mDNS discovery to automatically find a T100
 * device on the local network (device family "T100", server capability
 * "OpenDAQLTStreaming"), connects to it and streams the two measurement
 * signals "torque-1" and "speed-1".
 *
 * An optional UUID argument can be passed to pick a specific T100 out of
 * several devices found by the scan (matched against the device's system
 * UUID). Without it, the first matching T100 is used.
 *
 * NOTE: mDNS discovery relies on UDP multicast on the local subnet. Some
 * network setups (proxies, VPN/SASE clients, segmented networks) block this
 * traffic. If no device is found, use the "streaming_manual" example instead
 * and provide the device's connection string directly, e.g.:
 *
 *     ./streaming_manual daq.lt://<device-ip>/stream
 *
 * Usage:
 *     streaming_discovery [uuid]
 */

#include <opendaq/opendaq.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <thread>
#include <optional>
#include <vector>
#include <algorithm>
#include <cctype>

using namespace daq;
using namespace std::chrono_literals;

static std::string toLower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}

static bool isT100Device(const DeviceInfoPtr& info)
{
    const std::string model = toLower(info.getModel().toStdString());
    const std::string name = toLower(info.getName().toStdString());
    return model.find("t100") != std::string::npos || name.find("t100") != std::string::npos;
}

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

int main(int argc, char** argv)
{
    std::optional<std::string> uuidFilter;
    if (argc > 1)
        uuidFilter = std::string(argv[1]);

    // Create a fresh openDAQ(TM) instance
    // OPENDAQ_MODULES_DIR is defined by CMake to point at the SDK's modules folder,
    // since executables don't have modules alongside them like the Python package does
    InstancePtr instance = Instance(OPENDAQ_MODULES_DIR);

    std::cout << "Scanning the network for openDAQ(TM) LT devices (mDNS discovery)..." << std::endl;

    ListPtr<IDeviceInfo> availableDevices = instance.getAvailableDevices();

    // Only consider devices of the T100 family that advertise the "OpenDAQLTStreaming" capability
    std::vector<std::pair<DeviceInfoPtr, StringPtr>> candidates;
    for (const auto& info : availableDevices)
    {
        if (!isT100Device(info))
            continue;
        for (const auto& capability : info.getServerCapabilities())
        {
            if (capability.getProtocolName() == "OpenDAQLTStreaming")
            {
                candidates.emplace_back(info, capability.getConnectionString());
                break;
            }
        }
    }

    if (candidates.empty())
    {
        std::cerr << "No T100 openDAQ(TM) LT device found via discovery.\n"
                  << "If discovery is blocked in your network, use 'streaming_manual' instead\n"
                  << "and pass the device's connection string directly, e.g.:\n"
                  << "    ./streaming_manual daq.lt://<device-ip>/stream" << std::endl;
        return 1;
    }

    DeviceInfoPtr selected;
    StringPtr connectionString;
    if (uuidFilter)
    {
        auto it = std::find_if(candidates.begin(), candidates.end(), [&](const auto& candidate) {
            return toLower(candidate.first.getSystemUuid().toStdString()) == toLower(*uuidFilter);
        });
        if (it == candidates.end())
        {
            std::cerr << "No T100 device with UUID '" << *uuidFilter << "' found in scan results.\nUUIDs found: ";
            for (size_t i = 0; i < candidates.size(); ++i)
                std::cerr << (i > 0 ? ", " : "") << candidates[i].first.getSystemUuid();
            std::cerr << std::endl;
            return 1;
        }
        selected = it->first;
        connectionString = it->second;
    }
    else
    {
        selected = candidates.front().first;
        connectionString = candidates.front().second;
    }

    std::cout << "Found device: " << selected.getName() << " (uuid=" << selected.getSystemUuid() << ") -> "
               << connectionString << std::endl;

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
