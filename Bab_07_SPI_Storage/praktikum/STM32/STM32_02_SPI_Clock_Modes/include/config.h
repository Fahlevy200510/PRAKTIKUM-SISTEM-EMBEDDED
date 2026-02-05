/**
 * @file config.h
 * @brief Konfigurasi Pin STM32F103C8T6 - Program 02 SPI Clock Modes
 */

#ifndef CONFIG_H
#define CONFIG_H

// ==================== SPI PIN DEFINITIONS ====================
#define SPI_SCK     PA5     // SPI Clock
#define SPI_MOSI    PA7     // Master Out Slave In
#define SPI_MISO    PA6     // Master In Slave Out
#define SPI_CS      PA4     // Chip Select

// ==================== SERIAL ====================
#define SERIAL_BAUD 115200

// ==================== SPI CONFIG ====================
#define TEST_SIZE   64

#endif // CONFIG_H
