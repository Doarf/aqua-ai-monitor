#include "turbidity_sensor.h"

TurbiditySensor::TurbiditySensor() : _lastReadTime(0) {}

void TurbiditySensor::begin() {
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  Serial.println("[Turbidité] Capteur initialisé sur GPIO " + String(TURB_PIN));
}

float TurbiditySensor::read() {
  long sum = 0;
  for (int i = 0; i < TURB_SAMPLES; i++) {
    sum += analogRead(TURB_PIN);
    delay(10);
  }
  float raw     = sum / (float)TURB_SAMPLES;
  float voltage = (raw / 4095.0f) * TURB_VREF;
  float ntu     = _voltageToNTU(voltage);

  _lastReadTime = millis();
  Serial.printf("[Turbidité] Raw: %.0f | Voltage: %.3fV | NTU: %.1f\n", raw, voltage, ntu);
  return ntu;
}

bool TurbiditySensor::isReady() {
  return (millis() - _lastReadTime) >= TURB_READ_INTERVAL_MS;
}


float TurbiditySensor::_voltageToNTU(float voltage) {
  // 1.533V = eau claire = 0 NTU
  // 0.000V = eau très trouble = 3000 NTU
  float ntu = (1.533f - voltage) * (3000.0f / 1.533f);
  return constrain(ntu, 0.0f, 3000.0f);
}