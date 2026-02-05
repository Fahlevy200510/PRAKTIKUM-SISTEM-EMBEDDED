/**
 * @file config.h
 * @brief Konfigurasi Pin STM32F103C8T6 - Program 06 SD Card Read
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

// ==================== FILE CONFIG ====================
#define TEST_FILENAME   "TEST.TXT"
#define READ_CHUNK_SIZE 256

#endif // CONFIG_H
