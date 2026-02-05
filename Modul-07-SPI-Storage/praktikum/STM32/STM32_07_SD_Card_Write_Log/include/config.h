/**
 * @file config.h
 * @brief Konfigurasi Pin STM32F103C8T6 - Program 07 SD Card Write Log
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

// ==================== LOG CONFIG ====================
#define LOG_FILENAME    "LOG.TXT"
#define LOG_INTERVAL_MS 2000

// ==================== ADC ====================
#define ADC_PIN     PA0     // Analog input

#endif // CONFIG_H
