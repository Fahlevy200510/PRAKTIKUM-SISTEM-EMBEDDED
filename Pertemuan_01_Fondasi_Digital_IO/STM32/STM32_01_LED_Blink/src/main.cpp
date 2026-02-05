/**
 * @file main.cpp
 * @brief Program 01: LED Blink - STM32F103C8T6 Blue Pill
 * 
 * Deskripsi:
 * Program dasar LED blink untuk STM32. Menggunakan on-board LED
 * di PC13 (active LOW) dan optional external LED.
 * 
 * Hardware:
 * - STM32F103C8T6 Blue Pill
 * - ST-Link V2 programmer
 * - LED Built-in (PC13) atau LED External dengan resistor 220Ω
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include "config.h"

// ==================== VARIABEL GLOBAL ====================
unsigned long previousMillis = 0;
bool ledState = false;
uint32_t blinkCount = 0;

// ==================== SETUP ====================
void setup() {
    // Inisialisasi Serial (PA9=TX, PA10=RX)
    Serial.begin(SERIAL_BAUD);
    delay(2000);  // Tunggu serial ready
    
    Serial.println("\n========================================");
    Serial.println("Program 01: LED Blink - STM32F103C8T6");
    Serial.println("Praktikum Sistem Embedded");
    Serial.println("========================================\n");
    
    // Konfigurasi GPIO sebagai OUTPUT
    pinMode(LED_BUILTIN_PIN, OUTPUT);
    pinMode(LED_EXTERNAL, OUTPUT);
    
    // Initial state: LED OFF
    // PC13 is active LOW, so HIGH = OFF
    digitalWrite(LED_BUILTIN_PIN, HIGH);
    digitalWrite(LED_EXTERNAL, LOW);
    
    Serial.println("STM32F103C8T6 Blue Pill Configuration:");
    Serial.println("  Core: ARM Cortex-M3 @ 72MHz");
    Serial.println("  Flash: 64KB, RAM: 20KB");
    Serial.printf("  Built-in LED: PC13 (Active LOW)\n");
    Serial.printf("  External LED: PA0 (Active HIGH)\n");
    Serial.printf("  Blink Interval: %d ms\n", BLINK_DELAY_MS);
    Serial.println("\nProgram dimulai...\n");
}

// ==================== LOOP ====================
void loop() {
    unsigned long currentMillis = millis();
    
    // Non-blocking blink
    if (currentMillis - previousMillis >= BLINK_DELAY_MS) {
        previousMillis = currentMillis;
        
        // Toggle LED state
        ledState = !ledState;
        
        // PC13 is active LOW, so invert logic
        digitalWrite(LED_BUILTIN_PIN, !ledState);
        // External LED is active HIGH (normal)
        digitalWrite(LED_EXTERNAL, ledState);
        
        blinkCount++;
        
        Serial.printf("[%lu ms] LED: %s | Blink #%lu\n", 
                      currentMillis, 
                      ledState ? "ON " : "OFF",
                      blinkCount);
    }
}

/**
 * WIRING DIAGRAM:
 * 
 *   STM32F103C8T6 Blue Pill
 *   ┌─────────────────────────┐
 *   │                         │
 *   │   PC13 ●────[LED]───────│ (Built-in, Active LOW)
 *   │                         │
 *   │    PA0 ├───[220Ω]───[LED]───GND
 *   │                         │
 *   │   3.3V ├───────────────→│ Power
 *   │    GND ├───────────────→│ Ground
 *   │                         │
 *   │    PA9 ├───TX──────────→│ Serial Monitor
 *   │   PA10 ├───RX──────────→│ Serial Monitor
 *   │                         │
 *   │   SWDIO├───────────────→│ ST-Link
 *   │   SWCLK├───────────────→│ ST-Link
 *   │                         │
 *   └─────────────────────────┘
 * 
 * CATATAN PENTING STM32:
 * 1. PC13 LED adalah ACTIVE LOW (LED menyala saat pin LOW)
 * 2. PC13 memiliki drive current terbatas (~3mA)
 * 3. Gunakan external LED jika butuh brightness lebih
 * 4. Clock default 72MHz (HSE 8MHz x PLL)
 * 
 * EXPECTED OUTPUT:
 * ========================================
 * Program 01: LED Blink - STM32F103C8T6
 * Praktikum Sistem Embedded
 * ========================================
 * 
 * STM32F103C8T6 Blue Pill Configuration:
 *   Core: ARM Cortex-M3 @ 72MHz
 *   Flash: 64KB, RAM: 20KB
 *   Built-in LED: PC13 (Active LOW)
 *   External LED: PA0 (Active HIGH)
 *   Blink Interval: 500 ms
 * 
 * [500 ms] LED: ON  | Blink #1
 * [1000 ms] LED: OFF | Blink #2
 * [1500 ms] LED: ON  | Blink #3
 */
