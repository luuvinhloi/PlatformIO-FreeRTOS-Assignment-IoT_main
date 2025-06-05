#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <array>
#include <HttpClient.h>
#include <Update.h>

// Các hằng số cấu hình
constexpr uint32_t SERIAL_DEBUG_BAUD = 115200U;


// Hàm setup
void setup() {
  Serial.begin(SERIAL_DEBUG_BAUD);

}

// Hàm loop
void loop() {
  vTaskDelete(NULL);
}