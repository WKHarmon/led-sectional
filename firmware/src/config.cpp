#include "config.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

#define CONFIG_FILE "/config.json"

// Global configuration instance
static Config globalConfig;

Config& getConfig() {
    return globalConfig;
}

bool configInit() {
    if (!LittleFS.begin()) {
        Serial.println("Failed to mount LittleFS");
        return false;
    }
    Serial.println("LittleFS mounted successfully");
    return true;
}

void configSetDefaults(Config& config) {
    config = Config();  // Reset to constructor defaults

    // Default airport list with 50 LEDs for easy testing
    // Users should configure their own airports via the web interface
    config.airports = {
        "LIFR",   // 1 - Legend
        "IFR",    // 2 - Legend
        "MVFR",   // 3 - Legend
        "WVFR",   // 4 - Legend
        "VFR",    // 5 - Legend
        "KSFO",   // 6 - San Francisco International (for METAR test)
        "NULL",   // 7
        "NULL",   // 8
        "NULL",   // 9
        "NULL",   // 10
        "NULL",   // 11
        "NULL",   // 12
        "NULL",   // 13
        "NULL",   // 14
        "NULL",   // 15
        "NULL",   // 16
        "NULL",   // 17
        "NULL",   // 18
        "NULL",   // 19
        "NULL",   // 20
        "NULL",   // 21
        "NULL",   // 22
        "NULL",   // 23
        "NULL",   // 24
        "NULL",   // 25
        "NULL",   // 26
        "NULL",   // 27
        "NULL",   // 28
        "NULL",   // 29
        "NULL",   // 30
        "NULL",   // 31
        "NULL",   // 32
        "NULL",   // 33
        "NULL",   // 34
        "NULL",   // 35
        "NULL",   // 36
        "NULL",   // 37
        "NULL",   // 38
        "NULL",   // 39
        "NULL",   // 40
        "NULL",   // 41
        "NULL",   // 42
        "NULL",   // 43
        "NULL",   // 44
        "NULL",   // 45
        "NULL",   // 46
        "NULL",   // 47
        "NULL",   // 48
        "NULL",   // 49
        "NULL"    // 50
    };
}

bool configLoad(Config& config) {
    if (!LittleFS.exists(CONFIG_FILE)) {
        Serial.println("Config file not found, using defaults");
        configSetDefaults(config);
        return configSave(config);  // Save defaults for next boot
    }

    File file = LittleFS.open(CONFIG_FILE, "r");
    if (!file) {
        Serial.println("Failed to open config file");
        configSetDefaults(config);
        return false;
    }

    String json = file.readString();
    file.close();

    if (!configFromJson(json, config)) {
        Serial.println("Failed to parse config, using defaults");
        configSetDefaults(config);
        return false;
    }

    // Check version and migrate if needed
    if (config.version < CONFIG_VERSION) {
        Serial.printf("Migrating config from v%d to v%d\n", config.version, CONFIG_VERSION);
        config.version = CONFIG_VERSION;
        configSave(config);
    }

    Serial.printf("Config loaded: %d airports\n", config.airports.size());
    return true;
}

bool configSave(const Config& config) {
    String json = configToJson(config);

    File file = LittleFS.open(CONFIG_FILE, "w");
    if (!file) {
        Serial.println("Failed to open config file for writing");
        return false;
    }

    size_t written = file.print(json);
    file.close();

    if (written == 0) {
        Serial.println("Failed to write config file");
        return false;
    }

    Serial.println("Config saved successfully");
    return true;
}

String configToJson(const Config& config) {
    JsonDocument doc;

    doc["version"] = config.version;
    doc["dataPin"] = config.dataPin;
    doc["brightness"] = config.brightness;
    doc["ledType"] = config.ledType;
    doc["colorOrder"] = config.colorOrder;
    doc["requestInterval"] = config.requestInterval;
    doc["loopInterval"] = config.loopInterval;
    doc["windThreshold"] = config.windThreshold;
    doc["doLightning"] = config.doLightning;
    doc["doWinds"] = config.doWinds;
    doc["useLightSensor"] = config.useLightSensor;
    doc["lightSensorType"] = static_cast<int>(config.lightSensorType);
    doc["minBrightness"] = config.minBrightness;
    doc["maxBrightness"] = config.maxBrightness;
    doc["minLight"] = config.minLight;
    doc["maxLight"] = config.maxLight;
    doc["mqttEnabled"] = config.mqttEnabled;
    doc["mqttBroker"] = config.mqttBroker;
    doc["mqttPort"] = config.mqttPort;
    doc["mqttUsername"] = config.mqttUsername;
    doc["mqttPassword"] = config.mqttPassword;
    doc["powerOn"] = config.powerOn;

    JsonArray airportsArray = doc["airports"].to<JsonArray>();
    for (const String& airport : config.airports) {
        airportsArray.add(airport);
    }

    String output;
    serializeJson(doc, output);
    return output;
}

bool configFromJson(const String& json, Config& config) {
    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        Serial.printf("JSON parse error: %s\n", error.c_str());
        return false;
    }

    // Load values with defaults as fallback
    config.version = doc["version"] | CONFIG_VERSION;
    config.dataPin = doc["dataPin"] | DEFAULT_DATA_PIN;
    config.brightness = doc["brightness"] | DEFAULT_BRIGHTNESS;
    config.ledType = doc["ledType"] | DEFAULT_LED_TYPE;
    config.colorOrder = doc["colorOrder"] | DEFAULT_COLOR_ORDER;
    config.requestInterval = doc["requestInterval"] | DEFAULT_REQUEST_INTERVAL;
    config.loopInterval = doc["loopInterval"] | DEFAULT_LOOP_INTERVAL;
    config.windThreshold = doc["windThreshold"] | DEFAULT_WIND_THRESHOLD;
    config.doLightning = doc["doLightning"] | DEFAULT_DO_LIGHTNING;
    config.doWinds = doc["doWinds"] | DEFAULT_DO_WINDS;
    config.useLightSensor = doc["useLightSensor"] | DEFAULT_USE_LIGHT_SENSOR;
    config.lightSensorType = static_cast<LightSensorType>(doc["lightSensorType"] | 0);
    config.minBrightness = doc["minBrightness"] | DEFAULT_MIN_BRIGHTNESS;
    config.maxBrightness = doc["maxBrightness"] | DEFAULT_MAX_BRIGHTNESS;
    config.minLight = doc["minLight"] | DEFAULT_MIN_LIGHT;
    config.maxLight = doc["maxLight"] | DEFAULT_MAX_LIGHT;
    config.mqttEnabled = doc["mqttEnabled"] | DEFAULT_MQTT_ENABLED;
    config.mqttBroker = doc["mqttBroker"] | DEFAULT_MQTT_BROKER;
    config.mqttPort = doc["mqttPort"] | DEFAULT_MQTT_PORT;
    config.mqttUsername = doc["mqttUsername"] | DEFAULT_MQTT_USERNAME;
    config.mqttPassword = doc["mqttPassword"] | DEFAULT_MQTT_PASSWORD;
    config.powerOn = doc["powerOn"] | DEFAULT_POWER_ON;

    // Load airports array
    config.airports.clear();
    JsonArray airportsArray = doc["airports"];
    if (airportsArray) {
        for (JsonVariant airport : airportsArray) {
            config.airports.push_back(airport.as<String>());
        }
    }

    return true;
}
