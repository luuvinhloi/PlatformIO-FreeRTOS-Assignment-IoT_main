#include "ConnectTask/ConnectTask.h"
#include "WarningTask/WarningTask.h"
#include "SensorTask/SensorTask.h"
#include "LCDTask/LCDTask.h"
#include "SendMessageTask/SendMessageTask.h"
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <array>
#include <HttpClient.h>
#include <Update.h>

// Các hằng số cấu hình
constexpr uint32_t SERIAL_DEBUG_BAUD = 115200U;

#define CURRENT_FIRMWARE_VERSION "v1.0.8"

const char* host = "192.168.88.147";
const int port = 8000;

const char* version_path = "/version.txt";  // Đường dẫn file version
const char* bin_path = "/firmware.bin";

// Prototype các task của FreeRTOS
void WiFiTask(void *pvParameters);
void ThingsBoardTask(void *pvParameters);
void ReconnectTask(void *pvParameters);
void SensorTask(void *pvParameters);
void WarningTask(void *pvParameters);
void OTATask(void *pvParameters);

bool isNewFirmwareAvailable(const char* host, int port, const char* version_path) {
  WiFiClient wifiClient;
  HttpClient httpClient(wifiClient, host, port);
  String url = String(version_path);
  int err = httpClient.get(url);

  if (err != 0) {
    Serial.print("[OTA] Failed to fetch version file. Error: ");
    Serial.println(err);
    return false;
  }

  int statusCode = httpClient.responseStatusCode();
  if (statusCode != 200) {
    Serial.printf("[OTA] Version file HTTP error code: %d\n", statusCode);
    return false;
  }

  String newVersion = httpClient.responseBody();
  newVersion.trim(); // Loại bỏ khoảng trắng, newline

  Serial.printf("[OTA] Current version: %s\n", CURRENT_FIRMWARE_VERSION);
  Serial.printf("[OTA] New version on server: %s\n", newVersion.c_str());

  return newVersion != CURRENT_FIRMWARE_VERSION;
}

void checkOTAUpdate() {
  Serial.println("[OTA] Checking for firmware update...");

  if (!isNewFirmwareAvailable(host, port, version_path)) {
    Serial.println("[OTA] Firmware is up-to-date. Skipping update.");
    return;
  }

  Serial.printf("[OTA] New version found! Requesting http://%s:%d%s\n", host, port, bin_path);

  WiFiClient wifiClient;
  HttpClient httpClient(wifiClient, host, port);
  String url = String(bin_path);
  int err = httpClient.get(url);

  if (err != 0) {
    Serial.print("[OTA] HTTP GET failed, error: ");
    Serial.println(err);
    return;
  }

  int statusCode = httpClient.responseStatusCode();
  int contentLength = httpClient.contentLength();

  if (statusCode != 200) {
    Serial.printf("[OTA] HTTP error code: %d\n", statusCode);
    return;
  }

  if (!Update.begin(contentLength)) {
    Serial.println("[OTA] Not enough space to begin update.");
    return;
  }

  int written = 0;
  const int bufferSize = 512;
  uint8_t buffer[bufferSize];
  int lastPercent = -1;

  while (written < contentLength) {
    int available = httpClient.available();
    if (available <= 0) {
      delay(10);  // cho phép dữ liệu đến
      continue;
    }

    int toRead = min(bufferSize, contentLength - written);
    int bytesRead = httpClient.readBytes(buffer, toRead);

    if (bytesRead <= 0) {
      Serial.println("[OTA] Read timeout or error.");
      break;
    }

    size_t bytesWritten = Update.write(buffer, bytesRead);
    if (bytesWritten != bytesRead) {
      Serial.printf("[OTA] Write error! Wrote %d/%d bytes\n", bytesWritten, bytesRead);
      Update.abort();
      return;
    }

    written += bytesWritten;

    int percent = (written * 100) / contentLength;
    if (percent >= lastPercent + 5) {
      Serial.printf("[OTA] Progress: %d%%\n", percent);
      lastPercent = percent;
    }
  }

  if (written != contentLength) {
    Serial.printf("[OTA] Mismatch! Only wrote %d of %d bytes\n", written, contentLength);
    Update.abort();
    return;
  }


  if (Update.end()) {
    if (Update.isFinished()) {
      Serial.println("[OTA] Update complete. Rebooting...");
      ESP.restart();
    } else {
      Serial.println("[OTA] Update not complete.");
    }
  } else {
    Serial.printf("[OTA] Update error #%d\n", Update.getError());
  }

  httpClient.stop();
  Serial.println("[OTA] Update process finished.");
}

void OTATask(void *pvParameters) {
  // Đợi đến khi WiFi kết nối
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("[OTA] Waiting for WiFi...");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }

  // Chạy kiểm tra OTA đúng 1 lần sau khi kết nối WiFi
  checkOTAUpdate();

  // Xoá task sau khi kiểm tra xong
  Serial.println("[OTA] OTATask completed and deleted.");
  vTaskDelete(NULL);
}

// Hàm setup
void setup() {
  Serial.begin(SERIAL_DEBUG_BAUD);

  pinMode(RAIN_PIN, INPUT);
  
  dht.begin();

  // Khởi tạo LCD
  Wire.begin(SDA_PIN, SCL_PIN);
  lcd.init();
  lcd.backlight();  // Bật đèn nền
  lcd.setCursor(0, 0);
  lcd.print("Thong Tin Thoi Tiet");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");

  // Khởi tạo các task của FreeRTOS
  xTaskCreate(WiFiTask, "WiFiTask", 4096, NULL, 1, NULL);
  xTaskCreate(ThingsBoardTask, "ThingsBoardTask", 4096, NULL, 1, NULL);
  xTaskCreate(ReconnectTask, "ReconnectTask", 4096, NULL, 1, NULL);
  xTaskCreate(SensorTask, "SensorTask", 4096, NULL, 1, NULL);
  xTaskCreate(WarningTask, "WarningTask", 2048, NULL, 1, NULL);
  xTaskCreate(OTATask, "OTATask", 4096, NULL, 1, NULL);
}

// Hàm loop
void loop() {
  vTaskDelete(NULL);
}