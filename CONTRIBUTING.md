# Contributing & Development

This guide is for developers who want to modify the LED Sectional code or contribute improvements.

## Project Structure

```
led-sectional/
├── firmware/           # ESP8266 PlatformIO project
│   ├── src/           # Source code
│   │   ├── main.cpp       # Main application
│   │   ├── config.cpp/h   # Configuration management
│   │   ├── wifi_manager.cpp/h  # WiFi handling
│   │   ├── metar.cpp/h    # METAR fetching and parsing
│   │   ├── leds.cpp/h     # LED control
│   │   ├── mqtt.cpp/h     # MQTT / Home Assistant integration
│   │   ├── light_sensor.cpp/h  # Light sensor support
│   │   └── serial_cmd.cpp/h    # Serial command interface
│   ├── data/          # LittleFS filesystem (default config)
│   └── platformio.ini # Build configuration
├── web/               # React configuration tool
│   ├── src/           # TypeScript source
│   │   ├── components/    # React components
│   │   ├── hooks/         # Custom hooks (useSerial, etc.)
│   │   ├── services/      # Serial communication service
│   │   └── types/         # TypeScript type definitions
│   └── public/        # Static assets
├── docs/              # GitHub Pages site
│   ├── index.md       # Full build guide (Jekyll)
│   ├── kit.md         # Kit information
│   └── configure/     # Built web config tool (auto-deployed)
│       └── firmware/  # Firmware binary for web flashing
├── pcb files/         # PCB design files (Eagle)
└── enclosure stl files/  # 3D printable case designs
```

## Building the Firmware

### Prerequisites

- [PlatformIO](https://platformio.org/) (recommended) or Arduino IDE
- USB drivers for your board (CH340 or CP2102)

### Using PlatformIO (Recommended)

```bash
cd firmware

# Build
pio run

# Build and upload
pio run -t upload

# Upload filesystem (default config)
pio run -t uploadfs
```

### Using Arduino IDE

1. Install [ESP8266 board support](https://github.com/esp8266/Arduino)
2. Install required libraries via Library Manager (see Dependencies below)
3. Open `firmware/src/main.cpp`
4. Select board: `LOLIN(WEMOS) D1 mini Lite`
5. Upload

### Platform and Dependencies

The firmware uses specific pinned versions for compatibility. These versions have been carefully tested together.

**Platform:** `espressif8266@2.6.3` (Arduino ESP8266 Core 2.7.4)

This older platform version is required because:
- Arduino Core 3.x changed NMI (Non-Maskable Interrupt) handling in ways that conflict with FastLED's timing-critical WS2812 protocol
- Core 3.x causes first-LED flickering/corruption even with various workarounds (`FASTLED_ALLOW_INTERRUPTS`, `FASTLED_INTERRUPT_RETRY_COUNT`, double `show()` calls, etc.)
- Core 2.7.4 has stable interrupt handling that works reliably with FastLED

**Libraries** (PlatformIO manages these automatically):

| Library | Version | Notes |
|---------|---------|-------|
| FastLED | 3.7.6 (pinned) | Last version compatible with Core 2.7.4's GCC 4.8.2 toolchain. Version 3.7.7+ uses C++17 `static_assert` without message; 3.9+ uses structured bindings; 3.10+ uses `constexpr` in `alignas()` |
| WiFiManager | 2.0.16-rc.2 (pinned) | Version 2.0.17 crashes in AP mode on Core 2.7.4 (unaligned flash access in `getMenuOut()`). Installed via GitHub URL |
| ArduinoJson | ^7.4.2 | JSON parsing for configuration and serial commands |
| JsonStreamingParser | ^1.0.5 | Memory-efficient streaming parser for METAR data |
| PubSubClient | ^2.8 | MQTT client for Home Assistant integration |
| Adafruit TSL2561 | ^1.1.2 | Optional light sensor |
| Adafruit Unified Sensor | ^1.1.15 | Required by TSL2561 |

**Why not just upgrade everything?**

We tested systematically and found:
- FastLED 3.7.7-3.7.8: Fail with `static_assert` missing second parameter (C++17 feature)
- FastLED 3.9.x: Fail with structured bindings `for (auto [a, b] : ...)` (C++17 feature)
- FastLED 3.10.x: Fail with `constexpr` values in `alignas()` (GCC 4.8.2 limitation)
- WiFiManager 2.0.17: Crashes when client connects in AP mode on Core 2.7.4

The GCC 4.8.2 toolchain in Core 2.7.4 only supports C++11, so libraries using C++17 features won't compile.

For Arduino IDE, install the equivalent versions via Library Manager (you may need to manually download WiFiManager 2.0.16-rc.2 from GitHub).

## Building the Web App

### Prerequisites

- Node.js 18+
- npm

### Development

```bash
cd web

# Install dependencies
npm install

# Start development server
npm run dev

# Production build (for local testing)
VITE_BASE_PATH=/configure/ npm run build
```

### Deployment

The web app is deployed automatically via GitHub Actions:
- Pushes to `web/` trigger the deploy workflow
- Firmware builds also trigger redeployment (to include updated firmware binary)
- The app is served from `/configure/` on GitHub Pages

### Web App Dependencies

- React 18
- TypeScript
- Vite
- Tailwind CSS
- @dnd-kit (drag and drop)

## Hardware Wiring

| Component | ESP8266 Pin | Notes |
|-----------|-------------|-------|
| LED Data | GPIO 14 (D5) | Default data pin |
| LED Power | 5V | External power recommended for >10 LEDs |
| LED Ground | GND | Common ground with ESP8266 |
| TSL2561 SDA | GPIO 4 (D2) | Optional light sensor |
| TSL2561 SCL | GPIO 5 (D1) | Optional light sensor |
| Analog Light Sensor | A0 | Alternative to TSL2561 |
| Reset Button | GPIO 12 (D6) | Hold 3 seconds to reset WiFi |

## Serial Command Interface

The device accepts JSON commands over serial at 115200 baud. This is how the web configuration tool communicates with the device.

### Commands

```json
// Get current status
{"cmd": "get_status"}

// Get full configuration
{"cmd": "get_config"}

// Update configuration (partial updates supported)
{"cmd": "set_config", "config": {"brightness": 30}}

// Enable MQTT for Home Assistant
{"cmd": "set_config", "config": {"mqttEnabled": true, "mqttBroker": "192.168.1.100", "mqttPort": 1883}}

// Set WiFi credentials
{"cmd": "set_wifi", "ssid": "MyNetwork", "pass": "password"}

// Clear WiFi and reboot
{"cmd": "reset_wifi"}

// Reboot device
{"cmd": "reboot"}

// Factory reset (clears config and WiFi)
{"cmd": "factory_reset"}

// Test LED pattern
{"cmd": "test_leds"}

// Force METAR fetch
{"cmd": "fetch_metars"}
```

### Response Format

All commands return JSON responses:

```json
{"status": "ok", "message": "optional message"}
{"status": "error", "message": "error description"}
```

Status and config commands return additional data:

```json
{
  "status": "ok",
  "wifi_connected": true,
  "wifi_ssid": "MyNetwork",
  "ip_address": "192.168.1.100",
  "heap_free": 35000,
  "uptime": 3600,
  "last_metar_update": 300,
  "metar_count": 50,
  "airport_count": 55
}
```

## Importing Legacy Configuration

If upgrading from the original LED Sectional firmware (single-file Arduino sketch), the web tool can import the old airport array format:

```cpp
std::vector<String> airports({
  "LIFR",  // 1 - Legend
  "IFR",   // 2 - Legend
  "KSFO",  // 3
  "KOAK",  // 4
  // ...
});
```

Use the "Import from Legacy Config" feature in the Airports tab.

## Testing Features

### Special Airport Codes

The following special codes can be used in the airport list for testing and legend display:

| Code | Purpose |
|------|---------|
| `VFR` | Legend LED - shows VFR green |
| `MVFR` | Legend LED - shows MVFR blue |
| `IFR` | Legend LED - shows IFR red |
| `LIFR` | Legend LED - shows LIFR magenta |
| `WVFR` | Legend LED - shows windy VFR yellow |
| `NULL` | Placeholder - LED stays off |
| `LTNG` | **Dev only** - Tests lightning animation (shows VFR green with periodic white flash) |

The `LTNG` code is useful for validating that the lightning flash animation works correctly, since thunderstorms may not always be present at configured airports.

## Architecture Notes

### Firmware

- **Streaming JSON Parser**: METARs are parsed as they stream in to minimize memory usage. This allows support for 150+ airports on the memory-constrained ESP8266.
- **WiFi Manager**: Uses WiFiManager library for captive portal configuration. Fresh devices wait for serial WiFi configuration; AP mode is only used as fallback when saved credentials fail.
- **Configuration Storage**: Settings stored in LittleFS as JSON.
- **MQTT / Home Assistant**: Optional integration using PubSubClient. Uses MQTT Discovery for automatic device registration in Home Assistant. Device appears as a light entity with on/off and brightness control. Power state persists across reboots.

### Web App

- **Web Serial API**: Browser-based serial communication (Chrome/Edge/Opera only)
- **Chunked Transmission**: Large commands sent in 32-byte chunks with delays to prevent ESP8266 serial buffer overflow
- **Offline Mode**: Airport list can be configured before connecting to hardware

## CI/CD Workflows

The project uses GitHub Actions for automated builds and deployment:

### Build Firmware (`.github/workflows/build-firmware.yml`)
- Triggers on version tags (`v*`)
- Builds firmware using PlatformIO
- Creates GitHub Release with firmware binaries

### Build and Deploy Web Config (`.github/workflows/deploy.yml`)
- Triggers on pushes to `web/` or when firmware build completes
- Builds web app with Vite
- Downloads latest firmware from releases
- Commits built files to `docs/configure/`
- GitHub Pages serves from `/docs` (Jekyll for main site, static for `/configure`)

### Creating a Release

```bash
# Tag and push to trigger build
git tag v2.0.1
git push origin v2.0.1
```

This will:
1. Build firmware and create GitHub Release
2. Trigger web deploy to include new firmware binary

## Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly on actual hardware
5. Submit a pull request

### Code Style

- Firmware: Standard Arduino/C++ conventions
- Web: ESLint + Prettier (configured in project)

## License

This project is open source. See the repository for license details.
