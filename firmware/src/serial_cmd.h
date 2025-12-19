#ifndef SERIAL_CMD_H
#define SERIAL_CMD_H

#include <Arduino.h>

// Serial baud rate
#define SERIAL_BAUD 115200

// Initialize serial command interface
void serialCmdInit();

// Process any pending serial commands
// Call this in loop()
void serialCmdProcess();

// Send a JSON response
void serialCmdSendResponse(const String& status, const String& message = "");

// Send the current status
void serialCmdSendStatus();

// Send the current configuration
void serialCmdSendConfig();

// Check if a METAR fetch was requested via serial command
// Returns true and clears the flag if fetch was requested
bool serialCmdCheckForceFetch();

#endif // SERIAL_CMD_H
