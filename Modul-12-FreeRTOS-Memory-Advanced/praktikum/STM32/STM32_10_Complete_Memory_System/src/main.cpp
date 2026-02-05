/**
 * @file main.cpp
 * @brief Program 10: Complete Memory Management System pada STM32
 * 
 * Deskripsi:
 * Sistem manajemen memory lengkap yang mengintegrasikan:
 * - Heap monitoring dan statistics
 * - Stack overflow detection
 * - Memory pools
 * - Event groups untuk koordinasi
 * - Safe memory patterns
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include <event_groups.h>
#include <semphr.h>

// ==================== KONFIGURASI ====================
#define LED_PIN             PC13
#define SERIAL_BAUD         115200

// Memory Pool Configuration
#define POOL_BLOCK_SIZE     64
#define POOL_NUM_BLOCKS     16

// Event Bits
#define EVT_SENSOR_READY    (1 << 0)
#define EVT_PROCESSOR_DONE  (1 << 1)
#define EVT_MEMORY_WARNING  (1 << 2)
#define EVT_SYSTEM_ERROR    (1 << 3)

// Thresholds
#define HEAP_WARNING_THRESHOLD  2048    // Warning if heap < 2KB
#define STACK_WARNING_THRESHOLD 64      // Warning if HWM < 64 words

// ==================== MEMORY POOL IMPLEMENTATION ====================

typedef struct {
    uint8_t memory[POOL_BLOCK_SIZE * POOL_NUM_BLOCKS];
    uint8_t bitmap[POOL_NUM_BLOCKS];
    SemaphoreHandle_t mutex;
    uint16_t freeBlocks;
    uint32_t allocCount;
    uint32_t freeCount;
} MemPool_t;

MemPool_t sensorPool;

bool MemPool_Init(MemPool_t *pool) {
    memset(pool->memory, 0, sizeof(pool->memory));
    memset(pool->bitmap, 0, sizeof(pool->bitmap));
    pool->mutex = xSemaphoreCreateMutex();
    pool->freeBlocks = POOL_NUM_BLOCKS;
    pool->allocCount = 0;
    pool->freeCount = 0;
    return (pool->mutex != NULL);
}

void* MemPool_Alloc(MemPool_t *pool) {
    void *ptr = NULL;
    if (xSemaphoreTake(pool->mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        for (int i = 0; i < POOL_NUM_BLOCKS; i++) {
            if (pool->bitmap[i] == 0) {
                pool->bitmap[i] = 1;
                pool->freeBlocks--;
                pool->allocCount++;
                ptr = &pool->memory[i * POOL_BLOCK_SIZE];
                break;
            }
        }
        xSemaphoreGive(pool->mutex);
    }
    return ptr;
}

bool MemPool_Free(MemPool_t *pool, void *ptr) {
    if (ptr == NULL) return false;
    
    uint8_t *p = (uint8_t*)ptr;
    if (p < pool->memory || p >= pool->memory + sizeof(pool->memory)) {
        return false;
    }
    
    int index = (p - pool->memory) / POOL_BLOCK_SIZE;
    bool success = false;
    
    if (xSemaphoreTake(pool->mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        if (pool->bitmap[index] == 1) {
            pool->bitmap[index] = 0;
            pool->freeBlocks++;
            pool->freeCount++;
            success = true;
        }
        xSemaphoreGive(pool->mutex);
    }
    return success;
}

// ==================== DATA STRUCTURES ====================

typedef struct {
    uint32_t timestamp;
    float value;
    uint8_t sensorId;
    uint8_t status;
} SensorData_t;

typedef struct {
    uint32_t heapFree;
    uint32_t heapMinEver;
    uint16_t poolFree;
    uint8_t systemFlags;
} SystemStatus_t;

// ==================== VARIABEL GLOBAL ====================
EventGroupHandle_t xEventGroup = NULL;
QueueHandle_t xDataQueue = NULL;
SemaphoreHandle_t xStatusMutex = NULL;

TaskHandle_t xSensorTask = NULL;
TaskHandle_t xProcessorTask = NULL;
TaskHandle_t xMemoryMonitorTask = NULL;
TaskHandle_t xSystemControlTask = NULL;

SystemStatus_t systemStatus = {0};
volatile bool memoryWarningActive = false;

// ==================== HOOKS ====================

extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    Serial.printf("\n🔴 STACK OVERFLOW: %s\n", pcTaskName);
    
    // Set event bit
    if (xEventGroup != NULL) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xEventGroupSetBitsFromISR(xEventGroup, EVT_SYSTEM_ERROR, &xHigherPriorityTaskWoken);
    }
    
    // Halt with LED pattern
    while (1) {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        for (volatile int i = 0; i < 50000; i++);
    }
}

extern "C" void vApplicationMallocFailedHook(void) {
    Serial.println("\n🔴 MALLOC FAILED!");
    memoryWarningActive = true;
    
    if (xEventGroup != NULL) {
        xEventGroupSetBits(xEventGroup, EVT_SYSTEM_ERROR);
    }
}

// ==================== HELPER FUNCTIONS ====================

void updateSystemStatus() {
    if (xSemaphoreTake(xStatusMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        systemStatus.heapFree = xPortGetFreeHeapSize();
        systemStatus.heapMinEver = xPortGetMinimumEverFreeHeapSize();
        systemStatus.poolFree = sensorPool.freeBlocks;
        
        // Check thresholds
        if (systemStatus.heapFree < HEAP_WARNING_THRESHOLD) {
            systemStatus.systemFlags |= 0x01;  // Memory warning
            memoryWarningActive = true;
        } else {
            systemStatus.systemFlags &= ~0x01;
            memoryWarningActive = false;
        }
        
        xSemaphoreGive(xStatusMutex);
    }
}

void printSystemDashboard() {
    Serial.println("\n╔══════════════════════════════════════════════════════════════╗");
    Serial.println("║          COMPLETE MEMORY MANAGEMENT SYSTEM DASHBOARD         ║");
    Serial.println("╠══════════════════════════════════════════════════════════════╣");
    
    xSemaphoreTake(xStatusMutex, portMAX_DELAY);
    
    // Heap Status
    Serial.println("║ [HEAP STATUS]                                                ║");
    Serial.printf("║   Free:     %5lu bytes                                       ║\n", 
                 systemStatus.heapFree);
    Serial.printf("║   Min Ever: %5lu bytes                                       ║\n", 
                 systemStatus.heapMinEver);
    Serial.printf("║   Used:     %5lu bytes                                       ║\n",
                 configTOTAL_HEAP_SIZE - systemStatus.heapFree);
    
    // Usage bar
    int usage = 100 - (systemStatus.heapFree * 100 / configTOTAL_HEAP_SIZE);
    Serial.print("║   Usage:    [");
    for (int i = 0; i < 20; i++) {
        Serial.print(i < usage/5 ? "█" : "░");
    }
    Serial.printf("] %d%%                             ║\n", usage);
    
    Serial.println("╠══════════════════════════════════════════════════════════════╣");
    
    // Memory Pool Status
    Serial.println("║ [MEMORY POOL STATUS]                                         ║");
    Serial.printf("║   Free Blocks: %2d / %2d                                       ║\n",
                 sensorPool.freeBlocks, POOL_NUM_BLOCKS);
    Serial.printf("║   Allocations: %lu   Frees: %lu                              ║\n",
                 sensorPool.allocCount, sensorPool.freeCount);
    
    // Pool visualization
    Serial.print("║   Blocks:     [");
    xSemaphoreTake(sensorPool.mutex, portMAX_DELAY);
    for (int i = 0; i < POOL_NUM_BLOCKS; i++) {
        Serial.print(sensorPool.bitmap[i] ? "█" : "░");
    }
    xSemaphoreGive(sensorPool.mutex);
    Serial.println("]                        ║");
    
    Serial.println("╠══════════════════════════════════════════════════════════════╣");
    
    // Task Stack Status
    Serial.println("║ [TASK STACK STATUS] (High Water Marks)                       ║");
    
    if (xSensorTask) {
        UBaseType_t hwm = uxTaskGetStackHighWaterMark(xSensorTask);
        Serial.printf("║   Sensor:     %3d words %s                               ║\n",
                     hwm, hwm < STACK_WARNING_THRESHOLD ? "⚠️ LOW" : "✓");
    }
    if (xProcessorTask) {
        UBaseType_t hwm = uxTaskGetStackHighWaterMark(xProcessorTask);
        Serial.printf("║   Processor:  %3d words %s                               ║\n",
                     hwm, hwm < STACK_WARNING_THRESHOLD ? "⚠️ LOW" : "✓");
    }
    if (xMemoryMonitorTask) {
        UBaseType_t hwm = uxTaskGetStackHighWaterMark(xMemoryMonitorTask);
        Serial.printf("║   Monitor:    %3d words %s                               ║\n",
                     hwm, hwm < STACK_WARNING_THRESHOLD ? "⚠️ LOW" : "✓");
    }
    
    Serial.println("╠══════════════════════════════════════════════════════════════╣");
    
    // System Flags
    Serial.println("║ [SYSTEM FLAGS]                                               ║");
    Serial.printf("║   Memory Warning: %s                                       ║\n",
                 memoryWarningActive ? "🔴 ACTIVE" : "✅ OK    ");
    Serial.printf("║   Queue Items:    %d / 10                                    ║\n",
                 uxQueueMessagesWaiting(xDataQueue));
    
    xSemaphoreGive(xStatusMutex);
    
    Serial.println("╚══════════════════════════════════════════════════════════════╝");
}

// ==================== TASKS ====================

/**
 * @brief Sensor task - menggunakan memory pool untuk data
 */
void vSensorTask(void *pvParameters) {
    Serial.println("[Sensor] Started");
    
    for (;;) {
        // Allocate from pool (not heap!)
        SensorData_t *data = (SensorData_t*)MemPool_Alloc(&sensorPool);
        
        if (data != NULL) {
            // Fill data
            data->timestamp = xTaskGetTickCount();
            data->value = 20.0 + (random(0, 200) / 10.0);
            data->sensorId = 1;
            data->status = 0x01;
            
            // Send pointer to queue
            if (xQueueSend(xDataQueue, &data, pdMS_TO_TICKS(100)) == pdTRUE) {
                // Signal processor
                xEventGroupSetBits(xEventGroup, EVT_SENSOR_READY);
            } else {
                // Queue full - return to pool
                MemPool_Free(&sensorPool, data);
            }
        } else {
            Serial.println("[Sensor] Pool exhausted!");
            xEventGroupSetBits(xEventGroup, EVT_MEMORY_WARNING);
        }
        
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/**
 * @brief Processor task - memproses data dan mengembalikan ke pool
 */
void vProcessorTask(void *pvParameters) {
    Serial.println("[Processor] Started");
    
    SensorData_t *data = NULL;
    
    for (;;) {
        // Wait for data event
        EventBits_t bits = xEventGroupWaitBits(xEventGroup,
                                               EVT_SENSOR_READY,
                                               pdTRUE,  // Clear on exit
                                               pdFALSE, // Any bit
                                               pdMS_TO_TICKS(1000));
        
        if (bits & EVT_SENSOR_READY) {
            // Process all available data
            while (xQueueReceive(xDataQueue, &data, 0) == pdTRUE) {
                Serial.printf("[Processor] Data @%lu: %.1f (sensor %d)\n",
                             data->timestamp, data->value, data->sensorId);
                
                // Simulate processing
                vTaskDelay(pdMS_TO_TICKS(50));
                
                // Return to pool - IMPORTANT!
                MemPool_Free(&sensorPool, data);
            }
            
            xEventGroupSetBits(xEventGroup, EVT_PROCESSOR_DONE);
        }
    }
}

/**
 * @brief Memory monitor task - watchdog untuk memory
 */
void vMemoryMonitorTask(void *pvParameters) {
    Serial.println("[MemMonitor] Started");
    
    for (;;) {
        updateSystemStatus();
        printSystemDashboard();
        
        // Check for warnings
        EventBits_t bits = xEventGroupGetBits(xEventGroup);
        
        if (bits & EVT_MEMORY_WARNING) {
            Serial.println("\n⚠️  MEMORY WARNING - Taking corrective action!");
            // Could trigger garbage collection or request tasks to free memory
            xEventGroupClearBits(xEventGroup, EVT_MEMORY_WARNING);
        }
        
        if (bits & EVT_SYSTEM_ERROR) {
            Serial.println("\n🔴 SYSTEM ERROR - Critical condition!");
            // Log error, attempt recovery, or safe shutdown
        }
        
        // Heartbeat LED
        digitalWrite(LED_PIN, memoryWarningActive ? LOW : HIGH);
        vTaskDelay(pdMS_TO_TICKS(100));
        digitalWrite(LED_PIN, HIGH);
        
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

/**
 * @brief System control task - koordinasi sistem
 */
void vSystemControlTask(void *pvParameters) {
    Serial.println("[SysControl] Started");
    
    for (;;) {
        // Periodic system maintenance
        
        // Check task health
        UBaseType_t numTasks = uxTaskGetNumberOfTasks();
        
        // Simple watchdog - could be expanded
        static uint32_t lastSensorTick = 0;
        uint32_t currentTick = xTaskGetTickCount();
        
        // Reset watchdog (in real system, would pet hardware WDT)
        lastSensorTick = currentTick;
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// ==================== SETUP & LOOP ====================

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(2000);
    
    Serial.println("\n════════════════════════════════════════════════════════════════");
    Serial.println("        MODUL 12: COMPLETE MEMORY MANAGEMENT SYSTEM             ");
    Serial.println("════════════════════════════════════════════════════════════════\n");
    
    Serial.println("This system integrates:");
    Serial.println("  ✓ Heap monitoring and statistics");
    Serial.println("  ✓ Stack overflow detection");
    Serial.println("  ✓ Fixed-size memory pools");
    Serial.println("  ✓ Event groups for coordination");
    Serial.println("  ✓ Memory warning system");
    Serial.println("  ✓ Comprehensive dashboard");
    Serial.println("");
    
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    
    randomSeed(analogRead(PA0));
    
    // Initialize memory pool
    if (!MemPool_Init(&sensorPool)) {
        Serial.println("[ERROR] Pool init failed!");
        while (1);
    }
    Serial.println("✓ Memory pool initialized");
    
    // Create synchronization primitives
    xEventGroup = xEventGroupCreate();
    xDataQueue = xQueueCreate(10, sizeof(SensorData_t*));
    xStatusMutex = xSemaphoreCreateMutex();
    
    if (xEventGroup == NULL || xDataQueue == NULL || xStatusMutex == NULL) {
        Serial.println("[ERROR] Failed to create sync primitives!");
        while (1);
    }
    Serial.println("✓ Sync primitives created");
    
    // Create tasks
    xTaskCreate(vSensorTask, "Sensor", 256, NULL, 2, &xSensorTask);
    xTaskCreate(vProcessorTask, "Processor", 256, NULL, 2, &xProcessorTask);
    xTaskCreate(vMemoryMonitorTask, "MemMonitor", 384, NULL, 3, &xMemoryMonitorTask);
    xTaskCreate(vSystemControlTask, "SysControl", 256, NULL, 1, &xSystemControlTask);
    
    Serial.println("✓ Tasks created");
    Serial.println("\nStarting scheduler...\n");
    
    vTaskStartScheduler();
    
    Serial.println("[ERROR] Scheduler failed!");
}

void loop() {
    // Empty
}
