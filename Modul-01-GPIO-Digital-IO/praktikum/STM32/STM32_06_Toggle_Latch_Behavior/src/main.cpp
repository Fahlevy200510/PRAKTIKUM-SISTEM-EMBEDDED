/**
 * @file main.cpp
 * @brief Program 06: Toggle Latch Behavior - STM32
 */

#include <Arduino.h>

#define BUTTON_PIN      PB0
#define LED_PIN         PC13    // Active LOW
#define DEBOUNCE_MS     50

volatile bool latchState = false;
volatile bool toggleFlag = false;
unsigned long lastInterrupt = 0;

void buttonISR() {
    unsigned long now = millis();
    if (now - lastInterrupt > DEBOUNCE_MS) {
        toggleFlag = true;
        lastInterrupt = now;
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("Program 06: Toggle Latch - STM32\n");
    
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);  // OFF (active LOW)
    
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);
    
    Serial.println("Press button to toggle latch state");
}

void loop() {
    if (toggleFlag) {
        toggleFlag = false;
        latchState = !latchState;
        digitalWrite(LED_PIN, !latchState);  // Invert for active LOW
        
        Serial.printf("[%lu ms] LATCH = %s\n", millis(),
                     latchState ? "ON (SET)" : "OFF (RESET)");
    }
}
