# aqua-ai-monitor

<div align="center">

**Automated real-time monitoring platform for aquaculture basins**  
*IoT · Isolation Forest AI · MJPEG Video · Node.js · Flask · Chart.js*

![Status](https://img.shields.io/badge/status-operational-brightgreen?style=flat-square)
![Platform](https://img.shields.io/badge/platform-ESP32%20%2B%20PC-blue?style=flat-square)
![Lang](https://img.shields.io/badge/language-Python%20%7C%20C%2B%2B%20%7C%20JavaScript-informational?style=flat-square)
![License](https://img.shields.io/badge/license-MIT-green?style=flat-square)

</div>

---

## About the project

**aqua-ai-monitor** is a fully autonomous embedded platform for real-time aquaculture basin monitoring. It combines physicochemical sensor acquisition, live underwater video streaming, automatic CSV logging, and an on-the-fly trainable **Isolation Forest** anomaly detection model — all accessible from a web dashboard, including remote access via ngrok.

### The problem

Aquaculture basins require constant monitoring. A pH shift, rising turbidity or abnormal water conditions can cause significant losses within hours. Manual monitoring is expensive, discontinuous and reactive — the need is to be **proactive**.

### The solution

| Need | Solution |
|------|----------|
| Continuous acquisition | ESP32 reads 5 sensors every 2 s |
| Wi-Fi transmission | HTTP/JSON POST to Node.js server |
| Data history | Automatic CSV logging on PC |
| Anomaly detection | Isolation Forest (Python/Flask microservice) |
| Real-time visualization | Web dashboard with live charts (Chart.js) |
| Live video | ESP32-CAM + OV2640 → MJPEG stream |
| Remote access | ngrok HTTPS tunnel |

---

## Architecture

```
Physical sensors          OV2640 camera
      │                        │
      ▼ OneWire/ADC/I²C        ▼ MIPI CSI
  ESP32 Aqua              ESP32-CAM
      │                        │
      └──── Wi-Fi HTTP ─────────┘
                  │
                  ▼
        PC — Node.js (:3000) ◄──► Flask IA (:5001)
                  │                  │
                  │ ◄── /predict ─────┘
                  │
            data_aqua.csv  (auto-logged)
                  │
                  ▼
          Web Dashboard (:3000)
                  │
                  ▼ (optional)
          ngrok HTTPS tunnel
```

---

## Hardware components

| Component | Reference | Interface | Measured parameter |
|-----------|-----------|-----------|-------------------|
| Main microcontroller | ESP32-WROOM-32 | Wi-Fi, GPIO | Acquisition & transmission |
| Camera microcontroller | ESP32-CAM | Wi-Fi, MIPI CSI | Underwater video |
| Water temperature | DS18B20 | OneWire (GPIO 4) | Water temp (°C) |
| Air temp & humidity | DHT22 | Single-wire (GPIO 15) | Air temp (°C), humidity (%) |
| pH sensor | PH-4502C | ADC (GPIO 34) | pH (0–14) |
| Turbidity sensor | Keyestudio V1.0 | ADC (GPIO 35) | Turbidity (NTU) |
| Camera | OV2640 | MIPI CSI | JPEG VGA video |
| OLED display | SSD1306 128×64 | I²C (GPIO 21/22) | Local readout |

---

## Software stack

| Layer | Technology | Role |
|-------|-----------|------|
| Firmware | C++ / PlatformIO | Sensor reading, HTTP send |
| Server | Node.js / Express | Data relay, CSV logging, MJPEG proxy |
| AI service | Python / Flask | Isolation Forest training & inference |
| Dashboard | HTML/JS + Chart.js | Real-time charts, alerts, CSV upload |
| Remote access | ngrok | HTTPS tunnel |

---

## AI — Anomaly detection

The AI module uses an **Isolation Forest** model, trained on normal basin data collected by the system itself.

- **Unsupervised** — no labelled data required
- **Inputs** — pH and turbidity (NTU)
- **Double logic** — business rules thresholds + statistical IF score
- **Score** — normalized 0 (normal) → 1 (highly anomalous)
- **Trainable on the fly** — upload a CSV from the dashboard, click *Train*

```python
model = IsolationForest(n_estimators=200, contamination=0.05, random_state=42)
```

---

## Getting started

### Prerequisites

- Node.js ≥ 18
- Python ≥ 3.11
- PlatformIO
- ngrok (optional, for remote access)

### Launch the server & AI service

```bash
# Terminal 1 — Python AI microservice
cd esp32_cam_stream
python -m pip install flask flask-cors scikit-learn pandas numpy joblib
python ai_service.py          # → http://localhost:5001

# Terminal 2 — Node.js server
cd esp32_cam_stream
npm install multer form-data node-fetch@2
node server.js                # → http://localhost:3000

# Terminal 3 — Remote access (optional)
ngrok http 3000               # → https://xxxxxxxx.ngrok.io
```

### Flash the ESP32 firmware

```bash
cd esp32_code/esp32_aqua
# Edit src/config.h: WIFI_SSID, WIFI_PASSWORD, SERVER_URL
pio run --target upload
pio device monitor --baud 115200
```

### Train the AI model

1. Let the system run for ~30 min under normal conditions
2. Open `http://localhost:3000`
3. Click **Download CSV** to get `data_aqua.csv`
4. Drag & drop the CSV into the AI panel → click **Train**
5. Model is active in under 1 second

---

## Repository structure

```
aqua-ai-monitor/
├── esp32_code/
│   └── esp32_aqua/          # Sensor firmware (PlatformIO)
│       └── src/
│           ├── main.cpp
│           ├── config.h
│           ├── ph_sensor.cpp/h
│           ├── turbidity_sensor.cpp/h
│           ├── ds18b20_sensor.cpp/h
│           ├── dht_sensor.cpp/h
│           ├── screen_oled.cpp/h
│           └── http_sender.cpp/h
├── esp32_cam/               # Camera firmware (PlatformIO)
│   └── src/
│       ├── main.cpp
│       ├── camera.cpp/h
│       ├── streamer.cpp/h
│       └── config.h
├── esp32_cam_stream/        # Server + AI + Dashboard
│   ├── server.js            # Node.js/Express server
│   ├── ai_service.py        # Flask AI microservice
│   ├── requirements.txt
│   └── public/
│       └── index.html       # Web dashboard
├── SEBB_aqua/
│   └── carte_aqua/          # KiCad PCB project + Gerbers
├── rapport_aqua_ai_monitor.tex
└── rapport_aqua_ai_monitor.pdf
```


<div align="center">
  <sub>SPI Project · aqua-ai-monitor · IoT & AI for aquaculture</sub>
</div>