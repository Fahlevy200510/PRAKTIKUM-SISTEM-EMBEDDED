/**
 * @file main.cpp
 * @brief Program 04: One-Shot Timer - ESP32
 */

#include <Arduino.h>
#include "esp_timer.h"

#define LED_PIN     2
#define BUTTON_PIN  0

esp_timer_handle_t oneshot_timer;
volatile bool timerExpired = false;

void oneshotCallback(void* arg) {
    timerExpired = true;
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Program 04: One-Shot Timer - ESP32\n");
    
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    
    esp_timer_create_args_t timer_args = {
        .callback = &oneshotCallback,
        .name = "oneshot"
    };
    esp_timer_create(&timer_args, &oneshot_timer);
    
    Serial.println("Press button to start 2-second one-shot timer");
}

void loop() {
    static bool lastBtn = HIGH;
    bool btn = digitalRead(BUTTON_PIN);
    
    if (btn == LOW && lastBtn == HIGH) {
        digitalWrite(LED_PIN, HIGH);
        esp_timer_start_once(oneshot_timer, 2000000);  // 2 seconds
        Serial.println("One-shot timer started (2s)...");
    }
    lastBtn = btn;
    
    if (timerExpired) {
        timerExpired = false;
        digitalWrite(LED_PIN, LOW);
        Serial.println(">>> Timer expired! LED OFF");
    }
}
