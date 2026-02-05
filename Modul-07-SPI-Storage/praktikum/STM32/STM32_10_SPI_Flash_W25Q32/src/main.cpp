/**
 * @file main.cpp
 * @brief Program 10: SPI Flash W25Q32 - STM32F103C8T6 Blue Pill
 * 
 * Deskripsi:
 * Program untuk mengontrol SPI Flash memory W25Q32 (32 Megabit = 4 MB).
 * Fitur: Read, Write, Erase dengan verifikasi data.
 * 
 * Hardware:
 * - STM32F103C8T6 Blue Pill
 * - W25Q32 SPI Flash Module
 * 
 * Pinout W25Q32:
 * - 1: CS/SS
 * - 2: DO/MISO
 * - 3: /WP
 * - 4: GND
 * - 5: DI/MOSI
 * - 6: CLK/SCK
 * - 7: /HOLD
 * - 8: VCC
 * 
 * Applications:
 * - Configuration storage
 * - Firmware updates
 * - Sensor data logging (non-volatile)
 * - User data backup
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include "config.h"

SPIClass SPI_1(1);

// ==================== GLOBAL VARIABLES ====================
uint32_t testCount = 0;
uint8_t testBuffer[256];
uint8_t readBuffer[256];

// ==================== FUNCTION PROTOTYPES ====================
void initFlash();
uint8_t readFlashStatusRegister();
void writeFlashEnable();
void flashPageWrite(uint32_t address, uint8_t* data, uint16_t length);
void flashRead(uint32_t address, uint8_t* data, uint16_t length);
void flashSectorErase(uint32_t address);
void displayFlashInfo();

// ==================== SETUP ====================
void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(2000);
    
    Serial.println("\n========================================");
    Serial.println("Program 10: SPI Flash W25Q32 - STM32F103C8T6");
    Serial.println("Praktikum Sistem Embedded");
    Serial.println("========================================\n");
    
    // Initialize pins
    pinMode(FLASH_CS, OUTPUT);
    digitalWrite(FLASH_CS, HIGH);
    
    // Initialize SPI
    SPI_1.begin();
    SPI_1.setClockDivider(SPI_CLOCK_DIV4);  // 18 MHz
    SPI_1.setDataMode(SPI_MODE0);
    SPI_1.setBitOrder(MSBFIRST);
    
    Serial.println("Flash Configuration:");
    Serial.printf("  Type: W25Q32 (32 Mbit / 4 MB)\n");
    Serial.printf("  CS Pin: PB0\n");
    Serial.printf("  Page Size: %d bytes\n", FLASH_PAGE_SIZE);
    Serial.printf("  Sector Size: %d bytes\n", FLASH_SECTOR_SIZE);
    Serial.printf("  Block Size: %d bytes\n", FLASH_BLOCK_SIZE);
    Serial.printf("  Total Capacity: %d bytes\n", FLASH_CAPACITY);
    
    Serial.println("\nInitializing Flash memory...");
    delay(500);
    
    initFlash();
    displayFlashInfo();
    
    // Prepare test data
    for (int i = 0; i < 256; i++) {
        testBuffer[i] = (i ^ 0xAA) & 0xFF;
    }
}

// ==================== LOOP ====================
void loop() {
    testCount++;
    
    Serial.printf("\n========== Flash Test Cycle #%lu ==========\n", testCount);
    
    // Test address (first page)
    uint32_t testAddress = 0x000000;
    
    Serial.println("\n1. Erasing sector...");
    flashSectorErase(testAddress);
    delay(500);
    Serial.println("   ✓ Sector erased");
    
    Serial.println("\n2. Writing data to flash...");
    flashPageWrite(testAddress, testBuffer, 256);
    delay(500);
    Serial.println("   ✓ Data written");
    
    Serial.println("\n3. Reading data from flash...");
    flashRead(testAddress, readBuffer, 256);
    
    Serial.println("\n4. Verifying data...");
    bool dataValid = true;
    int errorCount = 0;
    for (int i = 0; i < 256; i++) {
        if (testBuffer[i] != readBuffer[i]) {
            dataValid = false;
            errorCount++;
        }
    }
    
    // Display first 16 bytes
    Serial.println("\nData Verification (first 16 bytes):");
    Serial.println("Addr  | TX Data | RX Data | Status");
    Serial.println("──────┼─────────┼─────────┼────────");
    for (int i = 0; i < 16; i++) {
        const char* status = (testBuffer[i] == readBuffer[i]) ? "✓" : "✗";
        Serial.printf(" %04X | 0x%02X    | 0x%02X    | %s\n",
                     i, testBuffer[i], readBuffer[i], status);
    }
    
    if (dataValid) {
        Serial.println("\n✓ FLASH TEST PASSED!");
    } else {
        Serial.printf("\n✗ FLASH TEST FAILED (%d errors)\n", errorCount);
    }
    
    delay(3000);
}

// ==================== FUNCTION IMPLEMENTATIONS ====================
void initFlash() {
    // Read JEDEC ID to verify flash
    digitalWrite(FLASH_CS, LOW);
    delayMicroseconds(10);
    
    SPI_1.transfer(CMD_JEDEC_ID);
    uint8_t manID = SPI_1.transfer(0x00);
    uint8_t devID_H = SPI_1.transfer(0x00);
    uint8_t devID_L = SPI_1.transfer(0x00);
    
    delayMicroseconds(10);
    digitalWrite(FLASH_CS, HIGH);
    
    Serial.printf("JEDEC ID: 0x%02X 0x%02X 0x%02X\n", manID, devID_H, devID_L);
    
    if (manID == 0xEF && devID_H == 0x30 && devID_L == 0x16) {
        Serial.println("✓ W25Q32 Detected!");
    } else {
        Serial.println("✗ Unknown Flash Chip!");
    }
}

uint8_t readFlashStatusRegister() {
    uint8_t status;
    
    digitalWrite(FLASH_CS, LOW);
    delayMicroseconds(5);
    
    SPI_1.transfer(CMD_READ_STATUS);
    status = SPI_1.transfer(0x00);
    
    delayMicroseconds(5);
    digitalWrite(FLASH_CS, HIGH);
    
    return status;
}

void writeFlashEnable() {
    digitalWrite(FLASH_CS, LOW);
    delayMicroseconds(5);
    
    SPI_1.transfer(CMD_WRITE_ENABLE);
    
    delayMicroseconds(5);
    digitalWrite(FLASH_CS, HIGH);
}

void flashSectorErase(uint32_t address) {
    writeFlashEnable();
    
    digitalWrite(FLASH_CS, LOW);
    delayMicroseconds(5);
    
    SPI_1.transfer(CMD_SECTOR_ERASE);
    SPI_1.transfer((address >> 16) & 0xFF);
    SPI_1.transfer((address >> 8) & 0xFF);
    SPI_1.transfer(address & 0xFF);
    
    delayMicroseconds(5);
    digitalWrite(FLASH_CS, HIGH);
    
    // Wait for erase complete (status register bit 0)
    while (readFlashStatusRegister() & 0x01) {
        delay(10);
    }
}

void flashPageWrite(uint32_t address, uint8_t* data, uint16_t length) {
    if (length > FLASH_PAGE_SIZE) {
        length = FLASH_PAGE_SIZE;
    }
    
    writeFlashEnable();
    
    digitalWrite(FLASH_CS, LOW);
    delayMicroseconds(5);
    
    SPI_1.transfer(CMD_PAGE_PROGRAM);
    SPI_1.transfer((address >> 16) & 0xFF);
    SPI_1.transfer((address >> 8) & 0xFF);
    SPI_1.transfer(address & 0xFF);
    
    for (uint16_t i = 0; i < length; i++) {
        SPI_1.transfer(data[i]);
    }
    
    delayMicroseconds(5);
    digitalWrite(FLASH_CS, HIGH);
    
    // Wait for write complete
    while (readFlashStatusRegister() & 0x01) {
        delay(1);
    }
}

void flashRead(uint32_t address, uint8_t* data, uint16_t length) {
    digitalWrite(FLASH_CS, LOW);
    delayMicroseconds(5);
    
    SPI_1.transfer(CMD_READ_DATA);
    SPI_1.transfer((address >> 16) & 0xFF);
    SPI_1.transfer((address >> 8) & 0xFF);
    SPI_1.transfer(address & 0xFF);
    
    for (uint16_t i = 0; i < length; i++) {
        data[i] = SPI_1.transfer(0x00);
    }
    
    delayMicroseconds(5);
    digitalWrite(FLASH_CS, HIGH);
}

void displayFlashInfo() {
    Serial.println("\nFlash Memory Information:");
    Serial.println("─────────────────────────────────────");
    Serial.printf("Memory Organization:\n");
    Serial.printf("  Total: %d bytes (4 MB)\n", FLASH_CAPACITY);
    Serial.printf("  Blocks: %d × %d bytes\n", FLASH_CAPACITY / FLASH_BLOCK_SIZE, FLASH_BLOCK_SIZE);
    Serial.printf("  Sectors: %d × %d bytes\n", FLASH_CAPACITY / FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE);
    Serial.printf("  Pages: %d × %d bytes\n", FLASH_CAPACITY / FLASH_PAGE_SIZE, FLASH_PAGE_SIZE);
    Serial.println("─────────────────────────────────────");
}

/**
 * W25Q32 MEMORY MAP:
 * 
 * ┌─────────────────────────────────┐
 * │ Address Range  │ Size           │
 * ├─────────────────────────────────┤
 * │ 0x000000-0x0000FF │ Page 0      │
 * │ 0x000100-0x0001FF │ Page 1      │
 * │    ...             │ ...         │
 * │ 0x000000-0x000FFF │ Sector 0    │
 * │ 0x001000-0x001FFF │ Sector 1    │
 * │    ...             │ ...         │
 * │ 0x000000-0x00FFFF │ Block 0     │
 * │ 0x010000-0x01FFFF │ Block 1     │
 * │    ...             │ ...         │
 * │ 0x000000-0x3FFFFF │ Total (4MB) │
 * └─────────────────────────────────┘
 * 
 * ERASE TIMES:
 * - Page Erase: Not supported (use sector)
 * - Sector Erase: ~50-100 ms
 * - Block Erase: ~400-800 ms
 * - Chip Erase: ~10-20 seconds
 * 
 */
