/**
 * @file main.cpp
 * @brief Program 01: LED Blink - Dasar GPIO Output ESP32
 * 
 * Deskripsi:
 * Program dasar untuk mengendalikan LED menggunakan GPIO.
 * LED akan berkedip dengan interval yang dapat dikonfigurasi.
 * 
 * Hardware:
 * - ESP32 DevKitC
 * - LED Built-in (GPIO2) atau LED External dengan resistor 220Ω
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include "config.h"

// ==================== VARIABEL GLOBAL ====================
unsigned long previousMillis = 0;   // Waktu sebelumnya
bool ledState = false;              // Status LED
uint32_t blinkCount = 0;            // Counter blink

// ==================== SETUP ====================
void setup() {
    // Inisialisasi Serial
    Serial.begin(SERIAL_BAUD);
    delay(1000);  // Tunggu serial ready
    
    Serial.println("\n========================================");
    Serial.println("Program 01: LED Blink - ESP32");
    Serial.println("Praktikum Sistem Embedded");
    Serial.println("========================================\n");
    
    // Konfigurasi GPIO sebagai OUTPUT
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);  // Mulai dengan LED mati
    
    Serial.printf("LED Pin: GPIO%d\n", LED_PIN);
    Serial.printf("Blink Interval: %d ms\n", BLINK_DELAY_MS);
    Serial.println("Program dimulai...\n");
}

// ==================== LOOP ====================
void loop() {
    unsigned long currentMillis = millis();
    
    // Non-blocking blink menggunakan millis()
    if (currentMillis - previousMillis >= BLINK_DELAY_MS) {
        previousMillis = currentMillis;
        
        // Toggle LED state
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);
        
        // Increment counter
        blinkCount++;
        
        // Debug output
        #if DEBUG_SERIAL
        Serial.printf("[%lu ms] LED: %s | Blink #%lu\n", 
                      currentMillis, 
                      ledState ? "ON " : "OFF",
                      blinkCount);
        #endif
    }
}

/**
 * WIRING DIAGRAM:
 * 
 *   ESP32 DevKitC
 *   ┌─────────────┐
 *   │             │
 *   │         3V3 ├───[LED Built-in sudah ada di GPIO2]
 *   │             │
 *   │        GPIO2├───┬───[220Ω]───[LED]───┐
 *   │             │   │                    │
 *   │         GND ├───┴────────────────────┘
 *   │             │
 *   └─────────────┘
 * 
 * Catatan:
 * - GPIO2 adalah LED built-in pada sebagian besar ESP32 DevKitC
 * - Untuk LED external, gunakan resistor 220Ω - 330Ω
 * 
 * EXPECTED OUTPUT (Serial Monitor):
 * ========================================
 * Program 01: LED Blink - ESP32
 * Praktikum Sistem Embedded
 * ========================================
 * 
 * LED Pin: GPIO2
 * Blink Interval: 500 ms
 * Program dimulai...
 * 
 * [500 ms] LED: ON  | Blink #1
 * [1000 ms] LED: OFF | Blink #2
 * [1500 ms] LED: ON  | Blink #3
 * ...
 */
