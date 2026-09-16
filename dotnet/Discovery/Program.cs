// T100 openDAQ(TM) example -- .NET / C# -- Device Discovery
//
// Uses openDAQ(TM)'s built-in mDNS discovery to find all T100 / openDAQ devices
// on the local network and prints their name, connection string and server
// capabilities (protocols they support).
//
// NOTE: mDNS discovery relies on UDP multicast on the local subnet. Corporate
// proxies / SASE clients such as Zscaler frequently block multicast traffic,
// in which case this example will find no devices even though the T100 is
// reachable. In that situation, connect directly by IP address instead
// (see the TorqueSpeedReader example) using a connection string such as
// "daq.lt://<device-ip>:7414" (the T100 only supports the openDAQ(TM) LT
// streaming protocol, server capability ID "OpenDAQLTStreaming").

using Daq.Core.Types;
using Daq.Core.OpenDAQ;

Instance instance = OpenDAQFactory.Instance();

Console.WriteLine("Scanning the network for openDAQ(TM) devices (mDNS discovery)...");

var availableDevices = instance.AvailableDevices;

if (availableDevices.Count == 0)
{
    Console.WriteLine("No devices found.");
    Console.WriteLine("If you know the device's IP address (e.g. behind a proxy/VPN such as");
    Console.WriteLine("Zscaler that blocks mDNS), connect directly instead, e.g.:");
    Console.WriteLine("    instance.AddDevice(\"daq.lt://172.19.211.82:7414\");");
    return;
}

foreach (var info in availableDevices)
{
    Console.WriteLine("-----------------------------------");
    Console.WriteLine($"Name:              {info.Name}");
    Console.WriteLine($"Connection string: {info.ConnectionString}");
    Console.WriteLine($"Manufacturer:      {info.Manufacturer}");
    Console.WriteLine($"Serial number:     {info.SerialNumber}");

    foreach (var capability in info.ServerCapabilities)
        Console.WriteLine($"  Protocol: {capability.ProtocolName}  ->  {capability.ConnectionString}");
}

Console.WriteLine("-----------------------------------");
