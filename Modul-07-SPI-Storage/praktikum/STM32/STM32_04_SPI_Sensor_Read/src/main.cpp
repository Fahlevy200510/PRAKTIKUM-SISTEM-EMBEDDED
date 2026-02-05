/**
 * @file main.cpp
 * @brief Program 04: SPI Sensor Read - STM32F103C8T6 Blue Pill
 * 
 * Deskripsi:
 * Program untuk membaca data dari sensor SPI (simulasi).
 * Contoh: MPU6050 accelerometer/gyroscope via SPI interface.
 * 
 * Hardware:
 * - STM32F103C8T6 Blue Pill
 * - SPI Sensor (misalnya: L3GD20, LIS3DSH, atau MPU6000)
 * 
 * SPI Sensor Protocol:
 * - Byte pertama: Register address (bit 7: Read/Write, bit 6-0: Address)
 * - Byte berikutnya: Data
 * - Read: Kirim address dengan bit 7 = 1
 * - Write: Kirim address dengan bit 7 = 0
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include "config.h"

SPIClass SPI_1(1);  // Use SPI1

// ==================== GLOBAL VARIABLES ====================
uint8_t sensorData[6];
uint32_t readCount = 0;

// ==================== FUNCTION PROTOTYPES ====================
uint8_t readSensorRegister(uint8_t regAddress);
void readSensorData();
void displaySensorData();

// ==================== SETUP ====================
void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(2000);
    
    Serial.println("\n========================================");
    Serial.println("Program 04: SPI Sensor Read - STM32F103C8T6");
    Serial.println("Praktikum Sistem Embedded");
    Serial.println("========================================\n");
    
    // Initialize SPI pins
    pinMode(SPI_SCK, OUTPUT);
    pinMode(SPI_MOSI, OUTPUT);
    pinMode(SPI_MISO, INPUT);
    pinMode(SPI_CS, OUTPUT);
    digitalWrite(SPI_CS, HIGH);
    
    // Initialize SPI
    SPI_1.begin();
    SPI_1.setClockDivider(SPI_CLOCK_DIV8);  // 72MHz / 8 = 9MHz
    SPI_1.setDataMode(SPI_MODE0);
    SPI_1.setBitOrder(MSBFIRST);
    
    Serial.println("SPI Configuration:");
    Serial.printf("  Clock: PA5 (SCK)\n");
    Serial.printf("  MOSI: PA7\n");
    Serial.printf("  MISO: PA6\n");
    Serial.printf("  CS: PA4\n");
    Serial.printf("  Speed: ~9 MHz (SPI_CLOCK_DIV8)\n");
    Serial.printf("  Mode: SPI_MODE0\n");
    
    Serial.println("\nSensor Protocol:");
    Serial.println("  Read Bit: Bit 7 = 1");
    Serial.println("  Address: Bit 6-0 = 7-bit address");
    Serial.println("  Multi-byte: Bit 6 = Auto-increment");
    
    Serial.println("\nInitializing Sensor...");
    delay(1000);
    
    // Verify sensor by reading sensor ID
    uint8_t sensorID = readSensorRegister(SENSOR_ID_REG | 0x80);  // Read command
    Serial.printf("Sensor ID Register (0x%02X): 0x%02X\n", 
                  SENSOR_ID_REG, sensorID);
    Serial.println("Sensor Ready!\n");
}

// ==================== LOOP ====================
void loop() {
    readCount++;
    
    Serial.printf("\n--- Sensor Read #%lu ---\n", readCount);
    
    // Read sensor data
    readSensorData();
    
    // Display results
    displaySensorData();
    
    delay(1000);
}

// ==================== FUNCTION IMPLEMENTATIONS ====================
uint8_t readSensorRegister(uint8_t regAddress) {
    uint8_t data;
    
    digitalWrite(SPI_CS, LOW);
    delayMicroseconds(10);
    
    // Send register address (with read bit)
    SPI_1.transfer(regAddress);
    // Read data
    data = SPI_1.transfer(0x00);
    
    delayMicroseconds(10);
    digitalWrite(SPI_CS, HIGH);
    
    return data;
}

void readSensorData() {
    // Simulate reading 6-axis sensor data (AccelX, AccelY, AccelZ, GyroX, GyroY, GyroZ)
    // In real implementation, these would be actual sensor values
    
    digitalWrite(SPI_CS, LOW);
    delayMicroseconds(10);
    
    // Send data register address with read bit and auto-increment bit
    SPI_1.transfer(SENSOR_DATA_REG | 0x80 | 0x40);  // Read + Auto-increment
    
    // Read 6 bytes of sensor data
    for (int i = 0; i < 6; i++) {
        sensorData[i] = SPI_1.transfer(0x00);
    }
    
    delayMicroseconds(10);
    digitalWrite(SPI_CS, HIGH);
}

void displaySensorData() {
    // Parse sensor data (2 bytes per axis, big-endian)
    int16_t accelX = (int16_t)((sensorData[0] << 8) | sensorData[1]);
    int16_t accelY = (int16_t)((sensorData[2] << 8) | sensorData[3]);
    int16_t accelZ = (int16_t)((sensorData[4] << 8) | sensorData[5]);
    
    Serial.println("Accelerometer Data (Raw):");
    Serial.printf("  AccelX: %d\n", accelX);
    Serial.printf("  AccelY: %d\n", accelY);
    Serial.printf("  AccelZ: %d\n", accelZ);
    
    // Calculate magnitude
    float magnitude = sqrt(accelX*accelX + accelY*accelY + accelZ*accelZ);
    Serial.printf("  Magnitude: %.2f\n", magnitude);
    
    // Detect orientation
    if (accelZ > 15000) {
        Serial.println("  Orientation: Flat (X-Y plane)");
    } else if (accelX > 15000) {
        Serial.println("  Orientation: Tilted (X axis down)");
    } else {
        Serial.println("  Orientation: Other");
    }
}

/**
 * SPI SENSOR READ PROTOCOL:
 * 
 * ┌────────────────────────────────────────────┐
 * │ Single Register Read Sequence:             │
 * ├────────────────────────────────────────────┤
 * │                                            │
 * │ CS:   ────┐               ┌────            │
 * │          └───────────────┘                │
 * │                                            │
 * │ CLK: ──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──  │
 * │        └──┘  └──┘  └──┘  └──┘  └──┘  └──  │
 * │                                            │
 * │ MOSI: ─┬─┬────────────────────────┬─┐    │
 * │        │R│ A6 A5 A4 A3 A2 A1 A0  │0│    │
 * │        └─┴────────────────────────┴─┘    │
 * │                                            │
 * │ MISO: ─┬─┬────────────────────────┬─┬─  │
 * │        │0│ D7 D6 D5 D4 D3 D2 D1 D0│?│   │
 * │        └─┴────────────────────────┴─┴─  │
 * │                                            │
 * │ Where: R = Read (1) / Write (0)           │
 * │        A6-A0 = 7-bit register address     │
 * │        D7-D0 = 8-bit data                 │
 * │                                            │
 * └────────────────────────────────────────────┘
 * 
 */
