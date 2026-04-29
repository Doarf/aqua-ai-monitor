#include "screen_oled.h"
#include <Arduino.h>

ScreenOLED::ScreenOLED()
  : _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1) {}

bool ScreenOLED::begin() {
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!_display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("[OLED] Non trouvé !");
    return false;
  }
  _display.setTextColor(SSD1306_WHITE);
  return true;
}

void ScreenOLED::showSplash() {
  _display.clearDisplay();
  _display.setTextSize(2);
  _display.setCursor(0, 20);
  _display.print("SPI Aqua");
  _display.display();
  delay(1500);
}

void ScreenOLED::showData(const SensorData& data, float ph, float ntu, float waterTemp) {
  // ── Page 1 : Ambiance ─────────────────────
  _display.clearDisplay();
  _display.setTextSize(2);
  _display.setCursor(0, 0);
  _display.print("Amb.");
  _display.drawLine(0, 18, SCREEN_WIDTH, 18, SSD1306_WHITE);
  _display.setCursor(0, 22);
  _display.print("T:");
  _display.print(data.temperature, 1);
  _display.print("C");
  _display.setCursor(0, 44);
  _display.print("H:");
  _display.print(data.humidity, 1);
  _display.print("%");
  _display.display();
  delay(3000);

  // ── Page 2 : Eau ──────────────────────────
  _display.clearDisplay();
  _display.setTextSize(2);
  _display.setCursor(0, 0);
  _display.print("Eau");
  _display.drawLine(0, 18, SCREEN_WIDTH, 18, SSD1306_WHITE);
  _display.setCursor(0, 22);
  _display.print("T:");
  _display.print(waterTemp, 1);
  _display.print("C");
  _display.setCursor(0, 44);
  _display.print("pH:");
  _display.print(ph, 1);
  _display.display();
  delay(3000);

  // ── Page 3 : Turbidité ────────────────────
  _display.clearDisplay();
  _display.setTextSize(2);
  _display.setCursor(0, 0);
  _display.print("Turb.");
  _display.drawLine(0, 18, SCREEN_WIDTH, 18, SSD1306_WHITE);
  _display.setCursor(0, 22);
  _display.print("NTU:");
  _display.print(ntu, 0);
  _display.display();
  delay(3000);
}

void ScreenOLED::showError(const String& msg) {
  _display.clearDisplay();
  _display.setTextSize(2);
  _display.setCursor(0, 20);
  _display.print(msg);
  _display.display();
}

void ScreenOLED::_drawHeader() {}