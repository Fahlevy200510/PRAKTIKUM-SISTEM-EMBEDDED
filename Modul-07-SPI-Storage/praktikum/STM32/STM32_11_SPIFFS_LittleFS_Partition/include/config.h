/**
 * @file config.h
 * @brief Konfigurasi Pin STM32F103C8T6 - Program 11 SPIFFS/LittleFS Partition
 */

#ifndef CONFIG_H
#define CONFIG_H

// ==================== SERIAL ====================
#define SERIAL_BAUD 115200

// ==================== STORAGE CONFIG ====================
// SPIFFS: SPI Flash File System
// LittleFS: Little File System (more efficient)

#define STORAGE_TYPE    "LittleFS"   // or "SPIFFS"
#define PARTITION_SIZE  1048576      // 1 MB
#define BLOCK_SIZE      4096         // 4 KB blocks
#define PAGE_SIZE       256          // 256 bytes

#endif // CONFIG_H
