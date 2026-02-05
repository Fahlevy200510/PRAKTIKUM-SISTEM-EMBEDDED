/**
 * @file main.cpp
 * @brief Program 04: Button Debounce State Machine - STM32
 */

#include <Arduino.h>

// ==================== KONFIGURASI ====================
#define BUTTON_PIN      PB0     // External button
#define LED_PIN         PC13    // Built-in LED (active LOW)
#define DEBOUNCE_MS     50

// ==================== STATE MACHINE ====================
typedef enum {
    BTN_IDLE,
    BTN_DEBOUNCE,
    BTN_PRESSED,
    BTN_RELEASED
} ButtonState_t;

// ==================== VARIABEL ====================
ButtonState_t buttonState = BTN_IDLE;
bool lastButtonRead = HIGH;
unsigned long debounceStartTime = 0;
unsigned long pressStartTime = 0;
uint32_t pressCount = 0;
bool ledState = false;

// ==================== SETUP ====================
void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n========================================");
    Serial.println("Program 04: Button Debounce - STM32");
    Serial.println("========================================\n");
    
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);  // LED OFF (active LOW)
    
    Serial.printf("Button: PB0, LED: PC13 (active LOW)\n");
    Serial.println("Press button to toggle LED\n");
}

// ==================== LOOP ====================
void loop() {
    bool currentRead = digitalRead(BUTTON_PIN);
    
    switch (buttonState) {
        case BTN_IDLE:
            if (currentRead == LOW && lastButtonRead == HIGH) {
                buttonState = BTN_DEBOUNCE;
                debounceStartTime = millis();
            }
            break;
            
        case BTN_DEBOUNCE:
            if (millis() - debounceStartTime >= DEBOUNCE_MS) {
                if (currentRead == LOW) {
                    buttonState = BTN_PRESSED;
                    pressStartTime = millis();
                    
                    // Toggle LED (invert for active LOW)
                    pressCount++;
                    ledState = !ledState;
                    digitalWrite(LED_PIN, !ledState);
                    
                    Serial.printf("Button PRESSED #%lu - LED %s\n",
                                 pressCount, ledState ? "ON" : "OFF");
                } else {
                    buttonState = BTN_IDLE;
                }
            }
            break;
            
        case BTN_PRESSED:
            if (currentRead == HIGH) {
                buttonState = BTN_RELEASED;
                debounceStartTime = millis();
            }
            break;
            
        case BTN_RELEASED:
            if (millis() - debounceStartTime >= DEBOUNCE_MS) {
                if (currentRead == HIGH) {
                    unsigned long duration = millis() - pressStartTime;
                    Serial.printf("Button RELEASED - Duration: %lu ms\n\n", duration);
                    buttonState = BTN_IDLE;
                } else {
                    buttonState = BTN_PRESSED;
                }
            }
            break;
    }
    
    lastButtonRead = currentRead;
}

/**
 * WIRING:
 *   PB0 → Button → GND (internal pull-up enabled)
 *   PC13 → Built-in LED (active LOW)
 */
