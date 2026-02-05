/**
 * @file main.cpp
 * @brief Program 05: LEDC PWM 50% Duty - ESP32
 */

#include <Arduino.h>

#define LED_PIN     2
#define PWM_FREQ    5000
#define PWM_RES     8

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Program 05: LEDC PWM 50% - ESP32\n");
    
    #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        ledcAttach(LED_PIN, PWM_FREQ, PWM_RES);
        ledcWrite(LED_PIN, 127);  // 50% of 255
    #else
        ledcSetup(0, PWM_FREQ, PWM_RES);
        ledcAttachPin(LED_PIN, 0);
        ledcWrite(0, 127);
    #endif
    
    Serial.printf("PWM configured: %d Hz, %d-bit, 50%% duty\n", PWM_FREQ, PWM_RES);
}

void loop() {
    // PWM runs automatically
    delay(1000);
}
