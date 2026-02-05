/**
 * @file main.cpp
 * @brief Program 03: Custom Memory Pool Implementation pada STM32
 * 
 * Deskripsi:
 * Implementasi custom memory pool untuk alokasi deterministik.
 * Menghindari fragmentasi dengan fixed-size block allocation.
 * 
 * Keuntungan Memory Pool:
 * - Alokasi O(1) - constant time
 * - Tidak ada fragmentasi
 * - Deterministik untuk real-time
 * - Mudah track memory usage
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include <semphr.h>

// ==================== KONFIGURASI ====================
#define LED_PIN         PC13
#define SERIAL_BAUD     115200

// Memory Pool Configuration
#define POOL_BLOCK_SIZE     64      // Ukuran tiap block (bytes)
#define POOL_NUM_BLOCKS     16      // Jumlah blocks
#define POOL_TOTAL_SIZE     (POOL_BLOCK_SIZE * POOL_NUM_BLOCKS)

// ==================== MEMORY POOL STRUCTURE ====================

/**
 * @brief Struktur untuk fixed-size memory pool
 */
typedef struct {
    uint8_t memory[POOL_TOTAL_SIZE];    // Actual memory storage
    uint8_t bitmap[POOL_NUM_BLOCKS];    // Block usage bitmap (0=free, 1=used)
    uint16_t blockSize;                  // Size of each block
    uint16_t numBlocks;                  // Total number of blocks
    uint16_t freeBlocks;                 // Current free blocks count
    SemaphoreHandle_t mutex;             // Thread-safe access
    // Statistics
    uint32_t allocCount;
    uint32_t freeCount;
    uint32_t failCount;
} MemoryPool_t;

// ==================== VARIABEL GLOBAL ====================
MemoryPool_t sensorDataPool;

TaskHandle_t xProducerTask = NULL;
TaskHandle_t xConsumerTask = NULL;
TaskHandle_t xMonitorTask = NULL;

QueueHandle_t xDataQueue = NULL;

// ==================== MEMORY POOL IMPLEMENTATION ====================

/**
 * @brief Inisialisasi memory pool
 * 
 * @param pool Pointer ke struktur pool
 * @param blockSize Ukuran tiap block
 * @param numBlocks Jumlah blocks
 * @return true jika berhasil
 */
bool MemPool_Init(MemoryPool_t *pool, uint16_t blockSize, uint16_t numBlocks) {
    if (pool == NULL || blockSize == 0 || numBlocks == 0) {
        return false;
    }
    
    // Initialize structure
    pool->blockSize = blockSize;
    pool->numBlocks = numBlocks;
    pool->freeBlocks = numBlocks;
    pool->allocCount = 0;
    pool->freeCount = 0;
    pool->failCount = 0;
    
    // Clear memory and bitmap
    memset(pool->memory, 0, blockSize * numBlocks);
    memset(pool->bitmap, 0, numBlocks);
    
    // Create mutex for thread safety
    pool->mutex = xSemaphoreCreateMutex();
    if (pool->mutex == NULL) {
        return false;
    }
    
    Serial.printf("[MemPool] Initialized: %d blocks x %d bytes = %d bytes total\n",
                 numBlocks, blockSize, numBlocks * blockSize);
    
    return true;
}

/**
 * @brief Alokasi satu block dari pool
 * 
 * @param pool Pointer ke struktur pool
 * @return Pointer ke block atau NULL jika full
 */
void* MemPool_Alloc(MemoryPool_t *pool) {
    void *block = NULL;
    
    if (pool == NULL) return NULL;
    
    // Thread-safe access
    if (xSemaphoreTake(pool->mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        // Find free block
        for (uint16_t i = 0; i < pool->numBlocks; i++) {
            if (pool->bitmap[i] == 0) {
                // Mark as used
                pool->bitmap[i] = 1;
                pool->freeBlocks--;
                pool->allocCount++;
                
                // Calculate block address
                block = &pool->memory[i * pool->blockSize];
                break;
            }
        }
        
        if (block == NULL) {
            pool->failCount++;
        }
        
        xSemaphoreGive(pool->mutex);
    }
    
    return block;
}

/**
 * @brief Mengembalikan block ke pool
 * 
 * @param pool Pointer ke struktur pool
 * @param block Pointer ke block yang akan dibebaskan
 * @return true jika berhasil
 */
bool MemPool_Free(MemoryPool_t *pool, void *block) {
    if (pool == NULL || block == NULL) return false;
    
    // Validate block address
    uint8_t *blockAddr = (uint8_t*)block;
    uint8_t *poolStart = pool->memory;
    uint8_t *poolEnd = pool->memory + POOL_TOTAL_SIZE;
    
    if (blockAddr < poolStart || blockAddr >= poolEnd) {
        Serial.println("[MemPool] ERROR: Invalid block address!");
        return false;
    }
    
    // Calculate block index
    uint16_t index = (blockAddr - poolStart) / pool->blockSize;
    
    if (index >= pool->numBlocks) {
        Serial.println("[MemPool] ERROR: Block index out of range!");
        return false;
    }
    
    bool success = false;
    
    if (xSemaphoreTake(pool->mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        if (pool->bitmap[index] == 1) {
            // Clear block data (optional, for security)
            memset(block, 0, pool->blockSize);
            
            // Mark as free
            pool->bitmap[index] = 0;
            pool->freeBlocks++;
            pool->freeCount++;
            success = true;
        } else {
            Serial.println("[MemPool] WARNING: Double free detected!");
        }
        
        xSemaphoreGive(pool->mutex);
    }
    
    return success;
}

/**
 * @brief Mendapatkan statistik pool
 */
void MemPool_PrintStats(MemoryPool_t *pool) {
    if (pool == NULL) return;
    
    xSemaphoreTake(pool->mutex, portMAX_DELAY);
    
    Serial.println("\n┌──────────────────────────────────────────┐");
    Serial.println("│         MEMORY POOL STATISTICS           │");
    Serial.println("├──────────────────────────────────────────┤");
    Serial.printf("│ Block Size:     %5d bytes              │\n", pool->blockSize);
    Serial.printf("│ Total Blocks:   %5d                    │\n", pool->numBlocks);
    Serial.printf("│ Free Blocks:    %5d                    │\n", pool->freeBlocks);
    Serial.printf("│ Used Blocks:    %5d                    │\n", 
                 pool->numBlocks - pool->freeBlocks);
    Serial.println("├──────────────────────────────────────────┤");
    Serial.printf("│ Allocations:    %5lu                    │\n", pool->allocCount);
    Serial.printf("│ Frees:          %5lu                    │\n", pool->freeCount);
    Serial.printf("│ Failed Allocs:  %5lu                    │\n", pool->failCount);
    Serial.println("├──────────────────────────────────────────┤");
    
    // Usage visualization
    Serial.print("│ Usage: [");
    for (uint16_t i = 0; i < pool->numBlocks; i++) {
        Serial.print(pool->bitmap[i] ? "█" : "░");
    }
    Serial.println("] │");
    
    float usage = 100.0 * (pool->numBlocks - pool->freeBlocks) / pool->numBlocks;
    Serial.printf("│ Utilization:    %5.1f%%                   │\n", usage);
    Serial.println("└──────────────────────────────────────────┘");
    
    xSemaphoreGive(pool->mutex);
}

// ==================== SENSOR DATA STRUCTURE ====================

typedef struct {
    uint32_t timestamp;
    float temperature;
    float humidity;
    float pressure;
    uint8_t sensorId;
    uint8_t status;
    char tag[16];
} SensorData_t;

// ==================== TASKS ====================

/**
 * @brief Producer task - membuat sensor data menggunakan memory pool
 */
void vProducerTask(void *pvParameters) {
    Serial.println("[Producer] Task started");
    
    uint32_t sequence = 0;
    
    for (;;) {
        // Allocate from pool instead of heap
        SensorData_t *data = (SensorData_t*)MemPool_Alloc(&sensorDataPool);
        
        if (data != NULL) {
            // Fill sensor data
            data->timestamp = xTaskGetTickCount();
            data->temperature = 20.0 + (random(0, 150) / 10.0);
            data->humidity = 40.0 + (random(0, 400) / 10.0);
            data->pressure = 1000.0 + (random(0, 500) / 10.0);
            data->sensorId = 1;
            data->status = 0x01;  // OK status
            snprintf(data->tag, sizeof(data->tag), "DATA_%lu", sequence++);
            
            Serial.printf("[Producer] Created %s: T=%.1f H=%.1f\n",
                         data->tag, data->temperature, data->humidity);
            
            // Send pointer to queue
            if (xQueueSend(xDataQueue, &data, pdMS_TO_TICKS(100)) != pdTRUE) {
                Serial.println("[Producer] Queue full! Freeing data");
                MemPool_Free(&sensorDataPool, data);
            }
        } else {
            Serial.println("[Producer] Pool exhausted! Waiting...");
        }
        
        // Random delay to simulate varying data rates
        vTaskDelay(pdMS_TO_TICKS(500 + random(0, 500)));
    }
}

/**
 * @brief Consumer task - memproses data dan mengembalikan ke pool
 */
void vConsumerTask(void *pvParameters) {
    Serial.println("[Consumer] Task started");
    
    SensorData_t *data = NULL;
    
    for (;;) {
        // Receive pointer from queue
        if (xQueueReceive(xDataQueue, &data, pdMS_TO_TICKS(2000)) == pdTRUE) {
            // Process data
            Serial.printf("[Consumer] Processing %s...\n", data->tag);
            
            // Simulate processing time
            vTaskDelay(pdMS_TO_TICKS(100));
            
            Serial.printf("[Consumer] Done: T=%.1f°C, H=%.1f%%, P=%.1fhPa\n",
                         data->temperature, data->humidity, data->pressure);
            
            // Return block to pool
            MemPool_Free(&sensorDataPool, data);
            data = NULL;
        } else {
            Serial.println("[Consumer] No data received (timeout)");
        }
    }
}

/**
 * @brief Monitor task - memantau status pool
 */
void vMonitorTask(void *pvParameters) {
    Serial.println("[Monitor] Task started");
    
    for (;;) {
        MemPool_PrintStats(&sensorDataPool);
        
        // Show queue status
        Serial.printf("Queue: %d/%d items\n\n", 
                     uxQueueMessagesWaiting(xDataQueue),
                     POOL_NUM_BLOCKS);
        
        // Blink LED
        digitalWrite(LED_PIN, LOW);
        vTaskDelay(pdMS_TO_TICKS(100));
        digitalWrite(LED_PIN, HIGH);
        
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// ==================== SETUP & LOOP ====================

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(2000);
    
    Serial.println("\n════════════════════════════════════════════════════════════");
    Serial.println("         MODUL 12: CUSTOM MEMORY POOL (STM32)               ");
    Serial.println("════════════════════════════════════════════════════════════\n");
    
    Serial.println("Memory Pool vs Heap:");
    Serial.println("  + Deterministic O(1) allocation time");
    Serial.println("  + No fragmentation");
    Serial.println("  + Easy tracking and debugging");
    Serial.println("  - Fixed block size (may waste memory)");
    Serial.println("  - Fixed total capacity");
    Serial.println("");
    
    // Initialize LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    
    // Initialize random
    randomSeed(analogRead(PA0));
    
    // Initialize memory pool
    Serial.println("Initializing memory pool...");
    if (!MemPool_Init(&sensorDataPool, POOL_BLOCK_SIZE, POOL_NUM_BLOCKS)) {
        Serial.println("[ERROR] Failed to initialize memory pool!");
        while (1);
    }
    
    Serial.printf("SensorData_t size: %d bytes (block: %d bytes)\n",
                 sizeof(SensorData_t), POOL_BLOCK_SIZE);
    
    // Create queue for passing pointers
    xDataQueue = xQueueCreate(POOL_NUM_BLOCKS, sizeof(SensorData_t*));
    if (xDataQueue == NULL) {
        Serial.println("[ERROR] Failed to create queue!");
        while (1);
    }
    
    // Create tasks
    xTaskCreate(vProducerTask, "Producer", 256, NULL, 1, &xProducerTask);
    xTaskCreate(vConsumerTask, "Consumer", 256, NULL, 1, &xConsumerTask);
    xTaskCreate(vMonitorTask, "Monitor", 256, NULL, 2, &xMonitorTask);
    
    Serial.println("\nTasks created. Starting scheduler...\n");
    
    vTaskStartScheduler();
    
    Serial.println("[ERROR] Scheduler failed!");
}

void loop() {
    // Empty
}

/**
 * PENJELASAN:
 * 
 * 1. Memory Pool Concept:
 *    - Pre-alokasi array of fixed-size blocks
 *    - Bitmap untuk tracking block status
 *    - O(1) allocation dengan linear search
 * 
 * 2. Thread Safety:
 *    - Mutex melindungi akses concurrent
 *    - Critical section minimal
 * 
 * 3. Validation:
 *    - Address range check
 *    - Double-free detection
 *    - Block index verification
 * 
 * 4. Kapan Menggunakan Memory Pool:
 *    - Real-time systems yang butuh deterministic timing
 *    - Banyak alokasi/free dengan ukuran sama
 *    - Limited RAM environment
 *    - Menghindari heap fragmentation
 * 
 * EXPECTED OUTPUT:
 * 
 * [Producer] Created DATA_0: T=25.3 H=55.7
 * [Consumer] Processing DATA_0...
 * [Consumer] Done: T=25.3°C, H=55.7%, P=1015.2hPa
 * 
 * ┌──────────────────────────────────────────┐
 * │         MEMORY POOL STATISTICS           │
 * │ Free Blocks:       14                    │
 * │ Used Blocks:        2                    │
 * │ Usage: [██░░░░░░░░░░░░░░]                │
 * │ Utilization:     12.5%                   │
 * └──────────────────────────────────────────┘
 */
