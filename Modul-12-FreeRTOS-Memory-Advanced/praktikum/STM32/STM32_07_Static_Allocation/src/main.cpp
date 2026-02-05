/**
 * @file main.cpp
 * @brief Program 07: Static Allocation pada STM32
 * 
 * Deskripsi:
 * Demonstrasi penggunaan static allocation di FreeRTOS.
 * Semua memory dialokasikan secara statis tanpa heap.
 * 
 * Keuntungan Static Allocation:
 * - Deterministic - no runtime allocation failures
 * - Smaller code size (no heap manager)
 * - Easier to verify memory usage at compile time
 * - Better for safety-critical applications
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include <STM32FreeRTOS.h>

// ==================== KONFIGURASI ====================
#define LED_PIN         PC13
#define SERIAL_BAUD     115200

// Stack sizes
#define TASK1_STACK_SIZE    256
#define TASK2_STACK_SIZE    256
#define TASK3_STACK_SIZE    256

// Queue parameters
#define QUEUE_LENGTH        10
#define QUEUE_ITEM_SIZE     sizeof(uint32_t)

// ==================== STATIC MEMORY ALLOCATION ====================

// Task stack buffers (allocated in .bss section)
static StackType_t xTask1Stack[TASK1_STACK_SIZE];
static StackType_t xTask2Stack[TASK2_STACK_SIZE];
static StackType_t xTask3Stack[TASK3_STACK_SIZE];

// Task control blocks (TCB)
static StaticTask_t xTask1TCB;
static StaticTask_t xTask2TCB;
static StaticTask_t xTask3TCB;

// Queue storage
static uint8_t ucQueueStorage[QUEUE_LENGTH * QUEUE_ITEM_SIZE];
static StaticQueue_t xStaticQueue;

// Semaphore storage
static StaticSemaphore_t xMutexBuffer;

// Timer storage
static StaticTimer_t xTimerBuffer;

// Idle task memory (required when configSUPPORT_STATIC_ALLOCATION=1)
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];
static StaticTask_t xIdleTCB;

// Timer task memory (if using software timers)
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];
static StaticTask_t xTimerTCB;

// ==================== VARIABEL GLOBAL ====================
TaskHandle_t xTask1Handle = NULL;
TaskHandle_t xTask2Handle = NULL;
TaskHandle_t xTask3Handle = NULL;

QueueHandle_t xStaticQueueHandle = NULL;
SemaphoreHandle_t xMutexHandle = NULL;
TimerHandle_t xTimerHandle = NULL;

volatile uint32_t timerCallbackCount = 0;

// ==================== REQUIRED CALLBACKS ====================

/**
 * @brief Callback untuk menyediakan memory untuk Idle task
 * REQUIRED ketika configSUPPORT_STATIC_ALLOCATION = 1
 */
extern "C" void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                              StackType_t **ppxIdleTaskStackBuffer,
                                              uint32_t *pulIdleTaskStackSize) {
    *ppxIdleTaskTCBBuffer = &xIdleTCB;
    *ppxIdleTaskStackBuffer = xIdleStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/**
 * @brief Callback untuk menyediakan memory untuk Timer task
 * REQUIRED ketika configUSE_TIMERS = 1 dan configSUPPORT_STATIC_ALLOCATION = 1
 */
extern "C" void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                               StackType_t **ppxTimerTaskStackBuffer,
                                               uint32_t *pulTimerTaskStackSize) {
    *ppxTimerTaskTCBBuffer = &xTimerTCB;
    *ppxTimerTaskStackBuffer = xTimerStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

// ==================== TIMER CALLBACK ====================

void vTimerCallback(TimerHandle_t xTimer) {
    timerCallbackCount++;
    // Quick toggle for visibility
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
}

// ==================== TASKS ====================

/**
 * @brief Producer task - mengirim data ke queue
 */
void vProducerTask(void *pvParameters) {
    Serial.println("[Producer] Static task started");
    
    uint32_t counter = 0;
    
    for (;;) {
        // Take mutex before sending
        if (xSemaphoreTake(xMutexHandle, pdMS_TO_TICKS(100)) == pdTRUE) {
            // Send to static queue
            if (xQueueSend(xStaticQueueHandle, &counter, pdMS_TO_TICKS(100)) == pdTRUE) {
                Serial.printf("[Producer] Sent: %lu\n", counter);
                counter++;
            } else {
                Serial.println("[Producer] Queue full!");
            }
            
            xSemaphoreGive(xMutexHandle);
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
 * @brief Consumer task - menerima data dari queue
 */
void vConsumerTask(void *pvParameters) {
    Serial.println("[Consumer] Static task started");
    
    uint32_t receivedValue;
    
    for (;;) {
        if (xQueueReceive(xStaticQueueHandle, &receivedValue, pdMS_TO_TICKS(2000)) == pdTRUE) {
            Serial.printf("[Consumer] Received: %lu\n", receivedValue);
        } else {
            Serial.println("[Consumer] Timeout waiting for data");
        }
    }
}

/**
 * @brief Monitor task - menampilkan status sistem
 */
void vMonitorTask(void *pvParameters) {
    Serial.println("[Monitor] Static task started");
    
    for (;;) {
        Serial.println("\n╔════════════════════════════════════════════════╗");
        Serial.println("║        STATIC ALLOCATION SYSTEM STATUS         ║");
        Serial.println("╠════════════════════════════════════════════════╣");
        
        // Static allocation info
        Serial.println("║ Memory Allocation: ALL STATIC                  ║");
        Serial.println("║ No heap used - deterministic behavior          ║");
        Serial.println("╠════════════════════════════════════════════════╣");
        
        // Task stacks (compile-time allocated)
        Serial.println("║ Static Task Stacks:                            ║");
        Serial.printf("║   Task1: %d words at 0x%08X             ║\n",
                     TASK1_STACK_SIZE, (unsigned int)xTask1Stack);
        Serial.printf("║   Task2: %d words at 0x%08X             ║\n",
                     TASK2_STACK_SIZE, (unsigned int)xTask2Stack);
        Serial.printf("║   Task3: %d words at 0x%08X             ║\n",
                     TASK3_STACK_SIZE, (unsigned int)xTask3Stack);
        
        Serial.println("╠════════════════════════════════════════════════╣");
        
        // Stack high water marks
        Serial.println("║ Stack Usage (High Water Marks):                ║");
        UBaseType_t hwm1 = uxTaskGetStackHighWaterMark(xTask1Handle);
        UBaseType_t hwm2 = uxTaskGetStackHighWaterMark(xTask2Handle);
        UBaseType_t hwm3 = uxTaskGetStackHighWaterMark(xTask3Handle);
        
        Serial.printf("║   Task1 (Producer): %3d words free            ║\n", hwm1);
        Serial.printf("║   Task2 (Consumer): %3d words free            ║\n", hwm2);
        Serial.printf("║   Task3 (Monitor):  %3d words free            ║\n", hwm3);
        
        Serial.println("╠════════════════════════════════════════════════╣");
        
        // Queue status
        UBaseType_t queueSpaces = uxQueueSpacesAvailable(xStaticQueueHandle);
        UBaseType_t queueWaiting = uxQueueMessagesWaiting(xStaticQueueHandle);
        
        Serial.printf("║ Static Queue: %d/%d items                      ║\n",
                     queueWaiting, QUEUE_LENGTH);
        Serial.printf("║ Timer callbacks: %lu                           ║\n",
                     timerCallbackCount);
        
        Serial.println("╠════════════════════════════════════════════════╣");
        
        // Memory summary
        Serial.println("║ Static Memory Summary:                         ║");
        size_t taskMem = (TASK1_STACK_SIZE + TASK2_STACK_SIZE + TASK3_STACK_SIZE) * sizeof(StackType_t);
        size_t tcbMem = 3 * sizeof(StaticTask_t);
        size_t queueMem = sizeof(ucQueueStorage) + sizeof(StaticQueue_t);
        size_t otherMem = sizeof(StaticSemaphore_t) + sizeof(StaticTimer_t);
        size_t idleMem = configMINIMAL_STACK_SIZE * sizeof(StackType_t) + sizeof(StaticTask_t);
        
        Serial.printf("║   Task stacks: %5d bytes                     ║\n", taskMem);
        Serial.printf("║   TCBs:        %5d bytes                     ║\n", tcbMem);
        Serial.printf("║   Queue:       %5d bytes                     ║\n", queueMem);
        Serial.printf("║   Mutex/Timer: %5d bytes                     ║\n", otherMem);
        Serial.printf("║   Idle task:   %5d bytes                     ║\n", idleMem);
        Serial.printf("║   ─────────────────────                        ║\n");
        Serial.printf("║   TOTAL:       %5d bytes                     ║\n",
                     taskMem + tcbMem + queueMem + otherMem + idleMem);
        
        Serial.println("╚════════════════════════════════════════════════╝");
        
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// ==================== SETUP & LOOP ====================

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(2000);
    
    Serial.println("\n════════════════════════════════════════════════════════════");
    Serial.println("           MODUL 12: STATIC ALLOCATION (STM32)              ");
    Serial.println("════════════════════════════════════════════════════════════\n");
    
    Serial.println("Static vs Dynamic Allocation:");
    Serial.println("");
    Serial.println("  Dynamic (default):");
    Serial.println("    + Flexible at runtime");
    Serial.println("    - Can fail if heap exhausted");
    Serial.println("    - Fragmentation possible");
    Serial.println("");
    Serial.println("  Static (this demo):");
    Serial.println("    + No runtime failures");
    Serial.println("    + Deterministic");
    Serial.println("    + No fragmentation");
    Serial.println("    - Must size at compile time");
    Serial.println("");
    
    // Initialize LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    
    // Create static queue
    xStaticQueueHandle = xQueueCreateStatic(QUEUE_LENGTH,
                                            QUEUE_ITEM_SIZE,
                                            ucQueueStorage,
                                            &xStaticQueue);
    
    if (xStaticQueueHandle == NULL) {
        Serial.println("[ERROR] Failed to create static queue!");
        while (1);
    }
    Serial.println("✓ Static queue created");
    
    // Create static mutex
    xMutexHandle = xSemaphoreCreateMutexStatic(&xMutexBuffer);
    
    if (xMutexHandle == NULL) {
        Serial.println("[ERROR] Failed to create static mutex!");
        while (1);
    }
    Serial.println("✓ Static mutex created");
    
    // Create static software timer
    xTimerHandle = xTimerCreateStatic("LEDTimer",
                                      pdMS_TO_TICKS(500),
                                      pdTRUE,  // Auto-reload
                                      (void*)0,
                                      vTimerCallback,
                                      &xTimerBuffer);
    
    if (xTimerHandle == NULL) {
        Serial.println("[ERROR] Failed to create static timer!");
        while (1);
    }
    Serial.println("✓ Static timer created");
    
    // Create static tasks
    xTask1Handle = xTaskCreateStatic(vProducerTask,
                                     "Producer",
                                     TASK1_STACK_SIZE,
                                     NULL,
                                     1,
                                     xTask1Stack,
                                     &xTask1TCB);
    
    xTask2Handle = xTaskCreateStatic(vConsumerTask,
                                     "Consumer",
                                     TASK2_STACK_SIZE,
                                     NULL,
                                     1,
                                     xTask2Stack,
                                     &xTask2TCB);
    
    xTask3Handle = xTaskCreateStatic(vMonitorTask,
                                     "Monitor",
                                     TASK3_STACK_SIZE,
                                     NULL,
                                     2,
                                     xTask3Stack,
                                     &xTask3TCB);
    
    Serial.println("✓ Static tasks created");
    
    // Start timer
    xTimerStart(xTimerHandle, 0);
    Serial.println("✓ Timer started");
    
    Serial.println("\nAll objects created statically. Starting scheduler...\n");
    
    vTaskStartScheduler();
    
    Serial.println("[ERROR] Scheduler failed!");
}

void loop() {
    // Empty
}

/**
 * PENJELASAN STATIC ALLOCATION:
 * 
 * 1. Konfigurasi:
 *    configSUPPORT_STATIC_ALLOCATION = 1
 *    configSUPPORT_DYNAMIC_ALLOCATION = 0 (optional, untuk pure static)
 * 
 * 2. API Static:
 *    - xTaskCreateStatic() - task dengan buffer yang disediakan
 *    - xQueueCreateStatic() - queue dengan buffer yang disediakan
 *    - xSemaphoreCreateMutexStatic() - mutex statis
 *    - xTimerCreateStatic() - timer statis
 * 
 * 3. Required Callbacks:
 *    - vApplicationGetIdleTaskMemory() - memory untuk idle task
 *    - vApplicationGetTimerTaskMemory() - memory untuk timer task
 * 
 * 4. Memory Calculation:
 *    - StackType_t = 4 bytes (32-bit)
 *    - Stack size dalam words, bukan bytes
 *    - TCB size varies (~88-100 bytes typically)
 * 
 * KAPAN MENGGUNAKAN STATIC ALLOCATION:
 * 
 * 1. Safety-critical systems (IEC 61508, ISO 26262)
 * 2. Systems dengan memory constraints ketat
 * 3. Ketika determinism diperlukan
 * 4. Menghindari runtime allocation failures
 * 
 * EXPECTED OUTPUT:
 * 
 * ╔════════════════════════════════════════════════╗
 * ║        STATIC ALLOCATION SYSTEM STATUS         ║
 * ╠════════════════════════════════════════════════╣
 * ║ Memory Allocation: ALL STATIC                  ║
 * ║ Static Task Stacks:                            ║
 * ║   Task1: 256 words at 0x20000100               ║
 * ...
 */
