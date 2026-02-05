/**
 * @file main.cpp
 * @brief Program 08: SD Card List Directory - STM32F103C8T6 Blue Pill
 * 
 * Deskripsi:
 * Program untuk menampilkan struktur direktori SD Card secara rekursif.
 * Menampilkan nama file, ukuran, dan struktur folder dengan indentasi.
 * 
 * Hardware:
 * - STM32F103C8T6 Blue Pill
 * - SD Card Module (SPI)
 * - SD Card dengan struktur folder
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include "config.h"
#include "SdFat.h"

SdFat sd;

// ==================== GLOBAL VARIABLES ====================
uint32_t listCount = 0;
bool sdReady = false;
uint32_t totalFiles = 0;
uint32_t totalDirs = 0;
uint64_t totalSize = 0;

// ==================== FUNCTION PROTOTYPES ====================
void initializeSD();
void listDirectory(File dir, uint8_t depth);
void displayStorageInfo();

// ==================== SETUP ====================
void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(2000);
    
    Serial.println("\n========================================");
    Serial.println("Program 08: SD Card List Directory - STM32F103C8T6");
    Serial.println("Praktikum Sistem Embedded");
    Serial.println("========================================\n");
    
    Serial.println("Initializing SD Card...");
    initializeSD();
    
    if (sdReady) {
        Serial.println("✓ SD Card Ready!\n");
    }
}

// ==================== LOOP ====================
void loop() {
    if (!sdReady) {
        delay(1000);
        initializeSD();
        return;
    }
    
    listCount++;
    
    Serial.printf("\n========== Directory Listing #%lu ==========\n\n", listCount);
    
    // Reset counters
    totalFiles = 0;
    totalDirs = 0;
    totalSize = 0;
    
    // Display storage info
    displayStorageInfo();
    
    // List root directory recursively
    Serial.println("\nDirectory Structure:");
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    
    File root = sd.open("/");
    listDirectory(root, 0);
    root.close();
    
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    Serial.printf("\nSummary:");
    Serial.printf("  Total Directories: %lu\n", totalDirs);
    Serial.printf("  Total Files: %lu\n", totalFiles);
    Serial.printf("  Total Size: %.2f MB\n", totalSize / 1024.0 / 1024.0);
    
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

void listDirectory(File dir, uint8_t depth) {
    if (depth > MAX_DEPTH) {
        return;  // Prevent too deep recursion
    }
    
    File entry;
    uint32_t entryCount = 0;
    
    while (entry = dir.openNextFile()) {
        if (entryCount >= MAX_FILES) {
            Serial.println("  ... (max files limit reached)");
            break;
        }
        
        // Create indentation
        for (uint8_t i = 0; i < depth; i++) {
            Serial.print("  ");
        }
        
        if (entry.isDirectory()) {
            totalDirs++;
            Serial.printf("📁 %s/\n", entry.name());
            
            // Recursively list subdirectory
            listDirectory(entry, depth + 1);
        } else {
            totalFiles++;
            uint32_t fileSize = entry.size();
            totalSize += fileSize;
            
            // Format file size
            const char* sizeUnit = "B";
            float displaySize = fileSize;
            
            if (fileSize >= 1024 * 1024) {
                displaySize = fileSize / (1024.0 * 1024.0);
                sizeUnit = "MB";
            } else if (fileSize >= 1024) {
                displaySize = fileSize / 1024.0;
                sizeUnit = "KB";
            }
            
            Serial.printf("📄 %-30s %10.2f %s\n", 
                         entry.name(), displaySize, sizeUnit);
        }
        
        entry.close();
        entryCount++;
    }
}

void displayStorageInfo() {
    if (!sd.card() || !sd.vol()) {
        Serial.println("SD Card not accessible!");
        return;
    }
    
    Serial.println("SD Card Storage Information:");
    Serial.println("─────────────────────────────────────");
    
    // Total capacity
    uint64_t totalCapacity = (uint64_t)sd.card()->cardCapacity();
    
    // Cluster info
    uint32_t clusterCount = sd.vol()->clusterCount();
    uint32_t freeClusterCount = sd.vol()->freeClusterCount();
    uint32_t usedClusterCount = clusterCount - freeClusterCount;
    uint32_t blocksPerCluster = sd.vol()->blocksPerCluster();
    
    // Calculate used space
    uint64_t usedSpace = (uint64_t)usedClusterCount * blocksPerCluster * 512;
    uint64_t freeSpace = (uint64_t)freeClusterCount * blocksPerCluster * 512;
    
    Serial.printf("Total Capacity: %.2f MB\n", totalCapacity / 1024.0 / 1024.0);
    Serial.printf("Used Space: %.2f MB\n", usedSpace / 1024.0 / 1024.0);
    Serial.printf("Free Space: %.2f MB\n", freeSpace / 1024.0 / 1024.0);
    
    uint8_t usagePercent = (usedSpace * 100) / totalCapacity;
    Serial.printf("Usage: %u%%\n", usagePercent);
    
    Serial.printf("Total Clusters: %u\n", clusterCount);
    Serial.printf("Free Clusters: %u\n", freeClusterCount);
    
    Serial.println("─────────────────────────────────────");
}

/**
 * FILE SYSTEM CONCEPTS:
 * 
 * ┌────────────────────────────────────────────┐
 * │ FAT32 File System Structure                │
 * ├────────────────────────────────────────────┤
 * │                                            │
 * │ Boot Sector (512 bytes)                    │
 * │  └─ File system info, sector size, etc.    │
 * │                                            │
 * │ FAT (File Allocation Table)                │
 * │  └─ Maps cluster chains                    │
 * │  └─ Tracks free clusters                   │
 * │                                            │
 * │ Root Directory                             │
 * │  └─ Lists root level files/folders         │
 * │  └─ File names, sizes, dates               │
 * │                                            │
 * │ Data Area (Clusters)                       │
 * │  └─ Actual file/folder data                │
 * │  └─ Cluster size: typically 4KB-32KB       │
 * │                                            │
 * │ Directory Entry Format (32 bytes):         │
 * │  ├─ Filename (8.3 format): 11 bytes        │
 * │  ├─ Attributes: 1 byte                     │
 * │  ├─ Reserved: 8 bytes                      │
 * │  ├─ Created/Modified time: 6 bytes         │
 * │  ├─ First cluster: 2 bytes (high)          │
 * │  ├─ File size: 4 bytes                     │
 * │  └─ First cluster: 2 bytes (low)           │
 * │                                            │
 * │ Cluster Chain Example:                     │
 * │ File = Cluster[100] → 101 → 102 → EOF     │
 * │                                            │
 * └────────────────────────────────────────────┘
 * 
 */
