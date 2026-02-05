/**
 * @file main.cpp
 * @brief Program 08: DIP Switch Reader - STM32
 */

#include <Arduino.h>

#define NUM_SWITCHES    4
const uint32_t SW_PINS[] = {PB12, PB13, PB14, PB15};  // 4-bit DIP
#define LED_PIN         PC13

uint8_t lastValue = 0xFF;

uint8_t readDipSwitch() {
    uint8_t value = 0;
    for (int i = 0; i < NUM_SWITCHES; i++) {
        if (digitalRead(SW_PINS[i]) == LOW) {
            value |= (1 << i);
        }
    }
    return value;
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("Program 08: DIP Switch Reader - STM32\n");
    
    for (int i = 0; i < NUM_SWITCHES; i++) {
        pinMode(SW_PINS[i], INPUT_PULLUP);
    }
    pinMode(LED_PIN, OUTPUT);
    
    Serial.println("DIP Switch: PB12-PB15");
    Serial.println("ON = 0 (grounded), OFF = 1 (pull-up)\n");
}

void loop() {
    uint8_t value = readDipSwitch();
    
    if (value != lastValue) {
        Serial.printf("DIP: B%d%d%d%d = Dec:%2d Hex:0x%X\n",
                     (value >> 3) & 1, (value >> 2) & 1,
                     (value >> 1) & 1, (value >> 0) & 1,
                     value, value);
        
        digitalWrite(LED_PIN, value > 0 ? LOW : HIGH);
        lastValue = value;
    }
    
    delay(50);
}
