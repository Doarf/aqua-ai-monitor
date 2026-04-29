#include "http_sender.h"

HttpSender::HttpSender() : _lastSendTime(0) {}

void HttpSender::begin() {
  Serial.println("[HTTP] Sender initialisé");
}

void HttpSender::send(const SensorData& data, float ph, float ntu, float waterTemp) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[HTTP] WiFi non connecté");
    return;
  }

  HTTPClient http;
  http.begin(SERVER_URL);
  http.addHeader("Content-Type", "application/json");

  String json = "{";
  json += "\"temperature\":"  + String(data.temperature, 1) + ",";
  json += "\"humidity\":"     + String(data.humidity, 1)    + ",";
  json += "\"ph\":"           + String(ph, 2)               + ",";
  json += "\"ntu\":"          + String(ntu, 1)              + ",";
  json += "\"waterTemp\":"    + String(waterTemp, 2);
  json += "}";

  int code = http.POST(json);
  Serial.printf("[HTTP] Envoi -> code %d\n", code);
  http.end();

  _lastSendTime = millis();
}

bool HttpSender::isReady() {
  return (millis() - _lastSendTime) >= 2000;
}