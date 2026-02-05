/**
 * @file main.cpp
 * @brief Program 01: SPI Loopback - STM32F103C8T6 Blue Pill
 * 
 * Deskripsi:
 * Program untuk menguji komunikasi SPI dengan mode loopback.
 * MOSI dan MISO dihubungkan bersama untuk menguji transmission.
 * 
 * Hardware:
 * - STM32F103C8T6 Blue Pill
 * - Jumper wire untuk loopback
 * 
 * Wiring:
 * - PA5 (SCK)
 * - PA7 (MOSI) --- PA6 (MISO) [Loopback]
 * - PA4 (CS)
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include "config.h"

SPIClass SPI_1(1);  // Use SPI1

// ==================== GLOBAL VARIABLES ====================
uint8_t txBuffer[BUFFER_SIZE];
uint8_t rxBuffer[BUFFER_SIZE];
uint32_t testCount = 0;

// ==================== SETUP ====================
void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(2000);
    
    Serial.println("\n========================================");
    Serial.println("Program 01: SPI Loopback - STM32F103C8T6");
    Serial.println("Praktikum Sistem Embedded");
    Serial.println("========================================\n");
    
    // Initialize SPI pins
    pinMode(SPI_SCK, OUTPUT);
    pinMode(SPI_MOSI, OUTPUT);
    pinMode(SPI_MISO, INPUT);
    pinMode(SPI_CS, OUTPUT);
    digitalWrite(SPI_CS, HIGH);
    
    // Initialize SPI
    SPI_1.begin();
    SPI_1.setClockDivider(SPI_CLOCK_DIV8);  // 72MHz / 8 = 9MHz
    SPI_1.setDataMode(SPI_MODE0);
    SPI_1.setBitOrder(MSBFIRST);
    
    Serial.println("SPI Configuration:");
    Serial.printf("  Clock: PA5 (SCK)\n");
    Serial.printf("  MOSI: PA7\n");
    Serial.printf("  MISO: PA6\n");
    Serial.printf("  CS: PA4\n");
    Serial.printf("  Speed: ~9 MHz (SPI_CLOCK_DIV8)\n");
    Serial.printf("  Mode: SPI_MODE0 (CPOL=0, CPHA=0)\n");
    Serial.println("\nLoopback Mode: MOSI and MISO connected");
    Serial.println("Starting SPI transmission test...\n");
    
    // Prepare test data
    for (int i = 0; i < BUFFER_SIZE; i++) {
        txBuffer[i] = i & 0xFF;
    }
}

// ==================== LOOP ====================
void loop() {
    testCount++;
    
    // Clear RX buffer
    memset(rxBuffer, 0, BUFFER_SIZE);
    
    Serial.printf("\n--- Test #%lu ---\n", testCount);
    Serial.println("Sending SPI Loopback Test...");
    
    // SPI Loopback Transfer
    digitalWrite(SPI_CS, LOW);
    delay(1);
    
    // Send data via SPI
    for (int i = 0; i < 16; i++) {
        rxBuffer[i] = SPI_1.transfer(txBuffer[i]);
    }
    
    delay(1);
    digitalWrite(SPI_CS, HIGH);
    
    // Display results
    Serial.println("TX Data: ");
    for (int i = 0; i < 16; i++) {
        Serial.printf("%02X ", txBuffer[i]);
    }
    Serial.println();
    
    Serial.println("RX Data: ");
    for (int i = 0; i < 16; i++) {
        Serial.printf("%02X ", rxBuffer[i]);
    }
    Serial.println();
    
    // Check if loopback successful
    bool match = true;
    for (int i = 0; i < 16; i++) {
        if (txBuffer[i] != rxBuffer[i]) {
            match = false;
            break;
        }
    }
    
    if (match) {
        Serial.println("✓ Loopback TEST PASSED");
    } else {
        Serial.println("✗ Loopback TEST FAILED");
    }
    
    delay(2000);
}

/**
 * WIRING DIAGRAM:
 * 
 *   STM32F103C8T6 Blue Pill
 *   ┌─────────────────────┐
 *   │                     │
 *   │  PA5 (SCK)  ●       │
 *   │                     │
 *   │  PA7 (MOSI) ●───────┤─── To PA6 (MISO) Loopback
 *   │                     │
 *   │  PA6 (MISO) ●───────┤─── From PA7 (MOSI)
 *   │                     │
 *   │  PA4 (CS)   ●       │
 *   │                     │
 *   │   3.3V ●────────────│ Power
 *   │    GND ●────────────│ Ground
 *   │                     │
 *   └─────────────────────┘
 * 
 */
