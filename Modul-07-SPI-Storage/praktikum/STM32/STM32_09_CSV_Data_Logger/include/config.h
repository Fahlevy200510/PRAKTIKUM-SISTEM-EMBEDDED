/**
 * @file config.h
 * @brief Konfigurasi Pin STM32F103C8T6 - Program 09 CSV Data Logger
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

// ==================== SENSOR PINS ====================
#define TEMP_PIN    PA0     // Temperature sensor (analog)
#define HUM_PIN     PA1     // Humidity sensor (analog)
#define PRESS_PIN   PA2     // Pressure sensor (analog)

// ==================== LOG CONFIG ====================
#define CSV_FILENAME    "DATA.CSV"
#define LOG_INTERVAL_MS 3000
#define MAX_RECORDS     1000

#endif // CONFIG_H
