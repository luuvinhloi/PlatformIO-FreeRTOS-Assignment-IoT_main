#ifndef WARNING_TASK_H
#define WARNING_TASK_H

#include <Arduino.h>

// Định nghĩa chân kết nối
#define LED_PIN 13       // Chân kết nối LED cảnh báo
#define BUZZER_PIN 26    // Chân kết nối buzzer

// Cảnh báo ngưỡng
constexpr float TEMP_THRESHOLD = 35.0;
constexpr float HUMIDITY_THRESHOLD = 30.0;

extern volatile float latestTemperature;
extern volatile float latestHumidity;
extern volatile int latestRainStatus;

// Khai báo WarningTask
void WarningTask(void *pvParameters);

#endif