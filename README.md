# LED Sectional

A WiFi-connected LED display for aviation sectional maps that shows real-time flight conditions at airports using color-coded LEDs.

![LED Sectional Example](docs/images/Final%201.jpg)

## What It Does

The LED Sectional automatically fetches current weather data (METARs) from aviationweather.gov and displays flight conditions on your sectional map:

| Color | Meaning |
|-------|---------|
| Green | VFR - Clear skies, good visibility |
| Blue | MVFR - Marginal conditions |
| Red | IFR - Instrument conditions |
| Magenta | LIFR - Low IFR, poor conditions |
| Yellow | High Winds - VFR with winds over 25 knots |
| White Flash | Thunderstorms reported |
| Off | No data available |

Weather updates automatically every 15 minutes.

## What You Need

### Hardware

- **Controller board** - ESP8266-based (Wemos D1 Mini Lite recommended), or purchase a [pre-made kit](docs/kit.md)
- **LED string** - WS2811 or WS2812 LEDs (one per airport)
- **Power supply** - 5V DC, sized for your LED count
- **USB cable** - For initial setup (must support data, not charge-only)

### Your Map

You'll need a framed sectional chart with holes drilled at each airport location. See the [full build guide](docs/index.md) for detailed instructions on creating your map.

## Setup

### Step 1: Install the Firmware

1. Visit the [LED Sectional Configuration Tool](https://led-sectional.kyleharmon.com/configure/)
2. Connect your controller via USB
3. Click the **Firmware** tab
4. Click **Install Firmware** and follow the prompts

### Step 2: Connect to WiFi

After installing firmware, the controller needs your WiFi credentials:

1. Keep the USB cable connected
2. In the Configuration Tool, click **Connect**
3. Go to the **WiFi** tab
4. Enter your network name and password
5. Click **Save** - the device will reboot and connect

**Note:** The controller only works with 2.4 GHz WiFi networks.

### Step 3: Add Your Airports

1. In the Configuration Tool, go to the **Airports** tab
2. Add airports in order matching your LED string (LED 1 first, LED 2 second, etc.)
3. Use standard ICAO codes (e.g., KSFO, KJFK, KORD)
4. Click **Save** to upload to the device

**Special codes:**
- `NULL` - Skip an LED (leave it off)
- `VFR`, `MVFR`, `IFR`, `LIFR`, `WVFR` - Legend LEDs that always show that color
- `WBNK` - Wind blink demo - alternates between green and yellow (requires "Alternate Colors" setting enabled)

### Step 4: Mount and Enjoy

Disconnect USB, connect your 5V power supply, and mount your sectional!

## Troubleshooting

### LEDs aren't lighting up

- Check that your power supply is 5V and has enough current for your LED count
- Verify the data wire is connected to the correct pin (GPIO 14 / D5 by default)
- Make sure the LED string and controller share a common ground

### Can't connect via Web Serial

- Use Chrome, Edge, or Opera (Safari and Firefox don't support Web Serial)
- Try a different USB cable - some cables only carry power, not data
- Close any other programs that might be using the serial port

### WiFi won't connect

- Confirm your network is 2.4 GHz (5 GHz is not supported)
- Double-check the password
- Move the controller closer to your router during setup

### Some airports show no color

- The airport may not have recent METAR data
- Verify the ICAO code is correct at [aviationweather.gov](https://aviationweather.gov/)

## Adjusting Settings

In the Configuration Tool's **Settings** tab, you can adjust:

- **Brightness** - LED brightness level (0-255)
- **Wind Threshold** - Wind speed that triggers yellow color (default: 25 knots)
- **Lightning Animation** - Enable/disable flashing for thunderstorms
- **High Wind Indication** - Enable/disable yellow for high winds
- **Light Sensor** - Enable automatic brightness based on room lighting (requires additional hardware)

### Home Assistant Integration

The LED Sectional can integrate with Home Assistant for smart home control. Once configured, your sectional appears as a light in Home Assistant where you can:

- Turn the display on/off
- Adjust brightness
- Include in automations (e.g., dim at night, turn off when away)

To set up, go to **Settings > Advanced Settings** in the Configuration Tool and enable MQTT. Enter your Home Assistant's MQTT broker address and credentials. The device will automatically appear in Home Assistant via MQTT Discovery.

## More Information

- [Full Build Guide](docs/index.md) - Detailed instructions for building your sectional
- [Kit Information](docs/kit.md) - Pre-made electronics kit
- [Contributing & Development](CONTRIBUTING.md) - For developers who want to modify the code

## Acknowledgments

- Inspired by [this Reddit post](https://www.reddit.com/r/flying/comments/7avr8q/flight_conditions_sectional_wall_art_thing/) and [Dylan Rush's blog](http://blog.dylanhrush.com/2017/07/live-sectional-map.html)
- Weather data from [aviationweather.gov](https://aviationweather.gov/)
- Web flashing powered by [ESP Web Tools](https://esphome.github.io/esp-web-tools/)
