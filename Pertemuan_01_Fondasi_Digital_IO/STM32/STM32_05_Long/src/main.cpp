/**
 * @file main.cpp
 * @brief Program 05: Long/Short Press Detection - STM32
 */

#include <Arduino.h>

#define BUTTON_PIN      PB0
#define LED_SHORT       PA0     // Short press LED
#define LED_LONG        PA1     // Long press LED
#define DEBOUNCE_MS     50
#define LONG_PRESS_MS   1000

bool buttonPressed = false;
unsigned long pressStartTime = 0;
unsigned long lastDebounce = 0;
bool lastState = HIGH;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("Program 05: Long/Short Press - STM32");
    
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_SHORT, OUTPUT);
    pinMode(LED_LONG, OUTPUT);
    
    digitalWrite(LED_SHORT, LOW);
    digitalWrite(LED_LONG, LOW);
    
    Serial.println("Short press (<1s): Toggle LED_SHORT (PA0)");
    Serial.println("Long press (>1s): Toggle LED_LONG (PA1)\n");
}

void loop() {
    bool currentState = digitalRead(BUTTON_PIN);
    
    if (currentState != lastState) {
        lastDebounce = millis();
    }
    
    if ((millis() - lastDebounce) > DEBOUNCE_MS) {
        if (currentState == LOW && !buttonPressed) {
            buttonPressed = true;
            pressStartTime = millis();
            Serial.print("Button pressed... ");
        }
        
        if (currentState == HIGH && buttonPressed) {
            buttonPressed = false;
            unsigned long duration = millis() - pressStartTime;
            Serial.printf("released (%lu ms)\n", duration);
            
            if (duration >= LONG_PRESS_MS) {
                digitalWrite(LED_LONG, !digitalRead(LED_LONG));
                Serial.println(">>> LONG PRESS - LED_LONG toggled");
            } else {
                digitalWrite(LED_SHORT, !digitalRead(LED_SHORT));
                Serial.println(">>> SHORT PRESS - LED_SHORT toggled");
            }
        }
    }
    
    lastState = currentState;
}
