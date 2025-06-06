#include "SensorTask.h"
#include "ConnectTask/ConnectTask.h"
#include "WarningTask/WarningTask.h"
#include "SendMessageTask/SendMessageTask.h"
#include "LCDTask/LCDTask.h"

// Lưu thời điểm gửi dữ liệu và kiểm tra kết nối
uint32_t previousDataSend;

// Khởi tạo các đối tượng
DHT dht(DHT_PIN, DHT11);

// Biến chia sẻ giữa SensorTask và WarningTask
volatile float latestTemperature = 0.0;
volatile float latestHumidity = 0.0;
volatile int latestRainStatus = HIGH;

// Task thu thập dữ liệu cảm biến và gửi telemetry
void SensorTask(void *pvParameters) {
    for (;;) {
        if (millis() - previousDataSend > telemetrySendInterval) {
            previousDataSend = millis();

            // Đọc dữ liệu từ cảm biến DHT11
            float temperature = dht.readTemperature();
            float humidity = dht.readHumidity();

            if (isnan(temperature) || isnan(humidity)) {
                vTaskDelay(2000 / portTICK_PERIOD_MS);
                temperature = dht.readTemperature();
                humidity = dht.readHumidity();
            }

            // Đọc dữ liệu cảm biến mưa
            // int rainStatus = digitalRead(RAIN_PIN);
            int rainStatus = 1;
            const char* rainText = (rainStatus == LOW) ? "Co" : "Khong";

            // Cập nhật giá trị cảm biến vào các biến chia sẻ
            latestTemperature = temperature;
            latestHumidity = humidity;
            latestRainStatus = rainStatus;

            checkAndSendAlerts(temperature, humidity, rainStatus);

            // In ra Serial
            Serial.printf("Temperature: %.2f°C, Humidity: %.2f%%, Rain: %s\n",
                            temperature, humidity, rainText);

            // Hiển thị lên LCD
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.printf("T:%.1fC H:%.1f%%", temperature, humidity);
            lcd.setCursor(0, 1);
            lcd.print("Rain: ");
            lcd.print(rainText);

            // Gửi telemetry
            if (!isnan(temperature) && !isnan(humidity)) {
                tb.sendTelemetryData("temperature", temperature);
                tb.sendTelemetryData("humidity", humidity);
            }
            tb.sendTelemetryData("rain", (rainStatus == LOW) ? true : false);
        }

        vTaskDelay(telemetrySendInterval / portTICK_PERIOD_MS);
    }
}