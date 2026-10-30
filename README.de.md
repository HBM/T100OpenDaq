# T100 Drehmomentaufnehmer – openDAQ™ Beispielprojekte

## Übersicht

Der **T100 Drehmomentaufnehmer** liefert seine Messwerte über das offene,
herstellerunabhängige **openDAQ™**-Protokoll aus. Dadurch können Sie mit
Standard-SDKs in **C++**, **Python** oder **.NET (C#)** auf die Messsignale
zugreifen – unabhängig davon, welche Programmiersprache in Ihrer bestehenden
Mess- und Auswerteumgebung eingesetzt wird.

Dieses Paket enthält für jede der drei Sprachen zwei lauffähige Beispiel-Apps,
die beide die Messsignale `torque-1` (Drehmoment) und `speed-1` (Drehzahl)
fortlaufend auslesen:

| App | Argumente | Zweck |
|---|---|---|
| **streaming_manual** | 1 Argument: Verbindungsstring | Verbindet sich direkt mit einem T100-Gerät über einen manuell angegebenen openDAQ™-Verbindungsstring – ohne Netzwerksuche. |
| **streaming_discovery** | keine | Sucht automatisch (mDNS) nach einem T100-Gerät im lokalen Netzwerk, verbindet sich mit dem ersten gefundenen Gerät und startet das Streaming. |

```
T100OpenDaq/
├── cpp/            C++ Beispiele (CMake-Projekt)
│   ├── streaming_manual/
│   └── streaming_discovery/
├── python/         Python Beispiele
│   ├── streaming_manual.py
│   └── streaming_discovery.py
└── dotnet/         .NET / C# Beispiele
    ├── StreamingManual/
    └── StreamingDiscovery/
```

## Wichtiger Hinweis: T100 unterstützt nur openDAQ LT

Das T100 implementiert ausschließlich das **openDAQ™ LT Streaming Protokoll**
(Server-Capability-ID `OpenDAQLTStreaming`) – **nicht** das vollständige
OPC UA Konfigurationsprotokoll. Das bedeutet konkret:

- Verbindungsstring-Präfix ist **`daq.lt://`** (nicht `daq://` oder
  `daq.opcua://`).
- Standardport ist **80** (unverschlüsselt) bzw. **7415** für die
  TLS-Variante `daq.lts://`, falls vom Gerät unterstützt.
- LT-Geräte liefern **keinen vollständigen Komponentenbaum** (keine
  Konfiguration von Funktionsblöcken, Eigenschaften etc. über das Netzwerk),
  sondern lediglich eine **flache Liste der verfügbaren Signale** direkt
  unterhalb des Geräts. Für unseren Anwendungsfall – Auslesen der Signale
  `torque-1` und `speed-1` – ist das ausreichend.

## `streaming_manual` – Streaming über einen manuellen Verbindungsstring

Diese App verbindet sich **ohne Netzwerksuche** direkt mit dem Gerät. Sie
wird mit genau **einem Argument** aufgerufen: dem openDAQ™-Verbindungsstring
des T100.

Format: `daq.lt://<IP-Adresse>:<Port>/stream`

```bash
# Python
python streaming_manual.py daq.lt://<IP-Adresse-des-T100>:80/stream

# C++
./streaming_manual daq.lt://<IP-Adresse-des-T100>:80/stream

# .NET
dotnet run -- daq.lt://<IP-Adresse-des-T100>:80/stream
```

Diese Variante eignet sich besonders, wenn die automatische Netzwerksuche
(siehe unten) aus Netzwerkgründen (Proxy, VPN, Firewall, segmentierte Netze)
nicht funktioniert – IP-Adresse und Port des T100 müssen dann bekannt sein
und werden direkt übergeben.

## `streaming_discovery` – Streaming über automatische Netzwerksuche

Diese App benötigt **keine Argumente**. Sie durchsucht das lokale Netzwerk
automatisch per **mDNS** nach openDAQ™-LT-Geräten, verbindet sich mit dem
ersten gefundenen T100 und beginnt sofort mit dem Streaming.

```bash
# Python
python streaming_discovery.py

# C++
./streaming_discovery

# .NET
dotnet run
```

**Hinweis:** mDNS-Discovery basiert auf UDP-Multicast im lokalen Subnetz.
Manche Unternehmensnetzwerke (z. B. mit Proxy-, VPN- oder SASE-Lösungen)
blockieren diesen Verkehr. Wird kein Gerät gefunden, obwohl es erreichbar
ist, nutzen Sie stattdessen `streaming_manual` mit der bekannten
IP-Adresse des Geräts.

## Voraussetzungen

- Das T100-Gerät muss im Netzwerk erreichbar sein (Ping auf die IP-Adresse
  sollte funktionieren, ggf. Firewall-Freigabe für die openDAQ™-Ports nötig).
- Für `streaming_discovery`: UDP-Multicast muss im Netzwerksegment erlaubt
  sein (funktioniert je nach Netzwerk-Konfiguration ggf. nicht über
  Proxy/VPN hinweg).

### C++
- CMake ≥ 3.20, ein C++17-Compiler
- installiertes openDAQ™ SDK (`find_package(openDAQ)` muss das SDK finden)

### Python
- Python ≥ 3.8
- Paket `opendaq` (siehe `python/requirements.txt`)

### .NET
- .NET SDK ≥ 8.0
- NuGet-Paket `OpenDAQ.Net` – dieses ist **nicht** über die öffentliche
  nuget.org-Quelle verfügbar. Es muss zuerst von
  [https://docs.opendaq.com/](https://docs.opendaq.com/) heruntergeladen und
  anschließend manuell als NuGet-Paket installiert werden, z. B. mit:
  ```bash
  dotnet add package OpenDAQ.Net --source <Pfad-zum-heruntergeladenen-Paket>
  ```
  oder indem der Downloadordner als lokale Paketquelle in der
  `nuget.config`/über `dotnet nuget add source` eingetragen wird.

## Bauen und Ausführen

### C++
```bash
cd cpp
cmake -B build -S .
cmake --build build

./build/streaming_manual/streaming_manual daq.lt://<IP-Adresse-des-T100>:80/stream
./build/streaming_discovery/streaming_discovery
```

### Python
```bash
cd python
pip install -r requirements.txt

python streaming_manual.py daq.lt://<IP-Adresse-des-T100>:80/stream
python streaming_discovery.py
```

### .NET
```bash
cd dotnet/StreamingManual
dotnet run -- daq.lt://<IP-Adresse-des-T100>:80/stream

cd ../StreamingDiscovery
dotnet run
```

## Anpassung

Die Namen der Signale (`torque-1`, `speed-1`) entsprechen den auf dem Gerät
konfigurierten Kanalbezeichnungen und können bei Bedarf im jeweiligen
Quellcode angepasst werden (Suche nach `LocalId("torque-1")` bzw.
`LocalId("speed-1")`).

## Support

Bei Fragen zur Netzwerk-Konfiguration (z. B. Freischaltung der benötigten
Ports in Ihrer Firewall/Proxy-Lösung) wenden Sie sich bitte an Ihre
IT-Abteilung. Für das T100 wird ausschließlich folgendes Protokoll/Port
benötigt:

- **openDAQ™ LT Streaming**: TCP-Port **80** (unverschlüsselt, `daq.lt://`)
  bzw. **7415** für TLS (`daq.lts://`), falls das Gerät dies unterstützt.
- **mDNS-Discovery** (nur für `streaming_discovery`, nicht zwingend
  erforderlich): UDP-Port 5353 (Multicast)
