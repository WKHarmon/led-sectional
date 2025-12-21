#include "metar.h"
#include "config.h"
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <JsonStreamingParser.h>

// Static variables for tracking fetch state
static unsigned long lastFetchTime = 0;
static int lastMetarCount = 0;

// Static variables for the current fetch operation
static MetarProcessCallback currentCallback = nullptr;

// Special airport codes that shouldn't be fetched
static bool isSpecialCode(const String& code) {
    return code == "NULL" || code == "VFR" || code == "MVFR" ||
           code == "IFR" || code == "LIFR" || code == "WVFR" ||
           code == "LTNG";
}

// Find the LED index for an airport ICAO code
// Returns -1 if not found
static int findAirportIndex(const String& icao) {
    Config& config = getConfig();
    for (size_t i = 0; i < config.airports.size(); i++) {
        if (config.airports[i] == icao) {
            return i;
        }
    }
    return -1;
}

// Callback from MetarListener when a METAR is parsed
static void onMetarParsed(const MetarData& metar) {
    if (currentCallback) {
        int ledIndex = findAirportIndex(metar.icaoId);
        if (ledIndex >= 0) {
            currentCallback(metar.icaoId, ledIndex, metar.fltCat,
                          metar.wspd, metar.wgst, metar.wxString);
        }
    }
}

void metarInit() {
    lastFetchTime = 0;
    lastMetarCount = 0;
}

String metarBuildAirportString() {
    Config& config = getConfig();

    // Pre-allocate to avoid O(n²) string concatenation
    // Each airport is ~5 chars (4 + comma), reserve for all airports
    String airportString;
    airportString.reserve(config.airports.size() * 6);

    bool first = true;
    for (const String& airport : config.airports) {
        if (!isSpecialCode(airport)) {
            if (first) {
                first = false;
                airportString = airport;
            } else {
                airportString += ',';
                airportString += airport;
            }
        }
    }

    return airportString;
}

bool metarFetch(MetarProcessCallback callback) {
    currentCallback = callback;

    String airportString = metarBuildAirportString();
    if (airportString.length() == 0) {
        Serial.println("No airports configured to fetch");
        return false;
    }

    Serial.printf("Fetching METARs for: %s\n", airportString.c_str());

    // Create secure client
    BearSSL::WiFiClientSecure client;
    client.setInsecure();  // Skip certificate verification (API uses valid cert)

    Serial.println("Connecting to aviationweather.gov...");
    if (!client.connect(METAR_SERVER, METAR_PORT)) {
        Serial.println("Connection failed!");
        return false;
    }
    Serial.println("Connected");

    // Build and send HTTP request
    String uri = String(METAR_BASE_URI) + airportString;
    Serial.printf("GET %s\n", uri.c_str());

    client.print("GET ");
    client.print(uri);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(METAR_SERVER);
    client.print("User-Agent: ");
    client.println(METAR_USER_AGENT);
    client.println("Connection: close");
    client.println();
    client.flush();

    // Wait for response with timeout
    unsigned long timeout = millis();
    while (!client.available()) {
        if (millis() - timeout > 15000) {
            Serial.println("Timeout waiting for response");
            client.stop();
            return false;
        }
        if (!client.connected()) {
            Serial.println("Connection lost while waiting for response");
            client.stop();
            return false;
        }
        delay(10);
        yield();
    }

    // Read headers until empty line
    bool headersComplete = false;
    timeout = millis();
    while (!headersComplete) {
        if (!client.connected() && !client.available()) {
            Serial.println("Connection closed before headers complete");
            break;
        }
        if (millis() - timeout > 10000) {
            Serial.println("Timeout reading headers");
            break;
        }
        if (client.available()) {
            String line = client.readStringUntil('\n');
            line.trim();
            if (line.length() == 0) {
                headersComplete = true;
            }
            timeout = millis();  // Reset timeout on successful read
        } else {
            delay(10);
        }
        yield();
    }

    if (!headersComplete) {
        Serial.println("Failed to read response headers");
        Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
        client.stop();
        return false;
    }

    // Create streaming parser
    JsonStreamingParser parser;
    MetarListener listener;
    listener.setCallback(onMetarParsed);
    parser.setListener(&listener);

    // Parse response body character by character
    // Limit response size to prevent memory exhaustion from malformed responses
    static const int MAX_RESPONSE_SIZE = 150000;  // ~150KB max
    Serial.print("Parsing response");
    timeout = millis();
    int charCount = 0;

    while (client.connected() || client.available()) {
        if (client.available()) {
            char c = client.read();
            parser.parse(c);
            charCount++;
            timeout = millis();  // Reset timeout on data received

            // Abort if response is too large
            if (charCount > MAX_RESPONSE_SIZE) {
                Serial.println(" response too large, aborting");
                break;
            }
        } else if (millis() - timeout > DEFAULT_READ_TIMEOUT * 1000) {
            Serial.println(" timeout");
            break;
        }
        yield();  // Let WiFi stack process
    }

    Serial.println(" done");
    client.stop();

    lastFetchTime = millis();
    lastMetarCount = listener.getMetarCount();

    Serial.printf("Parsed %d METARs (%d bytes)\n", lastMetarCount, charCount);

    currentCallback = nullptr;
    return lastMetarCount > 0;
}

unsigned long metarGetLastFetchTime() {
    return lastFetchTime;
}

int metarGetLastCount() {
    return lastMetarCount;
}
