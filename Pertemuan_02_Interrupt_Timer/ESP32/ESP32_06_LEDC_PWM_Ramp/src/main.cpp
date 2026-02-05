/**
 * @file main.cpp
 * @brief Program 06: LEDC PWM Ramp (Sawtooth) - ESP32
 */

#include <Arduino.h>

#define LED_PIN     2
#define PWM_FREQ    5000
#define PWM_RES     8

uint8_t duty = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Program 06: PWM Ramp - ESP32\n");
    
    #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        ledcAttach(LED_PIN, PWM_FREQ, PWM_RES);
    #else
        ledcSetup(0, PWM_FREQ, PWM_RES);
        ledcAttachPin(LED_PIN, 0);
    #endif
}

void loop() {
    #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        ledcWrite(LED_PIN, duty);
    #else
        ledcWrite(0, duty);
    #endif
    
    duty++;  // Auto wrap 255->0
    
    if (duty == 0) {
        Serial.println("Ramp complete, restarting...");
    }
    
    delay(10);
}
