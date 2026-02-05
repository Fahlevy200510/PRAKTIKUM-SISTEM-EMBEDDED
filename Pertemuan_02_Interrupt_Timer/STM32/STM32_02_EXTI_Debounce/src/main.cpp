#include <Arduino.h>

#define BUTTON_PIN  PB0
#define LED_PIN     PC13
#define DEBOUNCE_MS 50

volatile uint32_t lastInt = 0;
volatile uint32_t count = 0;
volatile bool flag = false;

void buttonISR() {
    uint32_t now = millis();
    if (now - lastInt > DEBOUNCE_MS) {
        lastInt = now;
        count++;
        flag = true;
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 02: EXTI Debounce - STM32\n");
    
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);
}

void loop() {
    if (flag) {
        flag = false;
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        Serial.printf("Debounced press #%lu at %lu ms\n", count, lastInt);
    }
}
