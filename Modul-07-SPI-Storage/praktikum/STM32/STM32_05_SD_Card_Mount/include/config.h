/**
 * @file config.h
 * @brief Konfigurasi Pin STM32F103C8T6 - Program 05 SD Card Mount
 */

#ifndef CONFIG_H
#define CONFIG_H

// ==================== SPI PIN DEFINITIONS ====================
#define SPI_SCK     PA5     // SPI Clock
#define SPI_MOSI    PA7     // Master Out Slave In
#define SPI_MISO    PA6     // Master In Slave Out
#define SD_CS       PB0     // SD Card Chip Select

// ==================== SERIAL ====================
#define SERIAL_BAUD 115200

// ==================== STATUS LED ====================
#define STATUS_LED  PC13    // Built-in LED (Active LOW)

#endif // CONFIG_H
