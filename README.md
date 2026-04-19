# ESP32 LCD Bridge

> Send text from any browser on your local network straight to a 16×2 I2C LCD — hosted entirely on the ESP32 itself, no internet required.

Made by **[amiteshsslv](https://github.com/amiteshsslv)** as a fun weekend hobby project. It's silly, it's simple, and it works great.

---

## What it does

- ESP32 hosts a tiny web server on your local WiFi
- Open the IP in any browser (phone, laptop, whatever)
- Type a message, hit **Transmit**
- It shows up on the LCD instantly
- On boot there's a little loading bar animation, then it shows you its own IP address so you know where to go

The web UI has a classic green-on-black terminal aesthetic because why not.

---

## Hardware you need

| Part | Notes |
|------|-------|
| ESP32 dev board | Any standard 30-pin or 38-pin board works |
| 16×2 I2C LCD module | The ones with a PCF8574 backpack soldered on |
| Jumper wires | 4 wires total |
| USB cable | For flashing |

### Wiring

```
LCD   -->   ESP32
VCC         5V  (some modules work on 3.3V too)
GND         GND
SDA         GPIO 21
SCL         GPIO 22
```

That's it. Four wires.

---

## Getting started

### Option A — Arduino IDE

**Step 1 — Install the ESP32 board package**

1. Open Arduino IDE → `File` → `Preferences`
2. Paste this into *Additional Boards Manager URLs*:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Go to `Tools` → `Board` → `Boards Manager`, search **esp32**, install the package by Espressif Systems

**Step 2 — Install the LCD library**

1. Go to `Sketch` → `Include Library` → `Manage Libraries`
2. Search for **LiquidCrystal I2C** by Frank de Brabander
3. Click Install

**Step 3 — Open the sketch**

1. Open `arduino/esp32_lcd_bridge/esp32_lcd_bridge.ino`
2. Near the top of the file, fill in your WiFi credentials:
   ```cpp
   const char* WIFI_SSID     = "your_network_name";
   const char* WIFI_PASSWORD = "your_password";
   ```

**Step 4 — Flash**

1. Connect your ESP32 via USB
2. `Tools` → `Board` → `ESP32 Dev Module`
3. Select the correct COM port under `Tools` → `Port`
4. Click Upload (the arrow button)
5. Hold the BOOT button on the ESP32 while uploading if it doesn't start automatically

**Step 5 — Use it**

1. Open `Tools` → `Serial Monitor`, set baud to **115200**
2. Watch the boot animation on the LCD
3. The LCD will show the IP address — open it in your browser
4. Type something and hit Transmit!

---

### Option B — PlatformIO (VS Code)

This is the cleaner way to do it, especially if you do embedded stuff regularly.

**Step 1 — Install PlatformIO**

1. Install [VS Code](https://code.visualstudio.com/)
2. Open Extensions (`Ctrl+Shift+X`), search **PlatformIO IDE**, install it
3. Restart VS Code

**Step 2 — Open the project**

1. In VS Code: `File` → `Open Folder`
2. Navigate to and open `platformio/esp32_lcd_bridge/`
3. PlatformIO will automatically detect the `platformio.ini` and download all dependencies (including the LCD library) — this takes a minute the first time

**Step 3 — Set your WiFi credentials**

Open `include/config.h` and fill in:
```cpp
#define WIFI_SSID      "your_network_name"
#define WIFI_PASSWORD  "your_password"
```

> **Tip:** If you're pushing to GitHub, don't commit your real credentials. Rename `config.h` to `config.example.h` with placeholder values, add `config.h` to `.gitignore`, and keep the real one only locally.

**Step 4 — Upload**

1. Click the **→ Upload** button in the PlatformIO toolbar at the bottom of VS Code
   - Or press `Ctrl+Alt+U`
   - Or run `pio run --target upload` in the terminal
2. Hold the BOOT button on your ESP32 if upload doesn't start

**Step 5 — Monitor**

Click the plug icon (Serial Monitor) in the PlatformIO toolbar. Set baud to 115200. You'll see the IP address printed there as well as on the LCD.

---

## Project structure

```
esp32-lcd-bridge/
├── arduino/
│   └── esp32_lcd_bridge/
│       └── esp32_lcd_bridge.ino     # everything in one file for Arduino IDE
│
├── platformio/
│   └── esp32_lcd_bridge/
│       ├── platformio.ini           # board + dependency config
│       ├── src/
│       │   └── main.cpp             # main application logic
│       └── include/
│           ├── config.h             # WiFi creds + pin/LCD settings
│           └── webui.h              # HTML page stored in flash (PROGMEM)
│
├── .gitignore
└── README.md
```

---

## Troubleshooting

**LCD shows nothing / random blocks**
- Try changing `LCD_I2C_ADDR` from `0x27` to `0x3F` — those are the two common addresses for I2C LCD backpacks
- Run an I2C scanner sketch to find the exact address (search "Arduino I2C scanner" — it's a standard 20-line sketch)
- Check your wiring, especially SDA and SCL aren't swapped

**Can't connect to the web UI**
- Make sure your phone/laptop is on the **same WiFi network** as the ESP32
- Check the IP shown on the LCD and in the Serial Monitor
- Try opening `http://<ip-address>` (not https)

**Upload fails**
- Hold the BOOT button on the ESP32 while clicking Upload, release once it says "Connecting..."
- Some boards need a capacitor between EN and GND — if you bought a cheap clone this might be the issue

**Message shows garbled characters**
- The LCD is 16×2 — that's 16 characters per line, 2 lines max (32 chars total)
- The code truncates automatically, but some special characters (emoji, extended Unicode) don't exist in the LCD's character set

---

## Customisation ideas

Since you've already got a web server running on the ESP32, here are some directions you could take this further:

- Add a `/clear` endpoint to wipe the screen remotely
- Store the last N messages and let you scroll through them via the web UI
- Add a message queue so texts appear one after another with a delay
- Hook it up to MQTT so other devices can push to the screen
- Add a marquee/scroll mode for messages longer than 32 chars
- Password-protect the UI so random people on your network can't mess with it

---

## License

MIT — do whatever you want with it, just give a shoutout if it helps you.

---

*Built with too much free time and a spare ESP32 lying around.*
*— [amiteshsslv](https://github.com/amiteshsslv)*
