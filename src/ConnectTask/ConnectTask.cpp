#include "ConnectTask.h"
#include "WarningTask/WarningTask.h"
#include "SensorTask/SensorTask.h"
#include "SendMessageTask/SendMessageTask.h"

// Các hằng số cấu hình
constexpr uint32_t MAX_MESSAGE_SIZE = 1024U;
constexpr uint32_t reconnectInterval = 180000U; // 3 phút connect module

// Lưu thời điểm gửi dữ liệu và kiểm tra kết nối
uint32_t previousReconnectCheck;

WiFiClient wifiClient;
Arduino_MQTT_Client mqttClient(wifiClient);
ThingsBoard tb(mqttClient, MAX_MESSAGE_SIZE);

// Cấu hình WiFi
// constexpr char WIFI_SSID[] = "ACLAB";
// constexpr char WIFI_PASSWORD[] = "ACLAB2023";

constexpr char WIFI_SSID[] = "E11_12";
constexpr char WIFI_PASSWORD[] = "Tiger@E1112";

// Cấu hình ThingsBoard
constexpr char TOKEN[] = "d7v718uBuJSjMIa7m9Kx";
constexpr char THINGSBOARD_SERVER[] = "app.coreiot.io";
constexpr uint16_t THINGSBOARD_PORT = 1883U;

// Task kết nối WiFi
void WiFiTask(void *pvParameters) {
    for (;;) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Connecting to WiFi...");
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
            while (WiFi.status() != WL_CONNECTED) {
                vTaskDelay(300 / portTICK_PERIOD_MS);
                Serial.print(".");
            }
            Serial.println("Connected to WiFi");
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

// Task kết nối ThingsBoard
void ThingsBoardTask(void *pvParameters) {
    for (;;) {
        // Kiểm tra kết nối ThingsBoard, nếu chưa kết nối thì thực hiện kết nối
        if (!tb.connected()) {
            Serial.println("Connecting to ThingsBoard...");
            if (tb.connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT)) {
                Serial.println("Connected to ThingsBoard");
                // Gửi MAC address và đăng ký RPC
                tb.sendAttributeData("macAddress", WiFi.macAddress().c_str());
            }
            else {
                Serial.println("Failed to connect");
            }
        }

        tb.loop();
        vTaskDelay(800 / portTICK_PERIOD_MS);
    }
}

// Task kiểm tra và kết nối lại WiFi và ThingsBoard
void ReconnectTask(void *pvParameters) {
    for (;;) {
        if (millis() - previousReconnectCheck > reconnectInterval) {
            previousReconnectCheck = millis();
            if (WiFi.status() != WL_CONNECTED) {
                Serial.println("Reconnecting WiFi...");
                WiFi.disconnect();
                WiFi.reconnect();
            }
            if (!tb.connected()) {
                Serial.println("Reconnecting to ThingsBoard...");
                tb.connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT);
            }
        }
        vTaskDelay(reconnectInterval / portTICK_PERIOD_MS);
    }
}