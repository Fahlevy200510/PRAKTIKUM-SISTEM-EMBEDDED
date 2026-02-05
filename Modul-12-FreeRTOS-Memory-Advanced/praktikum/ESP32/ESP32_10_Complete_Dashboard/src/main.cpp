/**
 * @file main.cpp
 * @brief Program 10: ESP32 Complete Memory Dashboard
 * 
 * Dashboard komprehensif untuk monitoring semua aspek
 * memory management pada ESP32.
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "esp_heap_caps.h"
#include "esp_system.h"

#define LED_PIN     2

// Event bits
#define EVT_SENSOR_READY    (1 << 0)
#define EVT_MEMORY_WARNING  (1 << 1)
#define EVT_MEMORY_CRITICAL (1 << 2)

// Memory Pool
#define POOL_SIZE       32
#define POOL_BLOCK_SIZE 64

typedef struct {
    uint8_t memory[POOL_SIZE * POOL_BLOCK_SIZE];
    uint8_t bitmap[POOL_SIZE];
    SemaphoreHandle_t mutex;
    uint16_t used;
    uint32_t allocations;
    uint32_t frees;
} MemPool_t;

MemPool_t dataPool;

// System status
typedef struct {
    // Heap
    size_t heapFree;
    size_t heapMin;
    size_t heapLargest;
    size_t heapTotal;
    
    // PSRAM
    size_t psramFree;
    size_t psramTotal;
    
    // Pool
    uint16_t poolUsed;
    uint16_t poolTotal;
    
    // Tasks
    uint8_t taskCount;
    
    // Flags
    bool memoryWarning;
    bool memoryCritical;
    
} SystemMemoryStatus_t;

SystemMemoryStatus_t memStatus;
EventGroupHandle_t xEvents = NULL;
SemaphoreHandle_t xStatusMutex = NULL;

TaskHandle_t xSensorTask, xProcessorTask, xDashboardTask, xWatchdogTask;

// Memory Pool Functions
void MemPool_Init(MemPool_t *p) {
    memset(p->memory, 0, sizeof(p->memory));
    memset(p->bitmap, 0, sizeof(p->bitmap));
    p->mutex = xSemaphoreCreateMutex();
    p->used = 0;
    p->allocations = 0;
    p->frees = 0;
}

void* MemPool_Alloc(MemPool_t *p) {
    void *ptr = NULL;
    if (xSemaphoreTake(p->mutex, pdMS_TO_TICKS(100))) {
        for (int i = 0; i < POOL_SIZE; i++) {
            if (!p->bitmap[i]) {
                p->bitmap[i] = 1;
                p->used++;
                p->allocations++;
                ptr = &p->memory[i * POOL_BLOCK_SIZE];
                break;
            }
        }
        xSemaphoreGive(p->mutex);
    }
    return ptr;
}

void MemPool_Free(MemPool_t *p, void *ptr) {
    if (!ptr) return;
    uint8_t *addr = (uint8_t*)ptr;
    if (addr >= p->memory && addr < p->memory + sizeof(p->memory)) {
        int idx = (addr - p->memory) / POOL_BLOCK_SIZE;
        if (xSemaphoreTake(p->mutex, pdMS_TO_TICKS(100))) {
            if (p->bitmap[idx]) {
                p->bitmap[idx] = 0;
                p->used--;
                p->frees++;
            }
            xSemaphoreGive(p->mutex);
        }
    }
}

void updateMemoryStatus() {
    if (xSemaphoreTake(xStatusMutex, pdMS_TO_TICKS(100))) {
        memStatus.heapFree = ESP.getFreeHeap();
        memStatus.heapMin = ESP.getMinFreeHeap();
        memStatus.heapTotal = ESP.getHeapSize();
        memStatus.heapLargest = heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);
        
        if (psramFound()) {
            memStatus.psramFree = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
            memStatus.psramTotal = ESP.getPsramSize();
        }
        
        memStatus.poolUsed = dataPool.used;
        memStatus.poolTotal = POOL_SIZE;
        memStatus.taskCount = uxTaskGetNumberOfTasks();
        
        // Check thresholds
        memStatus.memoryWarning = (memStatus.heapFree < 50000);
        memStatus.memoryCritical = (memStatus.heapFree < 20000);
        
        if (memStatus.memoryCritical) {
            xEventGroupSetBits(xEvents, EVT_MEMORY_CRITICAL);
        } else if (memStatus.memoryWarning) {
            xEventGroupSetBits(xEvents, EVT_MEMORY_WARNING);
        }
        
        xSemaphoreGive(xStatusMutex);
    }
}

void printDashboard() {
    xSemaphoreTake(xStatusMutex, portMAX_DELAY);
    
    Serial.println("\n");
    Serial.println("╔══════════════════════════════════════════════════════════════════════╗");
    Serial.println("║              ESP32 COMPLETE MEMORY MANAGEMENT DASHBOARD              ║");
    Serial.println("╠══════════════════════════════════════════════════════════════════════╣");
    
    // Heap Section
    Serial.println("║ ┌──────────────────────── HEAP MEMORY ────────────────────────┐     ║");
    float heapUsage = 100.0 - (memStatus.heapFree * 100.0 / memStatus.heapTotal);
    Serial.printf("║ │ Total: %7d bytes   Free: %7d bytes   Used: %5.1f%%    │     ║\n",
                 memStatus.heapTotal, memStatus.heapFree, heapUsage);
    Serial.printf("║ │ Min Ever: %7d bytes   Largest Block: %7d bytes       │     ║\n",
                 memStatus.heapMin, memStatus.heapLargest);
    Serial.print("║ │ [");
    int bars = (int)(heapUsage / 5);
    for (int i = 0; i < 20; i++) {
        if (i < bars) {
            if (heapUsage > 80) Serial.print("█");
            else if (heapUsage > 60) Serial.print("▓");
            else Serial.print("░");
        } else {
            Serial.print(" ");
        }
    }
    Serial.printf("] %5.1f%%                                       │     ║\n", heapUsage);
    Serial.println("║ └────────────────────────────────────────────────────────────┘     ║");
    
    // PSRAM Section
    Serial.println("║ ┌──────────────────────── PSRAM (SPI) ────────────────────────┐     ║");
    if (psramFound()) {
        float psramUsage = 100.0 - (memStatus.psramFree * 100.0 / memStatus.psramTotal);
        Serial.printf("║ │ Total: %7d KB   Free: %7d KB   Used: %5.1f%%         │     ║\n",
                     memStatus.psramTotal/1024, memStatus.psramFree/1024, psramUsage);
    } else {
        Serial.println("║ │ PSRAM: Not available on this board                         │     ║");
    }
    Serial.println("║ └────────────────────────────────────────────────────────────┘     ║");
    
    // Memory Pool Section
    Serial.println("║ ┌────────────────────── MEMORY POOL ──────────────────────────┐     ║");
    Serial.printf("║ │ Blocks: %2d / %2d used   Allocs: %6lu   Frees: %6lu       │     ║\n",
                 memStatus.poolUsed, memStatus.poolTotal, 
                 dataPool.allocations, dataPool.frees);
    Serial.print("║ │ [");
    xSemaphoreTake(dataPool.mutex, portMAX_DELAY);
    for (int i = 0; i < POOL_SIZE; i++) {
        Serial.print(dataPool.bitmap[i] ? "█" : "░");
    }
    xSemaphoreGive(dataPool.mutex);
    Serial.println("]                            │     ║");
    Serial.println("║ └────────────────────────────────────────────────────────────┘     ║");
    
    // Task Stack Section
    Serial.println("║ ┌──────────────────────── TASK STACKS ────────────────────────┐     ║");
    Serial.printf("║ │ Active tasks: %d                                             │     ║\n", 
                 memStatus.taskCount);
    if (xSensorTask) {
        UBaseType_t hwm = uxTaskGetStackHighWaterMark(xSensorTask);
        Serial.printf("║ │   Sensor:    %5d bytes free                               │     ║\n", hwm);
    }
    if (xProcessorTask) {
        UBaseType_t hwm = uxTaskGetStackHighWaterMark(xProcessorTask);
        Serial.printf("║ │   Processor: %5d bytes free                               │     ║\n", hwm);
    }
    if (xDashboardTask) {
        UBaseType_t hwm = uxTaskGetStackHighWaterMark(xDashboardTask);
        Serial.printf("║ │   Dashboard: %5d bytes free                               │     ║\n", hwm);
    }
    Serial.println("║ └────────────────────────────────────────────────────────────┘     ║");
    
    // System Status
    Serial.println("║ ┌────────────────────── SYSTEM STATUS ────────────────────────┐     ║");
    if (memStatus.memoryCritical) {
        Serial.println("║ │  🔴 CRITICAL: Memory critically low!                        │     ║");
    } else if (memStatus.memoryWarning) {
        Serial.println("║ │  ⚠️  WARNING: Memory running low                             │     ║");
    } else {
        Serial.println("║ │  ✅ HEALTHY: Memory levels normal                           │     ║");
    }
    
    float fragmentation = 100.0 - (memStatus.heapLargest * 100.0 / memStatus.heapFree);
    Serial.printf("║ │  Fragmentation: %5.1f%%                                      │     ║\n", 
                 fragmentation);
    Serial.printf("║ │  Uptime: %lu seconds                                        │     ║\n",
                 millis() / 1000);
    Serial.println("║ └────────────────────────────────────────────────────────────┘     ║");
    
    Serial.println("╚══════════════════════════════════════════════════════════════════════╝");
    
    xSemaphoreGive(xStatusMutex);
}

void vSensorTask(void *pvParameters) {
    Serial.println("[Sensor] Started");
    
    for (;;) {
        void *data = MemPool_Alloc(&dataPool);
        if (data) {
            // Simulate sensor data
            memset(data, random(0, 255), POOL_BLOCK_SIZE);
            xEventGroupSetBits(xEvents, EVT_SENSOR_READY);
            
            vTaskDelay(pdMS_TO_TICKS(500));
            
            MemPool_Free(&dataPool, data);
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vProcessorTask(void *pvParameters) {
    Serial.println("[Processor] Started");
    
    for (;;) {
        EventBits_t bits = xEventGroupWaitBits(xEvents, 
            EVT_SENSOR_READY | EVT_MEMORY_WARNING | EVT_MEMORY_CRITICAL,
            pdTRUE, pdFALSE, pdMS_TO_TICKS(2000));
        
        if (bits & EVT_MEMORY_CRITICAL) {
            Serial.println("[Processor] 🔴 Memory critical - reducing activity!");
            vTaskDelay(pdMS_TO_TICKS(5000));
        } else if (bits & EVT_MEMORY_WARNING) {
            Serial.println("[Processor] ⚠️  Memory warning");
        }
        
        if (bits & EVT_SENSOR_READY) {
            // Process sensor data
            digitalWrite(LED_PIN, HIGH);
            vTaskDelay(pdMS_TO_TICKS(50));
            digitalWrite(LED_PIN, LOW);
        }
    }
}

void vDashboardTask(void *pvParameters) {
    Serial.println("[Dashboard] Started");
    
    for (;;) {
        updateMemoryStatus();
        printDashboard();
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void vWatchdogTask(void *pvParameters) {
    Serial.println("[Watchdog] Started");
    
    for (;;) {
        // Check for memory issues
        if (ESP.getFreeHeap() < 10000) {
            Serial.println("[Watchdog] 🔴 EMERGENCY: Heap critically low!");
            // Could trigger reset or cleanup
        }
        
        // Check fragmentation
        size_t largest = heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);
        size_t total = ESP.getFreeHeap();
        float frag = 100.0 - (largest * 100.0 / total);
        
        if (frag > 70) {
            Serial.printf("[Watchdog] ⚠️  High fragmentation: %.1f%%\n", frag);
        }
        
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n═══════════════════════════════════════════════════════════════");
    Serial.println("       ESP32 COMPLETE MEMORY MANAGEMENT DASHBOARD");
    Serial.println("═══════════════════════════════════════════════════════════════\n");
    
    pinMode(LED_PIN, OUTPUT);
    
    // Initialize pool
    MemPool_Init(&dataPool);
    Serial.println("✓ Memory pool initialized");
    
    // Create sync objects
    xEvents = xEventGroupCreate();
    xStatusMutex = xSemaphoreCreateMutex();
    Serial.println("✓ Sync objects created");
    
    // Create tasks
    xTaskCreatePinnedToCore(vSensorTask, "Sensor", 4096, NULL, 2, &xSensorTask, 0);
    xTaskCreatePinnedToCore(vProcessorTask, "Processor", 4096, NULL, 2, &xProcessorTask, 1);
    xTaskCreatePinnedToCore(vDashboardTask, "Dashboard", 8192, NULL, 1, &xDashboardTask, 0);
    xTaskCreatePinnedToCore(vWatchdogTask, "Watchdog", 4096, NULL, 3, &xWatchdogTask, 1);
    
    Serial.println("✓ Tasks created");
    Serial.println("\nDashboard will update every 5 seconds...\n");
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
