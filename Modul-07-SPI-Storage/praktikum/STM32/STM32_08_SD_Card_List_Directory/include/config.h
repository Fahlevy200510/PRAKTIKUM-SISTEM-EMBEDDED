/**
 * @file config.h
 * @brief Konfigurasi Pin STM32F103C8T6 - Program 08 SD Card List Directory
 */

#ifndef CONFIG_H
#define CONFIG_H

// ==================== SPI PIN DEFINITIONS ====================
#define SPI_SCK     PA5
#define SPI_MOSI    PA7
#define SPI_MISO    PA6
#define SD_CS       PB0

// ==================== SERIAL ====================
#define SERIAL_BAUD 115200

// ==================== DIRECTORY CONFIG ====================
#define MAX_DEPTH   3
#define MAX_FILES   100

#endif // CONFIG_H
