#include "leds.h"
#include "config.h"
#include <vector>

// LED array
static CRGB leds[MAX_LEDS];
static int numLeds = 0;

// Lightning LEDs list
static std::vector<int> lightningLeds;

// Store original colors for lightning flash restore
static std::vector<CRGB> lightningOriginalColors;

void ledsInit() {
    Config& config = getConfig();
    numLeds = config.airports.size();

    if (numLeds > MAX_LEDS) {
        numLeds = MAX_LEDS;
        Serial.printf("Warning: Capping LEDs at %d (configured %d)\n", MAX_LEDS, config.airports.size());
    }

    // Initialize FastLED
    // Note: We use a template with compile-time pin, so we default to pin 14
    // For runtime pin configuration, we'd need a different approach
    FastLED.addLeds<WS2811, 14, RGB>(leds, numLeds).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(config.brightness);

    Serial.printf("LEDs initialized: %d LEDs at brightness %d\n", numLeds, config.brightness);
}

void ledsUpdateCount(int count) {
    if (count > MAX_LEDS) {
        count = MAX_LEDS;
    }
    numLeds = count;
    FastLED.addLeds<WS2811, 14, RGB>(leds, numLeds).setCorrection(TypicalLEDStrip);
}

CRGB ledsGetCategoryColor(const String& category, int windSpeed, int gusts) {
    Config& config = getConfig();

    if (category == "LIFR") {
        return COLOR_LIFR;
    } else if (category == "IFR") {
        return COLOR_IFR;
    } else if (category == "MVFR") {
        return COLOR_MVFR;
    } else if (category == "VFR") {
        // Check for high winds
        if (config.doWinds && (windSpeed > config.windThreshold || gusts > config.windThreshold)) {
            return COLOR_WVFR;
        }
        return COLOR_VFR;
    }

    return COLOR_UNKNOWN;
}

void ledsSetFlightCategory(int index, const String& category, int windSpeed, int gusts) {
    if (index < 0 || index >= numLeds) return;

    CRGB color = ledsGetCategoryColor(category, windSpeed, gusts);
    leds[index] = color;
}

void ledsSetColor(int index, CRGB color) {
    if (index < 0 || index >= numLeds) return;
    leds[index] = color;
}

void ledsSetAll(CRGB color) {
    fill_solid(leds, numLeds, color);
}

void ledsClear() {
    fill_solid(leds, numLeds, CRGB::Black);
}

void ledsShow() {
    FastLED.show();
}

void ledsSetBrightness(uint8_t brightness) {
    FastLED.setBrightness(brightness);
}

uint8_t ledsGetBrightness() {
    return FastLED.getBrightness();
}

void ledsAddLightning(int index) {
    if (index < 0 || index >= numLeds) return;

    // Check if already in list
    for (int i : lightningLeds) {
        if (i == index) return;
    }

    lightningLeds.push_back(index);
    Serial.printf("Lightning added for LED %d\n", index);
}

void ledsClearLightning() {
    lightningLeds.clear();
}

bool ledsHasLightning() {
    return !lightningLeds.empty();
}

bool ledsDoLightning() {
    if (lightningLeds.empty()) return false;

    Config& config = getConfig();
    if (!config.doLightning) return false;

    // Store original colors
    lightningOriginalColors.clear();
    for (int index : lightningLeds) {
        lightningOriginalColors.push_back(leds[index]);
        leds[index] = COLOR_LIGHTNING;
    }

    // Show white flash
    FastLED.show();
    delay(25);

    // Restore original colors
    for (size_t i = 0; i < lightningLeds.size(); i++) {
        leds[lightningLeds[i]] = lightningOriginalColors[i];
    }
    FastLED.show();

    return true;
}

void ledsSetLegend() {
    Config& config = getConfig();

    // Set legend LEDs based on special codes in airport list
    for (size_t i = 0; i < config.airports.size() && i < (size_t)numLeds; i++) {
        const String& code = config.airports[i];

        if (code == "VFR") {
            leds[i] = COLOR_VFR;
        } else if (code == "MVFR") {
            leds[i] = COLOR_MVFR;
        } else if (code == "IFR") {
            leds[i] = COLOR_IFR;
        } else if (code == "LIFR") {
            leds[i] = COLOR_LIFR;
        } else if (code == "WVFR") {
            leds[i] = COLOR_WVFR;
        }
        // NULL entries remain at whatever color they were (usually black)
    }
}
