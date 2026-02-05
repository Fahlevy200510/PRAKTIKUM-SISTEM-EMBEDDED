/**
 * @file main.cpp
 * @brief Program 07: GPIO Open-Drain Mode - STM32
 * 
 * STM32 GPIO dapat dikonfigurasi sebagai Open-Drain untuk:
 * - I2C bus compatibility
 * - Level shifting
 * - Wired-OR logic
 */

#include <Arduino.h>

#define LED_PUSH_PULL   PA0     // Normal push-pull output
#define LED_OPEN_DRAIN  PA1     // Open-drain output
#define BUTTON_PIN      PB0

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("Program 07: GPIO Open-Drain - STM32\n");
    
    // Push-Pull: Can source AND sink current
    pinMode(LED_PUSH_PULL, OUTPUT);
    
    // Open-Drain: Can only SINK current (needs external pull-up)
    pinMode(LED_OPEN_DRAIN, OUTPUT_OPEN_DRAIN);
    
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    
    Serial.println("PA0: Push-Pull (can source/sink)");
    Serial.println("PA1: Open-Drain (sink only, needs pull-up)");
    Serial.println("\nPress button to toggle both LEDs\n");
    
    Serial.println("Open-Drain requires external pull-up resistor!");
    Serial.println("Connect: PA1 --- [LED] --- [1k] --- 3.3V");
}

void loop() {
    static bool state = false;
    static unsigned long lastPress = 0;
    
    if (digitalRead(BUTTON_PIN) == LOW && millis() - lastPress > 300) {
        lastPress = millis();
        state = !state;
        
        // Push-Pull: HIGH = LED ON, LOW = LED OFF
        digitalWrite(LED_PUSH_PULL, state);
        
        // Open-Drain: LOW = LED ON (sinking), HIGH-Z = LED OFF
        digitalWrite(LED_OPEN_DRAIN, !state);  // Inverted logic
        
        Serial.printf("State: %s\n", state ? "ON" : "OFF");
        Serial.printf("  Push-Pull (PA0): %s\n", state ? "HIGH (source)" : "LOW (sink)");
        Serial.printf("  Open-Drain (PA1): %s\n", state ? "LOW (sink=ON)" : "HIGH-Z (OFF)");
    }
}

/**
 * Push-Pull vs Open-Drain:
 * 
 * PUSH-PULL:
 *   VCC ──┬── HIGH = LED ON (sourcing)
 *         │
 *   GPIO ─┼─→ Can drive both states
 *         │
 *   GND ──┴── LOW = Can sink current
 * 
 * OPEN-DRAIN:
 *   VCC ──[Pull-up]──┬── HIGH-Z = LED ON (via pull-up)
 *                    │
 *   GPIO ────────────┼─→ Can only pull LOW
 *                    │
 *   GND ─────────────┴── LOW = LED OFF (sinking)
 * 
 * Use cases:
 * - I2C (multiple devices share bus)
 * - Level shifting (3.3V to 5V)
 * - Wired-OR (multiple outputs to one line)
 */
