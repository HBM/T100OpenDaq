// T100 openDAQ(TM) example -- .NET / C# -- Discovery streaming
//
// Uses openDAQ(TM)'s built-in mDNS discovery to automatically find a T100
// device on the local network (server capability "OpenDAQLTStreaming"),
// connects to it and streams the two measurement signals "torque-1" and
// "speed-1".
//
// NOTE: mDNS discovery relies on UDP multicast on the local subnet. Some
// network setups (proxies, VPN/SASE clients, segmented networks) block this
// traffic. If no device is found, use the StreamingManual example instead
// and provide the device's connection string directly, e.g.:
//
//     dotnet run -- daq.lt://<device-ip>/stream   (in the StreamingManual project)
//
// Usage:
//     dotnet run

using Daq.Core.Types;
using Daq.Core.OpenDAQ;
using System.Linq;

Instance instance = OpenDAQFactory.Instance();

Console.WriteLine("Scanning the network for openDAQ(TM) LT devices (mDNS discovery)...");

var availableDevices = instance.AvailableDevices;

// Find the first device that advertises the "OpenDAQLTStreaming" capability (the T100's protocol)
string? connectionString = null;
string? deviceName = null;
foreach (var info in availableDevices)
{
    foreach (var capability in info.ServerCapabilities)
    {
        if (capability.ProtocolName == "OpenDAQLTStreaming")
        {
            connectionString = capability.ConnectionString;
            deviceName = info.Name;
            break;
        }
    }
    if (connectionString != null)
        break;
}

if (connectionString == null)
{
    Console.Error.WriteLine("No openDAQ(TM) LT device found via discovery.");
    Console.Error.WriteLine("If discovery is blocked in your network, use StreamingManual instead");
    Console.Error.WriteLine("and pass the device's connection string directly, e.g.:");
    Console.Error.WriteLine("    dotnet run -- daq.lt://<device-ip>/stream");
    return 1;
}

Console.WriteLine($"Found device: {deviceName} -> {connectionString}");

Device? device;
try
{
    device = instance.AddDevice(connectionString);
}
catch (Exception e)
{
    Console.Error.WriteLine($"Failed to connect to device: {e.Message}");
    return 1;
}

Console.WriteLine($"Connected to: {device.Info.Name}");

// Find the "torque-1" and "speed-1" signals anywhere in the device's component tree
// (matched by name instead of a LocalId search filter, which doesn't reliably
// match streamed signals in the .NET binding)
var allSignals = device.GetSignalsRecursive();

Console.WriteLine($"Available signals ({allSignals.Count}):");
foreach (var signal in allSignals)
{
    Console.WriteLine($"  - name={signal.Name}, local_id={signal.LocalId}");
}

var torqueSignal = allSignals.FirstOrDefault(s => s.Name == "torque-1");
var speedSignal = allSignals.FirstOrDefault(s => s.Name == "speed-1");

if (torqueSignal is null && speedSignal is null)
{
    Console.Error.WriteLine("Could not find signals 'torque-1' and/or 'speed-1' on the device.");
    return 1;
}

Console.WriteLine($"Found signals: {string.Join(", ", new[] { torqueSignal?.Name, speedSignal?.Name }.Where(name => name is not null))}");

// Create stream readers only for the signals that are present
// Wrap with a time reader to get human-readable timestamps for the domain values
var torqueTimeReader = torqueSignal is not null
    ? OpenDAQFactory.CreateTimeReader(OpenDAQFactory.CreateStreamReader<double, ulong>(torqueSignal), torqueSignal)
    : null;
var speedTimeReader = speedSignal is not null
    ? OpenDAQFactory.CreateTimeReader(OpenDAQFactory.CreateStreamReader<double, ulong>(speedSignal), speedSignal)
    : null;

var torqueSamples = new double[100];
var speedSamples = new double[100];
var torqueTimeStamps = new DateTime[100];
var speedTimeStamps = new DateTime[100];

Console.WriteLine("Reading torque-1 / speed-1 ... (Ctrl+C to stop)");

while (true)
{
    Thread.Sleep(100);

    if (torqueTimeReader is not null)
    {
        nuint torqueCount = 100;
        torqueTimeReader.ReadWithDomain(torqueSamples, torqueTimeStamps, ref torqueCount);
        if (torqueCount > 0)
            Console.WriteLine($"torque-1: {torqueSamples[torqueCount - 1]:F4} Nm  @ {torqueTimeStamps[torqueCount - 1]:o}");
    }

    if (speedTimeReader is not null)
    {
        nuint speedCount = 100;
        speedTimeReader.ReadWithDomain(speedSamples, speedTimeStamps, ref speedCount);
        if (speedCount > 0)
            Console.WriteLine($"speed-1:  {speedSamples[speedCount - 1]:F4} rpm @ {speedTimeStamps[speedCount - 1]:o}");
    }
}
