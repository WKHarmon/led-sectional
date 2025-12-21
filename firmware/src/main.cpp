/**
 * LED Sectional v2.0
 *
 * Aviation weather display using WS2811 LEDs and ESP8266
 * Fetches METAR data and displays flight categories on a sectional map
 *
 * Features:
 * - WiFiManager for easy WiFi configuration
 * - Streaming JSON parser for memory-efficient METAR parsing
 * - Web Serial configuration via browser
 * - Support for 150+ airports
 * - Lightning animation for thunderstorms
 * - Optional light sensor for automatic brightness
 *
 * Repository: https://github.com/WKHarmon/led-sectional
 */

#include <Arduino.h>
#include "config.h"
#include "wifi_manager.h"
#include "metar.h"
#include "leds.h"
#include "light_sensor.h"
#include "serial_cmd.h"
#include "mqtt.h"

// State tracking
static unsigned long lastMetarFetch = 0;
static bool initialFetchDone = false;
static bool ledStatusShown = false;

// Callback for METAR data processing
void onMetarReceived(const String& icao, int ledIndex, const String& fltCat,
                     int wspd, int wgst, const String& wxString) {
    Config& config = getConfig();

    // Log the METAR
    Serial.printf("%s: %s %dG%d kts LED %d", icao.c_str(), fltCat.c_str(), wspd, wgst, ledIndex);

    // Check for thunderstorm
    if (wxString.indexOf("TS") >= 0) {
        Serial.print(" [LIGHTNING]");
        ledsAddLightning(ledIndex);
    }
    Serial.println();

    // Set the LED color based on flight category
    ledsSetFlightCategory(ledIndex, fltCat, wspd, wgst);
}

void setup() {
    // Initialize serial first for debugging
    serialCmdInit();
    Serial.println();
    Serial.println("========================================");
    Serial.println("LED Sectional v2.0 Starting...");
    Serial.println("========================================");

    // Initialize filesystem and load configuration
    if (!configInit()) {
        Serial.println("FATAL: Failed to initialize filesystem");
        // Continue anyway, will use defaults
    }

    Config& config = getConfig();
    if (!configLoad(config)) {
        Serial.println("Using default configuration");
    }

    Serial.printf("Configuration: %d airports, brightness %d\n",
                  config.airports.size(), config.brightness);

    // Initialize LEDs
    ledsInit();
    ledsSetAll(COLOR_WIFI_CONNECTING);
    ledsShow();

    // Initialize WiFi
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);  // LED on during startup

    // Check if this is a fresh device (no saved WiFi credentials)
    // If so, wait indefinitely for serial configuration - no AP mode fallback
    // AP mode is only used when saved credentials fail to connect
    if (!wifiHasSavedCredentials()) {
        Serial.println("No saved WiFi credentials - waiting for serial configuration");
        Serial.println("Connect via Web Serial to configure WiFi");

        // Wait indefinitely for serial WiFi configuration
        // Device will reboot when credentials are set via set_wifi command
        while (true) {
            serialCmdProcess();

            // Blink LED to indicate waiting for config
            static unsigned long lastBlink = 0;
            if (millis() - lastBlink > 500) {
                digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
                lastBlink = millis();
            }

            delay(10);
            yield();
        }
    }

    if (!wifiInit()) {
        Serial.println("WiFi initialization failed - continuing anyway");
        ledsSetAll(COLOR_FETCH_ERROR);
        ledsShow();
    } else {
        ledsSetAll(COLOR_WIFI_CONNECTED);
        ledsShow();
        delay(500);
    }

    // Initialize light sensor if enabled
    if (config.useLightSensor) {
        lightSensorInit();
    }

    // Initialize METAR fetching
    metarInit();

    // Initialize MQTT for Home Assistant integration
    mqttInit();

    // Apply saved power state
    if (!config.powerOn) {
        ledsSetPower(false);
        Serial.println("Power state: OFF (from saved config)");
    }

    digitalWrite(LED_BUILTIN, HIGH);  // LED off, startup complete

    Serial.printf("Setup complete (free heap: %d bytes)\n", ESP.getFreeHeap());
    Serial.println("========================================");
}

void loop() {
    Config& config = getConfig();
    unsigned long currentTime = millis();

    // Process serial commands (from web interface)
    serialCmdProcess();

    // Check for WiFi reset button
    if (wifiCheckResetButton()) {
        Serial.println("WiFi reset requested via button");
        ledsSetAll(COLOR_WIFI_CONNECTING);
        ledsShow();
        wifiStartConfigPortal();
    }

    // Reconnect WiFi if disconnected
    if (!wifiIsConnected()) {
        if (!ledStatusShown) {
            ledsSetAll(COLOR_WIFI_CONNECTING);
            ledsShow();
            ledStatusShown = true;
        }

        // Try to reconnect
        if (!wifiInit()) {
            delay(5000);  // Wait before retry
            return;
        }

        ledStatusShown = false;
        ledsSetAll(COLOR_WIFI_CONNECTED);
        ledsShow();
        delay(500);
    }

    // MQTT loop - handles connection and message processing
    mqttLoop();

    // Skip display updates if power is off
    if (!config.powerOn) {
        // Still need to process serial and MQTT, but skip LED updates
        unsigned long waitStart = millis();
        while (millis() - waitStart < config.loopInterval) {
            serialCmdProcess();
            mqttLoop();
            delay(10);
            yield();
        }
        return;
    }

    // Adjust brightness if light sensor is enabled
    if (lightSensorIsEnabled()) {
        lightSensorAdjustBrightness();
    }

    // Lightning animation
    if (config.doLightning && ledsHasLightning()) {
        ledsDoLightning();
    }

    // Wind alternation animation (alternates between category color and yellow)
    if (config.windAlternate && ledsHasWindy()) {
        ledsDoWindAlternate();
    }

    // Determine if we should fetch METARs
    bool shouldFetch = false;

    if (!initialFetchDone) {
        // First fetch after boot
        shouldFetch = true;
    } else if (currentTime - lastMetarFetch >= config.requestInterval) {
        // Regular interval fetch
        shouldFetch = true;
    } else if (serialCmdCheckForceFetch()) {
        // Forced fetch via serial command
        shouldFetch = true;
    }

    if (shouldFetch && wifiIsConnected()) {
        Serial.println();
        Serial.println("----------------------------------------");
        Serial.println("Fetching METAR data...");

        digitalWrite(LED_BUILTIN, LOW);  // LED on during fetch

        // Clear lightning and windy lists, then clear LEDs before fetch
        ledsClearLightning();
        ledsClearWindy();
        ledsClear();

        // Check if there are any real airports to fetch
        String airportString = metarBuildAirportString();
        bool fetchSuccess = false;

        if (airportString.length() == 0) {
            // No real airports - just show legend/special codes
            Serial.println("No real airports to fetch - showing legend only");
            fetchSuccess = true;
        } else {
            // Fetch and process METARs
            fetchSuccess = metarFetch(onMetarReceived);
        }

        if (fetchSuccess) {
            if (airportString.length() > 0) {
                Serial.printf("Successfully fetched %d METARs\n", metarGetLastCount());
            }

            // Set legend LEDs (and LTNG for lightning test)
            ledsSetLegend();
            ledsShow();

            lastMetarFetch = currentTime;
            initialFetchDone = true;
        } else {
            Serial.println("METAR fetch failed");
            ledsSetAll(COLOR_FETCH_ERROR);
            ledsShow();

            // Retry after 60 seconds on failure (not too aggressive on the API)
            lastMetarFetch = currentTime - config.requestInterval + 60000;
        }

        digitalWrite(LED_BUILTIN, HIGH);  // LED off

        Serial.println("----------------------------------------");
    }

    // Refresh LEDs every loop to correct any timing glitches
    ledsRefresh();

    // Wait for next loop iteration, but keep processing serial and MQTT
    // This prevents serial buffer overflow during long delays
    unsigned long waitStart = millis();
    while (millis() - waitStart < config.loopInterval) {
        serialCmdProcess();  // Keep draining serial buffer
        mqttLoop();          // Keep MQTT connection alive
        delay(10);  // Small delay to prevent tight loop
        yield();
    }
}
