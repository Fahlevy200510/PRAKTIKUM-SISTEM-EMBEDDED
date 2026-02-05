/**
 * @file config.h
 * @brief Konfigurasi Pin STM32F103C8T6 - Program 12 NVS Key-Value Storage
 */

#ifndef CONFIG_H
#define CONFIG_H

// ==================== SERIAL ====================
#define SERIAL_BAUD 115200

// ==================== NVS CONFIG ====================
// NVS = Non-Volatile Storage (Key-Value store)
// Namespace untuk grouping data

#define NAMESPACE_DEVICE    "device"
#define NAMESPACE_SENSOR    "sensor"
#define NAMESPACE_CONFIG    "config"

// ==================== EEPROM EMULATION ====================
#define EEPROM_SIZE         256     // Bytes (emulated in flash)
#define EEPROM_START_ADDR   0x08007800  // Last 2KB of STM32F103C8T6

#endif // CONFIG_H
