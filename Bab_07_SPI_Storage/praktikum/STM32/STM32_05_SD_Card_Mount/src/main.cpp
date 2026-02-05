/**
 * @file main.cpp
 * @brief Program 05: SD Card Mount - STM32F103C8T6 Blue Pill
 * 
 * Deskripsi:
 * Program untuk menginisialisasi dan mount SD Card via SPI interface.
 * 
 * Hardware:
 * - STM32F103C8T6 Blue Pill
 * - SD Card Module (SPI interface)
 * - SD Card (microSD, SDHC, atau SDXC)
 * 
 * SD Card Pinout (Typical):
 * - CS: PB0
 * - CLK: PA5
 * - MOSI (DI): PA7
 * - MISO (DO): PA6
 * 
 * Wiring SD Card Module:
 * - VCC  → 3.3V
 * - GND  → GND
 * - MOSI → PA7
 * - MISO → PA6
 * - SCK  → PA5
 * - CS   → PB0
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include "config.h"
#include "SdFat.h"

// SD Card object
SdFat sd;

// ==================== GLOBAL VARIABLES ====================
uint32_t mountAttempt = 0;
bool sdInitialized = false;

// ==================== FUNCTION PROTOTYPES ====================
void blinkLED(int times, int delayMs);
void displaySDCardInfo();

// ==================== SETUP ====================
void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(2000);
    
    Serial.println("\n========================================");
    Serial.println("Program 05: SD Card Mount - STM32F103C8T6");
    Serial.println("Praktikum Sistem Embedded");
    Serial.println("========================================\n");
    
    // Initialize LED
    pinMode(STATUS_LED, OUTPUT);
    digitalWrite(STATUS_LED, HIGH);  // LED OFF (active LOW)
    
    Serial.println("SD Card Configuration:");
    Serial.printf("  CS Pin: PB0\n");
    Serial.printf("  Clock: PA5 (SCK)\n");
    Serial.printf("  MOSI: PA7\n");
    Serial.printf("  MISO: PA6\n");
    Serial.printf("  SPI Speed: ~18 MHz\n");
    
    Serial.println("\nAttempting to initialize SD Card...\n");
    delay(1000);
}

// ==================== LOOP ====================
void loop() {
    mountAttempt++;
    
    Serial.printf("\n========== Mount Attempt #%lu ==========\n", mountAttempt);
    
    // Try to initialize SD card
    if (!sdInitialized) {
        Serial.println("Initializing SD Card...");
        
        // Begin SD card with CS on PB0
        if (sd.begin(SD_CS, SPI_FULL_SPEED)) {
            Serial.println("✓ SD Card Initialized Successfully!");
            sdInitialized = true;
            
            // Blink LED to indicate success
            blinkLED(3, 200);
            
            // Display card information
            displaySDCardInfo();
        } else {
            Serial.println("✗ Failed to Initialize SD Card!");
            Serial.println("Troubleshooting:");
            Serial.println("  1. Check SD card wiring");
            Serial.println("  2. Verify SD card is formatted");
            Serial.println("  3. Try reinserting SD card");
            Serial.println("  4. Check if SD card is supported (SDHC/SDXC)");
            
            // Blink LED to indicate error
            blinkLED(5, 100);
        }
    } else {
        // SD Card already initialized, just show info
        Serial.println("SD Card is already mounted.");
        displaySDCardInfo();
    }
    
    delay(3000);
}

// ==================== FUNCTION IMPLEMENTATIONS ====================
void blinkLED(int times, int delayMs) {
    for (int i = 0; i < times; i++) {
        digitalWrite(STATUS_LED, LOW);   // ON
        delay(delayMs);
        digitalWrite(STATUS_LED, HIGH);  // OFF
        delay(delayMs);
    }
}

void displaySDCardInfo() {
    if (!sd.card() || !sd.vol()) {
        Serial.println("SD Card not accessible!");
        return;
    }
    
    Serial.println("\nSD Card Information:");
    
    // Card capacity
    uint64_t cardSize = (uint64_t)sd.card()->cardCapacity();
    Serial.printf("  Card Size: ");
    if (cardSize < 1024000) {
        Serial.printf("%.2f KB\n", cardSize / 1024.0);
    } else if (cardSize < 1024000000) {
        Serial.printf("%.2f MB\n", cardSize / 1024000.0);
    } else {
        Serial.printf("%.2f GB\n", cardSize / 1024000000.0);
    }
    
    // Volume info
    Serial.printf("  Cluster Count: %u\n", sd.vol()->clusterCount());
    Serial.printf("  Blocks per Cluster: %u\n", sd.vol()->blocksPerCluster());
    Serial.printf("  Total Blocks: %u\n", sd.vol()->blockCount());
    
    // File system info
    uint32_t freeClusters = sd.vol()->freeClusterCount();
    uint32_t totalClusters = sd.vol()->clusterCount();
    uint8_t percentUsed = 100 * (1 - (float)freeClusters / totalClusters);
    
    Serial.printf("  Free Clusters: %u / %u\n", freeClusters, totalClusters);
    Serial.printf("  Space Usage: %u%%\n", percentUsed);
    
    // Root directory
    Serial.println("\nRoot Directory Contents:");
    File root = sd.open("/");
    int fileCount = 0;
    
    while (File entry = root.openNextFile()) {
        if (fileCount < 10) {  // Show first 10 files
            if (entry.isDirectory()) {
                Serial.printf("  [DIR]  %s/\n", entry.name());
            } else {
                Serial.printf("  [FILE] %s (%ld bytes)\n", entry.name(), entry.size());
            }
        }
        fileCount++;
        entry.close();
    }
    
    if (fileCount > 10) {
        Serial.printf("  ... and %d more items\n", fileCount - 10);
    }
    Serial.printf("Total items in root: %d\n", fileCount);
    
    root.close();
}

/**
 * SD CARD INITIALIZATION SEQUENCE:
 * 
 * ┌─────────────────────────────────────────┐
 * │ SD Card Power-Up & Initialization       │
 * ├─────────────────────────────────────────┤
 * │                                         │
 * │ 1. Power On (3.3V)                      │
 * │    └─ Wait 1ms (Power stabilization)    │
 * │                                         │
 * │ 2. Clock Stabilization                  │
 * │    └─ CS = HIGH, Send 74 clock pulses   │
 * │    └─ Frequency: 100-400 kHz            │
 * │                                         │
 * │ 3. CMD0 (Reset)                         │
 * │    └─ Initialize card to SPI mode       │
 * │                                         │
 * │ 4. CMD8 (Interface Condition)           │
 * │    └─ Check voltage support             │
 * │                                         │
 * │ 5. ACMD41 (App Command)                 │
 * │    └─ Wait for card ready               │
 * │                                         │
 * │ 6. CMD58 (Read OCR)                     │
 * │    └─ Confirm card ready                │
 * │                                         │
 * │ 7. Speed Up SPI Clock                   │
 * │    └─ Frequency: 16-25 MHz              │
 * │                                         │
 * └─────────────────────────────────────────┘
 * 
 */
