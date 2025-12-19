#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>

// WiFi configuration
#define WIFI_AP_NAME "LED-Sectional-Setup"
#define WIFI_PORTAL_TIMEOUT 180  // 3 minutes

// Initialize WiFi with WiFiManager
// Returns true if connected successfully
bool wifiInit();

// Check if WiFi is connected
bool wifiIsConnected();

// Get current WiFi SSID
String wifiGetSSID();

// Get current IP address
String wifiGetIP();

// Start on-demand configuration portal
// Call this when user presses reset button
void wifiStartConfigPortal();

// Reset WiFi settings (clear saved credentials)
void wifiResetSettings();

// Set WiFi credentials programmatically (from serial command)
// Device will reboot after setting
void wifiSetCredentials(const String& ssid, const String& password);

// Check if reset button is being held
// Call in loop() to detect long-press
bool wifiCheckResetButton();

// Check if WiFi credentials are saved
// Returns true if there are saved credentials
bool wifiHasSavedCredentials();

#endif // WIFI_MANAGER_H
