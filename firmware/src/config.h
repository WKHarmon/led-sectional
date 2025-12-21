#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <vector>

// Configuration version - increment when schema changes
#define CONFIG_VERSION 2

// Hardware defaults
#define DEFAULT_DATA_PIN 14
#define DEFAULT_NUM_LEDS 80
#define DEFAULT_BRIGHTNESS 20
#define DEFAULT_LED_TYPE "WS2811"
#define DEFAULT_COLOR_ORDER "RGB"

// Timing defaults (milliseconds)
#define DEFAULT_REQUEST_INTERVAL 900000  // 15 minutes
#define DEFAULT_LOOP_INTERVAL 5000       // 5 seconds
#define DEFAULT_WIFI_TIMEOUT 60          // seconds
#define DEFAULT_READ_TIMEOUT 15          // seconds

// Feature defaults
#define DEFAULT_WIND_THRESHOLD 25
#define DEFAULT_DO_LIGHTNING true
#define DEFAULT_DO_WINDS true
#define DEFAULT_WIND_ALTERNATE false
#define DEFAULT_WIND_ALTERNATE_INTERVAL 2000  // milliseconds (1-10 seconds)

// Light sensor defaults
#define DEFAULT_USE_LIGHT_SENSOR false
#define DEFAULT_MIN_BRIGHTNESS 20
#define DEFAULT_MAX_BRIGHTNESS 30
#define DEFAULT_MIN_LIGHT 16
#define DEFAULT_MAX_LIGHT 30

// MQTT defaults
#define DEFAULT_MQTT_ENABLED false
#define DEFAULT_MQTT_BROKER ""
#define DEFAULT_MQTT_PORT 1883
#define DEFAULT_MQTT_USERNAME ""
#define DEFAULT_MQTT_PASSWORD ""
#define DEFAULT_POWER_ON true

// WiFi reset button
#define WIFI_RESET_PIN 12  // GPIO12 (D6 on D1 Mini)

// Light sensor type enumeration
enum LightSensorType {
    LIGHT_SENSOR_NONE = 0,
    LIGHT_SENSOR_ANALOG = 1,
    LIGHT_SENSOR_TSL2561 = 2
};

// Main configuration structure
struct Config {
    int version;

    // Airport list - can hold up to 150+ airports
    std::vector<String> airports;

    // LED hardware settings
    int dataPin;
    int brightness;
    String ledType;
    String colorOrder;

    // Timing settings
    unsigned long requestInterval;
    unsigned long loopInterval;

    // Feature toggles
    int windThreshold;
    bool doLightning;
    bool doWinds;
    bool windAlternate;  // Alternate between category color and yellow for windy airports
    unsigned long windAlternateInterval;  // Milliseconds between color switches (1000-10000)

    // Light sensor settings
    bool useLightSensor;
    LightSensorType lightSensorType;
    int minBrightness;
    int maxBrightness;
    int minLight;
    int maxLight;

    // MQTT settings
    bool mqttEnabled;
    String mqttBroker;
    uint16_t mqttPort;
    String mqttUsername;
    String mqttPassword;

    // Power state (controlled via MQTT or web)
    bool powerOn;

    // Constructor with defaults
    Config() {
        version = CONFIG_VERSION;
        dataPin = DEFAULT_DATA_PIN;
        brightness = DEFAULT_BRIGHTNESS;
        ledType = DEFAULT_LED_TYPE;
        colorOrder = DEFAULT_COLOR_ORDER;
        requestInterval = DEFAULT_REQUEST_INTERVAL;
        loopInterval = DEFAULT_LOOP_INTERVAL;
        windThreshold = DEFAULT_WIND_THRESHOLD;
        doLightning = DEFAULT_DO_LIGHTNING;
        doWinds = DEFAULT_DO_WINDS;
        windAlternate = DEFAULT_WIND_ALTERNATE;
        windAlternateInterval = DEFAULT_WIND_ALTERNATE_INTERVAL;
        useLightSensor = DEFAULT_USE_LIGHT_SENSOR;
        lightSensorType = LIGHT_SENSOR_NONE;
        minBrightness = DEFAULT_MIN_BRIGHTNESS;
        maxBrightness = DEFAULT_MAX_BRIGHTNESS;
        minLight = DEFAULT_MIN_LIGHT;
        maxLight = DEFAULT_MAX_LIGHT;
        mqttEnabled = DEFAULT_MQTT_ENABLED;
        mqttBroker = DEFAULT_MQTT_BROKER;
        mqttPort = DEFAULT_MQTT_PORT;
        mqttUsername = DEFAULT_MQTT_USERNAME;
        mqttPassword = DEFAULT_MQTT_PASSWORD;
        powerOn = DEFAULT_POWER_ON;
    }

    // Get number of LEDs (derived from airports size)
    int getNumLeds() const {
        return airports.size();
    }
};

// Configuration management functions
bool configInit();
bool configLoad(Config& config);
bool configSave(const Config& config);
void configSetDefaults(Config& config);
String configToJson(const Config& config);
bool configFromJson(const String& json, Config& config);

// Get the global configuration instance
Config& getConfig();

#endif // CONFIG_H
