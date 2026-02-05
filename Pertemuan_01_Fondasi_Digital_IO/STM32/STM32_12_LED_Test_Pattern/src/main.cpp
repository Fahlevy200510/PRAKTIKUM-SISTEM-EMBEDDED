/**
 * @file main.cpp
 * @brief Program 12: LED Test Pattern (POST) - STM32
 */

#include <Arduino.h>

#define NUM_LEDS        4
const uint32_t LEDS[] = {PA0, PA1, PA2, PA3};
#define TEST_DELAY      100

void allOn() { for(int i=0; i<NUM_LEDS; i++) digitalWrite(LEDS[i], HIGH); }
void allOff() { for(int i=0; i<NUM_LEDS; i++) digitalWrite(LEDS[i], LOW); }

void testSequential() {
    for (int i=0; i<NUM_LEDS; i++) {
        allOff();
        digitalWrite(LEDS[i], HIGH);
        delay(TEST_DELAY);
    }
    for (int i=NUM_LEDS-2; i>=0; i--) {
        allOff();
        digitalWrite(LEDS[i], HIGH);
        delay(TEST_DELAY);
    }
}

void testBinary() {
    for (int n=0; n<=15; n++) {
        for (int i=0; i<NUM_LEDS; i++) {
            digitalWrite(LEDS[i], (n >> i) & 1);
        }
        Serial.printf("  %2d = B%d%d%d%d\n", n,
                     (n>>3)&1, (n>>2)&1, (n>>1)&1, n&1);
        delay(100);
    }
}

void runPOST() {
    unsigned long start = millis();
    
    Serial.println("\n=== POWER-ON SELF-TEST ===\n");
    
    Serial.println("[1] All ON...");
    allOn(); delay(500);
    
    Serial.println("[2] All OFF...");
    allOff(); delay(300);
    
    Serial.println("[3] Sequential...");
    testSequential(); testSequential();
    
    Serial.println("[4] Binary count...");
    testBinary();
    
    allOff();
    
    Serial.printf("\n=== POST Complete (%lu ms) ===\n", millis() - start);
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("Program 12: LED Self-Test - STM32");
    
    for (int i=0; i<NUM_LEDS; i++) {
        pinMode(LEDS[i], OUTPUT);
        digitalWrite(LEDS[i], LOW);
    }
    
    runPOST();
    
    Serial.println("\nType 't' to run test again");
}

void loop() {
    if (Serial.available()) {
        if (Serial.read() == 't') runPOST();
    }
}
