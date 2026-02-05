/**
 * @file main.cpp
 * @brief Program 07: ESP32 Static Allocation Dual Core
 * 
 * Static allocation untuk aplikasi dual-core ESP32.
 * Semua memory dialokasikan compile-time.
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#define LED_PIN     2

// Stack sizes (bytes for ESP32)
#define TASK_STACK_SIZE     4096

// Static memory for tasks
static StackType_t xCore0Stack[TASK_STACK_SIZE / sizeof(StackType_t)];
static StackType_t xCore1Stack[TASK_STACK_SIZE / sizeof(StackType_t)];
static StackType_t xMonitorStack[TASK_STACK_SIZE / sizeof(StackType_t)];

static StaticTask_t xCore0TCB;
static StaticTask_t xCore1TCB;
static StaticTask_t xMonitorTCB;

// Static queue
#define QUEUE_LENGTH    10
#define QUEUE_ITEM_SIZE sizeof(uint32_t)

static uint8_t ucQueueStorage[QUEUE_LENGTH * QUEUE_ITEM_SIZE];
static StaticQueue_t xStaticQueue;
static QueueHandle_t xQueue = NULL;

// Static semaphore
static StaticSemaphore_t xMutexBuffer;
static SemaphoreHandle_t xMutex = NULL;

// Task handles
static TaskHandle_t xCore0Task = NULL;
static TaskHandle_t xCore1Task = NULL;
static TaskHandle_t xMonitorTask = NULL;

// Shared counter (protected by mutex)
static volatile uint32_t sharedCounter = 0;

void vCore0Task(void *pvParameters) {
    Serial.println("[Core0] Static task started");
    
    uint32_t localCounter = 0;
    
    for (;;) {
        // Protected increment
        if (xSemaphoreTake(xMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            sharedCounter++;
            localCounter = sharedCounter;
            xSemaphoreGive(xMutex);
        }
        
        // Send to queue
        if (xQueueSend(xQueue, &localCounter, pdMS_TO_TICKS(100)) == pdTRUE) {
            Serial.printf("[Core0] Sent: %lu\n", localCounter);
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vCore1Task(void *pvParameters) {
    Serial.println("[Core1] Static task started");
    
    uint32_t received;
    
    for (;;) {
        if (xQueueReceive(xQueue, &received, pdMS_TO_TICKS(2000)) == pdTRUE) {
            Serial.printf("[Core1] Received: %lu\n", received);
            
            // LED blink
            digitalWrite(LED_PIN, HIGH);
            vTaskDelay(pdMS_TO_TICKS(100));
            digitalWrite(LED_PIN, LOW);
        }
    }
}

void vMonitorTask(void *pvParameters) {
    Serial.println("[Monitor] Static task started");
    
    for (;;) {
        Serial.println("\n╔════════════════════════════════════════════════════╗");
        Serial.println("║     STATIC ALLOCATION DUAL CORE STATUS             ║");
        Serial.println("╠════════════════════════════════════════════════════╣");
        Serial.println("║ All memory statically allocated at compile time    ║");
        Serial.println("╠════════════════════════════════════════════════════╣");
        
        // Stack info
        Serial.println("║ Task Stack High Water Marks:                       ║");
        UBaseType_t hwm0 = uxTaskGetStackHighWaterMark(xCore0Task);
        UBaseType_t hwm1 = uxTaskGetStackHighWaterMark(xCore1Task);
        UBaseType_t hwmM = uxTaskGetStackHighWaterMark(xMonitorTask);
        
        Serial.printf("║   Core0:   %5lu bytes free                        ║\n", hwm0 * 4);
        Serial.printf("║   Core1:   %5lu bytes free                        ║\n", hwm1 * 4);
        Serial.printf("║   Monitor: %5lu bytes free                        ║\n", hwmM * 4);
        
        Serial.println("╠════════════════════════════════════════════════════╣");
        
        // Queue status
        Serial.printf("║ Static Queue: %d / %d items                        ║\n",
                     uxQueueMessagesWaiting(xQueue), QUEUE_LENGTH);
        
        Serial.println("╠════════════════════════════════════════════════════╣");
        
        // Memory usage
        size_t taskMem = 3 * TASK_STACK_SIZE;
        size_t tcbMem = 3 * sizeof(StaticTask_t);
        size_t queueMem = sizeof(ucQueueStorage) + sizeof(StaticQueue_t);
        size_t mutexMem = sizeof(StaticSemaphore_t);
        size_t totalStatic = taskMem + tcbMem + queueMem + mutexMem;
        
        Serial.println("║ Static Memory Usage:                               ║");
        Serial.printf("║   Task stacks: %5d bytes                         ║\n", taskMem);
        Serial.printf("║   TCBs:        %5d bytes                         ║\n", tcbMem);
        Serial.printf("║   Queue:       %5d bytes                         ║\n", queueMem);
        Serial.printf("║   Mutex:       %5d bytes                         ║\n", mutexMem);
        Serial.printf("║   TOTAL:       %5d bytes                         ║\n", totalStatic);
        
        Serial.println("╠════════════════════════════════════════════════════╣");
        Serial.printf("║ Shared Counter: %lu                                ║\n", sharedCounter);
        Serial.printf("║ Free Heap: %lu bytes (unused with static)          ║\n",
                     (unsigned long)ESP.getFreeHeap());
        Serial.println("╚════════════════════════════════════════════════════╝");
        
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n═══════════════════════════════════════════════════════");
    Serial.println("      ESP32 STATIC ALLOCATION DUAL CORE DEMO");
    Serial.println("═══════════════════════════════════════════════════════\n");
    
    pinMode(LED_PIN, OUTPUT);
    
    Serial.println("Creating static objects...\n");
    
    // Create static queue
    xQueue = xQueueCreateStatic(QUEUE_LENGTH, QUEUE_ITEM_SIZE, 
                                ucQueueStorage, &xStaticQueue);
    if (xQueue == NULL) {
        Serial.println("Queue creation failed!");
        while (1);
    }
    Serial.println("✓ Static queue created");
    
    // Create static mutex
    xMutex = xSemaphoreCreateMutexStatic(&xMutexBuffer);
    if (xMutex == NULL) {
        Serial.println("Mutex creation failed!");
        while (1);
    }
    Serial.println("✓ Static mutex created");
    
    // Create static tasks on specific cores
    xCore0Task = xTaskCreateStaticPinnedToCore(
        vCore0Task,
        "Core0",
        TASK_STACK_SIZE / sizeof(StackType_t),
        NULL,
        2,
        xCore0Stack,
        &xCore0TCB,
        0  // Core 0
    );
    Serial.println("✓ Core0 static task created");
    
    xCore1Task = xTaskCreateStaticPinnedToCore(
        vCore1Task,
        "Core1",
        TASK_STACK_SIZE / sizeof(StackType_t),
        NULL,
        2,
        xCore1Stack,
        &xCore1TCB,
        1  // Core 1
    );
    Serial.println("✓ Core1 static task created");
    
    xMonitorTask = xTaskCreateStaticPinnedToCore(
        vMonitorTask,
        "Monitor",
        TASK_STACK_SIZE / sizeof(StackType_t),
        NULL,
        1,
        xMonitorStack,
        &xMonitorTCB,
        0
    );
    Serial.println("✓ Monitor static task created");
    
    Serial.println("\nAll static objects created successfully!\n");
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
