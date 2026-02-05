/**
 * @file main.cpp
 * @brief Program 12: ISR IRAM Optimization - ESP32
 */

#include <Arduino.h>

#define BUTTON_PIN  0
#define LED_PIN     2

// Variables in IRAM for fast access
volatile IRAM_ATTR uint32_t isrCount = 0;
volatile IRAM_ATTR bool isrFlag = false;

// ISR must be in IRAM to avoid flash cache miss
void IRAM_ATTR fastISR() {
    isrCount++;
    isrFlag = true;
    // Fast GPIO toggle using direct register
    GPIO.out_w1ts = (1 << LED_PIN);  // Set HIGH
    GPIO.out_w1tc = (1 << LED_PIN);  // Set LOW (pulse)
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Program 12: IRAM ISR Optimization - ESP32\n");
    
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), fastISR, FALLING);
    
    Serial.println("IRAM_ATTR ensures ISR runs from fast RAM");
    Serial.println("Direct GPIO register access for minimal latency");
    Serial.println("\nPress button to trigger optimized ISR\n");
}

void loop() {
    if (isrFlag) {
        isrFlag = false;
        Serial.printf("ISR triggered #%lu\n", isrCount);
    }
}

/**
 * IRAM OPTIMIZATION NOTES:
 * 
 * 1. IRAM_ATTR on ISR function:
 *    - Places code in Internal RAM (fast)
 *    - Avoids flash cache miss during interrupt
 *    - Critical for timing-sensitive ISRs
 * 
 * 2. IRAM_ATTR on variables:
 *    - Keeps data in IRAM
 *    - Faster access than flash/PSRAM
 * 
 * 3. Direct register access:
 *    GPIO.out_w1ts = (1 << pin);  // Set HIGH
 *    GPIO.out_w1tc = (1 << pin);  // Set LOW
 *    Much faster than digitalWrite()
 * 
 * 4. Keep ISR short:
 *    - Set flag, process in main loop
 *    - Avoid Serial, delay, malloc in ISR
 */
