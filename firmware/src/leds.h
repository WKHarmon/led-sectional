#ifndef LEDS_H
#define LEDS_H

#include <Arduino.h>

#include <FastLED.h>

// Maximum number of LEDs supported
#define MAX_LEDS 200

// Flight category colors
#define COLOR_VFR     CRGB::Green
#define COLOR_MVFR    CRGB::Blue
#define COLOR_IFR     CRGB::Red
#define COLOR_LIFR    CRGB::Magenta
#define COLOR_WVFR    CRGB::Yellow    // VFR with high winds
#define COLOR_UNKNOWN CRGB::Black

// Status indicator colors
#define COLOR_WIFI_CONNECTING CRGB::Orange
#define COLOR_WIFI_CONNECTED  CRGB::Purple
#define COLOR_FETCH_ERROR     CRGB::Cyan
#define COLOR_LIGHTNING       CRGB::White

// Initialize LEDs with configuration
void ledsInit();

// Update LED array size when configuration changes
void ledsUpdateCount(int count);

// Set a single LED to a flight category color
void ledsSetFlightCategory(int index, const String& category, int windSpeed, int gusts);

// Set a single LED to a specific color
void ledsSetColor(int index, CRGB color);

// Set all LEDs to a color (for status indication)
void ledsSetAll(CRGB color);

// Clear all LEDs
void ledsClear();

// Show the current LED state (with ESP8266 timing workarounds)
void ledsShow();

// Simple refresh - call every loop to correct timing glitches
void ledsRefresh();

// Set brightness (0-255)
void ledsSetBrightness(uint8_t brightness);

// Get current brightness
uint8_t ledsGetBrightness();

// Set power state (on/off)
void ledsSetPower(bool on);

// Get power state
bool ledsGetPower();

// Add an LED to the lightning list (for thunderstorm animation)
void ledsAddLightning(int index);

// Clear the lightning list
void ledsClearLightning();

// Trigger lightning flash effect on all lightning LEDs
// Returns true if there are lightning LEDs to flash
bool ledsDoLightning();

// Check if there are any lightning LEDs
bool ledsHasLightning();

// Set legend LEDs (VFR, MVFR, IFR, LIFR, WVFR indicators)
void ledsSetLegend();

// Get the CRGB color for a flight category
CRGB ledsGetCategoryColor(const String& category, int windSpeed = 0, int gusts = 0);

#endif // LEDS_H
