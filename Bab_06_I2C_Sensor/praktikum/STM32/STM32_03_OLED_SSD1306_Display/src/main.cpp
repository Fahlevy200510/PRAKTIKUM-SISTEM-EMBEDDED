/**
 * @file main.cpp
 * @brief Program 03: OLED_SSD1306_Display - STM32F103C8T6
 * 
 * Deskripsi:
 * Kontrol OLED display 128x64 SSD1306
 * 
 * Hardware:
 * - STM32F103C8T6 Blue Pill
 * - I2C Bus: PB6(SCL), PB7(SDA)
 * - Sensor: OLED SSD1306 (0x3C/0x3D)
 * - Fitur: Graphics, Text display, Bitmap
 * 
 * Konfigurasi I2C:
 * - Clock Speed: 400kHz (Fast Mode)
 * - Timeout: 1000ms
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include <Wire.h>
#include "config.h"

// ==================== VARIABEL GLOBAL ====================
uint32_t loopCount = 0;
unsigned long lastPrintTime = 0;

// ==================== PROTOTYPES ====================
void initI2C();
void scanI2CBus();
void printSystemInfo();

// ==================== SETUP ====================
void setup() {
    // Inisialisasi Serial (PA9=TX, PA10=RX)
    Serial.begin(SERIAL_BAUD);
    delay(2000);  // Tunggu serial ready
    
    Serial.println("\n========================================");
    Serial.println("Program 03: OLED_SSD1306_Display");
    Serial.println("STM32F103C8T6 Blue Pill - I2C Sensor");
    Serial.println("Praktikum Sistem Embedded");
    Serial.println("========================================\n");
    
    // Print informasi sistem
    printSystemInfo();
    
    // Inisialisasi I2C
    initI2C();
    
    Serial.println("\n[SETUP COMPLETE] Siap melakukan operasi I2C\n");
    
    // TODO: Inisialisasi sensor spesifik di sini
}

// ==================== LOOP ====================
void loop() {
    loopCount++;
    
    // Baca data setiap 1 detik
    unsigned long currentMillis = millis();
    if (currentMillis - lastPrintTime >= 1000) {
        lastPrintTime = currentMillis;
        
        // TODO: Implementasi logic utama di sini
        Serial.printf("[%lu] Loop count: %lu\n", currentMillis, loopCount);
    }
    
    delay(10);  // Small delay untuk stabilitas
}

// ==================== FUNGSI HELPER ====================

/**
 * @brief Inisialisasi I2C Bus
 */
void initI2C() {
    Serial.println("[I2C] Initializing I2C1 bus...");
    
    Wire.setSDA(I2C_SDA_PIN);
    Wire.setSCL(I2C_SCL_PIN);
    Wire.begin();
    Wire.setClock(I2C_SPEED_HZ);
    
    Serial.printf("[I2C] I2C Bus initialized\n");
    Serial.printf("  SDA Pin: PB7\n");
    Serial.printf("  SCL Pin: PB6\n");
    Serial.printf("  Speed: %d Hz\n", I2C_SPEED_HZ);
    
    // Scan untuk device yang tersedia
    scanI2CBus();
}

/**
 * @brief Scan I2C Bus untuk menemukan device
 */
void scanI2CBus() {
    Serial.println("\n[I2C] Scanning I2C bus...");
    
    int deviceCount = 0;
    for (int addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        int result = Wire.endTransmission();
        
        if (result == 0) {
            deviceCount++;
            Serial.printf("  Found device at 0x%02X\n", addr);
        }
    }
    
    if (deviceCount == 0) {
        Serial.println("  No I2C devices found!");
    } else {
        Serial.printf("  Total devices found: %d\n\n", deviceCount);
    }
}

/**
 * @brief Print informasi sistem
 */
void printSystemInfo() {
    Serial.println("System Information:");
    Serial.println("  Core: ARM Cortex-M3 @ 72MHz");
    Serial.println("  Flash: 64KB");
    Serial.println("  RAM: 20KB");
    Serial.printf("  Baud Rate: %d bps\n", SERIAL_BAUD);
    Serial.printf("  I2C Speed: %d Hz\n", I2C_SPEED_HZ);
}

// ==================== I2C HELPER FUNCTIONS ====================

/**
 * @brief Write single byte ke I2C device
 */
bool i2cWriteByte(uint8_t addr, uint8_t reg, uint8_t value) {
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(value);
    return (Wire.endTransmission() == 0);
}

/**
 * @brief Read single byte dari I2C device
 */
bool i2cReadByte(uint8_t addr, uint8_t reg, uint8_t *value) {
    Wire.beginTransmission(addr);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) return false;
    
    if (Wire.requestFrom(addr, 1) != 1) return false;
    *value = Wire.read();
    return true;
}

/**
 * @brief Read multiple bytes dari I2C device
 */
bool i2cReadBytes(uint8_t addr, uint8_t reg, uint8_t *buffer, uint8_t length) {
    Wire.beginTransmission(addr);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) return false;
    
    if (Wire.requestFrom(addr, length) != length) return false;
    for (uint8_t i = 0; i < length; i++) {
        buffer[i] = Wire.read();
    }
    return true;
}
