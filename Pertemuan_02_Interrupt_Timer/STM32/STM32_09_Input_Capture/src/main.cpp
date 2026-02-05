#include <Arduino.h>

#define INPUT_PIN PB0
#define LED_PIN   PC13

volatile unsigned long pulseStart = 0;
volatile unsigned long pulseWidth = 0;
volatile bool newPulse = false;

void pulseISR() {
    if (digitalRead(INPUT_PIN) == LOW) {
        pulseStart = micros();
    } else {
        pulseWidth = micros() - pulseStart;
        newPulse = true;
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 09: Input Capture - STM32\n");
    
    pinMode(INPUT_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(INPUT_PIN), pulseISR, CHANGE);
    
    Serial.println("Measuring pulse width on PB0");
}

void loop() {
    if (newPulse) {
        newPulse = false;
        Serial.printf("Pulse: %lu us\n", pulseWidth);
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    }
}
