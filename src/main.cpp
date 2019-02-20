#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// define SECRET_SSID, SECRET_PASS - WiFi details in secrets.h
#include <secrets.h>
// timezone offset in hours (+2 for Finland)
#define TIME_OFFSET 2 * 60 * 60
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET -1    // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
ESP8266WiFiMulti WiFiMulti;

// define stops to flip through
String DOTS[] = {"", ".", "..", "..."};
String STOP_IDS[] = {"tampere:3635", "tampere:3636"};
int STOPS_AMOUNT = 2;
int DOTS_AMOUNT = 4;
int stopIndex = 0;
int dotIndex = 0;

void printToDisplay(String text)
{
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println(text);
  display.display();
}

void setup()
{
  Serial.begin(9600);
  Serial.println();

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally Address 0x3C for 128x32
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println("SSD1306 Display module allocation failed");
    for (;;)
      ; // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  printToDisplay("Initializing...");
  delay(1000);

  printToDisplay("Let's go!");
  delay(1000);

  // Clear the buffer
  display.clearDisplay();

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(SECRET_SSID, SECRET_PASS);
}

String addLeadingZero(int digit)
{
  String result = String("") + digit;
  if (digit < 10)
  {
    result = String("0") + digit;
  }
  return result;
}

void loop()
{
  if ((WiFiMulti.run() == WL_CONNECTED))
  {
    HTTPClient http;
    http.begin("http://api.digitransit.fi/routing/v1/routers/finland/index/graphql"); //HTTP
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(
        String("{\"query\":\"{ stop(id: \\\"") +
        STOP_IDS[stopIndex] +
        String("\\\") { name stoptimesWithoutPatterns(numberOfDepartures: 1, omitNonPickups: true) {headsign realtimeDeparture serviceDay }}}\"}"));

    // flip between stops
    stopIndex++;
    if (stopIndex == STOPS_AMOUNT)
    {
      stopIndex = 0;
    }

    // httpCode will be negative on error
    if (httpCode > 0)
    {
      const String json = http.getString();
      const size_t capacity = JSON_ARRAY_SIZE(2) + 2 * JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + 2 * JSON_OBJECT_SIZE(3) + 590;
      DynamicJsonBuffer jsonBuffer(capacity);

      JsonObject &root = jsonBuffer.parseObject(json);
      // const char *stopName = root["data"]["stop"]["name"];

      // todo check if we get the data as expected
      JsonObject &stoptimes = root["data"]["stop"]["stoptimesWithoutPatterns"][0]; // array size <= numberOfDepartures
      const char *headsign = stoptimes["headsign"];
      long busTime = stoptimes["realtimeDeparture"];
      long serviceDay = stoptimes["serviceDay"];

      setTime(serviceDay);
      adjustTime(busTime + TIME_OFFSET); // + arrival + timezone offset
      String hh = addLeadingZero(hour());
      String mm = addLeadingZero(minute());
      String ss = addLeadingZero(second());
      String delimiter = String(":");
      String timeString = hh + delimiter + mm + delimiter + ss;
      // String text = stopName + String(" -> ") + headsign;
      String text = String("-> ") + headsign;

      Serial.println(text);
      Serial.println(timeString);

      display.clearDisplay();
      display.setTextSize(1); // Draw 2X-scale text
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      display.println(text);
      display.println("");
      display.setTextSize(2);
      display.println(timeString);
      display.display();
    }
    else
    {
      printToDisplay("Network error" + String(DOTS[dotIndex]));
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  } else {
    printToDisplay("No network" + String(DOTS[dotIndex]));
  }

  dotIndex++;
  if (dotIndex == DOTS_AMOUNT)
  {
    dotIndex = 0;
  }

  delay(10000);
}