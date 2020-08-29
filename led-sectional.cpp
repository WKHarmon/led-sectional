#include <ESP8266WiFi.h>
#include <FastLED.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <vector>
#include <map>
using namespace std;

#define WIND_THRESHOLD 25 // Maximum windspeed for green
#define LIGHTNING_INTERVAL 5000 // ms - how often should lightning strike; not precise because we sleep in-between
#define DO_LIGHTNING true // Do lightning flashes when there is nearby thunderstorms
#define DO_WINDS true // color LEDs for high winds
#define BRIGHTNESS 30 // LED brightness
#define USE_LIGHT_SENSOR false // Set USE_LIGHT_SENSOR to true if you're using any light sensor.
// Set LIGHT_SENSOR_TSL2561 to true if you're using a TSL2561 digital light sensor.
// Kits shipped after March 1, 2019 have a digital light sensor. Setting this to false assumes an analog light sensor.
#define LIGHT_SENSOR_TSL2561 true

#define REQUEST_INTERVAL 900000 // how often to update in ms (15 min is 900000)

std::vector<String> airports({
  "LIFR", // 1 order of LEDs, starting with 1 should be KKIC; use VFR, WVFR, MVFR, IFR, LIFR for key; NULL for no airport
  "IFR", // 2
  "MVFR", // 3
  "WVFR", // 4
  "VFR", // 5
  "NULL", // 6
  "NULL", // 7
  "KMRY", // 8
  "KSNS", // 9
  "KCVH", // 10
  "KWVI", // 11
  "KE16", // 12
  "KRHV", // 13
  "KSJC", // 14
  "KNUQ", // 15
  "KPAO", // 16
  "KSQL", // 17
  "KHAF", // 18
  "KSFO", // 19
  "KOAK", // 20
  "KHWD", // 21
  "KLVK", // 22
  "KC83", // 23
  "NULL", // 24
  "KCCR", // 25
  "NULL", // 26
  "KDVO", // 27
  "KO69", // 28
  "KSTS", // 29
  "NULL", // 30
  "KAPC", // 31
  "KSUU", // 32
  "KVCB", // 33
  "KEDU", // 34
  "KWMF", // 35
  "KSAC", // 36
  "KMHR", // 37
  "KMCC", // 38
  "KLHM", // 39
  "KMYV", // 40
  "KBAB", // 41
  "NULL", // 42
  "KOVE", // 43
  "NULL", // 44
  "KCIC", // 45
  "NULL", // 46
  "KRBL", // 47
  "NULL", // 48
  "NULL", // 49
  "NULL", // 50
  "KGOO", // 51
  "KBLU", // 52
  "NULL", // 53
  "KTRK", // 54
  "KRNO", // 55
  "KCXP", // 56
  "KMEV", // 57
  "KTVL", // 58
  "NULL", // 59
  "NULL", // 60
  "KAUN", // 61
  "KPVF", // 62
  "KJAQ", // 63
  "KCPU", // 64
  "KO22", // 65
  "NULL", // 66
  "NULL", // 67
  "KSCK", // 68
  "KTCY", // 69
  "NULL", // 70
  "KMOD", // 71
  "NULL", // 72
  "KMER", // 73
  "MKCE", // 74
  "NULL", // 75
  "KMAE", // 76
  "NULL", // 77
  "KFAT", // 78
  "NULL", // 79
  "KNLC" // 80
});

/* This section only applies if you have an ambient light sensor connected */
#if USE_LIGHT_SENSOR
/* The sketch will automatically scale the light between MIN_BRIGHTNESS and
MAX_BRIGHTNESS on the ambient light values between MIN_LIGHT and MAX_LIGHT
Set MIN_BRIGHTNESS and MAX_BRIGHTNESS to the same value to achieve a simple on/off effect. */
#define MIN_BRIGHTNESS 20 // Recommend values above 4 as colors don't show well below that
#define MAX_BRIGHTNESS 20 // Recommend values between 20 and 30

// Light values are a raw reading for analog and lux for digital
#define MIN_LIGHT 16 // Recommended default is 16 for analog and 2 for lux
#define MAX_LIGHT 30 // Recommended default is 30 to 40 for analog and 20 for lux

#if LIGHT_SENSOR_TSL2561
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <Wire.h>
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
#else
#define LIGHTSENSORPIN A0 // A0 is the only valid pin for an analog light sensor
#endif

#endif
/* ----------------------------------------------------------------------- */

// You usually shouldn't need to edit anything below this.

#define FASTLED_ESP8266_RAW_PIN_ORDER
int status = WL_IDLE_STATUS;
#define READ_TIMEOUT 15 // Cancel query if no data received (seconds)
#define WIFI_TIMEOUT 60 // in seconds
#define RETRY_TIMEOUT 15000 // in ms
#define SERVER "www.aviationweather.gov"
#define BASE_URI "/adds/dataserver_current/httpparam?dataSource=metars&requestType=retrieve&format=xml&hoursBeforeNow=3&mostRecentForEachStation=false&stationString="
#define DEBUG false
#define PROGRAM_PIN 12
#define CONFIG_SSID "LED-Sectional"
#define CONFIG_PASS "metar123"

// Define the array of leds
CRGB* leds;
#define DATA_PIN 14 // new kits should use 14; old ones use 5
#define LED_TYPE WS2811
#define COLOR_ORDER RGB

class AirportState {
 public:
 AirportState() {
  condition_=NONE;
  wind_=0;
  gusts_=0;
  lightning_=false;
 }
  String observation_time_;
  String iaco_;
  void setCategory(const String& cat) {
    if (cat == "VFR") {
      condition_ = VFR;
    }
    if (cat == "MVFR") {
      condition_ = MVFR;
    }
    if (cat == "IFR") {
      condition_ = IFR;
    }
    if (cat == "LIFR") {
      condition_ = LIFR;
    }
  }
  void setWind(const String& w) {
    wind_ = w.toInt();
  }
  void setGust(const String& w) {
    gusts_ = w.toInt();
  }

  bool complete() {
    if (condition_ == NONE) {
      Serial.println("missing condition");
    }
    if (time_.length() == 0) {
      Serial.println("time");
      
    }
    if (iaco_.length() == 0) {
      Serial.println("iaco");
    }
    return condition_ != NONE && time_.length() != 0 && iaco_.length() != 0;
  }

  CRGB getColor() {
    if (lightning_ && (millis() % LIGHTNING_INTERVAL < 50)) {
      return CRGB::White;
    }
    CRGB color;
    if (condition_ ==LIFR) color = CRGB::Magenta;
    else if (condition_ == IFR) color = CRGB::Red;
    else if (condition_ == MVFR) color = CRGB::Blue;
    else if (condition_ == VFR) {
      if ((wind_ > WIND_THRESHOLD || gusts_ > WIND_THRESHOLD) && DO_WINDS) {
        color = CRGB::Yellow;
      } else {
        color = CRGB::Green;
      }
    } else color = CRGB::Black;
  
    return color;
  }

  String condition() {
    switch (condition_) {
      case VFR: return "VFR";
      case MVFR: return "MVFR";
      case IFR: return "IFR";
      case LIFR: return "LIFR";
    }
    return "Unknown";
  }

  String wind() {
    return String(wind_, DEC);
  }

  String gust() {
    return String(gusts_, DEC);
  }
  
  enum {
    NONE,
    VFR,
    MVFR,
    IFR,
    LIFR
  } condition_;
  int wind_;
  int gusts_;
  String time_;
  bool lightning_;
};

class SSLBuffer {
  public:
  SSLBuffer(BearSSL::WiFiClientSecure* client) {
    client_ = client;
    i_=0;
    total_read_=0;
  }
  int connected() {
    return client_->connected();
  }
  char read() {
    yield();
    if (i_ >= total_read_) {
      i_ = 0;
    }
    if (i_ == 0) {
      total_read_ = client_->read(buffer_, 512);
    }
    return buffer_[i_++];
  }
  BearSSL::WiFiClientSecure* client_;
  int i_;
  int total_read_;
  uint8_t buffer_[512];
};

std::map<String, AirportState> wx;

String airportString;

bool readNextFmt(SSLBuffer* client, const char* f, String* result) {
  const char* startover = f;
  bool copying = false;
  char c;
  for (; *f && client->connected() && (c=client->read());) {
    if (*f == 'X') {
      copying = true;
      f++;
    }
    if (c == *f) {
      f++;
      copying = false;
    } else if (copying) {
      *result += c;
    } else {
      // start over
      f=startover;
      copying=false;
      *result=String();
    }
  }
  if (*f == '\0') {
    return true;
  }
  
  return false;
}

AirportState getAirportState(SSLBuffer* client) {
  AirportState ret;
  String token;
  bool found_station= false;
  for (; readNextFmt(client, "<X>", &token);) {
    if (token == "/METAR") {
      break;
    }
    if (token == "station_id") {
      readNextFmt(client, "X<", &ret.iaco_);
      if (found_station) {
        Serial.println("Found station twice " + ret.iaco_); 
      }
      found_station = true;
    }
    if (token == "flight_category") {
      String category;
      readNextFmt(client, "X<", &category);
      ret.setCategory(category);
    }
    if (token == "observation_time") {
      readNextFmt(client, "X<", &ret.time_);
    }
    if (token == "wind_speed_kt") {
      String w;
      readNextFmt(client, "X<", &w);
      ret.setWind(w);
    }
    if (token == "wind_gust_kt") {
      String w;
      readNextFmt(client, "X<", &w);
      ret.setGust(w);
    }
    if (token == "wx_string") {
      String w;
      readNextFmt(client, "X<", &w);
      int ts_idx = w.indexOf("TS");
      if (ts_idx != -1) {
        ret.lightning_ = true;
      }
    }
  }
  return ret;
}

#if USE_LIGHT_SENSOR
void adjustBrightness() {
  byte brightness;
  float reading;

  #if LIGHT_SENSOR_TSL2561
  sensors_event_t event;
  tsl.getEvent(&event);
  reading = event.light;
  #else
  reading = analogRead(LIGHTSENSORPIN);
  #endif

  if (reading <= MIN_LIGHT) brightness = 0;
  else if (reading >= MAX_LIGHT) brightness = MAX_BRIGHTNESS;
  else {
    // Percentage in lux range * brightness range + min brightness
    float brightness_percent = (reading - MIN_LIGHT) / (MAX_LIGHT - MIN_LIGHT);
    brightness = brightness_percent * (MAX_BRIGHTNESS - MIN_BRIGHTNESS) + MIN_BRIGHTNESS;
  }

  FastLED.setBrightness(brightness);
}
#endif

#define LINE_BUFFER_SIZE 300
bool getMetars() {
  digitalWrite(LED_BUILTIN, LOW);
  //lightningLeds.clear(); // clear out existing lightning LEDs since they're global
  //fill_solid(leds, airports.size(), CRGB::Black); // Set everything to black just in case there is no report
  uint32_t t;
  
  BearSSL::WiFiClientSecure client;
  client.setInsecure();
  //Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  if (!client.connect(SERVER, 443)) {
    Serial.println("Connection failed!");
    client.stop();
    return false;
  } else {
    Serial.println("Connected ...");
    Serial.print("GET ");
    Serial.print(BASE_URI);
    Serial.print(airportString);
    Serial.println(" HTTP/1.1");
    Serial.print("Host: ");
    Serial.println(SERVER);
    Serial.println("Connection: close");
    Serial.println();
    // Make a HTTP request, and print it to console:
    client.print("GET ");
    client.print(BASE_URI);
    client.print(airportString);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(SERVER);
    client.println("Connection: close");
    client.println();
    client.flush();
    t = millis(); // start time
    FastLED.clear();

    //Serial.println("Getting data");

    while (!client.connected()) {
      if ((millis() - t) >= (READ_TIMEOUT * 1000)) {
        Serial.println("---Timeout---");
        client.stop();
        return false;
      }
      Serial.print(".");
      delay(1000);
    }

    Serial.println("parsing data");
    SSLBuffer client_wrapper(&client);

    wx.clear();
    wx["VFR"].condition_=AirportState::VFR;
    wx["IFR"].condition_=AirportState::IFR;
    wx["LIFR"].condition_=AirportState::LIFR;
    wx["MVFR"].condition_=AirportState::MVFR;
    wx["WVFR"].condition_=AirportState::VFR;
    wx["WVFR"].wind_=WIND_THRESHOLD + 1;
    while (client.connected()) {
      String tmp;
      if (readNextFmt(&client_wrapper, "<METAR>", &tmp)) {
          AirportState state = getAirportState(&client_wrapper);
          if (state.complete() && wx[state.iaco_].time_ < state.time_) {
            wx[state.iaco_]=state;
          }
        }
    }
    Serial.println("done");
    digitalWrite(LED_BUILTIN, HIGH);
  }

  // Dump results
  for (unsigned int i = 0; i < airports.size(); ++i) {
    AirportState& as = wx[airports[i]];
    Serial.println(airports[i] + " " + as.iaco_ + " " + as.time_ + " " + as.condition() + " " + as.wind() + " " + as.gust());
  }
  client.stop();
  return true;
}

void configModeCallback (WiFiManager *myWiFiManager) {
  fill_solid(leds, airports.size(), CRGB::Red); // Red means we're in config mode
  FastLED.show();
}

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(74880);

  pinMode(PROGRAM_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT); // give us control of the onboard LED
  digitalWrite(LED_BUILTIN, LOW);

  leds=new CRGB[airports.size()];

  // Initialize LEDs
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, airports.size());
  FastLED.setBrightness(BRIGHTNESS);

  bool firstAirport = true;
  for (unsigned int i = 0; i < airports.size(); i++) {
    if (airports[i] != "NULL" && airports[i] != "VFR" && airports[i] != "MVFR" && airports[i] != "WVFR" && airports[i] != "IFR" && airports[i] != "LIFR") {
      if (firstAirport) {
        firstAirport = false;
        airportString = airports[i];
      } else {
        airportString = airportString + "," + airports[i];
      }
    }
  }

  fill_solid(leds, airports.size(), CRGB::Orange); // indicate status with LEDs
  FastLED.show();

  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.autoConnect(CONFIG_SSID, CONFIG_PASS);

  fill_solid(leds, airports.size(), CRGB::Purple); // indicate status with LEDs
  FastLED.show();
}

unsigned long last_update = 0;
void loop() {
  int c;

  #if USE_LIGHT_SENSOR
  adjustBrightness();
  #endif

  if ( digitalRead(PROGRAM_PIN) == LOW ) {
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    ESP.restart();
  }

  if (last_update == 0 || (millis() > (last_update + REQUEST_INTERVAL))) {
    getMetars();
    last_update = millis();
  }
  for (unsigned int i = 0; i < airports.size(); ++i) {
    leds[i]=wx[airports[i]].getColor();
  }
  FastLED.show();
}