#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "config.h"
#include "dht_sensor.h"

#define SERVER_URL "http://192.168.1.73:3000/data"

class HttpSender {
public:
  HttpSender();
  void begin();
  void send(const SensorData& data, float ph, float ntu, float waterTemp);
  bool isReady();

private:
  unsigned long _lastSendTime;
};