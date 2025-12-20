#include "mqtt.h"
#include "config.h"
#include "leds.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// MQTT client
static WiFiClient espClient;
static PubSubClient mqttClient(espClient);

// Topic buffers
static char topicDiscovery[64];
static char topicCommand[48];
static char topicState[48];
static char topicAvailability[48];
static char clientId[32];

// Reconnection timing
static unsigned long lastReconnectAttempt = 0;
static unsigned long reconnectInterval = 1000;  // Start at 1 second
#define MAX_RECONNECT_INTERVAL 60000  // Max 60 seconds between attempts

// Build topics based on chip ID
static void buildTopics() {
    uint32_t chipId = ESP.getChipId();
    snprintf(clientId, sizeof(clientId), "led-sectional-%06X", chipId);
    snprintf(topicDiscovery, sizeof(topicDiscovery), "homeassistant/light/%s/config", clientId);
    snprintf(topicCommand, sizeof(topicCommand), "%s/set", clientId);
    snprintf(topicState, sizeof(topicState), "%s/state", clientId);
    snprintf(topicAvailability, sizeof(topicAvailability), "%s/availability", clientId);
}

// Publish Home Assistant discovery payload
static void publishDiscovery() {
    JsonDocument doc;

    // Setting name to null makes this the primary entity for the device,
    // so it shows as just "LED Sectional" instead of "LED Sectional LED Sectional"
    doc["name"] = (char*)NULL;
    doc["unique_id"] = clientId;
    doc["command_topic"] = topicCommand;
    doc["state_topic"] = topicState;
    doc["schema"] = "json";
    doc["brightness"] = true;
    doc["brightness_scale"] = 255;

    // Availability (LWT)
    doc["availability_topic"] = topicAvailability;
    doc["payload_available"] = "online";
    doc["payload_not_available"] = "offline";

    // Device info for HA device registry
    JsonObject device = doc["device"].to<JsonObject>();
    device["identifiers"].to<JsonArray>().add(clientId);
    device["name"] = "LED Sectional";
    device["manufacturer"] = "DIY";
    device["model"] = "ESP8266 METAR Map";

    char payload[512];
    serializeJson(doc, payload, sizeof(payload));

    if (mqttClient.publish(topicDiscovery, payload, true)) {
        Serial.println("MQTT: Discovery published");
    } else {
        Serial.println("MQTT: Discovery publish failed");
    }
}

// Handle incoming MQTT messages
static void mqttCallback(char* topic, byte* payload, unsigned int length) {
    // Null-terminate the payload
    char message[256];
    if (length >= sizeof(message)) length = sizeof(message) - 1;
    memcpy(message, payload, length);
    message[length] = '\0';

    Serial.printf("MQTT: Received on %s: %s\n", topic, message);

    // Parse JSON command
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, message);
    if (error) {
        Serial.printf("MQTT: JSON parse error: %s\n", error.c_str());
        return;
    }

    Config& config = getConfig();
    bool stateChanged = false;
    bool brightnessChanged = false;

    // Handle state (ON/OFF)
    if (doc["state"].is<const char*>()) {
        const char* state = doc["state"];
        bool newPowerOn = (strcmp(state, "ON") == 0);
        if (newPowerOn != config.powerOn) {
            config.powerOn = newPowerOn;
            ledsSetPower(config.powerOn);
            stateChanged = true;
            Serial.printf("MQTT: Power set to %s\n", config.powerOn ? "ON" : "OFF");
        }
    }

    // Handle brightness
    if (doc["brightness"].is<int>()) {
        int brightness = doc["brightness"];
        if (brightness < 0) brightness = 0;
        if (brightness > 255) brightness = 255;

        if (config.useLightSensor) {
            // Light sensor active: control maxBrightness instead
            // The sensor will auto-adjust between minBrightness and this new max
            if (brightness != config.maxBrightness) {
                config.maxBrightness = brightness;
                brightnessChanged = true;
                Serial.printf("MQTT: Max brightness set to %d (light sensor active)\n", brightness);
            }
        } else {
            // No light sensor: control brightness directly
            if (brightness != config.brightness) {
                config.brightness = brightness;
                if (config.powerOn) {
                    ledsSetBrightness(config.brightness);
                    ledsShow();
                }
                brightnessChanged = true;
                Serial.printf("MQTT: Brightness set to %d\n", config.brightness);
            }
        }
    }

    // Save config if anything changed
    if (stateChanged || brightnessChanged) {
        configSave(config);
        mqttPublishState();
    }
}

void mqttInit() {
    Config& config = getConfig();

    if (!config.mqttEnabled || config.mqttBroker.length() == 0) {
        Serial.println("MQTT: Disabled or no broker configured");
        return;
    }

    buildTopics();

    mqttClient.setServer(config.mqttBroker.c_str(), config.mqttPort);
    mqttClient.setCallback(mqttCallback);
    mqttClient.setBufferSize(512);  // Larger buffer for discovery payload

    Serial.printf("MQTT: Initialized for broker %s:%d\n",
                  config.mqttBroker.c_str(), config.mqttPort);
}

void mqttLoop() {
    Config& config = getConfig();

    if (!config.mqttEnabled || config.mqttBroker.length() == 0) {
        return;
    }

    if (mqttClient.connected()) {
        mqttClient.loop();
        return;
    }

    // Handle reconnection with exponential backoff
    unsigned long now = millis();
    if (now - lastReconnectAttempt < reconnectInterval) {
        return;
    }
    lastReconnectAttempt = now;

    Serial.printf("MQTT: Connecting to %s:%d...\n",
                  config.mqttBroker.c_str(), config.mqttPort);

    // Connect with LWT (Last Will and Testament) for availability tracking
    // LWT will publish "offline" if we disconnect unexpectedly
    bool connected;
    if (config.mqttUsername.length() > 0) {
        connected = mqttClient.connect(clientId,
                                       config.mqttUsername.c_str(),
                                       config.mqttPassword.c_str(),
                                       topicAvailability,  // LWT topic
                                       0,                   // LWT QoS
                                       true,                // LWT retain
                                       "offline");          // LWT message
    } else {
        connected = mqttClient.connect(clientId,
                                       topicAvailability,  // LWT topic
                                       0,                   // LWT QoS
                                       true,                // LWT retain
                                       "offline");          // LWT message
    }

    if (connected) {
        Serial.println("MQTT: Connected");
        reconnectInterval = 1000;  // Reset backoff on success

        // Publish availability (we're online)
        mqttClient.publish(topicAvailability, "online", true);

        // Subscribe to command topic
        if (mqttClient.subscribe(topicCommand)) {
            Serial.printf("MQTT: Subscribed to %s\n", topicCommand);
        }

        // Publish discovery and initial state
        publishDiscovery();
        mqttPublishState();
    } else {
        Serial.printf("MQTT: Connection failed, rc=%d\n", mqttClient.state());

        // Exponential backoff
        reconnectInterval *= 2;
        if (reconnectInterval > MAX_RECONNECT_INTERVAL) {
            reconnectInterval = MAX_RECONNECT_INTERVAL;
        }
        Serial.printf("MQTT: Next retry in %lu ms\n", reconnectInterval);
    }
}

bool mqttIsConnected() {
    return mqttClient.connected();
}

void mqttPublishState() {
    if (!mqttClient.connected()) {
        return;
    }

    Config& config = getConfig();

    JsonDocument doc;
    doc["state"] = config.powerOn ? "ON" : "OFF";
    // Report maxBrightness when light sensor is active (that's what MQTT controls),
    // otherwise report the direct brightness setting
    doc["brightness"] = config.useLightSensor ? config.maxBrightness : config.brightness;

    char payload[64];
    serializeJson(doc, payload, sizeof(payload));

    if (mqttClient.publish(topicState, payload, true)) {
        Serial.printf("MQTT: State published: %s\n", payload);
    } else {
        Serial.println("MQTT: State publish failed");
    }
}

void mqttReconnect() {
    // Force immediate reconnection attempt
    lastReconnectAttempt = 0;
    reconnectInterval = 1000;
}
