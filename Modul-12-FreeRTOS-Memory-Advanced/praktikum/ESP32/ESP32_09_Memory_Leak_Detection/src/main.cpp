/**
 * @file main.cpp
 * @brief Program 09: ESP32 Memory Leak Detection
 * 
 * Demonstrasi teknik deteksi memory leak pada ESP32.
 * Penting untuk aplikasi long-running.
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

// Memory tracking
typedef struct {
    uint32_t timestamp;
    size_t freeHeap;
    size_t minFreeHeap;
    size_t largestBlock;
} MemorySnapshot_t;

#define MAX_SNAPSHOTS   50
MemorySnapshot_t snapshots[MAX_SNAPSHOTS];
int snapshotIndex = 0;

// Simulated memory leak tracking
volatile uint32_t leakedBytes = 0;
volatile uint32_t leakCount = 0;
volatile bool leakSimulationActive = false;

void takeMemorySnapshot() {
    if (snapshotIndex < MAX_SNAPSHOTS) {
        snapshots[snapshotIndex].timestamp = millis();
        snapshots[snapshotIndex].freeHeap = ESP.getFreeHeap();
        snapshots[snapshotIndex].minFreeHeap = ESP.getMinFreeHeap();
        snapshots[snapshotIndex].largestBlock = heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);
        snapshotIndex++;
    }
}

void analyzeMemoryTrend() {
    if (snapshotIndex < 5) {
        Serial.println("Not enough snapshots for analysis");
        return;
    }
    
    Serial.println("\n╔══════════════════════════════════════════════════════════╗");
    Serial.println("║             MEMORY LEAK ANALYSIS REPORT                  ║");
    Serial.println("╠══════════════════════════════════════════════════════════╣");
    
    // Calculate trend
    int32_t totalChange = snapshots[snapshotIndex-1].freeHeap - snapshots[0].freeHeap;
    float avgChangePerSnapshot = (float)totalChange / (snapshotIndex - 1);
    uint32_t duration = snapshots[snapshotIndex-1].timestamp - snapshots[0].timestamp;
    float leakRatePerMin = (avgChangePerSnapshot * 60000.0) / (duration / (snapshotIndex - 1));
    
    Serial.println("║ Snapshot History:                                        ║");
    for (int i = 0; i < snapshotIndex; i += snapshotIndex / 5 + 1) {
        Serial.printf("║   [%2d] Time: %6lu ms, Free: %6d bytes              ║\n",
                     i, snapshots[i].timestamp, snapshots[i].freeHeap);
    }
    
    Serial.println("╠══════════════════════════════════════════════════════════╣");
    Serial.printf("║ Total memory change: %+ld bytes                          ║\n", totalChange);
    Serial.printf("║ Leak rate: %.1f bytes/minute                            ║\n", -leakRatePerMin);
    Serial.printf("║ Analysis duration: %.1f seconds                         ║\n", duration / 1000.0);
    
    Serial.println("╠══════════════════════════════════════════════════════════╣");
    
    // Diagnosis
    if (totalChange > 100) {
        Serial.println("║ ✓ Status: MEMORY INCREASING (unexpected allocation)     ║");
    } else if (totalChange < -1000) {
        Serial.println("║ ⚠️  Status: MEMORY LEAK DETECTED                          ║");
        Serial.printf("║    Estimated time to crash: %.1f hours                  ║\n",
                     (snapshots[snapshotIndex-1].freeHeap / (-avgChangePerSnapshot)) / 3600.0);
    } else if (totalChange < -100) {
        Serial.println("║ ⚡ Status: POSSIBLE MINOR LEAK                           ║");
    } else {
        Serial.println("║ ✓ Status: MEMORY STABLE                                  ║");
    }
    
    Serial.println("╚══════════════════════════════════════════════════════════╝");
}

void printDetailedHeapInfo() {
    Serial.println("\n┌────────────────────────────────────────────────┐");
    Serial.println("│         DETAILED HEAP INFORMATION              │");
    Serial.println("├────────────────────────────────────────────────┤");
    
    multi_heap_info_t info;
    heap_caps_get_info(&info, MALLOC_CAP_DEFAULT);
    
    Serial.printf("│ Total free:        %7d bytes               │\n", info.total_free_bytes);
    Serial.printf("│ Total allocated:   %7d bytes               │\n", info.total_allocated_bytes);
    Serial.printf("│ Largest free:      %7d bytes               │\n", info.largest_free_block);
    Serial.printf("│ Min ever free:     %7d bytes               │\n", info.minimum_free_bytes);
    Serial.printf("│ Allocated blocks:  %7d                     │\n", info.allocated_blocks);
    Serial.printf("│ Free blocks:       %7d                     │\n", info.free_blocks);
    Serial.printf("│ Total blocks:      %7d                     │\n", info.total_blocks);
    Serial.println("├────────────────────────────────────────────────┤");
    
    // Fragmentation metric
    float fragmentation = 100.0 * (1.0 - (float)info.largest_free_block / info.total_free_bytes);
    Serial.printf("│ Fragmentation:     %6.1f%%                    │\n", fragmentation);
    
    Serial.println("└────────────────────────────────────────────────┘");
}

// Simulated leaky task
void vLeakyTask(void *pvParameters) {
    Serial.println("[Leaky] Task started - will simulate memory leaks");
    
    for (;;) {
        if (leakSimulationActive) {
            // Intentionally allocate without freeing (LEAK!)
            size_t leakSize = random(100, 500);
            void *leaked = malloc(leakSize);
            
            if (leaked != NULL) {
                memset(leaked, 0xAA, leakSize);  // Touch the memory
                leakedBytes += leakSize;
                leakCount++;
                Serial.printf("[Leaky] 🔴 Leaked %d bytes (total: %lu)\n", 
                             leakSize, leakedBytes);
                // NOT FREEING - intentional leak for demo!
            } else {
                Serial.println("[Leaky] Allocation failed - heap exhausted!");
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// Good task that properly manages memory
void vGoodTask(void *pvParameters) {
    Serial.println("[Good] Task started - properly manages memory");
    
    for (;;) {
        // Proper allocation and deallocation
        size_t size = random(200, 800);
        void *buffer = malloc(size);
        
        if (buffer != NULL) {
            memset(buffer, 0xBB, size);
            vTaskDelay(pdMS_TO_TICKS(100));  // Use memory
            free(buffer);  // PROPERLY FREE!
            Serial.printf("[Good] ✓ Allocated and freed %d bytes\n", size);
        }
        
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void vMonitorTask(void *pvParameters) {
    Serial.println("[Monitor] Started - tracking memory over time");
    
    for (;;) {
        // Take snapshot
        takeMemorySnapshot();
        
        printDetailedHeapInfo();
        
        if (snapshotIndex >= 5) {
            analyzeMemoryTrend();
        }
        
        // LED blink
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n═══════════════════════════════════════════════════════");
    Serial.println("        ESP32 MEMORY LEAK DETECTION DEMONSTRATION");
    Serial.println("═══════════════════════════════════════════════════════\n");
    
    pinMode(LED_PIN, OUTPUT);
    
    Serial.println("Memory Leak Detection Techniques:");
    Serial.println("  1. Periodic heap snapshots");
    Serial.println("  2. Trend analysis over time");
    Serial.println("  3. Fragmentation monitoring");
    Serial.println("  4. Min-ever-free tracking\n");
    
    Serial.println("This demo includes:");
    Serial.println("  - A 'Leaky' task that intentionally leaks memory");
    Serial.println("  - A 'Good' task that properly frees memory");
    Serial.println("  - Monitor task that detects the leak\n");
    
    // Initial snapshot
    takeMemorySnapshot();
    
    // Create tasks
    xTaskCreatePinnedToCore(vLeakyTask, "Leaky", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(vGoodTask, "Good", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(vMonitorTask, "Monitor", 8192, NULL, 2, NULL, 0);
    
    // Enable leak simulation after 5 seconds
    Serial.println("Leak simulation will start in 5 seconds...\n");
    vTaskDelay(pdMS_TO_TICKS(5000));
    leakSimulationActive = true;
    Serial.println("🔴 Leak simulation ACTIVE - watch memory decrease!\n");
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}

/**
 * MEMORY LEAK DETECTION TIPS:
 * 
 * 1. Regular Snapshots:
 *    - Take heap snapshots periodically
 *    - Compare free heap over time
 *    - Look for consistent decrease
 * 
 * 2. Min Ever Free:
 *    - ESP.getMinFreeHeap() tracks lowest point
 *    - Should stabilize in healthy system
 *    - Continuous decrease = leak
 * 
 * 3. Allocation Tracking:
 *    - Use heap_caps_get_info() for detailed stats
 *    - Track allocation count vs free count
 * 
 * 4. Prevention:
 *    - Always pair malloc/free
 *    - Use RAII patterns
 *    - Consider memory pools for fixed allocations
 *    - Code review malloc/new usage
 */
