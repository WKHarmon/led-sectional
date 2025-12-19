#ifndef METAR_H
#define METAR_H

#include <Arduino.h>
#include "metar_listener.h"

// METAR API configuration
#define METAR_SERVER "aviationweather.gov"
#define METAR_PORT 443
#define METAR_BASE_URI "/api/data/metar?format=json&ids="
#define METAR_USER_AGENT "LED-Sectional/2.0"

// Callback type for processed METAR data
// Parameters: airport ICAO, LED index, flight category, wind speed, gusts, weather string
typedef void (*MetarProcessCallback)(const String& icao, int ledIndex,
                                      const String& fltCat, int wspd, int wgst,
                                      const String& wxString);

// Initialize METAR fetching
void metarInit();

// Fetch and parse METARs for configured airports
// Returns true if successful, false on error
// The callback will be called for each METAR that matches a configured airport
bool metarFetch(MetarProcessCallback callback);

// Build the airport request string from configuration
// Filters out NULL, VFR, MVFR, IFR, LIFR, WVFR entries
String metarBuildAirportString();

// Get the last fetch time (millis)
unsigned long metarGetLastFetchTime();

// Get the number of METARs received in the last fetch
int metarGetLastCount();

#endif // METAR_H
