// T100 openDAQ(TM) example -- .NET / C#
//
// Connects directly to a T100 device via its IP address (bypassing mDNS/discovery,
// e.g. when the network scan is blocked by a proxy such as Zscaler) and streams
// the two measurement signals "torque-1" and "speed-1".
//
// Adjust DeviceIp / DeviceConnectionString to match your T100 device.

using Daq.Core.Types;
using Daq.Core.OpenDAQ;

// IP address of the T100 device (from the customer's network)
const string DeviceIp = "172.19.211.82";

// The T100 only supports the openDAQ(TM) LT streaming protocol (server capability ID
// "OpenDAQLTStreaming"), not the full OPC UA configuration protocol. LT devices expose
// a flat list of Signals directly (no full component tree). The "daq.lt://" prefix
// selects this protocol explicitly; the default LT port is 7414 and can be omitted.
string deviceConnectionString = $"daq.lt://{DeviceIp}:7414";

// Create a fresh openDAQ(TM) instance
Instance instance = OpenDAQFactory.Instance();

Console.WriteLine($"Connecting to T100 at {deviceConnectionString} ...");

Device? device;
try
{
    device = instance.AddDevice(deviceConnectionString);
}
catch (Exception e)
{
    Console.Error.WriteLine($"Failed to connect to device: {e.Message}");
    return 1;
}

Console.WriteLine($"Connected to: {device.Info.Name}");

// Find the "torque-1" and "speed-1" signals anywhere in the device's component tree
var torqueSignals = device.GetSignalsRecursive(SearchFactory.LocalId("torque-1"));
var speedSignals = device.GetSignalsRecursive(SearchFactory.LocalId("speed-1"));

if (torqueSignals.Count == 0 || speedSignals.Count == 0)
{
    Console.Error.WriteLine("Could not find signals 'torque-1' and/or 'speed-1' on the device.");
    return 1;
}

var torqueSignal = torqueSignals[0];
var speedSignal = speedSignals[0];

Console.WriteLine($"Found signals: {torqueSignal.Name}, {speedSignal.Name}");

// Create stream readers for both signals
var torqueReader = OpenDAQFactory.CreateStreamReader<double, ulong>(torqueSignal);
var speedReader = OpenDAQFactory.CreateStreamReader<double, ulong>(speedSignal);

// Wrap with a time reader to get human-readable timestamps for the domain values
var torqueTimeReader = OpenDAQFactory.CreateTimeReader(torqueReader, torqueSignal);
var speedTimeReader = OpenDAQFactory.CreateTimeReader(speedReader, speedSignal);

var torqueSamples = new double[100];
var speedSamples = new double[100];
var torqueTimeStamps = new DateTime[100];
var speedTimeStamps = new DateTime[100];

Console.WriteLine("Reading torque-1 / speed-1 ... (Ctrl+C to stop)");

for (int i = 0; i < 200; ++i)
{
    Thread.Sleep(100);

    nuint torqueCount = 100;
    torqueTimeReader.ReadWithDomain(torqueSamples, torqueTimeStamps, ref torqueCount);
    if (torqueCount > 0)
        Console.WriteLine($"torque-1: {torqueSamples[torqueCount - 1]:F4} Nm  @ {torqueTimeStamps[torqueCount - 1]:o}");

    nuint speedCount = 100;
    speedTimeReader.ReadWithDomain(speedSamples, speedTimeStamps, ref speedCount);
    if (speedCount > 0)
        Console.WriteLine($"speed-1:  {speedSamples[speedCount - 1]:F4} rpm @ {speedTimeStamps[speedCount - 1]:o}");
}

return 0;
