/**
 * @file main.cpp
 * @brief Program 11: Emergency Stop Logic - STM32
 */

#include <Arduino.h>

#define ESTOP_PIN       PB0
#define RESET_PIN       PB1
#define OUTPUT_PIN      PA0
#define LED_RUN         PA1
#define LED_STOP        PC13    // Active LOW

typedef enum { STATE_STOPPED, STATE_RUNNING, STATE_ESTOP } SystemState_t;

volatile SystemState_t systemState = STATE_STOPPED;
volatile bool estopFlag = false;

void estopISR() {
    digitalWrite(OUTPUT_PIN, LOW);
    digitalWrite(LED_RUN, LOW);
    digitalWrite(LED_STOP, LOW);  // ON (active LOW)
    estopFlag = true;
    systemState = STATE_ESTOP;
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("Program 11: E-Stop System - STM32\n");
    
    pinMode(ESTOP_PIN, INPUT_PULLUP);
    pinMode(RESET_PIN, INPUT_PULLUP);
    pinMode(OUTPUT_PIN, OUTPUT);
    pinMode(LED_RUN, OUTPUT);
    pinMode(LED_STOP, OUTPUT);
    
    digitalWrite(OUTPUT_PIN, LOW);
    digitalWrite(LED_RUN, LOW);
    digitalWrite(LED_STOP, LOW);  // Red ON
    
    attachInterrupt(digitalPinToInterrupt(ESTOP_PIN), estopISR, FALLING);
    
    Serial.println("s=Start, x=Stop, r=Reset E-Stop");
    Serial.println("State: STOPPED\n");
}

void loop() {
    if (Serial.available()) {
        char cmd = Serial.read();
        
        if (cmd == 's' && systemState == STATE_STOPPED) {
            systemState = STATE_RUNNING;
            digitalWrite(OUTPUT_PIN, HIGH);
            digitalWrite(LED_RUN, HIGH);
            digitalWrite(LED_STOP, HIGH);  // OFF
            Serial.println(">>> SYSTEM STARTED");
        }
        else if (cmd == 'x' && systemState == STATE_RUNNING) {
            systemState = STATE_STOPPED;
            digitalWrite(OUTPUT_PIN, LOW);
            digitalWrite(LED_RUN, LOW);
            digitalWrite(LED_STOP, LOW);  // ON
            Serial.println(">>> SYSTEM STOPPED");
        }
        else if (cmd == 'r' && systemState == STATE_ESTOP) {
            estopFlag = false;
            systemState = STATE_STOPPED;
            digitalWrite(LED_STOP, LOW);  // Solid ON
            Serial.println(">>> E-STOP RESET");
        }
    }
    
    if (estopFlag) {
        estopFlag = false;
        Serial.println("\n!!! EMERGENCY STOP !!!\n");
    }
    
    // Blink LED in E-STOP state
    if (systemState == STATE_ESTOP) {
        static unsigned long lastBlink = 0;
        if (millis() - lastBlink > 200) {
            lastBlink = millis();
            digitalWrite(LED_STOP, !digitalRead(LED_STOP));
        }
    }
}
