/*
 * ============================================================
 *   ESP32 LCD Web Bridge
 *   by amiteshsslv  |  github.com/amiteshsslv
 * ============================================================
 *
 *   Send text from any browser on your local WiFi straight
 *   to a 16x2 I2C LCD wired up to an ESP32. Boots with a
 *   loading animation, shows its own IP, then sits and waits
 *   for your messages.
 *
 *   Wiring
 *   ------
 *   LCD VCC  ->  5V (or 3.3V if your module supports it)
 *   LCD GND  ->  GND
 *   LCD SDA  ->  GPIO 21
 *   LCD SCL  ->  GPIO 22
 *
 *   PlatformIO setup
 *   ----------------
 *   All dependencies are declared in platformio.ini.
 *   Just open the folder in VS Code + PlatformIO and hit Upload.
 *
 *   Usage
 *   -----
 *   1. Set your WiFi credentials in include/config.h
 *   2. Upload to ESP32
 *   3. Open Serial Monitor @ 115200 to see the IP
 *   4. Open that IP in any browser on the same network
 *   5. Type and hit Transmit — enjoy!
 *
 *   License: MIT
 *   Project: https://github.com/amiteshsslv/esp32-lcd-bridge
 * ============================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"
#include "webui.h"

// -----------------------------------------------------------
//  Globals
// -----------------------------------------------------------
LiquidCrystal_I2C lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);
WebServer server(80);

unsigned long bootTime    = 0;
unsigned long msgCount    = 0;
String        lastMessage = "";


// -----------------------------------------------------------
//  Boot animation
// -----------------------------------------------------------
void bootAnimation() {
  Serial.println(F("--------------------------------------------"));
  Serial.println(F("  ESP32 LCD Bridge  by amiteshsslv"));
  Serial.println(F("  github.com/amiteshsslv/esp32-lcd-bridge"));
  Serial.println(F("--------------------------------------------"));

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("  INITIALIZING  "));

  for (int i = 0; i < LCD_COLS; i++) {
    lcd.setCursor(i, 1);
    lcd.write(byte(255));
    delay(80);
  }

  delay(300);
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print(F("ESP32  BRIDGE"));
  lcd.setCursor(3, 1);
  lcd.print(F("by amiteshsslv"));
  delay(1800);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("  WiFi CONNECT  "));
}


// -----------------------------------------------------------
//  HTTP route handlers
// -----------------------------------------------------------
void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleSend() {
  if (!server.hasArg("msg") || server.arg("msg").length() == 0) {
    server.send(400, "text/plain", "No message provided");
    return;
  }

  String msg = server.arg("msg");
  msg.trim();
  if (msg.length() > 32) msg = msg.substring(0, 32);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(msg.substring(0, 16));

  if (msg.length() > 16) {
    lcd.setCursor(0, 1);
    lcd.print(msg.substring(16));
  }

  lastMessage = msg;
  msgCount++;

  Serial.print(F("[MSG #"));
  Serial.print(msgCount);
  Serial.print(F("] "));
  Serial.println(msg);

  server.send(200, "text/plain", "OK");
}

void handleStatus() {
  unsigned long upSec = (millis() - bootTime) / 1000;
  String json = "{";
  json += "\"author\":\"amiteshsslv\",";
  json += "\"version\":\"" + String(PROJECT_VERSION) + "\",";
  json += "\"uptime\":" + String(upSec) + ",";
  json += "\"msgCount\":" + String(msgCount) + ",";
  json += "\"lastMessage\":\"" + lastMessage + "\",";
  json += "\"ip\":\"" + WiFi.localIP().toString() + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

void handleNotFound() {
  server.send(404, "text/plain", "Not found | esp32-lcd-bridge by amiteshsslv");
}


// -----------------------------------------------------------
//  setup()
// -----------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(200);

  Wire.begin(PIN_SDA, PIN_SCL);
  lcd.init();
  lcd.backlight();

  bootAnimation();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int dots = 0;
  while (WiFi.status() != WL_CONNECTED) {
    String dotStr = "";
    for (int d = 0; d <= dots % 4; d++) dotStr += ".";
    lcd.setCursor(0, 1);
    lcd.print("Connecting" + dotStr + "    ");
    Serial.print(".");
    dots++;
    delay(400);
  }

  Serial.println();
  Serial.println(F("WiFi connected!"));
  Serial.print(F("IP address: "));
  Serial.println(WiFi.localIP());

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("IP:"));
  lcd.setCursor(3, 0);
  lcd.print(WiFi.localIP());
  lcd.setCursor(0, 1);
  lcd.print(F("-> Open in browser"));
  delay(3500);

  server.on("/",       handleRoot);
  server.on("/send",   handleSend);
  server.on("/status", handleStatus);
  server.onNotFound(handleNotFound);
  server.begin();

  bootTime = millis();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("   AWAITING...  "));
  lcd.setCursor(0, 1);
  lcd.print(F(" TRANSMISSION.. "));

  Serial.println(F("Web server started. Ready!"));
  Serial.println(F("Built by amiteshsslv | github.com/amiteshsslv/esp32-lcd-bridge"));
}


// -----------------------------------------------------------
//  loop()
// -----------------------------------------------------------
void loop() {
  server.handleClient();
}
