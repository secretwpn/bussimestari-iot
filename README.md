# bussimestari-iot
Small Arduino-based "smart" screen displaying relevant information about public transport departures

Hardware
 * ESP8266 (In my case - LoLin NodeMcu v3)
 * I2C 128x32 OLED Screen

Software
 * [PlatformIO](https://platformio.org/)
 * [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
 * [Time](https://github.com/PaulStoffregen/Time)
 * [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library)
 * [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306)
 * SPI
 * Wire
 * ESP8266WiFi
 * ESP8266WiFiMulti
 * ESP8266HTTPClient

API
 * Timetable data - [Digitransit](https://digitransit.fi/en/developers/apis/)

Device connects to WiFi and show next bus departure time for every configured stop
