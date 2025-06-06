#ifndef SEND_MESSAGE_TASK_H
#define SEND_MESSAGE_TASK_H

#include <Arduino.h>
#include <HTTPClient.h>

// Telegram Bot
#define TELEGRAM_BOT_TOKEN "8122936180:AAHVsRVDZrjwevWBwJnzZOHm_dgcJgkevuY"
#define TELEGRAM_CHAT_ID "1378242143"

// Cấu hình thời gian giữa 2 lần gửi cảnh báo
constexpr uint32_t alertInterval = 60000; // 1 phút (600.000 milliseconds)

// Biến toàn cục lưu thời điểm cảnh báo gần nhất
extern uint32_t lastAlertTime;

// Hàm gửi cảnh báo
void sendTelegramMessage(String message, String imageUrl);
void checkAndSendAlerts(float temperature, float humidity, bool rainStatus);

#endif