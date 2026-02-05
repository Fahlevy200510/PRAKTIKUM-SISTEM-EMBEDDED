/**
 * @file main.cpp
 * @brief Program 08: Pulse Counter using Interrupt - ESP32
 * 
 * Simple pulse counter using GPIO interrupt instead of PCNT peripheral
 * for better compatibility across ESP32 Arduino versions.
 */

#include <Arduino.h>

#define PULSE_PIN   4
#define BUTTON_PIN  0
#define LED_PIN     2

volatile int32_t pulseCount = 0;
volatile uint32_t lastPulseTime = 0;

void IRAM_ATTR pulseISR() {
    pulseCount++;
    lastPulseTime = millis();
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Program 08: Pulse Counter - ESP32\n");
    
    pinMode(PULSE_PIN, INPUT_PULLUP);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    
    attachInterrupt(digitalPinToInterrupt(PULSE_PIN), pulseISR, RISING);
    
    Serial.printf("Counting pulses on GPIO%d (rising edge)\n", PULSE_PIN);
    Serial.println("Press BOOT button to show count\n");
    Serial.println("Commands:");
    Serial.println("  'c' - Show count");
    Serial.println("  'r' - Reset count");
    Serial.println();
}

void loop() {
    // Physical button
    static bool lastBtn = HIGH;
    bool btn = digitalRead(BUTTON_PIN);
    
    if (btn == LOW && lastBtn == HIGH) {
        Serial.printf("Pulse count: %ld (last pulse: %lu ms ago)\n", 
                      pulseCount, millis() - lastPulseTime);
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    }
    lastBtn = btn;
    
    // Serial commands
    if (Serial.available()) {
        char cmd = Serial.read();
        if (cmd == 'c' || cmd == 'C') {
            Serial.printf("Pulse count: %ld\n", pulseCount);
        } else if (cmd == 'r' || cmd == 'R') {
            pulseCount = 0;
            Serial.println("Count reset to 0");
        }
    }
    
    delay(10);
}
