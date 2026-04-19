/*
 * webui.h
 * ESP32 LCD Bridge — embedded web UI
 * by amiteshsslv | github.com/amiteshsslv/esp32-lcd-bridge
 *
 * The entire front-end lives here in flash (PROGMEM) so the
 * ESP32 doesn't need SPIFFS or an SD card — just RAM + flash.
 * Classic green-on-black terminal aesthetic.
 */

#pragma once
#include <pgmspace.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>ESP32 LCD Bridge</title>
  <style>
    *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

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

    .card {
      width: 100%;
      max-width: 460px;
      border: 1.5px solid rgba(57,255,20,0.3);
      padding: 2rem 2rem 1.5rem;
    }

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
    }
    .cursor { display: inline-block; animation: blink 1s step-end infinite; }
    @keyframes blink { 50% { opacity: 0; } }

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
    .btn:hover  { background: rgba(57,255,20,0.08); }
    .btn:active { background: rgba(57,255,20,0.18); transform: scale(0.99); }
    .btn:disabled { opacity: 0.35; cursor: not-allowed; }

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

    .flash {
      display: none;
      font-size: 0.7rem;
      letter-spacing: 2px;
      color: rgba(57,255,20,0.7);
      margin-top: 8px;
      text-align: center;
    }

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
    var msgCount = 0;
    var startTime = Date.now();

    setInterval(function() {
      var s = Math.floor((Date.now() - startTime) / 1000);
      var h = String(Math.floor(s / 3600)).padStart(2, '0');
      var m = String(Math.floor((s % 3600) / 60)).padStart(2, '0');
      var sc = String(s % 60).padStart(2, '0');
      document.getElementById('uptime').textContent = 'UP: ' + h + ':' + m + ':' + sc;
    }, 1000);

    var msgInput = document.getElementById('msg');
    var charHint = document.getElementById('char-hint');
    msgInput.addEventListener('input', function() {
      charHint.textContent = msgInput.value.length + ' / 32';
    });
    msgInput.addEventListener('keydown', function(e) {
      if (e.key === 'Enter') transmit();
    });

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
          var preview = document.getElementById('lcd-preview');
          var line1 = text.substring(0, 16);
          var line2 = text.length > 16 ? text.substring(16, 32) : '';
          preview.innerHTML = line1 + (line2 ? '<br>' + line2 : '') + '<span class="cursor">_</span>';
          var flash = document.getElementById('flash');
          flash.style.display = 'block';
          setTimeout(function() { flash.style.display = 'none'; }, 2000);
          msgInput.value = '';
          charHint.textContent = '0 / 32';
        })
        .catch(function() {
          alert('Could not reach ESP32. Make sure you are on the same network.');
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
