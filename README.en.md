# T100 Torque Sensor – openDAQ™ Example Projects

## Overview

The **T100 torque sensor** exposes its measurement data via the open,
vendor-neutral **openDAQ™** protocol. This means you can access the
measurement signals using standard SDKs in **C++**, **Python**, or
**.NET (C#)** — regardless of which programming language your existing
measurement and evaluation environment uses.

This package contains two runnable example apps for each of the three
languages, both of which continuously read the measurement signals
`torque-1` (torque) and `speed-1` (speed):

| App | Arguments | Purpose |
|---|---|---|
| **streaming_manual** | 1 argument: connection string | Connects directly to a T100 device using a manually specified openDAQ™ connection string — no network discovery. |
| **streaming_discovery** | none | Automatically searches (mDNS) for a T100 device on the local network, connects to the first device found, and starts streaming. |

```
T100OpenDaq/
├── cpp/            C++ examples (CMake project)
│   ├── streaming_manual/
│   └── streaming_discovery/
├── python/         Python examples
│   ├── streaming_manual.py
│   └── streaming_discovery.py
└── dotnet/         .NET / C# examples
    ├── StreamingManual/
    └── StreamingDiscovery/
```

## Important note: T100 only supports openDAQ LT

The T100 exclusively implements the **openDAQ™ LT streaming protocol**
(server capability ID `OpenDAQLTStreaming`) — **not** the full OPC UA
configuration protocol. In practice, this means:

- The connection string prefix is **`daq.lt://`** (not `daq://` or
  `daq.opcua://`).
- The default port is **7414** (unencrypted), or **7415** for the TLS
  variant `daq.lts://`, if supported by the device.
- LT devices do **not** expose a full component tree (no configuration of
  function blocks, properties, etc. over the network), only a **flat list
  of available signals** directly under the device. For our use case —
  reading the `torque-1` and `speed-1` signals — this is sufficient.

## `streaming_manual` – Streaming via a manual connection string

This app connects **directly to the device without network discovery**.
It is invoked with exactly **one argument**: the T100's openDAQ™ connection
string.

Format: `daq.lt://<IP-address>:<port>`

```bash
# Python
python streaming_manual.py daq.lt://<T100-IP-address>:7414

# C++
./streaming_manual daq.lt://<T100-IP-address>:7414

# .NET
dotnet run -- daq.lt://<T100-IP-address>:7414
```

This variant is especially useful when automatic network discovery (see
below) does not work for network reasons (proxy, VPN, firewall, segmented
networks) — in that case, the T100's IP address and port must be known and
are passed directly.

## `streaming_discovery` – Streaming via automatic network discovery

This app requires **no arguments**. It automatically searches the local
network for openDAQ™ LT devices via **mDNS**, connects to the first T100
found, and immediately starts streaming.

```bash
# Python
python streaming_discovery.py

# C++
./streaming_discovery

# .NET
dotnet run
```

**Note:** mDNS discovery relies on UDP multicast on the local subnet. Some
corporate networks (e.g. with proxy, VPN, or SASE solutions) block this
traffic. If no device is found even though it is reachable, use
`streaming_manual` instead with the device's known IP address.

## Prerequisites

- The T100 device must be reachable on the network (pinging its IP address
  should work; a firewall rule for the openDAQ™ ports may be required).
- For `streaming_discovery`: UDP multicast must be allowed on the network
  segment (depending on network configuration, this may not work across a
  proxy/VPN).

### C++
- CMake ≥ 3.20, a C++17 compiler
- The openDAQ™ SDK installed (`find_package(openDAQ)` must be able to find
  the SDK)

### Python
- Python ≥ 3.8
- The `opendaq` package (see `python/requirements.txt`)

### .NET
- .NET SDK ≥ 8.0
- The `OpenDAQ.Net` NuGet package – this is **not** available from the public
  nuget.org feed. It must first be downloaded from
  [https://docs.opendaq.com/](https://docs.opendaq.com/) and then installed
  manually as a NuGet package, e.g. with:
  ```bash
  dotnet add package OpenDAQ.Net --source <path-to-downloaded-package>
  ```
  or by registering the download folder as a local package source in
  `nuget.config` / via `dotnet nuget add source`.

## Building and running

### C++
```bash
cd cpp
cmake -B build -S .
cmake --build build

./build/streaming_manual/streaming_manual daq.lt://<T100-IP-address>:7414
./build/streaming_discovery/streaming_discovery
```

### Python
```bash
cd python
pip install -r requirements.txt

python streaming_manual.py daq.lt://<T100-IP-address>:7414
python streaming_discovery.py
```

### .NET
```bash
cd dotnet/StreamingManual
dotnet run -- daq.lt://<T100-IP-address>:7414

cd ../StreamingDiscovery
dotnet run
```

## Customization

The signal names (`torque-1`, `speed-1`) correspond to the channel labels
configured on the device and can be adjusted in the respective source code
if needed (search for `LocalId("torque-1")` or `LocalId("speed-1")`).

## Support

For questions about network configuration (e.g. opening the required ports
in your firewall/proxy solution), please contact your IT department. The
T100 only requires the following protocol/port:

- **openDAQ™ LT streaming**: TCP port **7414** (unencrypted, `daq.lt://`),
  or **7415** for TLS (`daq.lts://`), if supported by the device.
- **mDNS discovery** (only for `streaming_discovery`, not strictly
  required): UDP port 5353 (multicast)
