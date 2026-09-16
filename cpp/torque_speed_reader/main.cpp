/**
 * T100 openDAQ(TM) example — C++
 *
 * Connects directly to a T100 device via its IP address (bypassing mDNS/discovery,
 * e.g. when the network scan is blocked by a proxy such as Zscaler) and streams
 * the two measurement signals "torque-1" and "speed-1".
 *
 * Adjust DEVICE_IP / DEVICE_CONNECTION_STRING to match your T100 device.
 */

#include <opendaq/opendaq.h>
#include <iostream>
#include <chrono>
#include <thread>

using namespace daq;
using namespace std::chrono_literals;

// IP address of the T100 device (from the customer's network)
static const std::string DEVICE_IP = "172.19.211.82";

// The T100 only supports the openDAQ(TM) LT streaming protocol (server capability ID
// "OpenDAQLTStreaming"), not the full OPC UA configuration protocol. LT devices expose
// a flat list of Signals directly (no full component tree). The "daq.lt://" prefix
// selects this protocol explicitly; the default LT port is 7414 and can be omitted.
static const std::string DEVICE_CONNECTION_STRING = "daq.lt://" + DEVICE_IP + ":7414";

int main()
{
    // Create a fresh openDAQ(TM) instance
    InstancePtr instance = Instance();

    std::cout << "Connecting to T100 at " << DEVICE_CONNECTION_STRING << " ..." << std::endl;

    DevicePtr device;
    try
    {
        device = instance.addDevice(DEVICE_CONNECTION_STRING);
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

    if (torqueSignals.getCount() == 0 || speedSignals.getCount() == 0)
    {
        std::cerr << "Could not find signals 'torque-1' and/or 'speed-1' on the device." << std::endl;
        return 1;
    }

    SignalPtr torqueSignal = torqueSignals[0];
    SignalPtr speedSignal = speedSignals[0];

    std::cout << "Found signals: " << torqueSignal.getName() << ", " << speedSignal.getName() << std::endl;

    // Create stream readers for both signals (value type double, domain type int64 / time)
    StreamReaderPtr torqueReader = StreamReader<double, uint64_t>(torqueSignal);
    StreamReaderPtr speedReader  = StreamReader<double, uint64_t>(speedSignal);

    // Wrap with a time reader to get human-readable timestamps for the domain values
    auto torqueTimeReader = TimeReader(torqueReader);
    auto speedTimeReader  = TimeReader(speedReader);

    double torqueSamples[100];
    double speedSamples[100];
    std::chrono::system_clock::time_point torqueTimeStamps[100];
    std::chrono::system_clock::time_point speedTimeStamps[100];

    std::cout << "Reading torque-1 / speed-1 ... (Ctrl+C to stop)" << std::endl;

    for (int i = 0; i < 200; ++i)
    {
        std::this_thread::sleep_for(100ms);

        SizeT torqueCount = 100;
        torqueTimeReader.readWithDomain(torqueSamples, torqueTimeStamps, &torqueCount);
        if (torqueCount > 0)
            std::cout << "torque-1: " << torqueSamples[torqueCount - 1]
                       << " Nm  @ " << torqueTimeStamps[torqueCount - 1] << std::endl;

        SizeT speedCount = 100;
        speedTimeReader.readWithDomain(speedSamples, speedTimeStamps, &speedCount);
        if (speedCount > 0)
            std::cout << "speed-1:  " << speedSamples[speedCount - 1]
                       << " rpm @ " << speedTimeStamps[speedCount - 1] << std::endl;
    }

    return 0;
}
