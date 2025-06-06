#include "WarningTask.h"
#include "ConnectTask/ConnectTask.h"
#include "SensorTask/SensorTask.h"
#include "SendMessageTask/SendMessageTask.h"

// Task cảnh báo thời tiết vượt ngưỡng
void WarningTask(void *pvParameters) {
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);

    for (;;) {
        // Kiểm tra điều kiện cảnh báo
        bool tempHigh = latestTemperature > TEMP_THRESHOLD;
        bool humidityLow = latestHumidity < HUMIDITY_THRESHOLD;
        bool isRaining = latestRainStatus == LOW;

        if (tempHigh || humidityLow || isRaining) {
            // Bật Buzzer và nhấp nháy LED
            // digitalWrite(BUZZER_PIN, HIGH);
            digitalWrite(LED_PIN, HIGH);
            tone(BUZZER_PIN, 1000);
            vTaskDelay(500 / portTICK_PERIOD_MS);
            digitalWrite(LED_PIN, LOW);
            noTone(BUZZER_PIN);
            vTaskDelay(500 / portTICK_PERIOD_MS);
        } else {
            // Tắt cảnh báo
            digitalWrite(BUZZER_PIN, LOW);
            noTone(BUZZER_PIN);
            digitalWrite(LED_PIN, LOW);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}