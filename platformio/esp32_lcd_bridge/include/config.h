/*
 * config.h
 * ESP32 LCD Bridge — user configuration
 * by amiteshsslv | github.com/amiteshsslv/esp32-lcd-bridge
 *
 * Edit this file before flashing. Keep your credentials
 * out of version control — add config.h to .gitignore if
 * you're pushing to a public repo, or use config.example.h
 * as a template and commit that instead.
 */

#pragma once

// -----------------------------------------------------------
//  WiFi  —  fill these in!
// -----------------------------------------------------------
#define WIFI_SSID      "YOUR_WIFI_SSID"
#define WIFI_PASSWORD  "YOUR_WIFI_PASSWORD"

// -----------------------------------------------------------
//  I2C LCD
//  Common addresses: 0x27 (most modules) or 0x3F (some clones)
//  Run an I2C scanner sketch if you're not sure which yours uses.
// -----------------------------------------------------------
#define LCD_I2C_ADDR  0x27
#define LCD_COLS      16
#define LCD_ROWS      2

// I2C pins (ESP32 hardware defaults)
#define PIN_SDA  21
#define PIN_SCL  22

// -----------------------------------------------------------
//  Project identity
// -----------------------------------------------------------
#define PROJECT_AUTHOR   "amiteshsslv"
#define PROJECT_VERSION  "v1.0"
#define PROJECT_URL      "github.com/amiteshsslv/esp32-lcd-bridge"
