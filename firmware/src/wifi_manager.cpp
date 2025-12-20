#include "wifi_manager.h"
#include "config.h"
#include <ESP8266WiFi.h>
#include <WiFiManager.h>

// Static WiFiManager instance
static WiFiManager wm;

// Button state tracking for long-press detection
static unsigned long buttonPressStart = 0;
static bool buttonWasPressed = false;
#define BUTTON_HOLD_TIME 3000  // 3 seconds to trigger reset

bool wifiInit() {
    // Set up the reset button pin
    pinMode(WIFI_RESET_PIN, INPUT_PULLUP);

    // Check if button is held during boot (immediate reset)
    if (digitalRead(WIFI_RESET_PIN) == LOW) {
        Serial.println("Reset button held during boot - clearing WiFi settings");
        delay(100);  // Debounce
        if (digitalRead(WIFI_RESET_PIN) == LOW) {
            wm.resetSettings();
            Serial.println("WiFi settings cleared");
        }
    }

    // Configure WiFiManager
    wm.setConfigPortalTimeout(WIFI_PORTAL_TIMEOUT);
    wm.setConnectTimeout(DEFAULT_WIFI_TIMEOUT);

    // Set custom hostname
    String hostname = "LED-Sectional-" + String(ESP.getChipId(), HEX);
    WiFi.hostname(hostname);
    wm.setHostname(hostname.c_str());

    // Set dark theme for config portal (easier on eyes)
    wm.setDarkMode(true);

    // Enable debug output
    wm.setDebugOutput(true);

    Serial.println("Connecting to WiFi...");

    // Try to connect, start config portal on failure
    if (!wm.autoConnect(WIFI_AP_NAME)) {
        Serial.println("Failed to connect to WiFi");
        Serial.println("Configuration portal timed out");
        return false;
    }

    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    return true;
}

bool wifiIsConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String wifiGetSSID() {
    return WiFi.SSID();
}

String wifiGetIP() {
    if (wifiIsConnected()) {
        return WiFi.localIP().toString();
    }
    return "0.0.0.0";
}

void wifiStartConfigPortal() {
    Serial.println("Starting on-demand configuration portal...");

    // Stop any current connection
    WiFi.disconnect(true);
    delay(100);

    // Start config portal (blocking)
    wm.setConfigPortalTimeout(WIFI_PORTAL_TIMEOUT);
    if (!wm.startConfigPortal(WIFI_AP_NAME)) {
        Serial.println("Config portal timed out or failed");
    } else {
        Serial.println("WiFi configured via portal");
    }
}

void wifiResetSettings() {
    Serial.println("Resetting WiFi settings...");
    wm.resetSettings();
    Serial.println("WiFi settings cleared - rebooting");
    delay(500);
    ESP.restart();
}

void wifiSetCredentials(const String& ssid, const String& password) {
    Serial.println("Setting WiFi credentials via serial...");

    // Disconnect current connection (false = don't erase saved credentials yet)
    WiFi.disconnect(false);
    delay(100);

    // Ensure credentials are saved to flash
    WiFi.persistent(true);

    // Connect with new credentials (this saves them to flash)
    WiFi.begin(ssid.c_str(), password.c_str());

    // Wait briefly for credentials to be saved
    delay(1000);

    Serial.println("WiFi credentials updated - rebooting");
    delay(500);
    ESP.restart();
}

bool wifiCheckResetButton() {
    bool buttonPressed = (digitalRead(WIFI_RESET_PIN) == LOW);

    if (buttonPressed && !buttonWasPressed) {
        // Button just pressed
        buttonPressStart = millis();
        buttonWasPressed = true;
    } else if (!buttonPressed && buttonWasPressed) {
        // Button released
        buttonWasPressed = false;
        buttonPressStart = 0;
    } else if (buttonPressed && buttonWasPressed) {
        // Button still held - check duration
        if (millis() - buttonPressStart >= BUTTON_HOLD_TIME) {
            Serial.println("Reset button held for 3 seconds");
            return true;
        }
    }

    return false;
}

bool wifiHasSavedCredentials() {
    // Check saved credentials using ESP8266 SDK directly
    // This works even before WiFiManager is initialized
    station_config conf;
    wifi_station_get_config_default(&conf);
    // Check first byte directly to avoid buffer overrun with non-null-terminated SSIDs
    return conf.ssid[0] != 0;
}
