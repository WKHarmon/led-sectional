#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

#include <Arduino.h>

// Analog light sensor pin
#define ANALOG_LIGHT_PIN A0

// Initialize the light sensor based on configuration
void lightSensorInit();

// Read the current light level
// Returns a normalized value that can be used for brightness calculation
float lightSensorRead();

// Calculate and apply brightness based on current light level
// Returns the calculated brightness value (0-255)
uint8_t lightSensorAdjustBrightness();

// Check if light sensor is enabled and working
bool lightSensorIsEnabled();

#endif // LIGHT_SENSOR_H
