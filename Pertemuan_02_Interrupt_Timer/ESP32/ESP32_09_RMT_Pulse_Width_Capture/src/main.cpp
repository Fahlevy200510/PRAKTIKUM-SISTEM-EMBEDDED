/**
 * @file main.cpp
 * @brief Program 09: RMT Pulse Width Capture - ESP32
 */

#include <Arduino.h>

#define INPUT_PIN   4
#define LED_PIN     2

volatile unsigned long pulseStart = 0;
volatile unsigned long pulseWidth = 0;
volatile bool newPulse = false;

void IRAM_ATTR pulseISR() {
    if (digitalRead(INPUT_PIN) == LOW) {
        pulseStart = micros();
    } else {
        pulseWidth = micros() - pulseStart;
        newPulse = true;
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Program 09: Pulse Width Capture - ESP32\n");
    
    pinMode(INPUT_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    
    attachInterrupt(digitalPinToInterrupt(INPUT_PIN), pulseISR, CHANGE);
    
    Serial.printf("Measuring pulse width on GPIO%d\n\n", INPUT_PIN);
}

void loop() {
    if (newPulse) {
        newPulse = false;
        
        Serial.printf("Pulse width: %lu us", pulseWidth);
        
        if (pulseWidth > 1000000) {
            Serial.printf(" (%.2f s)\n", pulseWidth / 1000000.0);
        } else if (pulseWidth > 1000) {
            Serial.printf(" (%.2f ms)\n", pulseWidth / 1000.0);
        } else {
            Serial.println(" us");
        }
        
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    }
}
