# T100 Drehmomentaufnehmer – openDAQ™ Beispielprojekte

## Übersicht

Der **T100 Drehmomentaufnehmer** liefert seine Messwerte über das offene,
herstellerunabhängige **openDAQ™**-Protokoll aus. Dadurch können Sie mit
Standard-SDKs in **C++**, **Python** oder **.NET (C#)** auf die Messsignale
zugreifen – unabhängig davon, welche Programmiersprache in Ihrer bestehenden
Mess- und Auswerteumgebung eingesetzt wird.

Dieses Paket enthält für jede der drei Sprachen zwei lauffähige Beispiele:

| Beispiel | Zweck |
|---|---|
| **torque_speed_reader** | Verbindet sich direkt mit dem T100 (per IP-Adresse) und liest fortlaufend die zwei Messsignale `torque-1` (Drehmoment) und `speed-1` (Drehzahl) aus. |
| **discovery** | Durchsucht das lokale Netzwerk automatisch (mDNS) nach openDAQ™-Geräten und zeigt Name, Verbindungsstring und unterstützte Protokolle an. |

```
T100OpenDaq/
├── cpp/            C++ Beispiele (CMake-Projekt)
│   ├── torque_speed_reader/
│   └── discovery/
├── python/         Python Beispiele
│   ├── torque_speed_reader.py
│   └── discovery.py
└── dotnet/         .NET / C# Beispiele
    ├── TorqueSpeedReader/
    └── Discovery/
```

## Wichtiger Hinweis: T100 unterstützt nur openDAQ LT

Das T100 implementiert ausschließlich das **openDAQ™ LT Streaming Protokoll**
(Server-Capability-ID `OpenDAQLTStreaming`) – **nicht** das vollständige
OPC UA Konfigurationsprotokoll. Das bedeutet konkret:

- Verbindungsstring-Präfix ist **`daq.lt://`** (nicht `daq://` oder
  `daq.opcua://`).
- Standardport ist **7414** (unverschlüsselt) bzw. **7415** für die
  TLS-Variante `daq.lts://`, falls vom Gerät unterstützt.
- LT-Geräte liefern **keinen vollständigen Komponentenbaum** (keine
  Konfiguration von Funktionsblöcken, Eigenschaften etc. über das Netzwerk),
  sondern lediglich eine **flache Liste der verfügbaren Signale** direkt
  unterhalb des Geräts. Für unseren Anwendungsfall – Auslesen der Signale
  `torque-1` und `speed-1` – ist das ausreichend.

## Wichtiger Hinweis zu Ihrem Netzwerk (Zscaler)

Das automatische **Discovery**-Beispiel nutzt **mDNS** (Multicast-DNS), um
Geräte im lokalen Netz zu finden. Viele Unternehmensnetzwerke – insbesondere
mit **Zscaler** oder anderen SASE-/Proxy-Lösungen – blockieren diesen
Multicast-Verkehr. In diesem Fall liefert das Discovery-Beispiel **keine
Treffer**, obwohl das Gerät selbst erreichbar ist.

**Lösung:** Verbinden Sie sich stattdessen direkt über die bekannte
IP-Adresse des Geräts – genau das macht das Beispiel
`torque_speed_reader`. In Ihrem Fall wurde das T100 unter folgender Adresse
erreicht:

```
172.19.211.82
```

Der Verbindungsstring dazu lautet:

```
daq.lt://172.19.211.82:7414
```

Das Präfix `daq.lt://` wählt gezielt das openDAQ™ LT Streaming-Protokoll,
das einzige vom T100 unterstützte Protokoll – eine Netzwerk-Suche per mDNS
ist dafür **nicht** erforderlich. Der Port `:7414` kann weggelassen werden,
sofern das Gerät den Standardport verwendet.

## Voraussetzungen

- Das T100-Gerät muss im Netzwerk erreichbar sein (Ping auf die IP-Adresse
  sollte funktionieren, ggf. Firewall-Freigabe für die openDAQ™-Ports nötig).
- Für die Discovery-Beispiele: UDP-Multicast muss im Netzwerksegment erlaubt
  sein (funktioniert i. d. R. nicht durch Zscaler/VPN hindurch).

### C++
- CMake ≥ 3.20, ein C++17-Compiler
- installiertes openDAQ™ SDK (`find_package(openDAQ)` muss das SDK finden)

### Python
- Python ≥ 3.8
- Paket `opendaq` (siehe `python/requirements.txt`)

### .NET
- .NET SDK ≥ 8.0
- NuGet-Paket `OpenDAQ`

## Ausführen der Beispiele

### C++
```bash
cd cpp
cmake -B build -S .
cmake --build build
./build/torque_speed_reader/torque_speed_reader
./build/discovery/discovery
```

### Python
```bash
cd python
pip install -r requirements.txt
python torque_speed_reader.py
python discovery.py
```

### .NET
```bash
cd dotnet/TorqueSpeedReader
dotnet run

cd ../Discovery
dotnet run
```

## Anpassung

Falls sich die IP-Adresse Ihres T100 ändert oder Sie mehrere Geräte
betreiben, passen Sie in den `torque_speed_reader`-Beispielen einfach die
Konstante `DEVICE_IP` bzw. `DeviceIp` an. Die Namen der Signale (`torque-1`,
`speed-1`) entsprechen den auf dem Gerät konfigurierten Kanalbezeichnungen
und können bei Bedarf ebenfalls angepasst werden.

## Support

Bei Fragen zur Netzwerk-Konfiguration (z. B. Freischaltung der benötigten
Ports in Zscaler) wenden Sie sich bitte an Ihre IT-Abteilung. Für das T100
wird ausschließlich folgendes Protokoll/Port benötigt:

- **openDAQ™ LT Streaming**: TCP-Port **7414** (unverschlüsselt, `daq.lt://`)
  bzw. **7415** für TLS (`daq.lts://`), falls das Gerät dies unterstützt.
- **mDNS-Discovery** (nur für die Discovery-Beispiele, nicht zwingend
  erforderlich): UDP-Port 5353 (Multicast)
