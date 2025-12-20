#ifndef MQTT_H
#define MQTT_H

#include <Arduino.h>

// Initialize MQTT client (call after WiFi is connected)
void mqttInit();

// Main MQTT loop - handles connection and message processing
// Call this regularly from main loop
void mqttLoop();

// Check if MQTT is connected
bool mqttIsConnected();

// Publish current state to Home Assistant
void mqttPublishState();

// Force reconnection attempt
void mqttReconnect();

#endif // MQTT_H
