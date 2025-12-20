#include "light_sensor.h"
#include "config.h"
#include "leds.h"

// Conditionally include TSL2561 library
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>

// TSL2561 sensor instance
static Adafruit_TSL2561_Unified* tsl = nullptr;
static bool sensorAvailable = false;

void lightSensorInit() {
    Config& config = getConfig();

    if (!config.useLightSensor) {
        sensorAvailable = false;
        Serial.println("Light sensor disabled");
        return;
    }

    if (config.lightSensorType == LIGHT_SENSOR_TSL2561) {
        // Initialize TSL2561 digital sensor
        Serial.println("Initializing TSL2561 light sensor...");

        // Initialize I2C on D2 (GPIO4) and D1 (GPIO5)
        Wire.begin(D2, D1);

        tsl = new Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

        if (!tsl->begin()) {
            Serial.println("TSL2561 not detected - check wiring!");
            sensorAvailable = false;
            delete tsl;
            tsl = nullptr;
        } else {
            // Configure sensor
            tsl->enableAutoRange(true);
            tsl->setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);
            sensorAvailable = true;
            Serial.println("TSL2561 initialized successfully");
        }
    } else if (config.lightSensorType == LIGHT_SENSOR_ANALOG) {
        // Initialize analog sensor
        Serial.println("Initializing analog light sensor...");
        pinMode(ANALOG_LIGHT_PIN, INPUT);
        sensorAvailable = true;
        Serial.println("Analog light sensor initialized");
    } else {
        sensorAvailable = false;
        Serial.println("No light sensor type configured");
    }
}

float lightSensorRead() {
    Config& config = getConfig();

    if (!sensorAvailable || !config.useLightSensor) {
        return -1;
    }

    float reading = 0;

    if (config.lightSensorType == LIGHT_SENSOR_TSL2561 && tsl != nullptr) {
        sensors_event_t event;
        tsl->getEvent(&event);
        reading = event.light;

        if (reading < 0) {
            // Sensor saturated or error
            reading = config.maxLight;
        }
    } else if (config.lightSensorType == LIGHT_SENSOR_ANALOG) {
        reading = analogRead(ANALOG_LIGHT_PIN);
    }

    return reading;
}

uint8_t lightSensorAdjustBrightness() {
    Config& config = getConfig();

    if (!sensorAvailable || !config.useLightSensor) {
        return config.brightness;
    }

    float reading = lightSensorRead();
    uint8_t brightness;

    Serial.printf("Light reading: %.1f", reading);

    if (reading <= config.minLight) {
        brightness = config.minBrightness;  // Use minimum brightness in dark conditions
    } else if (reading >= config.maxLight) {
        brightness = config.maxBrightness;
    } else {
        // Linear interpolation between min and max
        float percent = (reading - config.minLight) / (float)(config.maxLight - config.minLight);
        brightness = percent * (config.maxBrightness - config.minBrightness) + config.minBrightness;
    }

    Serial.printf(" -> brightness: %d\n", brightness);

    ledsSetBrightness(brightness);
    ledsShow();

    return brightness;
}

bool lightSensorIsEnabled() {
    Config& config = getConfig();
    return config.useLightSensor && sensorAvailable;
}
