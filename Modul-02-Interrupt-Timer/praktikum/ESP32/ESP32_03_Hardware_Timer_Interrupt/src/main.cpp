/**
 * @file main.cpp
 * @brief Program 03: Hardware Timer Interrupt - ESP32
 */

#include <Arduino.h>

#define LED_PIN     2

hw_timer_t *timer = NULL;
volatile uint32_t timerCount = 0;
volatile bool timerFlag = false;

void IRAM_ATTR onTimer() {
    timerCount++;
    timerFlag = true;
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Program 03: Hardware Timer - ESP32\n");
    
    pinMode(LED_PIN, OUTPUT);
    
    // API for ESP32 Arduino Core 3.x
    #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        timer = timerBegin(1000000);  // 1MHz
        timerAttachInterrupt(timer, &onTimer);
        timerAlarm(timer, 1000000, true, 0);  // 1 second
    #else
        // API for ESP32 Arduino Core 2.x
        timer = timerBegin(0, 80, true);  // Timer 0, divider 80 (1MHz), count up
        timerAttachInterrupt(timer, &onTimer, true);
        timerAlarmWrite(timer, 1000000, true);  // 1 second
        timerAlarmEnable(timer);
    #endif
    
    Serial.println("Timer configured: 1 second interval");
}

void loop() {
    if (timerFlag) {
        timerFlag = false;
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        Serial.printf("Timer tick #%lu\n", timerCount);
    }
}
