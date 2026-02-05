/**
 * @file main.cpp
 * @brief Program 07: SD Card Write Log - STM32F103C8T6 Blue Pill
 * 
 * Deskripsi:
 * Program untuk mencatat data sensor ke file log di SD Card.
 * Membaca nilai ADC dan menulis ke file dengan timestamp.
 * 
 * Hardware:
 * - STM32F103C8T6 Blue Pill
 * - SD Card Module (SPI)
 * - Potensiometer atau sensor analog (PA0)
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include "config.h"
#include "SdFat.h"

SdFat sd;

// ==================== GLOBAL VARIABLES ====================
uint32_t logCount = 0;
unsigned long lastLogTime = 0;
bool sdReady = false;
uint16_t sensorReading = 0;

// ==================== FUNCTION PROTOTYPES ====================
void initializeSD();
void readSensor();
void writeToLog();

// ==================== SETUP ====================
void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(2000);
    
    Serial.println("\n========================================");
    Serial.println("Program 07: SD Card Write Log - STM32F103C8T6");
    Serial.println("Praktikum Sistem Embedded");
    Serial.println("========================================\n");
    
    // Initialize ADC
    pinMode(ADC_PIN, INPUT);
    analogRead(ADC_PIN);  // Dummy read to initialize
    
    Serial.println("Configuration:");
    Serial.printf("  ADC Pin: PA0\n");
    Serial.printf("  Log File: %s\n", LOG_FILENAME);
    Serial.printf("  Log Interval: %d ms\n", LOG_INTERVAL_MS);
    
    Serial.println("\nInitializing SD Card...");
    initializeSD();
    
    if (sdReady) {
        Serial.println("✓ SD Card Ready!\n");
        Serial.println("Starting data logging...\n");
        lastLogTime = millis();
    }
}

// ==================== LOOP ====================
void loop() {
    if (!sdReady) {
        delay(1000);
        initializeSD();
        return;
    }
    
    unsigned long currentTime = millis();
    
    // Log data at specified interval
    if (currentTime - lastLogTime >= LOG_INTERVAL_MS) {
        lastLogTime = currentTime;
        
        readSensor();
        writeToLog();
        
        logCount++;
        Serial.printf("[%lu ms] Log #%lu: ADC = %d\n", 
                     currentTime, logCount, sensorReading);
    }
    
    delay(100);  // Small delay to prevent CPU hogging
}

// ==================== FUNCTION IMPLEMENTATIONS ====================
void initializeSD() {
    if (sd.begin(SD_CS, SPI_FULL_SPEED)) {
        sdReady = true;
        Serial.println("SD Card initialized successfully.");
        
        // Create or check log file header
        if (!sd.exists(LOG_FILENAME)) {
            File logFile = sd.open(LOG_FILENAME, FILE_WRITE);
            if (logFile) {
                logFile.println("=== DATA LOG ===");
                logFile.println("Timestamp(ms), ADC_Value, Voltage(V)");
                logFile.close();
                Serial.println("Log file created with header.");
            }
        }
    } else {
        sdReady = false;
        Serial.println("Failed to initialize SD Card!");
    }
}

void readSensor() {
    // Read ADC value (0-4095 for 12-bit)
    sensorReading = analogRead(ADC_PIN);
}

void writeToLog() {
    if (!sdReady) return;
    
    File logFile = sd.open(LOG_FILENAME, FILE_WRITE);
    
    if (logFile) {
        unsigned long timestamp = millis();
        float voltage = (sensorReading / 4095.0) * 3.3;  // Convert to voltage
        
        // Write data to file
        logFile.printf("%lu, %d, %.2f\n", 
                      timestamp, sensorReading, voltage);
        
        logFile.close();
    } else {
        Serial.printf("Error: Cannot write to %s\n", LOG_FILENAME);
    }
}

/**
 * DATA LOGGING BEST PRACTICES:
 * 
 * ┌──────────────────────────────────────────┐
 * │ Efficient Data Logging Strategy:         │
 * ├──────────────────────────────────────────┤
 * │                                          │
 * │ 1. Buffer Data in RAM                    │
 * │    └─ Collect multiple samples           │
 * │    └─ Write in bulk (faster)             │
 * │                                          │
 * │ 2. Use Efficient Data Format             │
 * │    └─ CSV for simple data                │
 * │    └─ Binary for large datasets          │
 * │    └─ JSON for structured data           │
 * │                                          │
 * │ 3. Include Timestamp                     │
 * │    └─ millis() for relative time         │
 * │    └─ RTC for absolute time              │
 * │                                          │
 * │ 4. File Organization                     │
 * │    └─ New file per day/session           │
 * │    └─ Roll-over when reaching size limit │
 * │                                          │
 * │ 5. Error Handling                        │
 * │    └─ Check write status                 │
 * │    └─ Verify file closure                │
 * │                                          │
 * │ Sample CSV Format:                       │
 * │ timestamp,adc_value,voltage              │
 * │ 0,512,0.41                               │
 * │ 2000,618,0.50                            │
 * │ 4000,725,0.59                            │
 * │                                          │
 * └──────────────────────────────────────────┘
 * 
 */
