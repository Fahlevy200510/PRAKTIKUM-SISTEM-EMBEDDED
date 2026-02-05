/**
 * @file main.cpp
 * @brief Program 09: Memory Fragmentation Analysis pada STM32
 * 
 * Deskripsi:
 * Demonstrasi dan analisis fragmentasi heap memory.
 * Menunjukkan bagaimana pola alokasi mempengaruhi fragmentasi.
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include <STM32FreeRTOS.h>

// ==================== KONFIGURASI ====================
#define LED_PIN             PC13
#define SERIAL_BAUD         115200

// Simulation parameters
#define NUM_ALLOCATIONS     20
#define MAX_ALLOC_SIZE      512
#define MIN_ALLOC_SIZE      32

// ==================== VARIABEL GLOBAL ====================
TaskHandle_t xFragmenterTask = NULL;
TaskHandle_t xMonitorTask = NULL;

void *allocations[NUM_ALLOCATIONS] = {NULL};
size_t allocSizes[NUM_ALLOCATIONS] = {0};

volatile uint32_t totalAllocated = 0;
volatile uint32_t totalFreed = 0;
volatile uint32_t allocAttempts = 0;
volatile uint32_t allocFailures = 0;

// ==================== HEAP ANALYSIS FUNCTIONS ====================

/**
 * @brief Calculate fragmentation index
 * 
 * Fragmentation = 1 - (largest_free_block / total_free)
 * 0% = no fragmentation, 100% = highly fragmented
 */
float calculateFragmentation() {
    size_t totalFree = xPortGetFreeHeapSize();
    size_t largestBlock = xPortGetMinimumEverFreeHeapSize();
    
    // Get actual largest block (this is an approximation)
    // In real heap_4/5, you'd need to walk the free list
    void *testBlock = pvPortMalloc(totalFree);
    if (testBlock != NULL) {
        largestBlock = totalFree;
        vPortFree(testBlock);
    } else {
        // Binary search for largest allocatable block
        size_t low = 0, high = totalFree;
        while (low < high - 1) {
            size_t mid = (low + high) / 2;
            void *test = pvPortMalloc(mid);
            if (test != NULL) {
                low = mid;
                vPortFree(test);
            } else {
                high = mid;
            }
        }
        largestBlock = low;
    }
    
    if (totalFree == 0) return 100.0;
    return (1.0 - (float)largestBlock / totalFree) * 100.0;
}

/**
 * @brief Print detailed heap status
 */
void printHeapStatus() {
    size_t freeHeap = xPortGetFreeHeapSize();
    size_t minEver = xPortGetMinimumEverFreeHeapSize();
    
    Serial.println("\n╔══════════════════════════════════════════════════╗");
    Serial.println("║           HEAP FRAGMENTATION ANALYSIS            ║");
    Serial.println("╠══════════════════════════════════════════════════╣");
    Serial.printf("║ Total Heap:       %5d bytes                    ║\n", configTOTAL_HEAP_SIZE);
    Serial.printf("║ Free Heap:        %5d bytes                    ║\n", freeHeap);
    Serial.printf("║ Used Heap:        %5d bytes                    ║\n", configTOTAL_HEAP_SIZE - freeHeap);
    Serial.printf("║ Min Ever Free:    %5d bytes                    ║\n", minEver);
    Serial.println("╠══════════════════════════════════════════════════╣");
    
    // Calculate fragmentation
    float fragmentation = calculateFragmentation();
    
    Serial.printf("║ Fragmentation:    %5.1f%%                         ║\n", fragmentation);
    
    // Visual fragmentation bar
    Serial.print("║ [");
    int filled = (int)(fragmentation / 5);
    for (int i = 0; i < 20; i++) {
        Serial.print(i < filled ? "█" : "░");
    }
    Serial.println("]                   ║");
    
    // Health indicator
    if (fragmentation < 20) {
        Serial.println("║ ✓ Heap Health: GOOD                              ║");
    } else if (fragmentation < 50) {
        Serial.println("║ ⚡ Heap Health: MODERATE                          ║");
    } else {
        Serial.println("║ ⚠️  Heap Health: POOR (high fragmentation)        ║");
    }
    
    Serial.println("╠══════════════════════════════════════════════════╣");
    Serial.println("║ Allocation Statistics:                           ║");
    Serial.printf("║   Attempts:    %5lu                              ║\n", allocAttempts);
    Serial.printf("║   Failures:    %5lu                              ║\n", allocFailures);
    Serial.printf("║   Total alloc: %5lu bytes                        ║\n", totalAllocated);
    Serial.printf("║   Total freed: %5lu bytes                        ║\n", totalFreed);
    Serial.println("╚══════════════════════════════════════════════════╝");
}

/**
 * @brief Print allocation map
 */
void printAllocationMap() {
    Serial.println("\n┌─────────────────────────────────────────────────┐");
    Serial.println("│             ALLOCATION MAP                      │");
    Serial.println("├─────────────────────────────────────────────────┤");
    
    int activeCount = 0;
    for (int i = 0; i < NUM_ALLOCATIONS; i++) {
        if (allocations[i] != NULL) {
            Serial.printf("│ [%2d] 0x%08X  Size: %4d bytes             │\n",
                         i, (unsigned int)allocations[i], allocSizes[i]);
            activeCount++;
        }
    }
    
    Serial.printf("│ Active allocations: %d / %d                     │\n",
                 activeCount, NUM_ALLOCATIONS);
    Serial.println("└─────────────────────────────────────────────────┘");
}

// ==================== TASKS ====================

/**
 * @brief Fragmenter task - creates fragmentation pattern
 */
void vFragmenterTask(void *pvParameters) {
    Serial.println("[Fragmenter] Task started");
    Serial.println("[Fragmenter] Will demonstrate fragmentation patterns\n");
    
    int phase = 0;
    
    for (;;) {
        phase++;
        Serial.printf("\n═══════════ PHASE %d ═══════════\n", phase);
        
        switch (phase) {
            case 1: {
                // Phase 1: Sequential allocation
                Serial.println("Phase 1: Sequential allocation of varying sizes");
                
                for (int i = 0; i < NUM_ALLOCATIONS && i < 10; i++) {
                    size_t size = MIN_ALLOC_SIZE + (i * 32);
                    allocAttempts++;
                    
                    allocations[i] = pvPortMalloc(size);
                    if (allocations[i] != NULL) {
                        allocSizes[i] = size;
                        totalAllocated += size;
                        memset(allocations[i], i & 0xFF, size);
                        Serial.printf("[Fragmenter] Allocated [%d]: %d bytes\n", i, size);
                    } else {
                        allocFailures++;
                        Serial.printf("[Fragmenter] FAILED [%d]: %d bytes\n", i, size);
                    }
                }
                
                printHeapStatus();
                vTaskDelay(pdMS_TO_TICKS(3000));
                break;
            }
            
            case 2: {
                // Phase 2: Free every other block (creates holes)
                Serial.println("Phase 2: Free alternating blocks (create fragmentation)");
                
                for (int i = 0; i < NUM_ALLOCATIONS; i += 2) {
                    if (allocations[i] != NULL) {
                        Serial.printf("[Fragmenter] Freeing [%d]: %d bytes\n", i, allocSizes[i]);
                        totalFreed += allocSizes[i];
                        vPortFree(allocations[i]);
                        allocations[i] = NULL;
                        allocSizes[i] = 0;
                    }
                }
                
                printHeapStatus();
                printAllocationMap();
                vTaskDelay(pdMS_TO_TICKS(3000));
                break;
            }
            
            case 3: {
                // Phase 3: Try large allocation (may fail due to fragmentation)
                Serial.println("Phase 3: Attempt large allocation (fragmentation test)");
                
                // Try to allocate a block larger than any single hole
                size_t largeSize = 800;
                allocAttempts++;
                
                Serial.printf("[Fragmenter] Attempting %d byte allocation...\n", largeSize);
                
                void *largeBlock = pvPortMalloc(largeSize);
                if (largeBlock != NULL) {
                    Serial.println("[Fragmenter] SUCCESS - heap not too fragmented");
                    vPortFree(largeBlock);
                } else {
                    allocFailures++;
                    Serial.println("[Fragmenter] FAILED - heap too fragmented!");
                    Serial.println("[Fragmenter] This demonstrates fragmentation problem");
                }
                
                printHeapStatus();
                vTaskDelay(pdMS_TO_TICKS(3000));
                break;
            }
            
            case 4: {
                // Phase 4: Free all and show recovery
                Serial.println("Phase 4: Free all - heap should coalesce (heap_4/5)");
                
                for (int i = 0; i < NUM_ALLOCATIONS; i++) {
                    if (allocations[i] != NULL) {
                        totalFreed += allocSizes[i];
                        vPortFree(allocations[i]);
                        allocations[i] = NULL;
                        allocSizes[i] = 0;
                    }
                }
                
                Serial.println("[Fragmenter] All memory freed");
                printHeapStatus();
                
                // Reset for next cycle
                phase = 0;
                vTaskDelay(pdMS_TO_TICKS(5000));
                break;
            }
        }
    }
}

/**
 * @brief Monitor task - periodic status updates
 */
void vMonitorTask(void *pvParameters) {
    Serial.println("[Monitor] Task started");
    
    for (;;) {
        // Just blink LED to show system alive
        digitalWrite(LED_PIN, LOW);
        vTaskDelay(pdMS_TO_TICKS(100));
        digitalWrite(LED_PIN, HIGH);
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// ==================== SETUP & LOOP ====================

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(2000);
    
    Serial.println("\n════════════════════════════════════════════════════════════");
    Serial.println("        MODUL 12: MEMORY FRAGMENTATION ANALYSIS             ");
    Serial.println("════════════════════════════════════════════════════════════\n");
    
    Serial.println("What is Fragmentation?");
    Serial.println("  External: Free memory split into non-contiguous blocks");
    Serial.println("  Internal: Wasted space within allocated blocks");
    Serial.println("");
    Serial.println("FreeRTOS Heap Schemes:");
    Serial.println("  heap_1: No free, no fragmentation");
    Serial.println("  heap_2: Free but no coalescing - fragments quickly");
    Serial.println("  heap_3: Uses standard library malloc");
    Serial.println("  heap_4: Coalesces adjacent free blocks - best for STM32");
    Serial.println("  heap_5: Like heap_4 but multiple memory regions");
    Serial.println("");
    
    Serial.printf("Using heap scheme with configTOTAL_HEAP_SIZE = %d bytes\n\n",
                 configTOTAL_HEAP_SIZE);
    
    // Initialize LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    
    // Initialize random
    randomSeed(analogRead(PA0));
    
    // Print initial heap status
    Serial.println("Initial heap status:");
    printHeapStatus();
    
    // Create tasks
    xTaskCreate(vFragmenterTask, "Fragmenter", 512, NULL, 1, &xFragmenterTask);
    xTaskCreate(vMonitorTask, "Monitor", 128, NULL, 1, &xMonitorTask);
    
    Serial.println("\nStarting fragmentation demonstration...\n");
    
    vTaskStartScheduler();
    
    Serial.println("[ERROR] Scheduler failed!");
}

void loop() {
    // Empty
}

/**
 * PENJELASAN FRAGMENTATION:
 * 
 * 1. External Fragmentation:
 *    - Total free memory cukup, tapi tidak contiguous
 *    - Alokasi besar gagal meski total free cukup
 *    - Solusi: heap_4/5 dengan coalescing
 * 
 * 2. Internal Fragmentation:
 *    - Wasted space dalam allocated blocks
 *    - Karena alignment requirements
 *    - Minimize dengan proper sizing
 * 
 * 3. Fragmentation Index:
 *    - 0%: No fragmentation (all free in one block)
 *    - 100%: Maximum fragmentation
 *    - Formula: 1 - (largest_free / total_free)
 * 
 * 4. Prevention Strategies:
 *    - Use memory pools untuk fixed-size allocations
 *    - Minimize dynamic allocation
 *    - Allocate long-lived objects first
 *    - Use static allocation where possible
 *    - Consider heap_4 atau heap_5
 * 
 * EXPECTED OUTPUT:
 * 
 * ═══════════ PHASE 1 ═══════════
 * Phase 1: Sequential allocation
 * [Fragmenter] Allocated [0]: 32 bytes
 * [Fragmenter] Allocated [1]: 64 bytes
 * ...
 * 
 * ═══════════ PHASE 2 ═══════════
 * Phase 2: Free alternating blocks
 * [Fragmenter] Freeing [0]: 32 bytes
 * [Fragmenter] Freeing [2]: 96 bytes
 * 
 * Fragmentation:    35.2%
 * 
 * ═══════════ PHASE 3 ═══════════
 * [Fragmenter] Attempting 800 byte allocation...
 * [Fragmenter] FAILED - heap too fragmented!
 */
