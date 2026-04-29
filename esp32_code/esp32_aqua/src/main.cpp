#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "dht_sensor.h"
#include "ph_sensor.h"
#include "turbidity_sensor.h"
#include "ds18b20_sensor.h"
#include "screen_oled.h"
#include "http_sender.h"

DHTSensor       dhtSensor;
PHSensor        phSensor;
TurbiditySensor turbSensor;
DS18B20Sensor   waterTempSensor;
ScreenOLED      screen;
HttpSender      sender;

SensorData lastData;
float      lastPh      = 7.0f;
float      lastNtu     = 0.0f;
float      lastWaterTemp = 0.0f;

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== SPI Aquaculture ===");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("[WiFi] Connexion");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[WiFi] Connecté ✓ IP : " + WiFi.localIP().toString());

  if (!screen.begin())
    Serial.println("[OLED] Écran KO !");
  screen.showSplash();

  dhtSensor.begin();
  phSensor.begin();
  turbSensor.begin();
  waterTempSensor.begin();
  sender.begin();
}

void loop() {
  if (dhtSensor.isReady())
    lastData = dhtSensor.read();

  if (phSensor.isReady())
    lastPh = phSensor.read();

  if (turbSensor.isReady())
    lastNtu = turbSensor.read();

  if (waterTempSensor.isReady())
    lastWaterTemp = waterTempSensor.read();

  if (lastData.valid) {
    screen.showData(lastData, lastPh, lastNtu, lastWaterTemp);
    if (sender.isReady())
      sender.send(lastData, lastPh, lastNtu, lastWaterTemp);
  } else {
    screen.showError("DHT22 KO");
  }
}