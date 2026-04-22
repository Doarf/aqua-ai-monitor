#pragma once
#include <Arduino.h>
#include "config.h"

class TurbiditySensor {
public:
  TurbiditySensor();
  void  begin();
  float read();       // retourne NTU
  bool  isReady();

private:
  unsigned long _lastReadTime;
  float _voltageToNTU(float voltage);
};