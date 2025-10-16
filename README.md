# 💀 ESP32 LED Control System [DEV MODE]

> Advanced LED control via Wi-Fi — no cloud, no BS, pure edge computing.  
> Built for hackers, by hackers. No karbit. No delay(). Just clean code.

---

## 🧠 Overview
Developer-grade IoT edge device using ESP32 to control LEDs via:
- RESTful API
- Responsive dark-terminal web UI
- Finite state machine (non-blocking)
- Modes: `static`, `blink`, `SOS`, `timer`

✅ Zero external dependencies  
✅ Runs entirely on ESP32  
✅ Hotspot mode — no home Wi-Fi needed

---

## 🛠️ Hardware Setup

**Required:**
- ESP32 DevKit V1 (30-pin, CP2102)
- LED
- 220Ω resistor
- Breadboard + jumper wires

**Wiring:**
ESP32 GPIO2 → LED (long leg)
ESP32 GND → 220Ω resistor → LED (short leg)

> ⚠️ Resistor is mandatory! Prevents damage.

---

## 📲 Usage

1. Upload `SOURCEBYRDZ.ino` to ESP32  
2. Connect phone to hotspot:  
   - **SSID**: `Redzskid-ESP32`  
   - **Password**: `hacktheplanet`  
3. Open browser → `http://192.168.4.1`  
4. Control LED in real-time:
   - ON / OFF
   - Timer (1–60s)
   - SOS emergency pattern
   - Animation modes

---

## 🧪 API Endpoints

| Method | Endpoint                     | Description               |
|--------|------------------------------|---------------------------|
| GET    | `/led`                       | Get LED state             |
| GET    | `/led/on`                    | Turn ON                   |
| GET    | `/led/off`                   | Turn OFF                  |
| GET    | `/led/mode?mode=X`           | Set mode (`static`, `blink`, `sos`) |
| GET    | `/led/timer?duration=N`      | Start timer (N = 1–60)    |

> 💡 Works with `curl`, Postman, or custom scripts!

---

## 🧬 Technical Highlights

- **State Machine**: No `delay()`, fully non-blocking  
- **Volatile State**: Prevents race conditions  
- **OTA-Ready**: Update over Wi-Fi  
- **Mobile-Optimized UI**: Works on any device  
- **Memory Efficient**: 15% dynamic memory used

---

## 🚫 Noob Zone (Avoid!)

- Don’t remove the 220Ω resistor  
- Don’t use GPIO 0, 1, 3, or 6–11 (reserved pins)  
- Don’t call this “just an LED project” — it’s an **IoT edge microservice** 😎

---

> **"Done."**  
> — Redzskid, White-Hat Hacker Junior 🔥
