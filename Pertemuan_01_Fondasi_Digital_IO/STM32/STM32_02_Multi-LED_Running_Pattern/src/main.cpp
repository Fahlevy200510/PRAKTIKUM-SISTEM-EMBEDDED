/**
 * @file main.cpp
 * @brief Program 02: Multi-LED Running Pattern - STM32F103C8T6
 */

#include <Arduino.h>

// ==================== KONFIGURASI ====================
#define NUM_LEDS        4
#define LED_1           PA0
#define LED_2           PA1
#define LED_3           PA2
#define LED_4           PA3

const uint32_t LED_PINS[NUM_LEDS] = {LED_1, LED_2, LED_3, LED_4};
#define PATTERN_DELAY   150

// ==================== VARIABEL ====================
uint8_t currentLed = 0;
uint8_t currentPattern = 0;
bool direction = true;
unsigned long previousMillis = 0;
uint8_t fillCount = 0;

// ==================== SETUP ====================
void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n========================================");
    Serial.println("Program 02: Multi-LED Running - STM32");
    Serial.println("========================================\n");
    
    // Initialize LED pins
    for (int i = 0; i < NUM_LEDS; i++) {
        pinMode(LED_PINS[i], OUTPUT);
        digitalWrite(LED_PINS[i], LOW);
    }
    
    Serial.println("LED Pins: PA0, PA1, PA2, PA3");
    Serial.println("Patterns: 0=Running, 1=Bounce, 2=Fill, 3=Blink");
    Serial.println("Type 0-3 to change pattern\n");
}

// ==================== LOOP ====================
void loop() {
    // Handle serial input
    if (Serial.available() > 0) {
        char input = Serial.read();
        if (input >= '0' && input <= '3') {
            currentPattern = input - '0';
            currentLed = 0;
            fillCount = 0;
            direction = true;
            Serial.printf("Pattern: %d\n", currentPattern);
        }
    }
    
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= PATTERN_DELAY) {
        previousMillis = currentMillis;
        
        // Turn all off first
        for (int i = 0; i < NUM_LEDS; i++) {
            digitalWrite(LED_PINS[i], LOW);
        }
        
        switch (currentPattern) {
            case 0: // Running
                digitalWrite(LED_PINS[currentLed], HIGH);
                currentLed = (currentLed + 1) % NUM_LEDS;
                break;
                
            case 1: // Bounce
                digitalWrite(LED_PINS[currentLed], HIGH);
                if (direction) {
                    currentLed++;
                    if (currentLed >= NUM_LEDS - 1) direction = false;
                } else {
                    currentLed--;
                    if (currentLed <= 0) direction = true;
                }
                break;
                
            case 2: // Fill
                for (int i = 0; i <= fillCount; i++) {
                    digitalWrite(LED_PINS[i], HIGH);
                }
                fillCount = (fillCount + 1) % NUM_LEDS;
                break;
                
            case 3: // Blink all
                static bool state = false;
                state = !state;
                for (int i = 0; i < NUM_LEDS; i++) {
                    digitalWrite(LED_PINS[i], state);
                }
                break;
        }
    }
}

/**
 * WIRING:
 *   PA0 → 220Ω → LED1 → GND
 *   PA1 → 220Ω → LED2 → GND
 *   PA2 → 220Ω → LED3 → GND
 *   PA3 → 220Ω → LED4 → GND
 */
