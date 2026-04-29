#include "ds18b20_sensor.h"

DS18B20Sensor::DS18B20Sensor()
  : _oneWire(DS18B20_PIN), _sensors(&_oneWire), _lastReadTime(0) {}

void DS18B20Sensor::begin() {
  _sensors.begin();
  Serial.println("[DS18B20] Capteur initialisé sur GPIO " + String(DS18B20_PIN));
}

float DS18B20Sensor::read() {
  _sensors.requestTemperatures();
  float temp = _sensors.getTempCByIndex(0);
  _lastReadTime = millis();

  if (temp == DEVICE_DISCONNECTED_C) {
    Serial.println("[DS18B20] Capteur non détecté !");
    return -127.0f;
  }

  Serial.printf("[DS18B20] Temp eau: %.2f °C\n", temp);
  return temp;
}

bool DS18B20Sensor::isReady() {
  return (millis() - _lastReadTime) >= 2000;
}