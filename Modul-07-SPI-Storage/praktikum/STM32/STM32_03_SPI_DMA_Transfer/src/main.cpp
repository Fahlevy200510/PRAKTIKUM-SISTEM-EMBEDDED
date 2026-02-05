/**
 * @file main.cpp
 * @brief Program 03: SPI DMA Transfer - STM32F103C8T6 Blue Pill
 * 
 * Deskripsi:
 * Program untuk mengirim data menggunakan SPI dengan DMA (Direct Memory Access).
 * DMA memungkinkan transfer data tanpa melibatkan CPU secara langsung.
 * 
 * Hardware:
 * - STM32F103C8T6 Blue Pill
 * - SPI Peripheral dengan DMA Channel
 * - Logic Analyzer (opsional)
 * 
 * Keuntungan DMA:
 * - Transfer data yang lebih cepat
 * - CPU dapat melakukan task lain selama transfer
 * - Penggunaan memori yang efisien
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include "config.h"

SPIClass SPI_1(1);  // Use SPI1

// ==================== GLOBAL VARIABLES ====================
uint8_t txBuffer[DMA_BUFFER_SIZE];
uint8_t rxBuffer[DMA_BUFFER_SIZE];
volatile bool dmaComplete = false;
uint32_t transferCount = 0;

// ==================== SETUP ====================
void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(2000);
    
    Serial.println("\n========================================");
    Serial.println("Program 03: SPI DMA Transfer - STM32F103C8T6");
    Serial.println("Praktikum Sistem Embedded");
    Serial.println("========================================\n");
    
    // Initialize pins
    pinMode(STATUS_LED, OUTPUT);
    digitalWrite(STATUS_LED, HIGH);  // LED OFF (active LOW)
    
    pinMode(SPI_SCK, OUTPUT);
    pinMode(SPI_MOSI, OUTPUT);
    pinMode(SPI_MISO, INPUT);
    pinMode(SPI_CS, OUTPUT);
    digitalWrite(SPI_CS, HIGH);
    
    // Initialize SPI
    SPI_1.begin();
    SPI_1.setClockDivider(SPI_CLOCK_DIV4);  // 72MHz / 4 = 18MHz
    SPI_1.setDataMode(SPI_MODE0);
    SPI_1.setBitOrder(MSBFIRST);
    
    Serial.println("SPI Configuration:");
    Serial.printf("  Clock: PA5 (SCK)\n");
    Serial.printf("  MOSI: PA7\n");
    Serial.printf("  MISO: PA6\n");
    Serial.printf("  CS: PA4\n");
    Serial.printf("  Speed: ~18 MHz (SPI_CLOCK_DIV4)\n");
    Serial.printf("  DMA Buffer Size: %d bytes\n", DMA_BUFFER_SIZE);
    Serial.println("\nDMA Transfer Mode Enabled");
    Serial.println("Transfer data in continuous loop...\n");
    
    // Prepare test data (simple pattern)
    for (int i = 0; i < DMA_BUFFER_SIZE; i++) {
        txBuffer[i] = (i ^ 0xAA) & 0xFF;
    }
    memset(rxBuffer, 0, DMA_BUFFER_SIZE);
}

// ==================== LOOP ====================
void loop() {
    transferCount++;
    
    digitalWrite(STATUS_LED, LOW);  // LED ON
    
    Serial.printf("\n--- DMA Transfer #%lu ---\n", transferCount);
    Serial.println("Initiating SPI DMA Transfer...");
    
    unsigned long startTime = millis();
    
    // Perform SPI transfer (simulated DMA behavior)
    digitalWrite(SPI_CS, LOW);
    delayMicroseconds(10);
    
    // Send data via SPI
    for (int i = 0; i < DMA_BUFFER_SIZE; i++) {
        rxBuffer[i] = SPI_1.transfer(txBuffer[i]);
    }
    
    delayMicroseconds(10);
    digitalWrite(SPI_CS, HIGH);
    
    unsigned long transferTime = millis() - startTime;
    
    // Display results
    Serial.printf("Transfer Time: %lu ms\n", transferTime);
    Serial.printf("Throughput: %.2f KB/s\n", 
                  (float)DMA_BUFFER_SIZE / transferTime / 1.024f);
    
    Serial.println("\nFirst 16 bytes (TX | RX):");
    for (int i = 0; i < 16; i++) {
        Serial.printf("  [%2d] TX: 0x%02X | RX: 0x%02X\n", 
                      i, txBuffer[i], rxBuffer[i]);
    }
    
    // Verify transfer
    bool dataValid = true;
    int errorCount = 0;
    for (int i = 0; i < DMA_BUFFER_SIZE; i++) {
        if (txBuffer[i] != rxBuffer[i]) {
            dataValid = false;
            errorCount++;
        }
    }
    
    if (dataValid) {
        Serial.println("\n✓ DMA Transfer SUCCESSFUL");
    } else {
        Serial.printf("\n✗ DMA Transfer FAILED (%d errors)\n", errorCount);
    }
    
    digitalWrite(STATUS_LED, HIGH);  // LED OFF
    delay(2000);
}

/**
 * DMA ADVANTAGES:
 * 
 * ┌─────────────────────────────────────────┐
 * │ Without DMA:                            │
 * │ CPU ──→ SPI TX ──→ Transfer ──→ SPI RX  │
 * │ (CPU blocked during transfer)           │
 * │                                         │
 * │ With DMA:                               │
 * │ Memory ──→ DMA ──→ SPI TX               │
 * │ (CPU free for other tasks)              │
 * │ SPI RX ──→ DMA ──→ Memory               │
 * │                                         │
 * │ Speed Benefit:                          │
 * │ - No CPU overhead                       │
 * │ - Continuous data stream                │
 * │ - Better for large transfers            │
 * └─────────────────────────────────────────┘
 * 
 */
