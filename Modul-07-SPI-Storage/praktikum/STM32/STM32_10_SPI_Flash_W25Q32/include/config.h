/**
 * @file config.h
 * @brief Konfigurasi Pin STM32F103C8T6 - Program 10 SPI Flash W25Q32
 */

#ifndef CONFIG_H
#define CONFIG_H

// ==================== SPI PIN DEFINITIONS ====================
#define SPI_SCK     PA5
#define SPI_MOSI    PA7
#define SPI_MISO    PA6
#define FLASH_CS    PB0    // Flash Chip Select

// ==================== SERIAL ====================
#define SERIAL_BAUD 115200

// ==================== FLASH CONFIG ====================
#define FLASH_PAGE_SIZE     256      // Bytes per page
#define FLASH_SECTOR_SIZE   4096     // Bytes per sector
#define FLASH_BLOCK_SIZE    65536    // Bytes per block
#define FLASH_CAPACITY      4194304  // 32 Mbit (4 MB)

// ==================== W25Q32 COMMANDS ====================
#define CMD_READ_ID         0x90    // Read Manufacturer & Device ID
#define CMD_JEDEC_ID        0x9F    // Read JEDEC ID
#define CMD_READ_STATUS     0x05    // Read Status Register
#define CMD_WRITE_ENABLE    0x06    // Write Enable
#define CMD_PAGE_PROGRAM    0x02    // Page Program
#define CMD_SECTOR_ERASE    0x20    // Sector Erase (4KB)
#define CMD_READ_DATA       0x03    // Read Data

#endif // CONFIG_H
