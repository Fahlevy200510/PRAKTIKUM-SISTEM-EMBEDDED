/**
 * @file main.cpp
 * @brief Button (pull-down) on PB0, LED on PB1 with debounce toggle
 */

#include <Arduino.h>

// ==================== CONFIG ====================
#define BUTTON_PIN PB0
#define LED_PIN    PB1
#define DEBOUNCE_MS 50

// ==================== STATE MACHINE ====================
typedef enum {
    BTN_IDLE,
    BTN_DEBOUNCE,
    BTN_PRESSED,
    BTN_RELEASED
} ButtonState_t;

// ==================== VARIABLES ====================
ButtonState_t buttonState = BTN_IDLE;
bool lastButtonRead = LOW; // pull-down idle = LOW
unsigned long debounceStartTime = 0;
unsigned long pressStartTime = 0;
uint32_t pressCount = 0;
bool ledState = false;

// ==================== SETUP ====================
void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("\n=== Button Pull-down (PB0) -> LED PB1 ===");

    // Button: one side to PB0, other side to 3V3 -> use internal pull-down
    pinMode(BUTTON_PIN, INPUT_PULLDOWN);

    // LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW); // LED off

    Serial.println("Wiring: PB0 -> Button -> 3V3 (INPUT_PULLDOWN)");
    Serial.println("        PB1 -> 220R -> LED -> GND");
    Serial.println("Press button to toggle LED\n");
}

// ==================== LOOP ====================
void loop() {
    bool currentRead = digitalRead(BUTTON_PIN);

    switch (buttonState) {
        case BTN_IDLE:
            // detect rising edge (LOW -> HIGH) when using pull-down
            if (currentRead == HIGH && lastButtonRead == LOW) {
                buttonState = BTN_DEBOUNCE;
                debounceStartTime = millis();
            }
            break;

        case BTN_DEBOUNCE:
            if (millis() - debounceStartTime >= DEBOUNCE_MS) {
                if (currentRead == HIGH) {
                    buttonState = BTN_PRESSED;
                    pressStartTime = millis();

                    // toggle LED
                    ledState = !ledState;
                    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
                    pressCount++;
                    Serial.printf("Button PRESSED #%lu - LED %s\n",
                                  pressCount, ledState ? "ON" : "OFF");
                } else {
                    buttonState = BTN_IDLE;
                }
            }
            break;

        case BTN_PRESSED:
            // wait for release (HIGH -> LOW)
            if (currentRead == LOW) {
                buttonState = BTN_RELEASED;
                debounceStartTime = millis();
            }
            break;

        case BTN_RELEASED:
            if (millis() - debounceStartTime >= DEBOUNCE_MS) {
                if (currentRead == LOW) {
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

/*
 * WIRING:
 *   PB0 -> Button -> 3V3 (internal pull-down enabled)
 *   PB1 -> 220R -> LED -> GND
 */
