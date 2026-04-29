#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "config.h"
#include "dht_sensor.h"

class ScreenOLED {
public:
  ScreenOLED();
  bool begin();
  void showSplash();
  void showData(const SensorData& data, float ph, float ntu, float waterTemp);
  void showError(const String& msg);
private:
  Adafruit_SSD1306 _display;
  void _drawHeader();
};