#ifndef METAR_LISTENER_H
#define METAR_LISTENER_H

#include <Arduino.h>
#include <JsonListener.h>

// Structure to hold parsed METAR data for a single airport
struct MetarData {
    String icaoId;
    String fltCat;      // Flight category: VFR, MVFR, IFR, LIFR
    int wspd;           // Wind speed in knots
    int wgst;           // Wind gusts in knots
    String wxString;    // Weather string (check for "TS" = thunderstorm)

    MetarData() : wspd(0), wgst(0) {}

    void reset() {
        icaoId = "";
        fltCat = "";
        wspd = 0;
        wgst = 0;
        wxString = "";
    }
};

// Callback type for when a complete METAR is parsed
typedef void (*MetarCallback)(const MetarData& metar);

// Streaming JSON listener for METAR data
// Processes the JSON array from aviationweather.gov character by character
class MetarListener : public JsonListener {
public:
    MetarListener();

    // Set the callback function to be called for each parsed METAR
    void setCallback(MetarCallback callback);

    // Reset parser state (call before starting a new parse)
    void reset();

    // Get the number of METARs parsed in this session
    int getMetarCount() const;

    // JsonListener interface implementation
    void whitespace(char c) override;
    void startDocument() override;
    void endDocument() override;
    void startArray() override;
    void endArray() override;
    void startObject() override;
    void endObject() override;
    void key(String key) override;
    void value(String value) override;

private:
    MetarCallback callback;
    MetarData currentMetar;
    String currentKey;
    int arrayDepth;
    int objectDepth;
    int metarCount;

    // Track if we're inside the clouds array (to ignore nested objects)
    bool inCloudsArray;
};

#endif // METAR_LISTENER_H
