/**
 * @file main.cpp
 * @brief Program 01: ESP32 Heap Info and Multi-Heap Analysis
 * 
 * Deskripsi:
 * Demonstrasi sistem memory multi-heap pada ESP32.
 * ESP32 memiliki beberapa region memory dengan karakteristik berbeda.
 * 
 * Memory Regions ESP32:
 * - Internal RAM (DRAM): ~320KB, fast
 * - Internal RAM (IRAM): ~128KB, for code/data
 * - PSRAM (External): 4MB (if available)
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "esp_system.h"

// ==================== KONFIGURASI ====================
#define LED_PIN         2
#define SERIAL_BAUD     115200
#define MONITOR_INTERVAL_MS  5000

// ==================== VARIABEL GLOBAL ====================
TaskHandle_t xMonitorTask = NULL;
TaskHandle_t xAllocatorTask = NULL;

uint32_t allocationCount = 0;
uint32_t freeCount = 0;

// ==================== FUNGSI HELPER ====================

/**
 * @brief Menampilkan info heap lengkap ESP32
 */
void printESP32HeapInfo() {
    Serial.println("\n╔══════════════════════════════════════════════════════════╗");
    Serial.println("║               ESP32 HEAP INFORMATION                      ║");
    Serial.println("╠══════════════════════════════════════════════════════════╣");
    
    // Total heap (all regions)
    Serial.println("║ [TOTAL HEAP - ALL REGIONS]                                ║");
    Serial.printf("║   Free:     %7d bytes                                  ║\n", 
                  ESP.getFreeHeap());
    Serial.printf("║   Min Free: %7d bytes                                  ║\n", 
                  ESP.getMinFreeHeap());
    Serial.printf("║   Max Alloc:%7d bytes                                  ║\n", 
                  ESP.getMaxAllocHeap());
    
    Serial.println("╠══════════════════════════════════════════════════════════╣");
    
    // Internal RAM (DRAM) - Default heap
    Serial.println("║ [INTERNAL DRAM - Default Heap]                            ║");
    Serial.printf("║   Free:     %7d bytes                                  ║\n", 
                  heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    Serial.printf("║   Min Free: %7d bytes                                  ║\n", 
                  heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL));
    Serial.printf("║   Largest:  %7d bytes                                  ║\n", 
                  heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
    
    Serial.println("╠══════════════════════════════════════════════════════════╣");
    
    // DMA-capable memory
    Serial.println("║ [DMA-CAPABLE MEMORY]                                      ║");
    Serial.printf("║   Free:     %7d bytes                                  ║\n", 
                  heap_caps_get_free_size(MALLOC_CAP_DMA));
    Serial.printf("║   Min Free: %7d bytes                                  ║\n", 
                  heap_caps_get_minimum_free_size(MALLOC_CAP_DMA));
    
    Serial.println("╠══════════════════════════════════════════════════════════╣");
    
    // 32-bit aligned memory (fast access)
    Serial.println("║ [32-BIT ALIGNED MEMORY]                                   ║");
    Serial.printf("║   Free:     %7d bytes                                  ║\n", 
                  heap_caps_get_free_size(MALLOC_CAP_32BIT));
    
    Serial.println("╠══════════════════════════════════════════════════════════╣");
    
    // PSRAM (if available)
    size_t psramFree = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    if (psramFree > 0) {
        Serial.println("║ [EXTERNAL PSRAM]                                          ║");
        Serial.printf("║   Free:     %7d bytes                                  ║\n", psramFree);
        Serial.printf("║   Total:    %7d bytes                                  ║\n", 
                      ESP.getPsramSize());
    } else {
        Serial.println("║ [EXTERNAL PSRAM] Not available                            ║");
    }
    
    Serial.println("╠══════════════════════════════════════════════════════════╣");
    
    // Memory usage percentage
    float usagePercent = 100.0 - ((float)ESP.getFreeHeap() / ESP.getHeapSize() * 100);
    Serial.printf("║ Heap Usage: %.1f%%                                         ║\n", usagePercent);
    
    // Allocation statistics
    Serial.printf("║ Allocations: %lu | Frees: %lu                             ║\n", 
                  allocationCount, freeCount);
    
    Serial.println("╠══════════════════════════════════════════════════════════╣");
    
    // Fragmentation indicator
    size_t largest = heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL);
    size_t total = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    float fragmentation = 100.0 - ((float)largest / total * 100);
    
    if (fragmentation < 20) {
        Serial.println("║ ✓ Memory health: GOOD (low fragmentation)                 ║");
    } else if (fragmentation < 50) {
        Serial.println("║ ⚡ Memory health: MODERATE (some fragmentation)            ║");
    } else {
        Serial.println("║ ⚠️  Memory health: WARNING (high fragmentation)            ║");
    }
    Serial.printf("║   Fragmentation Index: %.1f%%                              ║\n", fragmentation);
    
    Serial.println("╚══════════════════════════════════════════════════════════╝");
}

/**
 * @brief Menampilkan task stack info
 */
void printTaskStackInfo() {
    Serial.println("\n┌─────────────────────────────────────────────────┐");
    Serial.println("│           TASK STACK INFORMATION                │");
    Serial.println("├─────────────────────────────────────────────────┤");
    
    // Get all tasks info
    UBaseType_t numTasks = uxTaskGetNumberOfTasks();
    Serial.printf("│ Total tasks: %d                                 │\n", numTasks);
    
    // Current task
    if (xMonitorTask != NULL) {
        UBaseType_t hwm = uxTaskGetStackHighWaterMark(xMonitorTask);
        Serial.printf("│ Monitor Task HWM: %5d words (%5d bytes)   │\n", hwm, hwm * 4);
    }
    
    if (xAllocatorTask != NULL) {
        UBaseType_t hwm = uxTaskGetStackHighWaterMark(xAllocatorTask);
        Serial.printf("│ Allocator Task HWM: %5d words (%5d bytes) │\n", hwm, hwm * 4);
    }
    
    Serial.println("└─────────────────────────────────────────────────┘");
}

/**
 * @brief Demonstrasi alokasi di berbagai region
 */
void demonstrateCapabilitiesAllocation() {
    Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    Serial.println("      Demonstrasi Memory Capabilities Allocation");
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    
    // Allocate in default heap
    Serial.println("\n1. Default allocation (pvPortMalloc):");
    void *defaultPtr = pvPortMalloc(1024);
    if (defaultPtr) {
        allocationCount++;
        Serial.printf("   Allocated 1024 bytes at 0x%08X\n", (unsigned int)defaultPtr);
        vPortFree(defaultPtr);
        freeCount++;
        Serial.println("   Freed successfully");
    }
    
    // Allocate DMA-capable memory
    Serial.println("\n2. DMA-capable memory:");
    void *dmaPtr = heap_caps_malloc(512, MALLOC_CAP_DMA);
    if (dmaPtr) {
        allocationCount++;
        Serial.printf("   Allocated 512 bytes DMA-capable at 0x%08X\n", (unsigned int)dmaPtr);
        heap_caps_free(dmaPtr);
        freeCount++;
        Serial.println("   Freed successfully");
    } else {
        Serial.println("   Failed to allocate DMA memory!");
    }
    
    // Allocate 32-bit aligned memory
    Serial.println("\n3. 32-bit aligned memory:");
    void *alignedPtr = heap_caps_malloc(256, MALLOC_CAP_32BIT);
    if (alignedPtr) {
        allocationCount++;
        Serial.printf("   Allocated 256 bytes 32-bit aligned at 0x%08X\n", (unsigned int)alignedPtr);
        
        // Verify alignment
        if (((uintptr_t)alignedPtr & 0x3) == 0) {
            Serial.println("   ✓ Alignment verified (divisible by 4)");
        }
        
        heap_caps_free(alignedPtr);
        freeCount++;
        Serial.println("   Freed successfully");
    }
    
    // Try PSRAM allocation (will fail if no PSRAM)
    Serial.println("\n4. PSRAM allocation (if available):");
    void *psramPtr = heap_caps_malloc(4096, MALLOC_CAP_SPIRAM);
    if (psramPtr) {
        allocationCount++;
        Serial.printf("   Allocated 4096 bytes PSRAM at 0x%08X\n", (unsigned int)psramPtr);
        heap_caps_free(psramPtr);
        freeCount++;
        Serial.println("   Freed successfully");
    } else {
        Serial.println("   PSRAM not available or allocation failed");
    }
    
    Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
}

// ==================== TASKS ====================

/**
 * @brief Monitor task - menampilkan heap info periodik
 */
void vMonitorTask(void *pvParameters) {
    Serial.println("[Monitor] Task started");
    
    // Initial demonstration
    vTaskDelay(pdMS_TO_TICKS(1000));
    demonstrateCapabilitiesAllocation();
    
    for (;;) {
        printESP32HeapInfo();
        printTaskStackInfo();
        
        // LED blink to show activity
        digitalWrite(LED_PIN, HIGH);
        vTaskDelay(pdMS_TO_TICKS(100));
        digitalWrite(LED_PIN, LOW);
        
        vTaskDelay(pdMS_TO_TICKS(MONITOR_INTERVAL_MS));
    }
}

/**
 * @brief Allocator task - melakukan alokasi/free secara periodik
 */
void vAllocatorTask(void *pvParameters) {
    Serial.println("[Allocator] Task started");
    
    void *buffers[5] = {NULL};
    uint16_t sizes[] = {128, 256, 512, 1024, 2048};
    int currentIndex = 0;
    bool allocating = true;
    
    vTaskDelay(pdMS_TO_TICKS(3000));  // Wait for monitor to print first
    
    for (;;) {
        if (allocating) {
            // Allocate next buffer
            if (buffers[currentIndex] == NULL) {
                buffers[currentIndex] = pvPortMalloc(sizes[currentIndex]);
                if (buffers[currentIndex]) {
                    allocationCount++;
                    Serial.printf("[Allocator] Allocated %d bytes\n", sizes[currentIndex]);
                    memset(buffers[currentIndex], 0xAA, sizes[currentIndex]);
                }
            }
            
            currentIndex++;
            if (currentIndex >= 5) {
                currentIndex = 0;
                allocating = false;
                Serial.println("[Allocator] Switching to free mode");
            }
        } else {
            // Free buffers
            if (buffers[currentIndex] != NULL) {
                vPortFree(buffers[currentIndex]);
                buffers[currentIndex] = NULL;
                freeCount++;
                Serial.printf("[Allocator] Freed %d bytes\n", sizes[currentIndex]);
            }
            
            currentIndex++;
            if (currentIndex >= 5) {
                currentIndex = 0;
                allocating = true;
                Serial.println("[Allocator] Switching to allocate mode");
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// ==================== SETUP & LOOP ====================

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(2000);
    
    Serial.println("\n════════════════════════════════════════════════════════════");
    Serial.println("         MODUL 12: ESP32 HEAP INFO AND MULTI-HEAP           ");
    Serial.println("════════════════════════════════════════════════════════════\n");
    
    // Initialize LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    
    // Print initial system info
    Serial.println("ESP32 System Information:");
    Serial.printf("  Chip Model: %s\n", ESP.getChipModel());
    Serial.printf("  Chip Revision: %d\n", ESP.getChipRevision());
    Serial.printf("  CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("  Flash Size: %d bytes\n", ESP.getFlashChipSize());
    Serial.printf("  Heap Size: %d bytes\n", ESP.getHeapSize());
    
    if (ESP.getPsramSize() > 0) {
        Serial.printf("  PSRAM Size: %d bytes\n", ESP.getPsramSize());
    } else {
        Serial.println("  PSRAM: Not available");
    }
    
    Serial.println("\nCreating tasks...");
    
    // Create tasks
    xTaskCreatePinnedToCore(
        vMonitorTask,
        "Monitor",
        4096,           // Stack size (bytes for ESP32)
        NULL,
        2,              // Priority
        &xMonitorTask,
        0               // Core 0
    );
    
    xTaskCreatePinnedToCore(
        vAllocatorTask,
        "Allocator",
        4096,
        NULL,
        1,
        &xAllocatorTask,
        1               // Core 1
    );
    
    Serial.println("Tasks created. Scheduler running...\n");
}

void loop() {
    // FreeRTOS handles everything
    vTaskDelay(pdMS_TO_TICKS(1000));
}

/**
 * EXPECTED OUTPUT:
 * 
 * ╔══════════════════════════════════════════════════════════╗
 * ║               ESP32 HEAP INFORMATION                      ║
 * ╠══════════════════════════════════════════════════════════╣
 * ║ [TOTAL HEAP - ALL REGIONS]                                ║
 * ║   Free:     285432 bytes                                  ║
 * ║   Min Free: 280156 bytes                                  ║
 * ║   Max Alloc: 113792 bytes                                 ║
 * ╠══════════════════════════════════════════════════════════╣
 * ║ [INTERNAL DRAM - Default Heap]                            ║
 * ║   Free:     285432 bytes                                  ║
 * ║   Min Free: 280156 bytes                                  ║
 * ║   Largest:  113792 bytes                                  ║
 * ║ ... (more output)
 */
