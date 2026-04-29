#pragma once
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"

class DS18B20Sensor {
public:
  DS18B20Sensor();
  void  begin();
  float read();
  bool  isReady();

private:
  OneWire        _oneWire;
  DallasTemperature _sensors;
  unsigned long  _lastReadTime;
};