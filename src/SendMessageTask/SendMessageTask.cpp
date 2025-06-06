#include "SendMessageTask.h"
#include "SensorTask/SensorTask.h"
#include "ConnectTask/ConnectTask.h"
#include "WeatherTask/WeatherTask.h"

// Lưu thời điểm gửi cảnh báo gần nhất
uint32_t lastAlertTime = 0;

// Biến lưu trạng thái dự báo mưa trước đó
bool lastRainExpected = true;

// Hàm gửi tin nhắn cảnh báo qua Telegram và in ra Serial
void sendTelegramMessage(String messageTelegram, String messageSerial, String imageUrl) {
    // In ra nội dung gửi
    Serial.println("--------- GỬI CẢNH BÁO ---------");
    Serial.println(messageSerial);
    Serial.println("--------------------------------");

    String url = "https://api.telegram.org/bot" + String(TELEGRAM_BOT_TOKEN) + 
                "/sendPhoto?chat_id=" + String(TELEGRAM_CHAT_ID) + 
                "&photo=" + imageUrl + 
                "&caption=" + messageTelegram;

    HTTPClient http;
    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode < 0) {
        Serial.print("Lỗi gửi tin nhắn Telegram: ");
        Serial.println(httpResponseCode);
    }
    http.end();
}

// Hàm bổ sung thông báo cảnh báo vào hai chuỗi: 1 cho Telegram, 1 cho Serial
void appendAlertMessage(String &msgTelegram, String &msgSerial, const String &text, bool &alertSent) {
    msgTelegram += text + " %0A";
    msgSerial += text + "\n";
    alertSent = true;
}

// Hàm kiểm tra và gửi cảnh báo
void checkAndSendAlerts(float temperature, float humidity, bool rainStatus) {
    uint32_t currentMillis = millis();
    if (currentMillis - lastAlertTime < alertInterval) return;

    bool rainExpected = checkRainNext12Hours();
    bool alertSent = false;

    String msgTelegram = "🚨 CẢNH BÁO THỜI TIẾT 🚨 %0A";
    String msgSerial = "CẢNH BÁO THỜI TIẾT\n";
    String imageURL = "http://rangdong.com.vn/uploads/news/tin-san-pham/nguoi-ban-4.0-cua-nha-nong/smart-farm-rang-dong-nguoi-ban-4.0-cua-nha-nong-5.jpg";

    std::vector<std::pair<String, bool>> conditions = {
        {"Độ ẩm quá thấp! ( " + String(humidity) + "%)", humidity < 30},
        {"Độ ẩm quá cao! ( " + String(humidity) + "%)", humidity > 90},
        {"Nhiệt độ quá thấp! ( " + String(temperature) + "°C)", temperature < 20},
        {"Nhiệt độ quá cao! ( " + String(temperature) + "°C)", temperature > 35},
        {"Thời tiết hiện đang mưa!", rainStatus == LOW},
        {"Thời tiết hiện không mưa!", rainStatus == HIGH}
    };

    for (const auto &condition : conditions) {
        if (condition.second) {
            appendAlertMessage(msgTelegram, msgSerial, condition.first, alertSent);
        }
    }

    if (rainExpected != lastRainExpected) {
        String rainMessage = rainExpected ? "Dự báo Có Mưa trong 12 giờ tới" : "Dự báo Không có Mưa trong 12 giờ tới";
        appendAlertMessage(msgTelegram, msgSerial, rainMessage, alertSent);
        lastRainExpected = rainExpected;
    }

    if (alertSent) {
        sendTelegramMessage(msgTelegram, msgSerial, imageURL);
        lastAlertTime = currentMillis;
    }
}