#include "leds.h"
#include "config.h"
#include <vector>

// LED array
static CRGB leds[MAX_LEDS];
static int numLeds = 0;

// Lightning LEDs list
static std::vector<int> lightningLeds;

// Store original colors for lightning flash restore (fixed-size to avoid heap fragmentation)
static CRGB lightningOriginalColors[MAX_LEDS];

// Windy LEDs: store index and category color for alternation
struct WindyLed {
    int index;
    CRGB categoryColor;  // The non-yellow color to alternate with
};
static std::vector<WindyLed> windyLeds;
static unsigned long windAlternateLastSwitch = 0;
static bool windAlternateShowYellow = false;  // false = category color, true = yellow

void ledsInit() {
    Config& config = getConfig();
    numLeds = config.airports.size();

    if (numLeds > MAX_LEDS) {
        numLeds = MAX_LEDS;
        Serial.printf("Warning: Capping LEDs at %d (configured %d)\n", MAX_LEDS, config.airports.size());
    }

    // Initialize FastLED with appropriate pin based on configuration
    // GPIO 5 (D1) for old kits, GPIO 14 (D5) for new kits
    if (config.dataPin == 5) {
        FastLED.addLeds<WS2811, 5, RGB>(leds, numLeds).setCorrection(TypicalLEDStrip);
    } else {
        // Default to GPIO 14 for new kits
        FastLED.addLeds<WS2811, 14, RGB>(leds, numLeds).setCorrection(TypicalLEDStrip);
    }
    FastLED.setBrightness(config.brightness);

    Serial.printf("LEDs initialized: %d LEDs on GPIO %d at brightness %d\n",
                  numLeds, config.dataPin, config.brightness);
}

void ledsUpdateCount(int count) {
    Config& config = getConfig();
    if (count > MAX_LEDS) {
        count = MAX_LEDS;
    }
    numLeds = count;

    // Clear lightning and windy LEDs since indices may now be invalid
    lightningLeds.clear();
    windyLeds.clear();

    // Re-initialize with appropriate pin
    if (config.dataPin == 5) {
        FastLED.addLeds<WS2811, 5, RGB>(leds, numLeds).setCorrection(TypicalLEDStrip);
    } else {
        FastLED.addLeds<WS2811, 14, RGB>(leds, numLeds).setCorrection(TypicalLEDStrip);
    }
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

    Config& config = getConfig();

    // Check if this airport has high winds
    bool isWindy = config.doWinds &&
                   (windSpeed > config.windThreshold || gusts > config.windThreshold);

    if (isWindy && config.windAlternate) {
        // Wind alternation mode: get the pure category color (not yellow)
        CRGB categoryColor;
        if (category == "LIFR") {
            categoryColor = COLOR_LIFR;
        } else if (category == "IFR") {
            categoryColor = COLOR_IFR;
        } else if (category == "MVFR") {
            categoryColor = COLOR_MVFR;
        } else if (category == "VFR") {
            categoryColor = COLOR_VFR;
        } else {
            categoryColor = COLOR_UNKNOWN;
        }

        // Set to category color initially, add to windy list for alternation
        leds[index] = categoryColor;
        ledsAddWindy(index, categoryColor);
    } else {
        // Normal mode: use ledsGetCategoryColor which returns yellow for windy VFR
        CRGB color = ledsGetCategoryColor(category, windSpeed, gusts);
        leds[index] = color;
    }
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

void ledsRefresh() {
    FastLED.show();
}

void ledsShow() {
    ledsRefresh();
}

void ledsSetBrightness(uint8_t brightness) {
    FastLED.setBrightness(brightness);
}

uint8_t ledsGetBrightness() {
    return FastLED.getBrightness();
}

void ledsSetPower(bool on) {
    Config& config = getConfig();
    if (on) {
        // Restore configured brightness
        FastLED.setBrightness(config.brightness);
    } else {
        // Turn off by setting brightness to 0
        FastLED.setBrightness(0);
    }
    ledsRefresh();
}

bool ledsGetPower() {
    // Power is on if brightness > 0
    return FastLED.getBrightness() > 0;
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

    // Store original colors at matching indices, then set to lightning color
    for (int index : lightningLeds) {
        // Bounds check in case LED count changed
        if (index < 0 || index >= numLeds) continue;
        lightningOriginalColors[index] = leds[index];
        leds[index] = COLOR_LIGHTNING;
    }

    // Show white flash
    ledsRefresh();
    delay(25);

    // Restore original colors from matching indices
    for (int index : lightningLeds) {
        // Bounds check in case LED count changed
        if (index < 0 || index >= numLeds) continue;
        leds[index] = lightningOriginalColors[index];
    }
    ledsRefresh();

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
        } else if (code == "WBNK") {
            // Wind blink demo: alternates between VFR green and WVFR yellow
            leds[i] = COLOR_VFR;
            ledsAddWindy(i, COLOR_VFR);
        } else if (code == "LTNG") {
            // Test lightning: show VFR color and add to lightning list
            leds[i] = COLOR_VFR;
            ledsAddLightning(i);
        }
        // NULL entries remain at whatever color they were (usually black)
    }
}

void ledsAddWindy(int index, CRGB categoryColor) {
    if (index < 0 || index >= numLeds) return;

    // Check if already in list
    for (const WindyLed& wl : windyLeds) {
        if (wl.index == index) return;
    }

    windyLeds.push_back({index, categoryColor});
    Serial.printf("Windy LED added: %d\n", index);
}

void ledsClearWindy() {
    windyLeds.clear();
    windAlternateLastSwitch = 0;
    windAlternateShowYellow = false;
}

bool ledsHasWindy() {
    return !windyLeds.empty();
}

void ledsDoWindAlternate() {
    if (windyLeds.empty()) return;

    Config& config = getConfig();
    if (!config.windAlternate) return;

    unsigned long currentTime = millis();

    // Check if it's time to switch
    if (currentTime - windAlternateLastSwitch >= config.windAlternateInterval) {
        windAlternateShowYellow = !windAlternateShowYellow;
        windAlternateLastSwitch = currentTime;

        // Update all windy LEDs
        for (const WindyLed& wl : windyLeds) {
            if (wl.index < 0 || wl.index >= numLeds) continue;

            if (windAlternateShowYellow) {
                leds[wl.index] = COLOR_WVFR;  // Yellow
            } else {
                leds[wl.index] = wl.categoryColor;  // Original category color
            }
        }

        ledsRefresh();
    }
}
