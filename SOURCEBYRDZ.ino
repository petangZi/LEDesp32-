#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoOTA.h>

// === CONFIG ===
const char* AP_SSID = "Redzskid-ESP32";
const char* AP_PASS = "hacktheplanet";
const int LED_PIN = 2; // GPIO2 → built-in + breadboard LED

// === LED STATE MACHINE ===
enum LedState { OFF, ON, BLINKING, SOS, TIMER_MODE };
volatile LedState ledState = OFF;
unsigned long lastLedUpdate = 0;
int timerDurationSec = 0;
unsigned long timerStartMs = 0;

// === WEB SERVER ===
WebServer server(80);

// --- LED CORE LOGIC ---
void setLedState(LedState state) {
  ledState = state;
  lastLedUpdate = millis();
  if (state == OFF || state == TIMER_MODE) digitalWrite(LED_PIN, LOW);
  else if (state == ON) digitalWrite(LED_PIN, HIGH);
}

void updateLed() {
  unsigned long now = millis();
  switch (ledState) {
    case BLINKING:
      if (now - lastLedUpdate > 500) {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        lastLedUpdate = now;
      }
      break;

    case SOS:
      {
        static int sosStep = 0;
        static unsigned long sosStartTime = now;
        const int sosPattern[9] = {200,200,200,600,600,600,200,200,200};
        if (sosStep < 9) {
          if (now - sosStartTime < (unsigned long)sosPattern[sosStep]) {
            digitalWrite(LED_PIN, (sosStep % 2 == 0));
          } else {
            sosStep++;
            sosStartTime = now;
            if (sosStep >= 9) setLedState(OFF);
          }
        }
      }
      break;

    case TIMER_MODE:
      if (now - timerStartMs >= (unsigned long)timerDurationSec * 1000) {
        setLedState(OFF);
      } else {
        digitalWrite(LED_PIN, HIGH);
      }
      break;
  }
}

// --- API HANDLERS ---
void handleNotFound() {
  server.send(404, "text/plain", "404 Not Found");
}

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>REDZSKID LED CONTROL</title>
  <style>
    :root { --bg: #0a0a0a; --fg: #00ff41; --panel: #121212; --border: #00ff41; }
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      background: var(--bg);
      color: var(--fg);
      font-family: 'Courier New', monospace;
      line-height: 1.6;
      padding: 20px;
    }
    header {
      text-align: center;
      margin-bottom: 25px;
      padding-bottom: 15px;
      border-bottom: 1px solid var(--border);
    }
    h1 {
      font-size: 1.8em;
      letter-spacing: 1px;
      text-shadow: 0 0 10px var(--fg);
    }
    .card {
      background: var(--panel);
      border: 1px solid var(--border);
      border-radius: 8px;
      padding: 20px;
      max-width: 500px;
      margin: 0 auto 20px;
    }
    .btn-group {
      display: flex;
      flex-wrap: wrap;
      gap: 10px;
      margin: 15px 0;
    }
    button {
      flex: 1;
      min-width: 100px;
      padding: 10px;
      background: transparent;
      color: var(--fg);
      border: 1px solid var(--border);
      border-radius: 4px;
      font-family: inherit;
      cursor: pointer;
      transition: all 0.2s;
    }
    button:hover {
      background: rgba(0, 255, 65, 0.1);
      box-shadow: 0 0 8px var(--fg);
    }
    input, select {
      width: 100%;
      padding: 10px;
      margin: 8px 0;
      background: #000;
      color: var(--fg);
      border: 1px solid var(--border);
      border-radius: 4px;
      font-family: inherit;
    }
    #status {
      margin-top: 20px;
      padding: 12px;
      background: rgba(0, 0, 0, 0.5);
      border-radius: 4px;
      font-size: 0.95em;
    }
    .pulse { animation: pulse 1.5s infinite; }
    @keyframes pulse {
      0% { box-shadow: 0 0 0 0 rgba(0, 255, 65, 0.4); }
      70% { box-shadow: 0 0 0 10px rgba(0, 255, 65, 0); }
      100% { box-shadow: 0 0 0 0 rgba(0, 255, 65, 0); }
    }
  </style>
</head>
<body>
  <header>
    <h1>💀 REDZSKID LED CONTROL [DEV MODE]</h1>
  </header>

  <main>
    <div class="card">
      <div class="btn-group">
        <button onclick="sendCmd('/led/on')">ON</button>
        <button onclick="sendCmd('/led/off')">OFF</button>
      </div>

      <label>Timer (1-60s)</label>
      <input type="number" id="duration" min="1" max="60" placeholder="Seconds">

      <button onclick="startTimer()" class="pulse">START TIMER</button>

      <label>Animation Mode</label>
      <select id="mode">
        <option value="static">Static</option>
        <option value="blink">Blink</option>
        <option value="sos">SOS</option>
      </select>
      <button onclick="setMode()">APPLY MODE</button>

      <div id="status">Initializing...</div>
    </div>
  </main>

  <script>
    function sendCmd(url) {
      fetch(url)
        .then(res => {
          if (res.ok) updateStatus();
          else alert('Command failed');
        })
        .catch(err => console.error(err));
    }

    function startTimer() {
      const dur = document.getElementById('duration').value;
      if (dur && dur >= 1 && dur <= 60) {
        sendCmd(`/led/timer?duration=${dur}`);
      } else {
        alert('Enter 1-60 seconds');
      }
    }

    function setMode() {
      const mode = document.getElementById('mode').value;
      sendCmd(`/led/mode?mode=${mode}`);
    }

    function updateStatus() {
      fetch('/led')
        .then(res => res.json())
        .then(data => {
          document.getElementById('status').innerText = 
            `LED STATE: ${data.state.toUpperCase()}`;
        })
        .catch(err => console.error(err));
    }

    // Auto-refresh status
    setInterval(updateStatus, 1000);
    // Initial load
    updateStatus();
  </script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

void handleLedOn() {
  setLedState(ON);
  server.send(200, "application/json", "{\"state\":\"ON\"}");
}

void handleLedOff() {
  setLedState(OFF);
  server.send(200, "application/json", "{\"state\":\"OFF\"}");
}

void handleLedStatus() {
  String stateStr;
  switch (ledState) {
    case OFF: stateStr = "off"; break;
    case ON: stateStr = "on"; break;
    case BLINKING: stateStr = "blinking"; break;
    case SOS: stateStr = "sos"; break;
    case TIMER_MODE: stateStr = "timer"; break;
    default: stateStr = "unknown";
  }
  server.send(200, "application/json", "{\"state\":\"" + stateStr + "\"}");
}

void handleSetMode() {
  String mode = server.arg("mode");
  if (mode == "blink") setLedState(BLINKING);
  else if (mode == "sos") setLedState(SOS);
  else if (mode == "static") setLedState(ON);
  else { server.send(400, "text/plain", "Invalid mode"); return; }
  server.send(200, "text/plain", "OK");
}

void handleTimer() {
  timerDurationSec = server.arg("duration").toInt();
  if (timerDurationSec >= 1 && timerDurationSec <= 60) {
    timerStartMs = millis();
    setLedState(TIMER_MODE);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Duration must be 1-60s");
  }
}

// === SETUP ===
void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.begin(115200);

  // Create AP (no need for home Wi-Fi)
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.println("\n✅ Hotspot created:");
  Serial.println("SSID: " + String(AP_SSID));
  Serial.println("IP: " + WiFi.softAPIP().toString());

  // Web server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/led/on", HTTP_GET, handleLedOn);
  server.on("/led/off", HTTP_GET, handleLedOff);
  server.on("/led", HTTP_GET, handleLedStatus);
  server.on("/led/mode", HTTP_GET, handleSetMode);
  server.on("/led/timer", HTTP_GET, handleTimer);
  server.onNotFound(handleNotFound);
  server.begin();

  // OTA for future updates
  ArduinoOTA.setHostname("redzskid-led-pro");
  ArduinoOTA.begin();

  Serial.println("🚀 Server & OTA ready!");
}

// === MAIN LOOP ===
void loop() {
  server.handleClient();
  ArduinoOTA.handle();
  updateLed(); // Non-blocking LED control
}
