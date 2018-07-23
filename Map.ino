#include <ArduinoLowPower.h>
#include <WiFi101.h>
#include <FastLED.h>

const char *airports[] = {
  "KUKI", // order of LEDs, starting with 0
  "KSTS",
  "KO69",
  "KDVO",
  "KHAF",
  "KSFO",
  "KWVI",
  "KMRY",
  "KSNS",
  "KCVH",
  "KRHV",
  "KSJC",
  "KNUQ",
  "KSQL",
  "KHWD",
  "KOAK",
  "KAPC",
  "KVCB",
  "KSUU",
  "KCCR",
  "KLVK",
  "KC83",
  "KSCK",
  "KMOD",
  "KMER",
  "KMCE",
  "KMAE",
  "KFCH",
  "KFAT",
  "KNLC",
  "KHJO",
  "KPTV",
  "KVIS",
  "KO32",
  "KBIH",
  "KMMH",
  "KBAN",
  "KCPU",
  "KJAQ",
  "KMHR",
  "KSAC",
  "KEDU",
  "KSMF",
  "KMCC",
  "KPVF",
  "KTVL",
  "KMEV",
  "KCXP",
  "KNFL",
  "KLOL", // LED 49, end of first strand
  "KRNO",
  "KRTS",
  "KTRK",
  "KBLU",
  "KAUN",
  "KLHM",
  "KBAB",
  "KMYV",
  "KOVE",
  "KCIC",
  "KRBL"
};

#define NUM_AIRPORTS 61
#define WIND_THRESHOLD 24 // Maximum windspeed for green
#define LIGHTNING_INTERVAL 5000 // ms - how often should lightning strike; not precise because we sleep in-between
#define DO_LIGHTNING true // Lightning uses more power, but is cool.
#define DO_WINDS true // color LEDs for high winds

#define SERVER "www.aviationweather.gov"
#define BASE_URI "/adds/dataserver_current/httpparam?dataSource=metars&requestType=retrieve&format=xml&hoursBeforeNow=3&mostRecentForEachStation=true&stationString="

#define DEBUG false

char ssid[] = "TODO";        // your network SSID (name)
char pass[] = "TODO";    // your network password (use for WPA, or use as key for WEP)
boolean ledStatus = true; // used so leds only indicate connection status on first boot, or after failure
boolean lightningLeds[NUM_AIRPORTS] = {0};
bool isLightning = false;
unsigned int lightningLoops;

int status = WL_IDLE_STATUS;

#define READ_TIMEOUT 15 // Cancel query if no data received (seconds)
#define WIFI_TIMEOUT 60 // in seconds
#define RETRY_TIMEOUT 15000 // in ms
#define REQUEST_INTERVAL 900000 // in ms (15 min is 900000)

// Define the array of leds
CRGB leds[NUM_AIRPORTS];
#define DATA_PIN    0
#define LED_TYPE    WS2811
#define COLOR_ORDER RGB
#define BRIGHTNESS 96

WiFiClient client;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
//  while (!Serial) {
//    ; // wait for serial port to connect. Needed for native USB
//  }

  pinMode(LED_BUILTIN, OUTPUT); // give us control of the onboard LED
  digitalWrite(LED_BUILTIN, HIGH);

  // Initialize LEDs
  
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_AIRPORTS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  // setup power savings
  WiFi.maxLowPowerMode();
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH); // on if we're awake
  int c;
  const unsigned int lightningLoopThreshold = REQUEST_INTERVAL / LIGHTNING_INTERVAL;
  
  // Do some lightning
  if (DO_LIGHTNING && isLightning) {
    lightningLoops++;
    CRGB lightning[NUM_AIRPORTS];
    for (int i = 0; i < NUM_AIRPORTS; i++) {
      if (lightningLeds[i] == true) {
        lightning[i] = leds[i]; // temporarily store original color
        leds[i] = CRGB::White; // set to white briefly
      }
    }
    FastLED.show();
    delay(25);
    for (int i = 0; i < NUM_AIRPORTS; i++) {
      if (lightningLeds[i] == true) {
        leds[i] = lightning[i]; // restore original color
      }
    }
    FastLED.show();
    digitalWrite(LED_BUILTIN, LOW);
    LowPower.deepSleep(LIGHTNING_INTERVAL - 200); // pause during the interval
    delay(200);
  }

  if (!isLightning || !DO_LIGHTNING || lightningLoops >= lightningLoopThreshold) {
    Serial.print("Iterating at: ");
    Serial.println(millis());
    if (DEBUG) {
      fill_gradient_RGB(leds, NUM_AIRPORTS, CRGB::Red, CRGB::Blue); // Just let us know we're running
      FastLED.show();
   }
    
    if(WiFi.status() != WL_CONNECTED) {
      if (ledStatus) fill_solid(leds, NUM_AIRPORTS, CRGB::Orange); // indicate status with LEDs, but only on first run or error
      FastLED.show();
      Serial.print("WiFi connecting..");
      WiFi.begin(ssid, pass);
      // Wait up to 1 minute for connection...
      for(c=0; (c < WIFI_TIMEOUT) && (WiFi.status() != WL_CONNECTED); c++) {
        Serial.write('.');
        delay(1000);
      }
      if(c >= WIFI_TIMEOUT) { // If it didn't connect within WIFI_TIMEOUT
        Serial.println("Failed. Will retry...");
        fill_solid(leds, NUM_AIRPORTS, CRGB::Orange);
        FastLED.show();
        ledStatus = true;
        return;
      }
      Serial.println("OK!");
      printWiFiStatus();
      if (ledStatus) fill_solid(leds, NUM_AIRPORTS, CRGB::Purple); // indicate status with LEDs
      FastLED.show();
      ledStatus = false;
      Serial.println("Getting METARs ...");
      if (getMetars()) {
        WiFi.end();
        if (DO_LIGHTNING && isLightning) {
          lightningLoops = 0;
          Serial.print("There is lightning, so no long sleep at ");
          Serial.println(millis());
        } else {
          Serial.print("No lightning; Going into sleep at: ");
          Serial.println(millis());
          Serial.print(", sleep time: ");
          Serial.println(REQUEST_INTERVAL);
          digitalWrite(LED_BUILTIN, LOW);
          LowPower.deepSleep(REQUEST_INTERVAL - 10000); // for some reason this never works on first run
          delay(10000); // delay 10 seconds to give it time to go
        }
      } else {
        LowPower.idle(RETRY_TIMEOUT); // try again if unsuccessful
      }
    }
  }
}

bool getMetars() {
  uint32_t t;
  char c;
  struct ledConditions {
    int wind;
    String condition;
    String wxstring;
  } airportLeds[NUM_AIRPORTS] {0, "Unreported", "None"};
  boolean timedOut = false;
  boolean readingAirport = false;
  boolean readingCondition = false;
  boolean readingWind = false;
  boolean readingWxstring = false;

  int led = 99;
  String currentAirport = "";
  String currentCondition = "";
  String currentLine = "";
  String currentWind = "";
  String currentWxstring = "";
  String airportString = airports[0];
  for (int i = 1; i < (NUM_AIRPORTS); i++) {
    airportString = airportString + "," + airports[i];
  }

  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  if (client.connectSSL(SERVER, 443)) {
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

    Serial.print("Getting data");

    while (!client.connected()) {
      if((millis() - t) >= (READ_TIMEOUT * 1000)) {
        Serial.println("---Timeout---");
        timedOut = true;
        break;
      }
      Serial.print(".");
      delay(1000);
    }

    Serial.println();
    
    while(client.connected()) {
      if((c = client.read()) >= 0) {
        currentLine += c;
        if (c == '\n') currentLine = "";
        if (currentLine.endsWith("<station_id>")) { // start paying attention
          currentAirport = ""; // Reset everything when the airport changes
          readingAirport = true;
          currentCondition = "";
          currentWind = "";
        } else if (readingAirport) {
          if (!currentLine.endsWith("<")) {
            currentAirport += c;
          } else {
            readingAirport = false;
            for (int i = 0; i < NUM_AIRPORTS; i++) {
              if (String(airports[i]) == currentAirport) led = i;
            }
          }
        } else if (currentLine.endsWith("<wind_speed_kt>")) {
          readingWind = true;
        } else if (readingWind) {
          if (!currentLine.endsWith("<")) {
            currentWind += c;
          } else {
            readingWind = false;
            airportLeds[led].wind = currentWind.toInt();
          }
        } else if (currentLine.endsWith("<flight_category>")) {
          readingCondition = true;
        } else if (readingCondition) {
          if (!currentLine.endsWith("<")) {
            currentCondition += c;
          } else {
            readingCondition = false;
            airportLeds[led].condition = currentCondition;
          }
        } else if (currentLine.endsWith("<wx_string>")) {
          readingWxstring = true;
        } else if (readingWxstring) {
          if (!currentLine.endsWith("<")) {
            currentWxstring += c;
          } else {
            readingWxstring = false;
            airportLeds[led].wxstring = currentWxstring;
          }
        }      
      t = millis(); // Reset timeout clock
      } else if((millis() - t) >= (READ_TIMEOUT * 1000)) {
        Serial.println("---Timeout---");
        timedOut = true;
        fill_solid(leds, NUM_AIRPORTS, CRGB::Cyan); // indicate status with LEDs
        FastLED.show();
        ledStatus = true;
        break;
      }
    }

    if(!timedOut) { //Refresh LEDs if it was successful
      isLightning = false; // set to False so we can make it true if there is any
      for (int i = 0; i < NUM_AIRPORTS; i++) {
        Serial.print(airports[i]);
        Serial.print(": ");
        Serial.print(airportLeds[i].condition);
        Serial.print(" ");
        Serial.print(airportLeds[i].wind);
        Serial.print("kts LED ");
        Serial.print(i);
        Serial.print(" WX: ");
        Serial.println(airportLeds[i].wxstring);
        if (airportLeds[i].wxstring.indexOf("TS") != -1) {
          Serial.println("... found lightning!");
          lightningLeds[i] = true;
          isLightning = true;
        } else {
          lightningLeds[i] = false;
        }
        if (airportLeds[i].condition == "LIFR") leds[i] = CRGB::Magenta;
        else if (airportLeds[i].condition == "IFR") leds[i] = CRGB::Red;
        else if (airportLeds[i].condition == "MVFR") leds[i] = CRGB::Blue;
        else if (airportLeds[i].condition == "VFR") {
          if (airportLeds[i].wind > WIND_THRESHOLD && DO_WINDS) {
            leds[i] = CRGB::Yellow;
          } else {
            leds[i] = CRGB::Green;
          }
        }
      }
      Serial.println("Writing to LEDs.");
      FastLED.show();
    }
  }

  client.stop();
  if (timedOut) return false;
  else return true;
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
