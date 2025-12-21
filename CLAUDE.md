# Claude Code Context

This file provides context for Claude Code sessions working on the LED Sectional project.

## Project Overview

LED Sectional is a WiFi-connected LED display for aviation sectional maps showing real-time flight conditions (METARs) at airports using color-coded WS2811/WS2812 LEDs on an ESP8266.

## Repository Structure

```
led-sectional/
├── firmware/           # ESP8266 PlatformIO project
│   ├── src/           # C++ source files
│   ├── platformio.ini # Build config with pinned versions
│   └── .pio/          # Build artifacts (gitignored)
├── web/               # React/TypeScript configuration tool
│   └── src/           # Web app source
├── docs/              # GitHub Pages site (Jekyll)
│   └── configure/     # Built web app (auto-deployed)
└── .github/workflows/ # CI/CD for firmware builds and web deployment
```

## Critical Version Constraints

The firmware requires specific library versions due to ESP8266 platform limitations:

| Component | Version | Constraint |
|-----------|---------|------------|
| espressif8266 | 2.6.3 | Arduino Core 2.7.4 - stable interrupt handling for WS2812 timing |
| FastLED | 3.7.6 | Last version compatible with GCC 4.8.2 (C++11 only) |
| WiFiManager | 2.0.16-rc.2 | 2.0.17 crashes in AP mode on Core 2.7.4 |
| ArduinoJson | 7.4.2 | Pinned for reproducible builds |
| JsonStreamingParser | 1.0.5 | Pinned for reproducible builds |
| PubSubClient | 2.8.0 | Pinned for reproducible builds |

**Why Arduino Core 2.7.4?**
- Core 3.x changed NMI handling, causing first-LED flickering with FastLED
- No workaround works reliably (tried `FASTLED_ALLOW_INTERRUPTS`, `FASTLED_INTERRUPT_RETRY_COUNT`, double `show()`, delays)
- Core 2.7.4 just works without workarounds

**Why FastLED 3.7.6?**
- 3.7.7+: Uses C++17 `static_assert` without message parameter
- 3.9.x: Uses C++17 structured bindings
- 3.10.x: Uses `constexpr` in `alignas()` which GCC 4.8.2 doesn't support

**Why WiFiManager 2.0.16-rc.2?**
- 2.0.17 causes Exception 3 (LoadStoreError) when clients connect in AP mode
- Crash occurs in `getMenuOut()` due to unaligned flash access
- Must be installed via GitHub URL (not on PlatformIO registry)

## Build Commands

### Firmware

```bash
cd firmware

# Build
pio run -e d1_mini

# Upload via USB (may need slower baud rate)
pio run -e d1_mini -t upload

# If upload fails at 921600 baud, use 115200:
~/.local/pipx/venvs/platformio/bin/python ~/.platformio/packages/tool-esptoolpy*/esptool.py \
  --chip esp8266 --port /dev/cu.usbserial-* --baud 115200 \
  write_flash 0x0 .pio/build/d1_mini/firmware.bin

# Clean build (needed after changing library versions)
rm -rf .pio/libdeps .pio/build && pio run -e d1_mini
```

### Web App

```bash
cd web
npm install
npm run dev              # Development server
npm run build            # Production build
```

## Hardware Configuration

| Function | Pin | Notes |
|----------|-----|-------|
| LED Data | GPIO 14 (D5) | Default, configurable |
| LED Data (legacy) | GPIO 5 (D1) | For old kits |
| WiFi Reset Button | GPIO 12 (D6) | Hold 3 seconds |
| I2C SDA | GPIO 4 (D2) | TSL2561 light sensor |
| I2C SCL | GPIO 5 (D1) | TSL2561 light sensor |

## Release Process

```bash
# Tag triggers GitHub Actions to build and create release
git tag v2.x.y
git push origin v2.x.y
```

The CI will:
1. Build firmware binaries
2. Create GitHub Release with binaries attached
3. Trigger web deployment to include new firmware for web flashing

## Key Files

- `firmware/platformio.ini` - Library versions and build config
- `firmware/src/config.h` - Hardware pin definitions and defaults
- `firmware/src/leds.h` - LED constants including MAX_LEDS (300)
- `firmware/src/leds.cpp` - FastLED integration
- `firmware/src/wifi_manager.cpp` - WiFiManager integration
- `firmware/src/metar.cpp` - METAR fetching and parsing
- `web/src/App.tsx` - Main web app component
- `web/src/components/AirportConfig.tsx` - Airport config with MAX_AIRPORTS validation

## Debugging

### Decode crash stack traces

```bash
~/.platformio/packages/toolchain-xtensa/bin/xtensa-lx106-elf-addr2line \
  -pfiaC -e .pio/build/d1_mini/firmware.elf 0x40xxxxxx 0x40yyyyyy ...
```

### Serial monitor

```bash
pio device monitor -b 115200
```

### Common issues

- **First LED flickering**: Usually means Arduino Core version is wrong (should be 2.7.4)
- **Crash in AP mode**: WiFiManager version incompatibility (use 2.0.16-rc.2)
- **Build fails with C++ errors**: FastLED version too new for GCC 4.8.2 toolchain
- **Upload fails "Invalid head of packet"**: Use slower baud rate (115200)

## Testing

The firmware has special airport codes for testing:
- `VFR`, `MVFR`, `IFR`, `LIFR`, `WVFR` - Legend LEDs (fixed colors)
- `NULL` - Skip LED (stays off)
- `LTNG` - Test lightning animation
