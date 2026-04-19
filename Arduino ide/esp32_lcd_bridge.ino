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
 *   Libraries needed (Arduino Library Manager)
 *   -------------------------------------------
 *   - LiquidCrystal I2C  by Frank de Brabander
 *   - ESP32 board package (has WiFi + WebServer built in)
 *
 *   Usage
 *   -----
 *   1. Fill in your WiFi credentials below
 *   2. Flash to ESP32
 *   3. Watch the boot animation, note the IP shown on LCD
 *   4. Open that IP in any browser on the same network
 *   5. Type something and hit Transmit — enjoy!
 *
 *   License: MIT
 *   Project: https://github.com/amiteshsslv/esp32-lcd-bridge
 * ============================================================
 */

#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// -----------------------------------------------------------
//  WiFi credentials  —  change these before flashing!
// -----------------------------------------------------------
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// -----------------------------------------------------------
//  I2C LCD config
//  Most modules use address 0x27; some use 0x3F.
//  If you see nothing or garbled text, swap the address.
//  Run the I2C scanner sketch if you're unsure.
// -----------------------------------------------------------
#define LCD_I2C_ADDR  0x27
#define LCD_COLS      16
#define LCD_ROWS      2

// I2C pins — these are the ESP32 defaults
#define PIN_SDA  21
#define PIN_SCL  22

// -----------------------------------------------------------
//  Project identity  (shows up in Serial monitor + LCD)
// -----------------------------------------------------------
#define PROJECT_AUTHOR   "amiteshsslv"
#define PROJECT_VERSION  "v1.0"
#define PROJECT_URL      "github.com/amiteshsslv"

// -----------------------------------------------------------
//  Globals
// -----------------------------------------------------------
LiquidCrystal_I2C lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);
WebServer server(80);

unsigned long bootTime    = 0;
unsigned long msgCount    = 0;
String        lastMessage = "";


// -----------------------------------------------------------
//  Web UI  —  the whole page lives here in flash memory
//  so the ESP32 doesn't have to hit an SD card or SPIFFS.
//  Classic terminal aesthetic, fully responsive.
//  Made by amiteshsslv
// -----------------------------------------------------------
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>ESP32 LCD Bridge</title>
  <style>
    /* ---- reset ---- */
    *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

    /* ---- base ---- */
    body {
      background: #0c0c0c;
      color: #39ff14;
      font-family: 'Courier New', Courier, monospace;
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      padding: 1.5rem;
    }

    /* ---- card ---- */
    .card {
      width: 100%;
      max-width: 460px;
      border: 1.5px solid rgba(57,255,20,0.3);
      padding: 2rem 2rem 1.5rem;
    }

    /* ---- header ---- */
    .header {
      display: flex;
      align-items: center;
      justify-content: space-between;
      margin-bottom: 1.5rem;
      padding-bottom: 1rem;
      border-bottom: 1px solid rgba(57,255,20,0.15);
    }
    .header h1 {
      font-size: 0.95rem;
      letter-spacing: 3px;
      font-weight: normal;
    }
    .badge {
      font-size: 0.65rem;
      letter-spacing: 1px;
      border: 1px solid rgba(57,255,20,0.35);
      padding: 3px 8px;
      color: rgba(57,255,20,0.6);
    }

    /* ---- lcd sim ---- */
    .section-label {
      font-size: 0.65rem;
      letter-spacing: 2.5px;
      color: rgba(57,255,20,0.45);
      margin-bottom: 8px;
    }
    .lcd-display {
      background: #112211;
      border: 1.5px solid #1e3e1e;
      padding: 14px 18px;
      font-size: 1.25rem;
      letter-spacing: 4px;
      color: #6fff00;
      line-height: 2;
      min-height: 72px;
      margin-bottom: 1.5rem;
      word-break: break-all;
      position: relative;
    }
    .cursor {
      display: inline-block;
      animation: blink 1s step-end infinite;
    }
    @keyframes blink { 50% { opacity: 0; } }

    /* ---- input area ---- */
    .input-wrap {
      margin-bottom: 0.5rem;
    }
    .text-input {
      width: 100%;
      background: transparent;
      border: none;
      border-bottom: 1px solid rgba(57,255,20,0.3);
      color: #39ff14;
      font-family: inherit;
      font-size: 0.9rem;
      padding: 8px 0;
      outline: none;
      caret-color: #39ff14;
      letter-spacing: 1px;
    }
    .text-input::placeholder { color: rgba(57,255,20,0.25); }

    .char-hint {
      font-size: 0.65rem;
      color: rgba(57,255,20,0.35);
      text-align: right;
      margin-top: 5px;
    }

    /* ---- button ---- */
    .btn {
      display: block;
      width: 100%;
      margin-top: 1.2rem;
      background: transparent;
      border: 1.5px solid rgba(57,255,20,0.7);
      color: #39ff14;
      font-family: inherit;
      font-size: 0.8rem;
      letter-spacing: 3px;
      padding: 12px;
      cursor: pointer;
      transition: background 0.15s;
    }
    .btn:hover { background: rgba(57,255,20,0.08); }
    .btn:active { background: rgba(57,255,20,0.18); transform: scale(0.99); }
    .btn:disabled { opacity: 0.35; cursor: not-allowed; }

    /* ---- status bar ---- */
    .status-bar {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-top: 1.5rem;
      padding-top: 1rem;
      border-top: 1px solid rgba(57,255,20,0.1);
      font-size: 0.65rem;
      color: rgba(57,255,20,0.4);
      letter-spacing: 1px;
    }
    .dot {
      display: inline-block;
      width: 7px; height: 7px;
      border-radius: 50%;
      background: #39ff14;
      margin-right: 5px;
      animation: pulse 2.5s ease-in-out infinite;
    }
    @keyframes pulse { 0%,100% { opacity: 1; } 50% { opacity: 0.25; } }

    /* ---- feedback flash ---- */
    .flash {
      display: none;
      font-size: 0.7rem;
      letter-spacing: 2px;
      color: rgba(57,255,20,0.7);
      margin-top: 8px;
      text-align: center;
    }

    /* ---- footer ---- */
    .footer {
      margin-top: 2rem;
      font-size: 0.6rem;
      color: rgba(57,255,20,0.2);
      letter-spacing: 2px;
      text-align: center;
    }
    .footer a { color: rgba(57,255,20,0.35); text-decoration: none; }
    .footer a:hover { color: rgba(57,255,20,0.6); }
  </style>
</head>
<body>

  <div class="card">
    <div class="header">
      <h1>ESP32 :: LCD BRIDGE</h1>
      <span class="badge">16x2 I2C</span>
    </div>

    <p class="section-label">CURRENT DISPLAY</p>
    <div class="lcd-display" id="lcd-preview">
      READY...<span class="cursor">_</span>
    </div>

    <p class="section-label">SEND TO SCREEN</p>
    <div class="input-wrap">
      <input
        class="text-input"
        type="text"
        id="msg"
        maxlength="32"
        placeholder="type your message  (max 32 chars)..."
        autocomplete="off"
        spellcheck="false"
      />
      <p class="char-hint" id="char-hint">0 / 32</p>
    </div>

    <button class="btn" id="send-btn" onclick="transmit()">
      [&nbsp;&nbsp;TRANSMIT&nbsp;&nbsp;]
    </button>

    <p class="flash" id="flash">MESSAGE SENT OK</p>

    <div class="status-bar">
      <span><span class="dot"></span>CONNECTED</span>
      <span id="stats">MSGS: 0</span>
      <span id="uptime">UP: 00:00:00</span>
    </div>
  </div>

  <p class="footer">
    made with love by
    <a href="https://github.com/amiteshsslv" target="_blank">amiteshsslv</a>
    &nbsp;&bull;&nbsp; esp32-lcd-bridge
  </p>

  <script>
    // ---------- state ----------
    var msgCount = 0;
    var startTime = Date.now();

    // ---------- uptime ticker ----------
    setInterval(function() {
      var s = Math.floor((Date.now() - startTime) / 1000);
      var h = String(Math.floor(s / 3600)).padStart(2, '0');
      var m = String(Math.floor((s % 3600) / 60)).padStart(2, '0');
      var sc = String(s % 60).padStart(2, '0');
      document.getElementById('uptime').textContent = 'UP: ' + h + ':' + m + ':' + sc;
    }, 1000);

    // ---------- char counter ----------
    var msgInput = document.getElementById('msg');
    var charHint = document.getElementById('char-hint');
    msgInput.addEventListener('input', function() {
      charHint.textContent = msgInput.value.length + ' / 32';
    });

    // ---------- enter key ----------
    msgInput.addEventListener('keydown', function(e) {
      if (e.key === 'Enter') transmit();
    });

    // ---------- transmit ----------
    function transmit() {
      var text = msgInput.value.trim();
      if (!text) return;

      var btn = document.getElementById('send-btn');
      btn.disabled = true;
      btn.textContent = '[  SENDING...  ]';

      fetch('/send?msg=' + encodeURIComponent(text))
        .then(function(r) { return r.text(); })
        .then(function() {
          msgCount++;
          document.getElementById('stats').textContent = 'MSGS: ' + msgCount;

          // update the on-page LCD preview
          var preview = document.getElementById('lcd-preview');
          var line1 = text.substring(0, 16);
          var line2 = text.length > 16 ? text.substring(16, 32) : '';
          preview.innerHTML =
            line1 + (line2 ? '<br>' + line2 : '') + '<span class="cursor">_</span>';

          // flash feedback
          var flash = document.getElementById('flash');
          flash.style.display = 'block';
          setTimeout(function() { flash.style.display = 'none'; }, 2000);

          msgInput.value = '';
          charHint.textContent = '0 / 32';
        })
        .catch(function() {
          alert('Could not reach ESP32. Make sure you\'re on the same network.');
        })
        .finally(function() {
          btn.disabled = false;
          btn.textContent = '[  TRANSMIT  ]';
        });
    }
  </script>

</body>
</html>
)rawliteral";


// -----------------------------------------------------------
//  Boot animation
//  Fills a progress bar on line 2, then shows a splash screen
// -----------------------------------------------------------
void bootAnimation() {
  Serial.println(F("--------------------------------------------"));
  Serial.println(F("  ESP32 LCD Bridge  by amiteshsslv"));
  Serial.println(F("  github.com/amiteshsslv/esp32-lcd-bridge"));
  Serial.println(F("--------------------------------------------"));

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("  INITIALIZING  "));

  // fill a progress bar across row 2 using the full-block char (255)
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

  // connecting splash
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("  WiFi CONNECT  "));
}


// -----------------------------------------------------------
//  HTTP route  /  — serve the web UI
// -----------------------------------------------------------
void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}


// -----------------------------------------------------------
//  HTTP route  /send?msg=...  — write text to LCD
// -----------------------------------------------------------
void handleSend() {
  if (!server.hasArg("msg") || server.arg("msg").length() == 0) {
    server.send(400, "text/plain", "No message provided");
    return;
  }

  String msg = server.arg("msg");
  msg.trim();
  // cap at 32 chars (two full lines of 16)
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


// -----------------------------------------------------------
//  HTTP route  /status  — lightweight JSON for debugging
// -----------------------------------------------------------
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


// -----------------------------------------------------------
//  404 handler
// -----------------------------------------------------------
void handleNotFound() {
  server.send(404, "text/plain", "Not found | esp32-lcd-bridge by amiteshsslv");
}


// -----------------------------------------------------------
//  setup()
// -----------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(200);

  // init I2C + LCD
  Wire.begin(PIN_SDA, PIN_SCL);
  lcd.init();
  lcd.backlight();

  bootAnimation();

  // connect to WiFi, update LCD each dot
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

  // show IP on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("IP:"));
  lcd.setCursor(3, 0);
  lcd.print(WiFi.localIP());
  lcd.setCursor(0, 1);
  lcd.print(F("-> Open in browser"));
  delay(3500);

  // register routes
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
  // nothing else needed — everything is interrupt/callback driven
}
