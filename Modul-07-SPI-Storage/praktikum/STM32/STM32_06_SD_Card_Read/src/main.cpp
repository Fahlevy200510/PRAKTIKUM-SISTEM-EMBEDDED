/**
 * @file main.cpp
 * @brief Program 06: SD Card Read - STM32F103C8T6 Blue Pill
 * 
 * Deskripsi:
 * Program untuk membaca file dari SD Card.
 * Membaca file text dan menampilkan isinya di serial monitor.
 * 
 * Hardware:
 * - STM32F103C8T6 Blue Pill
 * - SD Card Module (SPI)
 * - SD Card dengan file TEST.TXT
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include "config.h"
#include "SdFat.h"

SdFat sd;

// ==================== GLOBAL VARIABLES ====================
uint32_t readCount = 0;
bool sdReady = false;

// ==================== FUNCTION PROTOTYPES ====================
void initializeSD();
void readFile(const char* filename);
void listFiles();

// ==================== SETUP ====================
void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(2000);
    
    Serial.println("\n========================================");
    Serial.println("Program 06: SD Card Read - STM32F103C8T6");
    Serial.println("Praktikum Sistem Embedded");
    Serial.println("========================================\n");
    
    Serial.println("Initializing SD Card...");
    initializeSD();
    
    if (sdReady) {
        Serial.println("✓ SD Card Ready!\n");
        listFiles();
    }
}

// ==================== LOOP ====================
void loop() {
    if (!sdReady) {
        Serial.println("Retrying SD Card initialization...");
        delay(2000);
        initializeSD();
        return;
    }
    
    readCount++;
    
    Serial.printf("\n========== Read Cycle #%lu ==========\n", readCount);
    Serial.printf("Reading file: %s\n\n", TEST_FILENAME);
    
    readFile(TEST_FILENAME);
    
    delay(3000);
}

// ==================== FUNCTION IMPLEMENTATIONS ====================
void initializeSD() {
    if (sd.begin(SD_CS, SPI_FULL_SPEED)) {
        sdReady = true;
        Serial.println("SD Card initialized successfully.");
    } else {
        sdReady = false;
        Serial.println("Failed to initialize SD Card!");
    }
}

void readFile(const char* filename) {
    if (!sdReady) {
        Serial.println("SD Card not ready!");
        return;
    }
    
    File file = sd.open(filename, FILE_READ);
    
    if (!file) {
        Serial.printf("Error: Cannot open file '%s'\n", filename);
        Serial.println("Make sure the file exists on SD Card.\n");
        Serial.println("Creating a sample file...");
        
        // Create a sample file
        File newFile = sd.open(filename, FILE_WRITE);
        if (newFile) {
            newFile.println("=== SD Card Read Test ===");
            newFile.println("This is a test file created by STM32");
            newFile.printf("Created on test cycle #%lu\n", readCount);
            newFile.println("Praktikum Sistem Embedded");
            newFile.println("");
            newFile.println("Temperature: 25.5°C");
            newFile.println("Humidity: 60%");
            newFile.println("Pressure: 1013.25 hPa");
            newFile.close();
            Serial.println("Sample file created!\n");
        }
        return;
    }
    
    Serial.printf("File size: %lu bytes\n\n", file.fileSize());
    Serial.println("File Contents:");
    Serial.println("─────────────────────────────────────");
    
    // Read file in chunks
    char buffer[READ_CHUNK_SIZE];
    size_t bytesRead;
    size_t totalRead = 0;
    
    while ((bytesRead = file.read(buffer, READ_CHUNK_SIZE)) > 0) {
        for (size_t i = 0; i < bytesRead; i++) {
            Serial.write(buffer[i]);
        }
        totalRead += bytesRead;
    }
    
    Serial.println("\n─────────────────────────────────────");
    Serial.printf("Total bytes read: %u\n", totalRead);
    
    file.close();
}

void listFiles() {
    Serial.println("Files on SD Card:");
    Serial.println("─────────────────────────────────────");
    
    File root = sd.open("/");
    int count = 0;
    
    while (File entry = root.openNextFile()) {
        count++;
        if (entry.isDirectory()) {
            Serial.printf("[DIR]  %s/\n", entry.name());
        } else {
            Serial.printf("[FILE] %-20s %10lu bytes\n", 
                         entry.name(), entry.size());
        }
        entry.close();
    }
    
    Serial.println("─────────────────────────────────────");
    Serial.printf("Total: %d items\n\n", count);
    
    root.close();
}

/**
 * FILE OPERATIONS ON SD CARD:
 * 
 * ┌─────────────────────────────────────┐
 * │ Read Sequence:                      │
 * ├─────────────────────────────────────┤
 * │                                     │
 * │ 1. Initialize SD Card               │
 * │    └─ SPI begin, detect card        │
 * │                                     │
 * │ 2. Open File                        │
 * │    └─ Locate file in FAT table      │
 * │    └─ Get file size & first block   │
 * │                                     │
 * │ 3. Read Data                        │
 * │    └─ Read blocks sequentially      │
 * │    └─ Buffer data in RAM            │
 * │                                     │
 * │ 4. Close File                       │
 * │    └─ Release file handle           │
 * │                                     │
 * │ Supported File Systems:             │
 * │ - FAT12 (Floppy disks)              │
 * │ - FAT16 (USB drives, old SD cards)  │
 * │ - FAT32 (Most SD cards)             │
 * │ - exFAT (SDXC cards > 32GB)         │
 * │                                     │
 * └─────────────────────────────────────┘
 * 
 */
