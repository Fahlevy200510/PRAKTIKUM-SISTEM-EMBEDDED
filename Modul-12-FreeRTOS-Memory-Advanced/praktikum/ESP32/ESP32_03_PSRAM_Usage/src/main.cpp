/**
 * @file main.cpp
 * @brief Program 03: ESP32 PSRAM Usage
 * 
 * Demonstrasi penggunaan external PSRAM (4MB)
 * untuk aplikasi yang butuh memory besar.
 * 
 * PSRAM: SPI RAM eksternal, lebih lambat tapi besar
 * Internal: Lebih cepat tapi terbatas (~320KB)
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "esp_system.h"

#define LED_PIN     2

// Large buffer structure for demo
typedef struct {
    float data[1024];       // 4KB
    uint32_t timestamp;
    char description[64];
} LargeDataBlock_t;

TaskHandle_t xPSRAMTask = NULL;
TaskHandle_t xInternalTask = NULL;

void printMemoryInfo() {
    Serial.println("\n╔═════════════════════════════════════════════════════════╗");
    Serial.println("║              ESP32 MEMORY REGIONS INFO                  ║");
    Serial.println("╠═════════════════════════════════════════════════════════╣");
    
    // Internal DRAM
    Serial.println("║ [INTERNAL DRAM]                                         ║");
    Serial.printf("║   Total: %7d bytes                                   ║\n", 
                 heap_caps_get_total_size(MALLOC_CAP_INTERNAL));
    Serial.printf("║   Free:  %7d bytes                                   ║\n",
                 heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    
    // PSRAM
    Serial.println("╠═════════════════════════════════════════════════════════╣");
    if (psramFound()) {
        Serial.println("║ [EXTERNAL PSRAM] ✓ Available                            ║");
        Serial.printf("║   Total: %7d bytes (%.1f MB)                        ║\n",
                     ESP.getPsramSize(), ESP.getPsramSize() / 1048576.0);
        Serial.printf("║   Free:  %7d bytes                                   ║\n",
                     heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    } else {
        Serial.println("║ [EXTERNAL PSRAM] ✗ Not available                        ║");
        Serial.println("║   This demo works best with PSRAM module                ║");
    }
    
    Serial.println("╠═════════════════════════════════════════════════════════╣");
    Serial.println("║ [MEMORY CAPABILITIES]                                   ║");
    Serial.printf("║   DMA-capable:  %7d bytes free                       ║\n",
                 heap_caps_get_free_size(MALLOC_CAP_DMA));
    Serial.printf("║   32-bit align: %7d bytes free                       ║\n",
                 heap_caps_get_free_size(MALLOC_CAP_32BIT));
    Serial.printf("║   Exec-capable: %7d bytes free                       ║\n",
                 heap_caps_get_free_size(MALLOC_CAP_EXEC));
    
    Serial.println("╚═════════════════════════════════════════════════════════╝");
}

void vPSRAMTask(void *pvParameters) {
    Serial.println("[PSRAM] Task started");
    
    if (!psramFound()) {
        Serial.println("[PSRAM] No PSRAM found - simulating with internal RAM");
    }
    
    for (;;) {
        Serial.println("\n[PSRAM] Allocating large buffers in PSRAM...");
        
        // Allocate in PSRAM (or internal if no PSRAM)
        size_t allocSize = 100 * 1024;  // 100KB
        
        void *psramBuffer = NULL;
        if (psramFound()) {
            psramBuffer = heap_caps_malloc(allocSize, MALLOC_CAP_SPIRAM);
        }
        
        if (psramBuffer != NULL) {
            Serial.printf("[PSRAM] Allocated %d KB in PSRAM at 0x%08X\n",
                         allocSize / 1024, (unsigned int)psramBuffer);
            
            // Use the buffer
            memset(psramBuffer, 0xAA, allocSize);
            
            // Verify
            uint8_t *p = (uint8_t*)psramBuffer;
            bool ok = true;
            for (int i = 0; i < 100; i++) {
                if (p[i] != 0xAA) ok = false;
            }
            Serial.printf("[PSRAM] Data integrity: %s\n", ok ? "OK" : "FAILED");
            
            vTaskDelay(pdMS_TO_TICKS(2000));
            
            heap_caps_free(psramBuffer);
            Serial.println("[PSRAM] Buffer freed");
        } else {
            Serial.println("[PSRAM] Allocation failed - trying smaller size");
            
            // Try smaller internal allocation
            void *smallBuf = malloc(10240);
            if (smallBuf) {
                Serial.println("[PSRAM] Fallback: 10KB internal allocation OK");
                free(smallBuf);
            }
        }
        
        printMemoryInfo();
        
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void vInternalTask(void *pvParameters) {
    Serial.println("[Internal] Task started - using internal RAM only");
    
    for (;;) {
        // Allocate explicitly in internal RAM
        LargeDataBlock_t *data = (LargeDataBlock_t*)
            heap_caps_malloc(sizeof(LargeDataBlock_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        
        if (data != NULL) {
            data->timestamp = millis();
            strcpy(data->description, "Internal RAM data block");
            
            // Fill with sensor-like data
            for (int i = 0; i < 1024; i++) {
                data->data[i] = (float)i * 0.1;
            }
            
            Serial.printf("[Internal] Block at 0x%08X, size %d bytes\n",
                         (unsigned int)data, sizeof(LargeDataBlock_t));
            
            heap_caps_free(data);
        }
        
        // Blink
        digitalWrite(LED_PIN, HIGH);
        vTaskDelay(pdMS_TO_TICKS(100));
        digitalWrite(LED_PIN, LOW);
        
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n═══════════════════════════════════════════════════");
    Serial.println("           ESP32 PSRAM USAGE DEMONSTRATION");
    Serial.println("═══════════════════════════════════════════════════");
    
    pinMode(LED_PIN, OUTPUT);
    
    // Check PSRAM
    if (psramFound()) {
        Serial.println("\n✓ PSRAM detected!");
        Serial.printf("  Size: %.2f MB\n", ESP.getPsramSize() / 1048576.0);
    } else {
        Serial.println("\n⚠️  No PSRAM detected");
        Serial.println("  Demo will use internal RAM only");
    }
    
    printMemoryInfo();
    
    // Create tasks
    xTaskCreatePinnedToCore(vPSRAMTask, "PSRAM", 8192, NULL, 1, &xPSRAMTask, 0);
    xTaskCreatePinnedToCore(vInternalTask, "Internal", 4096, NULL, 1, &xInternalTask, 1);
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}

/**
 * PSRAM USE CASES:
 * - Large buffers (audio, video frames)
 * - Web server content
 * - TLS/SSL certificates and buffers
 * - Machine learning models
 * - Caching
 * 
 * PERFORMANCE NOTE:
 * PSRAM is ~10x slower than internal RAM
 * Use for data, not frequently accessed variables
 */
