/**
 * @file main.cpp
 * @brief Program 02: SPI Clock Modes - STM32F103C8T6 Blue Pill
 * 
 * Deskripsi:
 * Program untuk menguji berbagai mode clock SPI (MODE 0-3).
 * Setiap mode memiliki kombinasi CPOL dan CPHA yang berbeda.
 * 
 * Hardware:
 * - STM32F103C8T6 Blue Pill
 * - Oscilloscope (untuk menganalisis clock waveform)
 * 
 * SPI Modes:
 * - MODE 0: CPOL=0, CPHA=0 (Default, most common)
 * - MODE 1: CPOL=0, CPHA=1
 * - MODE 2: CPOL=1, CPHA=0
 * - MODE 3: CPOL=1, CPHA=1
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include "config.h"

SPIClass SPI_1(1);  // Use SPI1

// ==================== GLOBAL VARIABLES ====================
uint8_t testData[TEST_SIZE];
uint32_t modeCounter = 0;

// ==================== FUNCTION PROTOTYPES ====================
void testSPIMode(uint8_t mode, const char* modeName);
void displayClockInfo(uint8_t mode);

// ==================== SETUP ====================
void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(2000);
    
    Serial.println("\n========================================");
    Serial.println("Program 02: SPI Clock Modes - STM32F103C8T6");
    Serial.println("Praktikum Sistem Embedded");
    Serial.println("========================================\n");
    
    // Initialize SPI pins
    pinMode(SPI_SCK, OUTPUT);
    pinMode(SPI_MOSI, OUTPUT);
    pinMode(SPI_MISO, INPUT);
    pinMode(SPI_CS, OUTPUT);
    digitalWrite(SPI_CS, HIGH);
    
    Serial.println("SPI Pin Configuration:");
    Serial.printf("  Clock: PA5 (SCK)\n");
    Serial.printf("  MOSI: PA7\n");
    Serial.printf("  MISO: PA6\n");
    Serial.printf("  CS: PA4\n");
    Serial.println("\nSPI Mode Definitions:");
    Serial.println("  MODE 0: CPOL=0, CPHA=0 - Idle Low, Sample Leading Edge");
    Serial.println("  MODE 1: CPOL=0, CPHA=1 - Idle Low, Sample Trailing Edge");
    Serial.println("  MODE 2: CPOL=1, CPHA=0 - Idle High, Sample Trailing Edge");
    Serial.println("  MODE 3: CPOL=1, CPHA=1 - Idle High, Sample Leading Edge");
    Serial.println("\nStarting SPI Mode Test...\n");
    
    // Prepare test data
    for (int i = 0; i < TEST_SIZE; i++) {
        testData[i] = (i * 17) & 0xFF;  // Pattern: 0, 17, 34, 51, ...
    }
}

// ==================== LOOP ====================
void loop() {
    modeCounter++;
    
    Serial.printf("\n========== Test Cycle #%lu ==========\n\n", modeCounter);
    
    // Test all 4 SPI modes
    testSPIMode(SPI_MODE0, "MODE 0 (CPOL=0, CPHA=0)");
    delay(1000);
    
    testSPIMode(SPI_MODE1, "MODE 1 (CPOL=0, CPHA=1)");
    delay(1000);
    
    testSPIMode(SPI_MODE2, "MODE 2 (CPOL=1, CPHA=0)");
    delay(1000);
    
    testSPIMode(SPI_MODE3, "MODE 3 (CPOL=1, CPHA=1)");
    delay(2000);
}

// ==================== FUNCTION IMPLEMENTATIONS ====================
void testSPIMode(uint8_t mode, const char* modeName) {
    Serial.printf("Testing %s\n", modeName);
    
    // Configure SPI
    SPI_1.begin();
    SPI_1.setDataMode(mode);
    SPI_1.setClockDivider(SPI_CLOCK_DIV16);  // 72MHz / 16 = 4.5MHz
    SPI_1.setBitOrder(MSBFIRST);
    
    // Display mode info
    displayClockInfo(mode);
    
    // Send test data
    digitalWrite(SPI_CS, LOW);
    delayMicroseconds(10);
    
    for (int i = 0; i < 8; i++) {
        uint8_t rxByte = SPI_1.transfer(testData[i]);
        Serial.printf("  [%d] TX: 0x%02X | RX: 0x%02X\n", i, testData[i], rxByte);
    }
    
    delayMicroseconds(10);
    digitalWrite(SPI_CS, HIGH);
    
    SPI_1.end();
    Serial.println();
}

void displayClockInfo(uint8_t mode) {
    const char* cpol = (mode & SPI_CPOL_MASK) ? "HIGH" : "LOW";
    const char* cpha = (mode & SPI_CPHA_MASK) ? "Trailing" : "Leading";
    
    Serial.printf("  Clock Polarity (CPOL): %s\n", cpol);
    Serial.printf("  Clock Phase (CPHA): %s Edge\n", cpha);
    Serial.printf("  Speed: ~4.5 MHz (SPI_CLOCK_DIV16)\n");
    Serial.println("  Transmission Data:");
}

/**
 * WIRING DIAGRAM & TIMING:
 * 
 * ┌─────────────────────────────────────────────┐
 * │ SPI MODE TIMING DIAGRAMS                    │
 * ├─────────────────────────────────────────────┤
 * │                                             │
 * │ MODE 0 (CPOL=0, CPHA=0):                   │
 * │ CLK: ────╮   ╭───╮   ╭───╮   ╭───╮        │
 * │          ╰───╯   ╰───╯   ╰───╯            │
 * │ MOSI: ───╦═════╬═════╬═════╬─────          │
 * │          ┃ D7  ┃ D6  ┃ D5  ┃ D4           │
 * │                                             │
 * │ MODE 1 (CPOL=0, CPHA=1):                   │
 * │ CLK: ──╮   ╭───╮   ╭───╮   ╭──             │
 * │        ╰───╯   ╰───╯   ╰───╯              │
 * │ MOSI: ─╦═════╬═════╬═════╬──────           │
 * │        ┃ D7  ┃ D6  ┃ D5  ┃ D4              │
 * │                                             │
 * │ MODE 2 (CPOL=1, CPHA=0):                   │
 * │ CLK: ───╯   ╰───╯   ╰───╯   ╰─            │
 * │       ╭───╮   ╭───╮   ╭───╮               │
 * │ MOSI: ─╦═════╬═════╬═════╬──────           │
 * │        ┃ D7  ┃ D6  ┃ D5  ┃ D4              │
 * │                                             │
 * │ MODE 3 (CPOL=1, CPHA=1):                   │
 * │ CLK: ──╯   ╰───╯   ╰───╯   ╰──             │
 * │       ╭───╮   ╭───╮   ╭───╮                │
 * │ MOSI: ─╦═════╬═════╬═════╬──────           │
 * │        ┃ D7  ┃ D6  ┃ D5  ┃ D4              │
 * │                                             │
 * └─────────────────────────────────────────────┘
 * 
 */
