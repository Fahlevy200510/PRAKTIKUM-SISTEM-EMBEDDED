/**
 * @file main.cpp
 * @brief Program 10: ESP Timer millis/micros - ESP32
 */

#include <Arduino.h>

#define LED_PIN     2

unsigned long lastMillis = 0;
unsigned long lastMicros = 0;
uint32_t loopCount = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Program 10: Timer Functions - ESP32\n");
    
    pinMode(LED_PIN, OUTPUT);
    
    Serial.println("Demonstrating millis() and micros() precision");
    Serial.println("millis() wraps after ~49.7 days");
    Serial.println("micros() wraps after ~71.6 minutes\n");
}

void loop() {
    loopCount++;
    
    // Every second using millis()
    if (millis() - lastMillis >= 1000) {
        lastMillis = millis();
        
        unsigned long currentMicros = micros();
        unsigned long elapsed = currentMicros - lastMicros;
        lastMicros = currentMicros;
        
        Serial.printf("[%lu ms] Loops: %lu, Actual interval: %lu us\n",
                     millis(), loopCount, elapsed);
        
        loopCount = 0;
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    }
}
