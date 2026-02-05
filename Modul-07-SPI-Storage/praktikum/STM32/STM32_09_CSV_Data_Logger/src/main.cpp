/**
 * @file main.cpp
 * @brief Program 09: CSV Data Logger - STM32F103C8T6 Blue Pill
 * 
 * Deskripsi:
 * Program untuk mencatat data sensor multiple dalam format CSV.
 * CSV (Comma-Separated Values) mudah dibuka di Excel/LibreOffice.
 * 
 * Hardware:
 * - STM32F103C8T6 Blue Pill
 * - SD Card Module (SPI)
 * - Sensor analog (Temperature, Humidity, Pressure)
 * 
 * CSV Format:
 * Timestamp,Temperature,Humidity,Pressure
 * 1000,23.5,45.2,1013.25
 * 4000,23.7,44.8,1013.20
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include "config.h"
#include "SdFat.h"

SdFat sd;

// ==================== GLOBAL VARIABLES ====================
uint32_t dataCount = 0;
unsigned long lastLogTime = 0;
bool sdReady = false;
float temperature = 0, humidity = 0, pressure = 0;

// ==================== STRUCT ====================
struct SensorData {
    unsigned long timestamp;
    float temp;
    float hum;
    float press;
};

// ==================== FUNCTION PROTOTYPES ====================
void initializeSD();
void readSensors();
void writeCSVData();
void displayStatistics();

// ==================== SETUP ====================
void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(2000);
    
    Serial.println("\n========================================");
    Serial.println("Program 09: CSV Data Logger - STM32F103C8T6");
    Serial.println("Praktikum Sistem Embedded");
    Serial.println("========================================\n");
    
    // Initialize analog pins
    pinMode(TEMP_PIN, INPUT);
    pinMode(HUM_PIN, INPUT);
    pinMode(PRESS_PIN, INPUT);
    
    Serial.println("Configuration:");
    Serial.printf("  Temperature Sensor: PA0\n");
    Serial.printf("  Humidity Sensor: PA1\n");
    Serial.printf("  Pressure Sensor: PA2\n");
    Serial.printf("  CSV File: %s\n", CSV_FILENAME);
    Serial.printf("  Log Interval: %d ms\n", LOG_INTERVAL_MS);
    
    Serial.println("\nInitializing SD Card...");
    initializeSD();
    
    if (sdReady) {
        Serial.println("✓ SD Card Ready!\n");
        Serial.println("Starting CSV data logging...\n");
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
        
        readSensors();
        writeCSVData();
        
        dataCount++;
        Serial.printf("[%lu ms] Record #%lu: T=%.1f°C, H=%.1f%%, P=%.2f hPa\n",
                     currentTime, dataCount, temperature, humidity, pressure);
        
        // Check if max records reached
        if (dataCount >= MAX_RECORDS) {
            Serial.printf("\nMax records (%d) reached!\n", MAX_RECORDS);
            displayStatistics();
            dataCount = 0;  // Reset for next session
        }
    }
    
    delay(100);
}

// ==================== FUNCTION IMPLEMENTATIONS ====================
void initializeSD() {
    if (sd.begin(SD_CS, SPI_FULL_SPEED)) {
        sdReady = true;
        Serial.println("SD Card initialized successfully.");
        
        // Create CSV file with header if not exists
        if (!sd.exists(CSV_FILENAME)) {
            File csvFile = sd.open(CSV_FILENAME, FILE_WRITE);
            if (csvFile) {
                csvFile.println("Timestamp(ms),Temperature(C),Humidity(%),Pressure(hPa)");
                csvFile.close();
                Serial.println("CSV file created with header.");
            }
        }
    } else {
        sdReady = false;
        Serial.println("Failed to initialize SD Card!");
    }
}

void readSensors() {
    // Simulate sensor readings (in real project, use calibrated values)
    // Temperature: -40°C to 85°C mapped to 0-4095 ADC
    uint16_t tempRaw = analogRead(TEMP_PIN);
    temperature = -40 + (tempRaw / 4095.0) * 125;
    
    // Humidity: 0% to 100% mapped to 0-4095 ADC
    uint16_t humRaw = analogRead(HUM_PIN);
    humidity = (humRaw / 4095.0) * 100;
    
    // Pressure: 300 hPa to 1100 hPa mapped to 0-4095 ADC
    uint16_t pressRaw = analogRead(PRESS_PIN);
    pressure = 300 + (pressRaw / 4095.0) * 800;
}

void writeCSVData() {
    if (!sdReady) return;
    
    File csvFile = sd.open(CSV_FILENAME, FILE_WRITE);
    
    if (csvFile) {
        unsigned long timestamp = millis();
        
        // Write CSV formatted data
        csvFile.printf("%lu,%.2f,%.2f,%.2f\n",
                      timestamp, temperature, humidity, pressure);
        
        csvFile.close();
    } else {
        Serial.printf("Error: Cannot write to %s\n", CSV_FILENAME);
    }
}

void displayStatistics() {
    Serial.println("\n═══════════════════════════════════════");
    Serial.println("         SESSION STATISTICS          ");
    Serial.println("═══════════════════════════════════════");
    
    File csvFile = sd.open(CSV_FILENAME, FILE_READ);
    
    if (!csvFile) {
        Serial.println("Error: Cannot open CSV file!");
        return;
    }
    
    // Parse CSV and calculate statistics
    float minTemp = 999, maxTemp = -999;
    float minHum = 999, maxHum = -999;
    float minPress = 9999, maxPress = 0;
    float sumTemp = 0, sumHum = 0, sumPress = 0;
    uint32_t recordCount = 0;
    
    char line[100];
    while (csvFile.fgets(line, sizeof(line))) {
        // Skip header
        if (strchr(line, 'T') != NULL && strchr(line, 'e') != NULL) {
            continue;
        }
        
        // Parse CSV (simplified)
        float t, h, p;
        if (sscanf(line, "%*lu,%f,%f,%f", &t, &h, &p) == 3) {
            minTemp = min(minTemp, t); maxTemp = max(maxTemp, t);
            minHum = min(minHum, h); maxHum = max(maxHum, h);
            minPress = min(minPress, p); maxPress = max(maxPress, p);
            sumTemp += t; sumHum += h; sumPress += p;
            recordCount++;
        }
    }
    
    csvFile.close();
    
    if (recordCount > 0) {
        Serial.printf("Total Records: %lu\n", recordCount);
        Serial.println("\nTemperature (°C):");
        Serial.printf("  Min: %.2f, Max: %.2f, Avg: %.2f\n",
                     minTemp, maxTemp, sumTemp / recordCount);
        Serial.println("Humidity (%):");
        Serial.printf("  Min: %.2f, Max: %.2f, Avg: %.2f\n",
                     minHum, maxHum, sumHum / recordCount);
        Serial.println("Pressure (hPa):");
        Serial.printf("  Min: %.2f, Max: %.2f, Avg: %.2f\n",
                     minPress, maxPress, sumPress / recordCount);
    }
    
    Serial.println("═══════════════════════════════════════\n");
}

/**
 * CSV FORMAT BENEFITS:
 * 
 * ┌─────────────────────────────────────────┐
 * │ CSV Data Format Advantages              │
 * ├─────────────────────────────────────────┤
 * │                                         │
 * │ ✓ Human readable                        │
 * │ ✓ Easy to parse programmatically        │
 * │ ✓ Compatible with spreadsheet software  │
 * │ ✓ Compact storage                       │
 * │ ✓ Version control friendly              │
 * │ ✓ No special libraries needed           │
 * │                                         │
 * │ Example CSV Content:                    │
 * │ Time,Temp,Humid,Press                  │
 * │ 0,23.5,45.2,1013.25                    │
 * │ 3000,23.7,44.8,1013.20                 │
 * │ 6000,24.1,44.5,1013.10                 │
 * │                                         │
 * │ Import to Excel:                        │
 * │ 1. File → Open                          │
 * │ 2. Select CSV file                      │
 * │ 3. Text to Columns → Comma separator    │
 * │ 4. Data is ready for analysis!          │
 * │                                         │
 * └─────────────────────────────────────────┘
 * 
 */
