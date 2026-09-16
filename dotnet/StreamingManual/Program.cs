// T100 openDAQ(TM) example -- .NET / C# -- Manual connection string streaming
//
// Connects to a T100 device using an explicitly given openDAQ(TM) connection
// string (no mDNS/network discovery involved) and streams the two measurement
// signals "torque-1" and "speed-1".
//
// The T100 only supports the openDAQ(TM) LT streaming protocol
// (server capability ID "OpenDAQLTStreaming"), so the connection string must
// use the "daq.lt://" prefix, e.g.:
//
//     dotnet run -- daq.lt://192.168.0.10:7414
//
// Usage:
//     dotnet run -- <connection-string>

using Daq.Core.Types;
using Daq.Core.OpenDAQ;
using System.Linq;

if (args.Length != 1)
{
    Console.Error.WriteLine("Usage: dotnet run -- <connection-string>");
    Console.Error.WriteLine("Example: dotnet run -- daq.lt://192.168.0.10:7414");
    return 1;
}

string connectionString = args[0];

// Create a fresh openDAQ(TM) instance
Instance instance = OpenDAQFactory.Instance();

Console.WriteLine($"Connecting to {connectionString} ...");

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

if (torqueSignal is null || speedSignal is null)
{
    Console.Error.WriteLine("Could not find signals 'torque-1' and/or 'speed-1' on the device.");
    return 1;
}

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

while (true)
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
