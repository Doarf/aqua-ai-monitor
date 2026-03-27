# aqua-ai-monitor
<div align="center">
  
**Automated monitoring system for aquaculture basins**  
*IoT · Artificial Intelligence · Computer Vision · Real-time*

![Status](https://img.shields.io/badge/status-in%20development-yellow?style=flat-square)
![Platform](https://img.shields.io/badge/platform-ESP32%20%2B%20PC-blue?style=flat-square)
![Lang](https://img.shields.io/badge/language-Python%20%7C%20C%2B%2B-informational?style=flat-square)
![License](https://img.shields.io/badge/license-MIT-green?style=flat-square)

</div>

---

## About the project

**aqua-ai-monitor** is an embedded, AI-powered system designed to monitor aquaculture basins in real time. It combines physicochemical sensors, an underwater camera and a remote AI model (GPU) to automatically detect any anomaly — before it becomes critical.

### The problem

Aquaculture basins require constant monitoring. A pH shift, rising turbidity or abnormal fish behavior can cause significant losses within hours. Manual monitoring is expensive, discontinuous and reactive — where the need is to be **proactive**.

### The solution

A fully autonomous system, requiring no permanent human intervention, capable of:

-  **Collecting** sensor data continuously via an ESP32
-  **Transmitting** data over Wi-Fi to a central PC
-  **Analyzing** data and video streams using an AI model (GPU)
-  **Displaying** metrics and alerts through a graphical interface (GUI)
-  **Alerting** in real time (LED + GUI) whenever an anomaly is detected

---

### Data flow

```
Sensors → ESP32 → Wi-Fi → PC → AI Model (GPU) → Analysis → GUI + LED
```

### Functional layers

| Layer | Components | Role |
|-------|-----------|------|
| **Acquisition** | ESP32 + sensors + camera | Data collection & transmission |
| **AI Processing** | PC + GPU | Analysis & anomaly detection |
| **Visualization** | GUI + LED + OLED | Display, alerts & supervision |

---

## Hardware components

| Component | Role | Measured parameter |
|-----------|------|--------------------|
| **ESP32** | Main microcontroller — collects & sends data over Wi-Fi | Communication, coordination |
| **DS18B20** | Waterproof digital temperature sensor | Water temperature (°C) |
| **DHT22** | Ambient temperature & humidity sensor | Air temperature, humidity (%) |
| **pH module** | Water acidity measurement | pH (0–14) |
| **Turbidity sensor** | Water clarity measurement | Turbidity (NTU) |
| **Underwater camera** | Video capture for AI behavioral analysis | Fish behavior, basin state |
| **OLED 128×32 (IIC)** | Local embedded display | Real-time data readout |
| **LED 5V** | Visual alert indicator | System status |
| **5V 2A power supply** | Powers the entire system | — |

### Detectable anomalies

| Anomaly | Detection source | Alert triggered |
|---------|-----------------|-----------------|
| Fish stress | Camera + AI | GUI + LED |
| Overcrowding | Camera + AI | GUI + LED |
| Mortality | Camera + AI | Critical GUI alert |
| Abnormal pH | pH sensor | GUI + LED |
| Critical temperature | DS18B20 | GUI + LED |
| High turbidity | Turbidity sensor | GUI + LED |

---

## Repository structure

```
aqua-ai-monitor/
├── firmware/               # ESP32 code (C++ / Arduino)
│   ├── main.cpp
│   ├── sensors/
│   └── wifi_sender/
├── ai_model/               # AI model (Python)
│   ├── train/
│   ├── inference/
│   └── models/
├── gui/                    # Graphical interface (Python / C++)
│   └── dashboard/
├── docs/                   # Documentation & schematics
└── README.md
```

---

## 🛠️ Tech stack

| Side | Technology |
|------|-----------|
| Embedded firmware | C++ (Arduino / ESP-IDF) |
| AI processing | Python (PyTorch / OpenCV) |
| GUI | Python (PyQt5 / Tkinter) or C++ (Qt) |
| Communication | Wi-Fi — TCP Socket / MQTT |
| Acceleration | GPU (CUDA) |

---

<div align="center">
  <sub>SPI Project — aqua-ai-monitor · IoT & AI for aquaculture</sub>
</div>
