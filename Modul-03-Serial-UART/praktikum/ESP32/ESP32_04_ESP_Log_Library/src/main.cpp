#include <Arduino.h>
#include "esp_log.h"

static const char* TAG = "DEMO";

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Program 04: ESP Log Library - ESP32\n");
    
    esp_log_level_set("*", ESP_LOG_VERBOSE);
    
    ESP_LOGE(TAG, "This is an ERROR message");
    ESP_LOGW(TAG, "This is a WARNING message");
    ESP_LOGI(TAG, "This is an INFO message");
    ESP_LOGD(TAG, "This is a DEBUG message");
    ESP_LOGV(TAG, "This is a VERBOSE message");
}

void loop() {
    static int count = 0;
    ESP_LOGI(TAG, "Loop count: %d", count++);
    delay(2000);
}
