#pragma once

// ── WiFi ─────────────────────────────────
#define WIFI_SSID     "Arthur"
#define WIFI_PASSWORD "mister95570"

// ── Serveur ───────────────────────────────
#define SERVER_URL "http://192.168.1.73:3000/data"
// ── DHT22 ────────────────────────────────
#define DHTPIN     15
#define DHTTYPE    DHT22
#define DHT_READ_INTERVAL_MS  2000

// ── OLED SSD1306 (I2C) ───────────────────
#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT  64
#define OLED_ADDR      0x3C
#define SDA_PIN        21
#define SCL_PIN        22

// ── pH PH-4502C ──────────────────────────
#define PH_PIN               34
#define PH_READ_INTERVAL_MS  2000
#define PH_SAMPLES           10
#define PH_VREF              3.3f
#define PH_OFFSET            1.00f

// ── Turbidité Keyestudio V1.0 ─────────────
#define TURB_PIN               35
#define TURB_READ_INTERVAL_MS  2000
#define TURB_SAMPLES           10
#define TURB_VREF              3.3f

#define DS18B20_PIN    4