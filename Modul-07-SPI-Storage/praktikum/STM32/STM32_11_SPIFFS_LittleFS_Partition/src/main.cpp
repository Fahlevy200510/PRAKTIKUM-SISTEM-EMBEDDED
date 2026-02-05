/**
 * @file main.cpp
 * @brief Program 11: SPIFFS/LittleFS Partition - STM32F103C8T6 Blue Pill
 * 
 * Deskripsi:
 * Program untuk menggunakan file system di internal flash memory atau eksternal SPI flash.
 * SPIFFS = SPI Flash File System
 * LittleFS = Little File System (lebih modern dan efisien)
 * 
 * Keuntungan File System:
 * - Abstraksi dari hardware
 * - Automatic wear leveling
 * - File organization
 * - Directory support
 * - Safe deletion
 * 
 * Hardware:
 * - STM32F103C8T6 Blue Pill
 * - External SPI Flash (W25Q32, W25Q64, dll) - opsional
 * 
 * Partisi Flash:
 * ┌─────────────────────────────┐
 * │ Bootloader (0x0000-0x3FFF)  │ 16 KB
 * │ Firmware (0x4000-0xBFFF)    │ 32 KB
 * │ SPIFFS (0xC000-0xFFFFF)     │ 16 KB (Blue Pill: 64KB)
 * └─────────────────────────────┘
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include "config.h"

// Pseudo-implementation untuk demonstrasi
// Dalam proyek nyata, gunakan library seperti LittleFS atau SPIFFS

// ==================== GLOBAL VARIABLES ====================
uint32_t operationCount = 0;
struct FileEntry {
    char filename[32];
    uint32_t size;
    uint32_t timestamp;
};

FileEntry files[10];
uint32_t fileCount = 0;

// ==================== FUNCTION PROTOTYPES ====================
void initializeFileSystem();
void createTestFile(const char* filename, const char* content);
void listFiles();
void deleteFile(const char* filename);
void displayStorageStats();

// ==================== SETUP ====================
void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(2000);
    
    Serial.println("\n========================================");
    Serial.println("Program 11: SPIFFS/LittleFS - STM32F103C8T6");
    Serial.println("Praktikum Sistem Embedded");
    Serial.println("========================================\n");
    
    Serial.println("File System Configuration:");
    Serial.printf("  Type: %s\n", STORAGE_TYPE);
    Serial.printf("  Partition Size: %d bytes\n", PARTITION_SIZE);
    Serial.printf("  Block Size: %d bytes\n", BLOCK_SIZE);
    Serial.printf("  Page Size: %d bytes\n", PAGE_SIZE);
    
    Serial.println("\nInitializing file system...");
    initializeFileSystem();
    
    Serial.println("✓ File System Ready!\n");
}

// ==================== LOOP ====================
void loop() {
    operationCount++;
    
    Serial.printf("\n========== File System Operation #%lu ==========\n", operationCount);
    
    // Create test files
    Serial.println("\n1. Creating test files...");
    createTestFile("config.txt", "System Configuration\nVersion: 1.0\nDevice: STM32F103");
    createTestFile("log.txt", "System Log\nStartup: OK\nTemperature: 25C");
    createTestFile("data.bin", "Binary data file");
    delay(500);
    
    // List all files
    Serial.println("\n2. Listing all files...");
    listFiles();
    
    // Display storage info
    Serial.println("\n3. Storage Statistics:");
    displayStorageStats();
    
    // Optional: Delete oldest file
    if (fileCount > 5) {
        Serial.printf("\n4. Deleting oldest file: %s\n", files[0].filename);
        deleteFile(files[0].filename);
    }
    
    delay(3000);
}

// ==================== FUNCTION IMPLEMENTATIONS ====================
void initializeFileSystem() {
    // Pseudo-initialization
    fileCount = 0;
    Serial.println("Formatting file system...");
    delay(500);
    Serial.println("✓ File system formatted and mounted");
}

void createTestFile(const char* filename, const char* content) {
    // Simulate file creation
    if (fileCount < 10) {
        strcpy(files[fileCount].filename, filename);
        files[fileCount].size = strlen(content);
        files[fileCount].timestamp = millis();
        fileCount++;
        
        Serial.printf("  ✓ Created: %s (%d bytes)\n", filename, strlen(content));
    }
}

void listFiles() {
    Serial.println("─────────────────────────────────────────────");
    Serial.println("Filename          | Size      | Timestamp");
    Serial.println("─────────────────────────────────────────────");
    
    for (uint32_t i = 0; i < fileCount; i++) {
        Serial.printf("%-17s | %9d | %lu\n",
                     files[i].filename,
                     files[i].size,
                     files[i].timestamp);
    }
    
    Serial.println("─────────────────────────────────────────────");
    Serial.printf("Total files: %lu\n", fileCount);
}

void deleteFile(const char* filename) {
    for (uint32_t i = 0; i < fileCount; i++) {
        if (strcmp(files[i].filename, filename) == 0) {
            // Shift remaining files
            for (uint32_t j = i; j < fileCount - 1; j++) {
                strcpy(files[j].filename, files[j + 1].filename);
                files[j].size = files[j + 1].size;
                files[j].timestamp = files[j + 1].timestamp;
            }
            fileCount--;
            Serial.printf("  ✓ Deleted: %s\n", filename);
            return;
        }
    }
    Serial.printf("  ✗ File not found: %s\n", filename);
}

void displayStorageStats() {
    uint32_t totalSize = 0;
    for (uint32_t i = 0; i < fileCount; i++) {
        totalSize += files[i].size;
    }
    
    uint32_t usedPercent = (totalSize * 100) / PARTITION_SIZE;
    uint32_t freeSpace = PARTITION_SIZE - totalSize;
    
    Serial.printf("  Total Capacity: %d bytes\n", PARTITION_SIZE);
    Serial.printf("  Used Space: %d bytes (%d%%)\n", totalSize, usedPercent);
    Serial.printf("  Free Space: %d bytes\n", freeSpace);
    Serial.printf("  File Count: %lu\n", fileCount);
}

/**
 * COMPARISON: SPIFFS vs LittleFS
 * 
 * ┌──────────────────────┬──────────┬──────────┐
 * │ Feature              │ SPIFFS   │ LittleFS │
 * ├──────────────────────┼──────────┼──────────┤
 * │ Wear Leveling        │ Yes      │ Yes      │
 * │ Power Loss Safe      │ Partial  │ Yes      │
 * │ File Deletion        │ Slow     │ Fast     │
 * │ Garbage Collection   │ Manual   │ Auto     │
 * │ Memory Efficiency    │ 44 bytes │ 40 bytes │
 * │ Page Size            │ 256 B    │ 256 B    │
 * │ Block Size           │ 4 KB     │ 4 KB     │
 * │ Compatibility        │ High     │ Growing  │
 * │ Documentation        │ Good     │ Very Good│
 * │ Maintenance Status   │ Legacy   │ Active   │
 * └──────────────────────┴──────────┴──────────┘
 * 
 * RECOMMENDATION:
 * Use LittleFS for new projects (better performance & safety)
 * Use SPIFFS for legacy projects with existing partitions
 * 
 * Flash Wear Levels (Typical):
 * - Internal Flash: ~10,000 - 100,000 cycles
 * - SPI Flash (W25Q): ~100,000 cycles
 * - With wear leveling: +10x-100x lifetime
 * 
 */
