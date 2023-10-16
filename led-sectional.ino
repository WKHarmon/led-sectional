#include <ESP8266WiFi.h>
#include <FastLED.h>
#include <vector>
using namespace std;

#define FASTLED_ESP8266_RAW_PIN_ORDER

#define NUM_AIRPORTS 80 // This is really the number of LEDs
#define WIND_THRESHOLD 25 // Maximum windspeed for green, otherwise the LED turns yellow
#define LOOP_INTERVAL 5000 // ms - interval between brightness updates and lightning strikes
#define DO_LIGHTNING true // Lightning uses more power, but is cool.
#define DO_WINDS true // color LEDs for high winds
#define REQUEST_INTERVAL 900000 // How often we update. In practice LOOP_INTERVAL is added. In ms (15 min is 900000)

#define USE_LIGHT_SENSOR false // Set USE_LIGHT_SENSOR to true if you're using any light sensor.
// Set LIGHT_SENSOR_TSL2561 to true if you're using a TSL2561 digital light sensor.
// Kits shipped after March 1, 2019 have a digital light sensor. Setting this to false assumes an analog light sensor.
#define LIGHT_SENSOR_TSL2561 false

const char ssid[] = "EDITME"; // your network SSID (name)
const char pass[] = "EDITME"; // your network password (use for WPA, or use as key for WEP)

// Define the array of leds
CRGB leds[NUM_AIRPORTS];
#define DATA_PIN    14 // Kits shipped after March 1, 2019 should use 14. Earlier kits us 5.
#define LED_TYPE    WS2811
#define COLOR_ORDER RGB
#define BRIGHTNESS 20 // 20-30 recommended. If using a light sensor, this is the initial brightness on boot.

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

std::vector<unsigned short int> lightningLeds;
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
  "KSMF", // 35
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

#define DEBUG false

#define READ_TIMEOUT 15 // Cancel query if no data received (seconds)
#define WIFI_TIMEOUT 60 // in seconds
#define RETRY_TIMEOUT 15000 // in ms

#define SERVER "aviationweather.gov"
#define BASE_URI "/cgi-bin/data/dataserver.php?dataSource=metars&requestType=retrieve&format=xml&hoursBeforeNow=3&mostRecentForEachStation=true&stationString="

boolean ledStatus = true; // used so leds only indicate connection status on first boot, or after failure
int loops = -1;

int status = WL_IDLE_STATUS;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(74880);
  //pinMode(D1, OUTPUT); //Declare Pin mode
  //while (!Serial) {
  //    ; // wait for serial port to connect. Needed for native USB
  //}

  pinMode(LED_BUILTIN, OUTPUT); // give us control of the onboard LED
  digitalWrite(LED_BUILTIN, LOW);

  #if USE_LIGHT_SENSOR
  #if LIGHT_SENSOR_TSL2561
  Wire.begin(D2, D1);
  if(!tsl.begin()) {
    /* There was a problem detecting the TSL2561 ... check your connections */
    Serial.println("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
  } else {
    tsl.enableAutoRange(true);
    tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);
  }
  #else
  pinMode(LIGHTSENSORPIN, INPUT);
  #endif
  #endif

  // Initialize LEDs
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_AIRPORTS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
}

#if USE_LIGHT_SENSOR
void adjustBrightness() {
  unsigned char brightness;
  float reading;

  #if LIGHT_SENSOR_TSL2561
  sensors_event_t event;
  tsl.getEvent(&event);
  reading = event.light;
  #else
  reading = analogRead(LIGHTSENSORPIN);
  #endif

  Serial.print("Light reading: ");
  Serial.print(reading);
  Serial.print(" raw, ");

  if (reading <= MIN_LIGHT) brightness = 0;
  else if (reading >= MAX_LIGHT) brightness = MAX_BRIGHTNESS;
  else {
    // Percentage in lux range * brightness range + min brightness
    float brightness_percent = (reading - MIN_LIGHT) / (MAX_LIGHT - MIN_LIGHT);
    brightness = brightness_percent * (MAX_BRIGHTNESS - MIN_BRIGHTNESS) + MIN_BRIGHTNESS;
  }

  Serial.print(brightness);
  Serial.println(" brightness");
  FastLED.setBrightness(brightness);
  FastLED.show();
}
#endif

void loop() {
  digitalWrite(LED_BUILTIN, LOW); // on if we're awake

  #if USE_LIGHT_SENSOR
  adjustBrightness();
  #endif

  int c;
  loops++;
  Serial.print("Loop: ");
  Serial.println(loops);
  unsigned int loopThreshold = 1;
  if (DO_LIGHTNING || USE_LIGHT_SENSOR) loopThreshold = REQUEST_INTERVAL / LOOP_INTERVAL;

  // Connect to WiFi. We always want a wifi connection for the ESP8266
  if (WiFi.status() != WL_CONNECTED) {
    if (ledStatus) fill_solid(leds, NUM_AIRPORTS, CRGB::Orange); // indicate status with LEDs, but only on first run or error
    FastLED.show();
    WiFi.mode(WIFI_STA);
    WiFi.hostname("LED Sectional " + WiFi.macAddress());
    //wifi_set_sleep_type(LIGHT_SLEEP_T); // use light sleep mode for all delays
    Serial.print("WiFi connecting..");
    WiFi.begin(ssid, pass);
    // Wait up to 1 minute for connection...
    for (c = 0; (c < WIFI_TIMEOUT) && (WiFi.status() != WL_CONNECTED); c++) {
      Serial.write('.');
      delay(1000);
    }
    if (c >= WIFI_TIMEOUT) { // If it didn't connect within WIFI_TIMEOUT
      Serial.println("Failed. Will retry...");
      fill_solid(leds, NUM_AIRPORTS, CRGB::Orange);
      FastLED.show();
      ledStatus = true;
      return;
    }
    Serial.println("OK!");
    if (ledStatus) fill_solid(leds, NUM_AIRPORTS, CRGB::Purple); // indicate status with LEDs
    FastLED.show();
    ledStatus = false;
  }

  // Do some lightning
  if (DO_LIGHTNING && lightningLeds.size() > 0) {
    std::vector<CRGB> lightning(lightningLeds.size());
    for (unsigned short int i = 0; i < lightningLeds.size(); ++i) {
      unsigned short int currentLed = lightningLeds[i];
      lightning[i] = leds[currentLed]; // temporarily store original color
      leds[currentLed] = CRGB::White; // set to white briefly
      Serial.print("Lightning on LED: ");
      Serial.println(currentLed);
    }
    delay(25); // extra delay seems necessary with light sensor
    FastLED.show();
    delay(25);
    for (unsigned short int i = 0; i < lightningLeds.size(); ++i) {
      unsigned short int currentLed = lightningLeds[i];
      leds[currentLed] = lightning[i]; // restore original color
    }
    FastLED.show();
  }

  if (loops >= loopThreshold || loops == 0) {
    loops = 0;
    if (DEBUG) {
      fill_gradient_RGB(leds, NUM_AIRPORTS, CRGB::Red, CRGB::Blue); // Just let us know we're running
      FastLED.show();
    }

    Serial.println("Getting METARs ...");
    if (getMetars()) {
      Serial.println("Refreshing LEDs.");
      FastLED.show();
      if ((DO_LIGHTNING && lightningLeds.size() > 0) || USE_LIGHT_SENSOR) {
        Serial.println("There is lightning or we're using a light sensor, so no long sleep.");
        digitalWrite(LED_BUILTIN, HIGH);
        delay(LOOP_INTERVAL); // pause during the interval
      } else {
        Serial.print("No lightning; Going into sleep for: ");
        Serial.println(REQUEST_INTERVAL);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(REQUEST_INTERVAL);
      }
    } else {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(RETRY_TIMEOUT); // try again if unsuccessful
    }
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(LOOP_INTERVAL); // pause during the interval
  }
}

bool getMetars(){
  lightningLeds.clear(); // clear out existing lightning LEDs since they're global
  fill_solid(leds, NUM_AIRPORTS, CRGB::Black); // Set everything to black just in case there is no report
  uint32_t t;
  char c;
  boolean readingAirport = false;
  boolean readingCondition = false;
  boolean readingWind = false;
  boolean readingGusts = false;
  boolean readingWxstring = false;

  std::vector<unsigned short int> led;
  String currentAirport = "";
  String currentCondition = "";
  String currentLine = "";
  String currentWind = "";
  String currentGusts = "";
  String currentWxstring = "";
  String airportString = "";
  bool firstAirport = true;
  for (int i = 0; i < NUM_AIRPORTS; i++) {
    if (airports[i] != "NULL" && airports[i] != "VFR" && airports[i] != "MVFR" && airports[i] != "WVFR" && airports[i] != "IFR" && airports[i] != "LIFR") {
      if (firstAirport) {
        firstAirport = false;
        airportString = airports[i];
      } else airportString = airportString + "," + airports[i];
    }
  }

  BearSSL::WiFiClientSecure client;
  client.setInsecure();
  Serial.println("\nStarting connection to server...");
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
    Serial.println("User-Agent: LED Map Client");
    Serial.println("Connection: close");
    Serial.println();
    // Make a HTTP request, and print it to console:
    client.print("GET ");
    client.print(BASE_URI);
    client.print(airportString);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(SERVER);
    client.println("User-Agent: LED Sectional Client");
    client.println("Connection: close");
    client.println();
    client.flush();
    t = millis(); // start time
    FastLED.clear();

    Serial.print("Getting data");

    while (!client.connected()) {
      if ((millis() - t) >= (READ_TIMEOUT * 1000)) {
        Serial.println("---Timeout---");
        client.stop();
        return false;
      }
      Serial.print(".");
      delay(1000);
    }

    Serial.println();

    while (client.connected()) {
      if ((c = client.read()) >= 0) {
        yield(); // Otherwise the WiFi stack can crash
        currentLine += c;
        if (c == '\n') currentLine = "";
        if (currentLine.endsWith("<station_id>")) { // start paying attention
          if (!led.empty()) { // we assume we are recording results at each change in airport
            for (vector<unsigned short int>::iterator it = led.begin(); it != led.end(); ++it) {
              doColor(currentAirport, *it, currentWind.toInt(), currentGusts.toInt(), currentCondition, currentWxstring);
            }
            led.clear();
          }
          currentAirport = ""; // Reset everything when the airport changes
          readingAirport = true;
          currentCondition = "";
          currentWind = "";
          currentGusts = "";
          currentWxstring = "";
        } else if (readingAirport) {
          if (!currentLine.endsWith("<")) {
            currentAirport += c;
          } else {
            readingAirport = false;
            for (unsigned short int i = 0; i < NUM_AIRPORTS; i++) {
              if (airports[i] == currentAirport) {
                led.push_back(i);
              }
            }
          }
        } else if (currentLine.endsWith("<wind_speed_kt>")) {
          readingWind = true;
        } else if (readingWind) {
          if (!currentLine.endsWith("<")) {
            currentWind += c;
          } else {
            readingWind = false;
          }
        } else if (currentLine.endsWith("<wind_gust_kt>")) {
          readingGusts = true;
        } else if (readingGusts) {
          if (!currentLine.endsWith("<")) {
            currentGusts += c;
          } else {
            readingGusts = false;
          }
        } else if (currentLine.endsWith("<flight_category>")) {
          readingCondition = true;
        } else if (readingCondition) {
          if (!currentLine.endsWith("<")) {
            currentCondition += c;
          } else {
            readingCondition = false;
          }
        } else if (currentLine.endsWith("<wx_string>")) {
          readingWxstring = true;
        } else if (readingWxstring) {
          if (!currentLine.endsWith("<")) {
            currentWxstring += c;
          } else {
            readingWxstring = false;
          }
        }
        t = millis(); // Reset timeout clock
      } else if ((millis() - t) >= (READ_TIMEOUT * 1000)) {
        Serial.println("---Timeout---");
        fill_solid(leds, NUM_AIRPORTS, CRGB::Cyan); // indicate status with LEDs
        FastLED.show();
        ledStatus = true;
        client.stop();
        return false;
      }
    }
  }
  // need to doColor this for the last airport
  for (vector<unsigned short int>::iterator it = led.begin(); it != led.end(); ++it) {
    doColor(currentAirport, *it, currentWind.toInt(), currentGusts.toInt(), currentCondition, currentWxstring);
  }
  led.clear();

  // Do the key LEDs now if they exist
  for (int i = 0; i < (NUM_AIRPORTS); i++) {
    // Use this opportunity to set colors for LEDs in our key then build the request string
    if (airports[i] == "VFR") leds[i] = CRGB::Green;
    else if (airports[i] == "WVFR") leds[i] = CRGB::Yellow;
    else if (airports[i] == "MVFR") leds[i] = CRGB::Blue;
    else if (airports[i] == "IFR") leds[i] = CRGB::Red;
    else if (airports[i] == "LIFR") leds[i] = CRGB::Magenta;
  }

  client.stop();
  return true;
}

void doColor(String identifier, unsigned short int led, int wind, int gusts, String condition, String wxstring) {
  CRGB color;
  Serial.print(identifier);
  Serial.print(": ");
  Serial.print(condition);
  Serial.print(" ");
  Serial.print(wind);
  Serial.print("G");
  Serial.print(gusts);
  Serial.print("kts LED ");
  Serial.print(led);
  Serial.print(" WX: ");
  Serial.println(wxstring);
  if (wxstring.indexOf("TS") != -1) {
    Serial.println("... found lightning!");
    lightningLeds.push_back(led);
  }
  if (condition == "LIFR" || identifier == "LIFR") color = CRGB::Magenta;
  else if (condition == "IFR") color = CRGB::Red;
  else if (condition == "MVFR") color = CRGB::Blue;
  else if (condition == "VFR") {
    if ((wind > WIND_THRESHOLD || gusts > WIND_THRESHOLD) && DO_WINDS) {
      color = CRGB::Yellow;
    } else {
      color = CRGB::Green;
    }
  } else color = CRGB::Black;

  leds[led] = color;
}
