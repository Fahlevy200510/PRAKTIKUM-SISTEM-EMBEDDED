/**
 * @file config.h
 * @brief Konfigurasi STM32F103C8T6 - Program 01: I2C_Bus_Scan
 * 
 * I2C Configuration untuk STM32F103C8T6 Blue Pill
 * I2C1: PB6=SCL, PB7=SDA
 */

#ifndef CONFIG_H
#define CONFIG_H

// ==================== I2C CONFIGURATION ====================
#define I2C_SPEED_HZ        400000      // I2C Clock: 400kHz (Fast Mode)
#define I2C_SDA_PIN         PB7         // I2C1 SDA
#define I2C_SCL_PIN         PB6         // I2C1 SCL

// ==================== COMMON I2C ADDRESSES ====================
// BME280/BMP280
#define BME280_ADDR         0x76        // If SDO=GND, otherwise 0x77

// OLED SSD1306
#define SSD1306_ADDR        0x3C        // If SA0=GND, otherwise 0x3D
#define SSD1306_WIDTH       128
#define SSD1306_HEIGHT      64

// RTC DS3231
#define DS3231_ADDR         0x68

// EEPROM 24LC
#define EEPROM_24LC_ADDR    0x50        // A0=A1=A2=GND, can be 0x50-0x57
#define EEPROM_PAGE_SIZE    64          // 64 bytes for 24LC256/512
#define EEPROM_MAX_SIZE     32768       // 256Kb = 32KB for 24LC256

// ==================== TIMING ====================
#define SERIAL_BAUD         115200
#define I2C_TIMEOUT_MS      1000
#define LOOP_DELAY_MS       100

// ==================== DEBUGGING ====================
#define DEBUG_ENABLED       1
#define DEBUG_I2C_VERBOSE   0           // Set 1 untuk debug I2C transaksi

#endif // CONFIG_H
