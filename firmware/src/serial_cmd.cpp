#include "serial_cmd.h"
#include "config.h"
#include "wifi_manager.h"
#include "metar.h"
#include "leds.h"
#include "mqtt.h"
#include <ArduinoJson.h>

// Buffer for incoming serial data
static String serialBuffer = "";
static const size_t MAX_BUFFER_SIZE = 8192;  // Max command size

// Flag to force a METAR fetch (set by fetch_metars command)
static bool forceFetchMetars = false;

void serialCmdInit() {
    Serial.begin(SERIAL_BAUD);
    delay(100);
    Serial.println();
    Serial.print("LED Sectional v");
    Serial.println(FIRMWARE_VERSION);
    Serial.println("Serial command interface ready");
}

void serialCmdSendResponse(const String& status, const String& message) {
    JsonDocument doc;
    doc["status"] = status;
    if (message.length() > 0) {
        doc["message"] = message;
    }
    serializeJson(doc, Serial);
    Serial.println();
}

void serialCmdSendStatus() {
    JsonDocument doc;

    doc["status"] = "ok";
    doc["firmware_version"] = FIRMWARE_VERSION;
    doc["wifi_connected"] = wifiIsConnected();
    doc["wifi_ssid"] = wifiGetSSID();
    doc["ip_address"] = wifiGetIP();
    doc["needs_wifi_config"] = !wifiHasSavedCredentials();
    doc["heap_free"] = ESP.getFreeHeap();
    doc["uptime"] = millis() / 1000;
    doc["last_metar_update"] = metarGetLastFetchTime() / 1000;
    doc["metar_count"] = metarGetLastCount();
    doc["airport_count"] = getConfig().airports.size();
    doc["brightness"] = ledsGetBrightness();
    doc["has_lightning"] = ledsHasLightning();

    serializeJson(doc, Serial);
    Serial.println();
}

void serialCmdSendConfig() {
    Config& config = getConfig();

    JsonDocument doc;

    doc["status"] = "ok";

    JsonObject cfg = doc["config"].to<JsonObject>();
    cfg["version"] = config.version;
    cfg["dataPin"] = config.dataPin;
    cfg["brightness"] = config.brightness;
    cfg["ledType"] = config.ledType;
    cfg["colorOrder"] = config.colorOrder;
    cfg["requestInterval"] = config.requestInterval;
    cfg["loopInterval"] = config.loopInterval;
    cfg["windThreshold"] = config.windThreshold;
    cfg["doLightning"] = config.doLightning;
    cfg["doWinds"] = config.doWinds;
    cfg["windAlternate"] = config.windAlternate;
    cfg["windAlternateInterval"] = config.windAlternateInterval;
    cfg["useLightSensor"] = config.useLightSensor;
    cfg["lightSensorType"] = static_cast<int>(config.lightSensorType);
    cfg["minBrightness"] = config.minBrightness;
    cfg["maxBrightness"] = config.maxBrightness;
    cfg["minLight"] = config.minLight;
    cfg["maxLight"] = config.maxLight;
    cfg["mqttEnabled"] = config.mqttEnabled;
    cfg["mqttBroker"] = config.mqttBroker;
    cfg["mqttPort"] = config.mqttPort;
    cfg["mqttUsername"] = config.mqttUsername;
    cfg["mqttPassword"] = config.mqttPassword;
    cfg["powerOn"] = config.powerOn;

    JsonArray airports = cfg["airports"].to<JsonArray>();
    for (const String& airport : config.airports) {
        airports.add(airport);
    }

    serializeJson(doc, Serial);
    Serial.println();
}

static void processCommand(const String& cmdJson) {
    // Debug: show command length and heap
    Serial.printf("Processing command (%d bytes, heap: %d)\n",
                  cmdJson.length(), ESP.getFreeHeap());

    // Parse incoming JSON command
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, cmdJson);

    if (error) {
        Serial.printf("JSON parse error: %s (heap: %d)\n", error.c_str(), ESP.getFreeHeap());
        serialCmdSendResponse("error", String("JSON error: ") + error.c_str());
        return;
    }

    String cmd = doc["cmd"] | "";

    if (cmd.length() == 0) {
        Serial.println("Warning: empty command field");
        Serial.printf("First 100 chars: %.100s\n", cmdJson.c_str());
    }

    if (cmd == "get_config") {
        serialCmdSendConfig();
    }
    else if (cmd == "set_config") {
        Config& config = getConfig();

        // Update configuration from JSON
        JsonObject cfg = doc["config"];
        if (cfg) {
            if (cfg["brightness"].is<int>()) config.brightness = cfg["brightness"];
            if (cfg["windThreshold"].is<int>()) config.windThreshold = cfg["windThreshold"];
            if (cfg["doLightning"].is<bool>()) config.doLightning = cfg["doLightning"];
            if (cfg["doWinds"].is<bool>()) config.doWinds = cfg["doWinds"];
            if (cfg["windAlternate"].is<bool>()) config.windAlternate = cfg["windAlternate"];
            if (cfg["windAlternateInterval"].is<unsigned long>()) config.windAlternateInterval = cfg["windAlternateInterval"];
            if (cfg["useLightSensor"].is<bool>()) config.useLightSensor = cfg["useLightSensor"];
            if (cfg["lightSensorType"].is<int>()) config.lightSensorType = static_cast<LightSensorType>((int)cfg["lightSensorType"]);
            if (cfg["minBrightness"].is<int>()) config.minBrightness = cfg["minBrightness"];
            if (cfg["maxBrightness"].is<int>()) config.maxBrightness = cfg["maxBrightness"];
            if (cfg["minLight"].is<int>()) config.minLight = cfg["minLight"];
            if (cfg["maxLight"].is<int>()) config.maxLight = cfg["maxLight"];
            if (cfg["requestInterval"].is<unsigned long>()) {
                config.requestInterval = cfg["requestInterval"];
            }
            if (cfg["loopInterval"].is<unsigned long>()) {
                config.loopInterval = cfg["loopInterval"];
            }
            if (cfg["dataPin"].is<int>()) config.dataPin = cfg["dataPin"];
            if (cfg["colorOrder"].is<const char*>()) config.colorOrder = cfg["colorOrder"].as<String>();

            // MQTT settings
            bool mqttChanged = false;
            if (cfg["mqttEnabled"].is<bool>()) {
                if (config.mqttEnabled != cfg["mqttEnabled"].as<bool>()) mqttChanged = true;
                config.mqttEnabled = cfg["mqttEnabled"];
            }
            if (cfg["mqttBroker"].is<const char*>()) {
                if (config.mqttBroker != cfg["mqttBroker"].as<const char*>()) mqttChanged = true;
                config.mqttBroker = cfg["mqttBroker"].as<String>();
            }
            if (cfg["mqttPort"].is<int>()) {
                if (config.mqttPort != cfg["mqttPort"].as<int>()) mqttChanged = true;
                config.mqttPort = cfg["mqttPort"];
            }
            if (cfg["mqttUsername"].is<const char*>()) {
                config.mqttUsername = cfg["mqttUsername"].as<String>();
            }
            if (cfg["mqttPassword"].is<const char*>()) {
                config.mqttPassword = cfg["mqttPassword"].as<String>();
            }
            if (cfg["powerOn"].is<bool>()) {
                config.powerOn = cfg["powerOn"];
            }

            // Validate and clamp config values
            if (config.brightness < 0) config.brightness = 0;
            if (config.brightness > 255) config.brightness = 255;
            if (config.windThreshold < 0) config.windThreshold = 0;
            if (config.windThreshold > 100) config.windThreshold = 100;
            if (config.requestInterval < 60000) config.requestInterval = 60000;  // Min 1 minute
            if (config.requestInterval > 3600000) config.requestInterval = 3600000;  // Max 1 hour
            if (config.loopInterval < 50) config.loopInterval = 50;  // Min 50ms
            if (config.loopInterval > 60000) config.loopInterval = 60000;  // Max 1 minute
            if (config.dataPin != 5 && config.dataPin != 14) config.dataPin = 14;  // Only GPIO 5 or 14 supported
            // Validate colorOrder - must be one of the supported values
            if (config.colorOrder != "RGB" && config.colorOrder != "GRB" &&
                config.colorOrder != "BRG" && config.colorOrder != "RBG" &&
                config.colorOrder != "GBR" && config.colorOrder != "BGR") {
                config.colorOrder = "RGB";  // Default to RGB if invalid
            }
            if (config.minBrightness < 0) config.minBrightness = 0;
            if (config.minBrightness > 255) config.minBrightness = 255;
            if (config.maxBrightness < 0) config.maxBrightness = 0;
            if (config.maxBrightness > 255) config.maxBrightness = 255;
            if (config.mqttPort < 1 || config.mqttPort > 65535) config.mqttPort = 1883;
            if (config.windAlternateInterval < 1000) config.windAlternateInterval = 1000;  // Min 1 second
            if (config.windAlternateInterval > 10000) config.windAlternateInterval = 10000;  // Max 10 seconds

            // Handle airports array
            bool airportsChanged = false;
            JsonArray airports = cfg["airports"];
            if (airports) {
                config.airports.clear();
                for (JsonVariant airport : airports) {
                    config.airports.push_back(airport.as<String>());
                }
                airportsChanged = true;
            }

            // Save to flash
            if (configSave(config)) {
                // Apply brightness change immediately (if power is on)
                if (config.powerOn) {
                    ledsSetBrightness(config.brightness);
                }

                // Apply power state
                ledsSetPower(config.powerOn);

                // Update LED count if airports changed
                if (airportsChanged) {
                    ledsUpdateCount(config.airports.size());
                }

                // Reinitialize MQTT if settings changed
                if (mqttChanged) {
                    mqttInit();
                }

                // Publish state to MQTT if connected
                if (mqttIsConnected()) {
                    mqttPublishState();
                }

                ledsShow();
                serialCmdSendResponse("ok", "Configuration saved");
            } else {
                serialCmdSendResponse("error", "Failed to save configuration");
            }
        } else {
            serialCmdSendResponse("error", "Missing config object");
        }
    }
    else if (cmd == "get_status") {
        serialCmdSendStatus();
    }
    else if (cmd == "set_wifi") {
        String ssid = doc["ssid"] | "";
        String pass = doc["pass"] | "";

        if (ssid.length() > 0) {
            serialCmdSendResponse("ok", "WiFi credentials updated - rebooting");
            delay(100);
            wifiSetCredentials(ssid, pass);
            // Note: wifiSetCredentials will reboot the device
        } else {
            serialCmdSendResponse("error", "Missing SSID");
        }
    }
    else if (cmd == "reset_wifi") {
        serialCmdSendResponse("ok", "WiFi settings cleared - rebooting");
        delay(100);
        wifiResetSettings();
        // Note: wifiResetSettings will reboot the device
    }
    else if (cmd == "reboot") {
        serialCmdSendResponse("ok", "Rebooting");
        delay(100);
        ESP.restart();
    }
    else if (cmd == "factory_reset") {
        Config& config = getConfig();
        configSetDefaults(config);
        configSave(config);
        serialCmdSendResponse("ok", "Factory reset complete - rebooting");
        delay(100);
        wifiResetSettings();
    }
    else if (cmd == "test_leds") {
        // Test all LEDs with a rainbow pattern
        int numLeds = getConfig().airports.size();
        for (int i = 0; i < numLeds; i++) {
            ledsSetColor(i, CHSV((i * 255) / numLeds, 255, 255));
        }
        ledsShow();
        serialCmdSendResponse("ok", "LED test pattern displayed");
    }
    else if (cmd == "fetch_metars") {
        // Force an immediate METAR fetch
        forceFetchMetars = true;
        serialCmdSendResponse("ok", "METAR fetch queued");
    }
    else {
        serialCmdSendResponse("error", "Unknown command: " + cmd);
    }
}

// Track when we last received data for command completion detection
static unsigned long lastSerialDataTime = 0;
static const unsigned long SERIAL_IDLE_TIMEOUT = 50;  // ms to wait for more data

void serialCmdProcess() {
    // Read all available data into buffer
    while (Serial.available()) {
        char c = Serial.read();
        lastSerialDataTime = millis();

        if (c == '\n' || c == '\r') {
            // End of command
            if (serialBuffer.length() > 0) {
                processCommand(serialBuffer);
                serialBuffer = "";
                lastSerialDataTime = 0;  // Reset timeout tracking after successful command
            }
        } else {
            // Add to buffer
            if (serialBuffer.length() < MAX_BUFFER_SIZE) {
                serialBuffer += c;
            } else {
                // Buffer overflow - discard
                Serial.println("{\"status\":\"error\",\"message\":\"Command too long\"}");
                serialBuffer = "";
            }
        }
    }

    // If we have partial data and haven't received anything for a while,
    // it might be a truncated command - log for debugging
    if (serialBuffer.length() > 0 && lastSerialDataTime > 0) {
        if (millis() - lastSerialDataTime > SERIAL_IDLE_TIMEOUT * 10) {
            Serial.printf("Warning: incomplete command in buffer (%d bytes), clearing\n", serialBuffer.length());
            serialBuffer = "";
            lastSerialDataTime = 0;
        }
    }
}

bool serialCmdCheckForceFetch() {
    if (forceFetchMetars) {
        forceFetchMetars = false;
        return true;
    }
    return false;
}
