/**
 * T100 openDAQ(TM) example -- C++ -- Manual connection string streaming
 *
 * Connects to a T100 device using an explicitly given openDAQ(TM) connection
 * string (no mDNS/network discovery involved) and streams the two measurement
 * signals "torque-1" and "speed-1".
 *
 * The T100 only supports the openDAQ(TM) LT streaming protocol
 * (server capability ID "OpenDAQLTStreaming"), so the connection string must
 * use the "daq.lt://" prefix, e.g.:
 *
 *     ./streaming_manual daq.lt://<device-ip>/stream
 *
 * Usage:
 *     streaming_manual <connection-string>
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

int main(int argc, const char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <connection-string>" << std::endl;
        std::cerr << "Example: " << argv[0] << " daq.lt://192.168.0.10/stream" << std::endl;
        return 1;
    }

    const std::string connectionString = argv[1];

    // Create a fresh openDAQ(TM) instance
    // OPENDAQ_MODULES_DIR is defined by CMake to point at the SDK's modules folder,
    // since executables don't have modules alongside them like the Python package does
    InstancePtr instance = Instance(OPENDAQ_MODULES_DIR);

    std::cout << "Connecting to " << connectionString << " ..." << std::endl;

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

    // List all available signals on the device for debugging/reference
    auto allSignals = device.getSignalsRecursive();
    std::cout << "Available signals (" << allSignals.getCount() << "):" << std::endl;
    for (const auto& signal : allSignals)
        std::cout << "  - name=" << signal.getName() << ", local_id=" << signal.getLocalId() << std::endl;

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
