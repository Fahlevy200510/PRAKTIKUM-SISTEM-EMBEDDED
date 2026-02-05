/**
 * @file main.cpp
 * @brief Program 12: NVS Key-Value Storage - STM32F103C8T6 Blue Pill
 * 
 * Deskripsi:
 * Program untuk menyimpan dan membaca key-value pairs di non-volatile storage.
 * NVS adalah abstraksi high-level di atas EEPROM atau flash memory.
 * Ideal untuk menyimpan konfigurasi, calibration data, device ID, dll.
 * 
 * Use Cases:
 * - Device Configuration (SSID, Password, IP)
 * - Calibration Data (Sensor offsets, scales)
 * - Device Information (Serial Number, Model)
 * - User Preferences (Brightness, Language)
 * - Operational Counters (Uptime, Cycle Count)
 * 
 * Keuntungan NVS:
 * - Key-value interface (mudah digunakan)
 * - Multiple data types (int, float, string)
 * - Namespace untuk organizing data
 * - Automatic wear leveling
 * - Atomic operations (crash-safe)
 * 
 * Hardware:
 * - STM32F103C8T6 Blue Pill
 * - Internal Flash memory (64KB) atau EEPROM emulation
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include "config.h"
#include <EEPROM.h>

// ==================== SIMPLE NVS IMPLEMENTATION ====================
class SimpleNVS {
private:
    struct Entry {
        char key[32];
        uint32_t value;
        uint8_t type;  // 0=empty, 1=uint32, 2=int32, 3=float
    };
    
    Entry entries[16];
    uint8_t count;
    
public:
    SimpleNVS() : count(0) {}
    
    void begin() {
        EEPROM.begin(EEPROM_SIZE);
        loadFromEEPROM();
    }
    
    void putUInt(const char* key, uint32_t value) {
        for (int i = 0; i < count; i++) {
            if (strcmp(entries[i].key, key) == 0) {
                entries[i].value = value;
                entries[i].type = 1;
                saveToEEPROM();
                return;
            }
        }
        
        if (count < 16) {
            strcpy(entries[count].key, key);
            entries[count].value = value;
            entries[count].type = 1;
            count++;
            saveToEEPROM();
        }
    }
    
    void putInt(const char* key, int32_t value) {
        putUInt(key, (uint32_t)value);
    }
    
    void putFloat(const char* key, float value) {
        uint32_t* ptr = (uint32_t*)&value;
        putUInt(key, *ptr);
    }
    
    uint32_t getUInt(const char* key, uint32_t defaultVal = 0) {
        for (int i = 0; i < count; i++) {
            if (strcmp(entries[i].key, key) == 0) {
                return entries[i].value;
            }
        }
        return defaultVal;
    }
    
    int32_t getInt(const char* key, int32_t defaultVal = 0) {
        return (int32_t)getUInt(key, (uint32_t)defaultVal);
    }
    
    float getFloat(const char* key, float defaultVal = 0.0) {
        uint32_t* ptr = (uint32_t*)&defaultVal;
        uint32_t val = getUInt(key, *ptr);
        return *(float*)&val;
    }
    
    void printAll() {
        Serial.println("NVS Storage Contents:");
        Serial.println("─────────────────────────────────────");
        for (int i = 0; i < count; i++) {
            Serial.printf("  %s = 0x%08lX\n", entries[i].key, entries[i].value);
        }
        Serial.println("─────────────────────────────────────");
    }
    
private:
    void saveToEEPROM() {
        // Simple save: write count + entries
        EEPROM.write(0, count);
        int addr = 1;
        for (int i = 0; i < count; i++) {
            // Write key (32 bytes max)
            for (int j = 0; j < 32; j++) {
                EEPROM.write(addr++, entries[i].key[j]);
            }
            // Write value (4 bytes)
            EEPROM.write(addr++, (entries[i].value >> 24) & 0xFF);
            EEPROM.write(addr++, (entries[i].value >> 16) & 0xFF);
            EEPROM.write(addr++, (entries[i].value >> 8) & 0xFF);
            EEPROM.write(addr++, entries[i].value & 0xFF);
        }
        EEPROM.commit();
    }
    
    void loadFromEEPROM() {
        count = EEPROM.read(0);
        if (count > 16) count = 0;  // Sanity check
        
        int addr = 1;
        for (int i = 0; i < count; i++) {
            // Read key
            for (int j = 0; j < 32; j++) {
                entries[i].key[j] = EEPROM.read(addr++);
            }
            // Read value
            entries[i].value = 0;
            entries[i].value |= (uint32_t)EEPROM.read(addr++) << 24;
            entries[i].value |= (uint32_t)EEPROM.read(addr++) << 16;
            entries[i].value |= (uint32_t)EEPROM.read(addr++) << 8;
            entries[i].value |= (uint32_t)EEPROM.read(addr++);
        }
    }
};

SimpleNVS nvs;

// ==================== GLOBAL VARIABLES ====================
uint32_t operationCount = 0;

// ==================== SETUP ====================
void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(2000);
    
    Serial.println("\n========================================");
    Serial.println("Program 12: NVS Key-Value Storage - STM32F103C8T6");
    Serial.println("Praktikum Sistem Embedded");
    Serial.println("========================================\n");
    
    Serial.println("NVS Configuration:");
    Serial.printf("  EEPROM Size: %d bytes\n", EEPROM_SIZE);
    Serial.printf("  Start Address: 0x%08X\n", EEPROM_START_ADDR);
    Serial.printf("  Max Entries: 16\n");
    Serial.printf("  Entry Size: ~40 bytes (key 32 + value 4 + overhead)\n");
    
    Serial.println("\nInitializing NVS...");
    nvs.begin();
    
    Serial.println("✓ NVS Ready!\n");
}

// ==================== LOOP ====================
void loop() {
    operationCount++;
    
    Serial.printf("\n========== NVS Operation #%lu ==========\n", operationCount);
    
    // Store different types of data
    Serial.println("\n1. Writing data to NVS...");
    
    // Device info
    nvs.putUInt("device_id", 0x1234ABCD);
    Serial.println("   ✓ device_id = 0x1234ABCD");
    
    nvs.putUInt("fw_version", 0x00010002);  // v1.2
    Serial.println("   ✓ fw_version = 0x00010002 (v1.2)");
    
    // Sensor calibration
    nvs.putInt("temp_offset", -5);  // -5°C offset
    Serial.println("   ✓ temp_offset = -5");
    
    nvs.putFloat("temp_scale", 1.05f);  // 5% scale adjustment
    Serial.println("   ✓ temp_scale = 1.05");
    
    // Counters
    uint32_t bootCount = nvs.getUInt("boot_count", 0) + 1;
    nvs.putUInt("boot_count", bootCount);
    Serial.printf("   ✓ boot_count = %lu\n", bootCount);
    
    // Configuration
    nvs.putInt("wifi_enabled", 1);
    nvs.putInt("ota_enabled", 0);
    Serial.println("   ✓ Config flags updated");
    
    delay(500);
    
    // Read back all data
    Serial.println("\n2. Reading data from NVS...");
    nvs.printAll();
    
    // Display individual values
    Serial.println("\n3. Retrieved Values:");
    uint32_t devID = nvs.getUInt("device_id");
    Serial.printf("   Device ID: 0x%08lX\n", devID);
    
    int tempOffset = nvs.getInt("temp_offset");
    Serial.printf("   Temperature Offset: %d°C\n", tempOffset);
    
    float tempScale = nvs.getFloat("temp_scale");
    Serial.printf("   Temperature Scale: %.2f\n", tempScale);
    
    uint32_t boots = nvs.getUInt("boot_count");
    Serial.printf("   Boot Count: %lu\n", boots);
    
    delay(3000);
}

/**
 * NVS USE CASES:
 * 
 * ┌─────────────────────────────────────────┐
 * │ Device Information:                     │
 * │ - device_id (MAC address, serial #)     │
 * │ - fw_version                            │
 * │ - hw_version                            │
 * │ - manufacture_date                      │
 * │                                         │
 * │ Configuration:                          │
 * │ - wifi_ssid, wifi_password              │
 * │ - mqtt_broker, mqtt_port                │
 * │ - ip_address, gateway                   │
 * │ - ntp_server, timezone                  │
 * │                                         │
 * │ Calibration Data:                       │
 * │ - temp_offset, temp_scale               │
 * │ - humidity_offset                       │
 * │ - pressure_baseline                     │
 * │                                         │
 * │ Counters & Statistics:                  │
 * │ - boot_count                            │
 * │ - uptime_seconds                        │
 * │ - operation_hours                       │
 * │ - error_count                           │
 * │                                         │
 * │ User Preferences:                       │
 * │ - display_brightness                    │
 * │ - display_language                      │
 * │ - update_interval                       │
 * │ - logging_level                         │
 * │                                         │
 * │ Security:                               │
 * │ - api_key                               │
 * │ - encryption_key                        │
 * │ - last_known_state (for recovery)       │
 * │                                         │
 * └─────────────────────────────────────────┘
 * 
 * BEST PRACTICES:
 * 
 * 1. Use meaningful keys:
 *    ✓ "temp_offset"
 *    ✗ "t_off"
 * 
 * 2. Version your data:
 *    - store_version = 1 (for future compatibility)
 * 
 * 3. Set defaults:
 *    - nvs.getInt("key", DEFAULT_VALUE)
 * 
 * 4. Limit writes:
 *    - Flash has limited write cycles (~100k)
 *    - Don't write in every loop!
 * 
 * 5. Use namespaces:
 *    - "wifi:ssid", "sensor:offset"
 * 
 */
