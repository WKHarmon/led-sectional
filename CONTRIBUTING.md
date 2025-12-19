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
│   └── public/        # Static assets + firmware binary
├── docs/              # GitHub Pages documentation
│   ├── index.md       # Full build guide
│   └── kit.md         # Kit information
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

### Dependencies

PlatformIO manages these automatically. For Arduino IDE, install via Library Manager:

- FastLED ^3.10.3
- WiFiManager ^2.0.17
- JsonStreamingParser ^1.0.5
- ArduinoJson ^7.4.2
- Adafruit TSL2561 ^1.1.2
- Adafruit Unified Sensor ^1.1.15

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

# Production build
npm run build

# Deploy to GitHub Pages
npm run deploy
```

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
| Reset Button | GPIO 0 (D3/FLASH) | Hold 3 seconds to reset WiFi |

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

## Architecture Notes

### Firmware

- **Streaming JSON Parser**: METARs are parsed as they stream in to minimize memory usage. This allows support for 150+ airports on the memory-constrained ESP8266.
- **WiFi Manager**: Uses WiFiManager library for captive portal configuration. Fresh devices wait for serial WiFi configuration; AP mode is only used as fallback when saved credentials fail.
- **Configuration Storage**: Settings stored in LittleFS as JSON.

### Web App

- **Web Serial API**: Browser-based serial communication (Chrome/Edge/Opera only)
- **Chunked Transmission**: Large commands sent in 32-byte chunks with delays to prevent ESP8266 serial buffer overflow
- **Offline Mode**: Airport list can be configured before connecting to hardware

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
