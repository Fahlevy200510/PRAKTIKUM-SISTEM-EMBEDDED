/**
 * @file main.cpp
 * @brief Program 01: GPIO Interrupt - ESP32
 */

#include <Arduino.h>

#define BUTTON_PIN  0
#define LED_PIN     2

volatile uint32_t risingCount = 0;
volatile uint32_t fallingCount = 0;
volatile bool stateChanged = false;

void IRAM_ATTR buttonISR() {
    if (digitalRead(BUTTON_PIN) == LOW) {
        fallingCount++;
    } else {
        risingCount++;
    }
    stateChanged = true;
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Program 01: GPIO Interrupt - ESP32\n");
    
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, CHANGE);
    
    Serial.println("Interrupt attached to GPIO0 (CHANGE mode)");
    Serial.println("Press button to trigger interrupts\n");
}

void loop() {
    if (stateChanged) {
        stateChanged = false;
        
        Serial.printf("Rising: %lu, Falling: %lu\n", risingCount, fallingCount);
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    }
}
