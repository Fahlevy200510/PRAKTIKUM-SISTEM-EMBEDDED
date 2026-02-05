/**
 * @file main.cpp
 * @brief Program 01: Heap Statistics Monitor - STM32F103C8T6
 * 
 * Deskripsi:
 * Program untuk memonitor penggunaan heap memory FreeRTOS secara real-time.
 * Menampilkan statistik heap lengkap termasuk fragmentasi.
 * 
 * Fitur:
 * - Monitoring heap size dan free heap
 * - Tracking minimum ever free heap
 * - Mendeteksi fragmentasi
 * - Alert saat heap rendah
 * 
 * Hardware:
 * - STM32F103C8T6 Blue Pill
 * - LED Built-in (PC13) untuk status
 * - Serial Monitor untuk output
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include <STM32FreeRTOS.h>

// ==================== KONFIGURASI ====================
#define LED_PIN         PC13
#define SERIAL_BAUD     115200

#define HEAP_WARNING_THRESHOLD  2048    // Warning jika free heap < 2KB
#define MONITOR_INTERVAL_MS     5000    // Interval monitoring 5 detik

// ==================== VARIABEL GLOBAL ====================
TaskHandle_t xMonitorTask = NULL;
TaskHandle_t xWorkerTask1 = NULL;
TaskHandle_t xWorkerTask2 = NULL;

volatile uint32_t allocationCount = 0;
volatile uint32_t freeCount = 0;

// ==================== FUNGSI HELPER ====================

/**
 * @brief Menampilkan statistik heap lengkap
 */
void printHeapStats(void) {
    size_t freeHeap = xPortGetFreeHeapSize();
    size_t minEverFree = xPortGetMinimumEverFreeHeapSize();
    
    Serial.println("\n╔══════════════════════════════════════════╗");
    Serial.println("║         HEAP STATISTICS REPORT           ║");
    Serial.println("╠══════════════════════════════════════════╣");
    
    Serial.printf("║ Total Heap Size:     %6d bytes        ║\n", configTOTAL_HEAP_SIZE);
    Serial.printf("║ Free Heap Now:       %6d bytes        ║\n", freeHeap);
    Serial.printf("║ Used Heap:           %6d bytes        ║\n", configTOTAL_HEAP_SIZE - freeHeap);
    Serial.printf("║ Minimum Ever Free:   %6d bytes        ║\n", minEverFree);
    Serial.printf("║ Peak Usage:          %6d bytes        ║\n", configTOTAL_HEAP_SIZE - minEverFree);
    
    // Persentase penggunaan
    float usagePercent = ((float)(configTOTAL_HEAP_SIZE - freeHeap) / configTOTAL_HEAP_SIZE) * 100;
    Serial.printf("║ Usage:               %5.1f%%              ║\n", usagePercent);
    
    // Allocation statistics
    Serial.printf("║ Total Allocations:   %6lu              ║\n", allocationCount);
    Serial.printf("║ Total Frees:         %6lu              ║\n", freeCount);
    
    Serial.println("╠══════════════════════════════════════════╣");
    
    // Status indicator
    if (freeHeap < HEAP_WARNING_THRESHOLD) {
        Serial.println("║ ⚠️  WARNING: LOW HEAP SPACE!             ║");
    } else if (freeHeap < HEAP_WARNING_THRESHOLD * 2) {
        Serial.println("║ ⚡ CAUTION: Heap getting low             ║");
    } else {
        Serial.println("║ ✓  STATUS: Heap OK                       ║");
    }
    
    Serial.println("╚══════════════════════════════════════════╝");
}

/**
 * @brief Mencetak high water mark semua task
 */
void printTaskStacks(void) {
    Serial.println("\n┌─────────────────────────────────────────┐");
    Serial.println("│       TASK STACK HIGH WATER MARK        │");
    Serial.println("├─────────────────────────────────────────┤");
    
    if (xMonitorTask != NULL) {
        UBaseType_t hwm = uxTaskGetStackHighWaterMark(xMonitorTask);
        Serial.printf("│ Monitor Task:  %4d words free          │\n", hwm);
    }
    
    if (xWorkerTask1 != NULL) {
        UBaseType_t hwm = uxTaskGetStackHighWaterMark(xWorkerTask1);
        Serial.printf("│ Worker Task 1: %4d words free          │\n", hwm);
    }
    
    if (xWorkerTask2 != NULL) {
        UBaseType_t hwm = uxTaskGetStackHighWaterMark(xWorkerTask2);
        Serial.printf("│ Worker Task 2: %4d words free          │\n", hwm);
    }
    
    Serial.println("└─────────────────────────────────────────┘");
}

// ==================== TASKS ====================

/**
 * @brief Task untuk memonitor heap
 */
void vMonitorTask(void *pvParameters) {
    Serial.println("[Monitor] Task started");
    
    for (;;) {
        printHeapStats();
        printTaskStacks();
        
        // Blink LED sesuai status heap
        size_t freeHeap = xPortGetFreeHeapSize();
        if (freeHeap < HEAP_WARNING_THRESHOLD) {
            // Fast blink - warning
            for (int i = 0; i < 5; i++) {
                digitalWrite(LED_PIN, LOW);
                vTaskDelay(pdMS_TO_TICKS(100));
                digitalWrite(LED_PIN, HIGH);
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        } else {
            // Single blink - OK
            digitalWrite(LED_PIN, LOW);
            vTaskDelay(pdMS_TO_TICKS(200));
            digitalWrite(LED_PIN, HIGH);
        }
        
        vTaskDelay(pdMS_TO_TICKS(MONITOR_INTERVAL_MS));
    }
}

/**
 * @brief Worker task yang melakukan alokasi dinamis
 */
void vWorkerTask1(void *pvParameters) {
    Serial.println("[Worker1] Task started");
    
    for (;;) {
        // Alokasi memory
        uint8_t *buffer = (uint8_t *)pvPortMalloc(128);
        
        if (buffer != NULL) {
            allocationCount++;
            Serial.printf("[Worker1] Allocated 128 bytes at 0x%08X\n", (unsigned int)buffer);
            
            // Simulasi penggunaan
            memset(buffer, 0xAA, 128);
            vTaskDelay(pdMS_TO_TICKS(2000));
            
            // Free memory
            vPortFree(buffer);
            freeCount++;
            Serial.println("[Worker1] Freed 128 bytes");
        } else {
            Serial.println("[Worker1] ERROR: Allocation failed!");
        }
        
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

/**
 * @brief Worker task dengan alokasi berbagai ukuran
 */
void vWorkerTask2(void *pvParameters) {
    Serial.println("[Worker2] Task started");
    uint16_t sizes[] = {64, 128, 256, 512};
    uint8_t sizeIndex = 0;
    
    for (;;) {
        uint16_t allocSize = sizes[sizeIndex];
        sizeIndex = (sizeIndex + 1) % 4;
        
        // Alokasi memory
        uint8_t *buffer = (uint8_t *)pvPortMalloc(allocSize);
        
        if (buffer != NULL) {
            allocationCount++;
            Serial.printf("[Worker2] Allocated %d bytes at 0x%08X\n", allocSize, (unsigned int)buffer);
            
            // Simulasi penggunaan
            memset(buffer, 0x55, allocSize);
            vTaskDelay(pdMS_TO_TICKS(1500));
            
            // Free memory
            vPortFree(buffer);
            freeCount++;
            Serial.printf("[Worker2] Freed %d bytes\n", allocSize);
        } else {
            Serial.printf("[Worker2] ERROR: Allocation of %d bytes failed!\n", allocSize);
        }
        
        vTaskDelay(pdMS_TO_TICKS(2500));
    }
}

// ==================== HOOK FUNCTIONS ====================

/**
 * @brief Stack overflow hook
 */
extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    Serial.printf("\n!!! STACK OVERFLOW in task: %s !!!\n", pcTaskName);
    while (1) {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        delay(100);
    }
}

/**
 * @brief Malloc failed hook
 */
extern "C" void vApplicationMallocFailedHook(void) {
    Serial.println("\n!!! MALLOC FAILED !!!");
}

// ==================== SETUP & LOOP ====================

void setup() {
    // Initialize Serial
    Serial.begin(SERIAL_BAUD);
    while (!Serial && millis() < 3000);
    
    Serial.println("\n========================================");
    Serial.println("   MODUL 12: HEAP STATISTICS MONITOR   ");
    Serial.println("   STM32F103C8T6 - FreeRTOS           ");
    Serial.println("========================================\n");
    
    // Initialize LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);  // LED off (active low)
    
    // Print initial heap status
    Serial.printf("Initial Free Heap: %d bytes\n", xPortGetFreeHeapSize());
    
    // Create tasks
    BaseType_t result;
    
    result = xTaskCreate(
        vMonitorTask,
        "Monitor",
        256,            // Stack size in words
        NULL,
        3,              // High priority
        &xMonitorTask
    );
    Serial.printf("Monitor Task created: %s\n", result == pdPASS ? "OK" : "FAIL");
    Serial.printf("Free Heap after Monitor: %d bytes\n", xPortGetFreeHeapSize());
    
    result = xTaskCreate(
        vWorkerTask1,
        "Worker1",
        192,
        NULL,
        1,
        &xWorkerTask1
    );
    Serial.printf("Worker1 Task created: %s\n", result == pdPASS ? "OK" : "FAIL");
    Serial.printf("Free Heap after Worker1: %d bytes\n", xPortGetFreeHeapSize());
    
    result = xTaskCreate(
        vWorkerTask2,
        "Worker2",
        192,
        NULL,
        1,
        &xWorkerTask2
    );
    Serial.printf("Worker2 Task created: %s\n", result == pdPASS ? "OK" : "FAIL");
    Serial.printf("Free Heap after Worker2: %d bytes\n", xPortGetFreeHeapSize());
    
    Serial.println("\nStarting FreeRTOS scheduler...\n");
    
    // Start scheduler
    vTaskStartScheduler();
    
    // Should never reach here
    Serial.println("ERROR: Scheduler failed to start!");
    while (1);
}

void loop() {
    // Not used with FreeRTOS
}

/**
 * EXPECTED OUTPUT:
 * 
 * ╔══════════════════════════════════════════╗
 * ║         HEAP STATISTICS REPORT           ║
 * ╠══════════════════════════════════════════╣
 * ║ Total Heap Size:     15360 bytes        ║
 * ║ Free Heap Now:       10234 bytes        ║
 * ║ Used Heap:            5126 bytes        ║
 * ║ Minimum Ever Free:    9856 bytes        ║
 * ║ Peak Usage:           5504 bytes        ║
 * ║ Usage:                33.4%              ║
 * ║ Total Allocations:       5              ║
 * ║ Total Frees:             4              ║
 * ╠══════════════════════════════════════════╣
 * ║ ✓  STATUS: Heap OK                       ║
 * ╚══════════════════════════════════════════╝
 */
