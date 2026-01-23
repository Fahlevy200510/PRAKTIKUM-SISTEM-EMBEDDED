/**
 * ============================================================================
 * FILE: FreeRTOSConfig.h
 * PROJECT: 37-Memory_Pool_Pattern
 * 
 * JUDUL: Memory Pool Pattern - Fixed Size Block Allocator
 * 
 * ============================================================================
 * KONSEP MEMORY POOL
 * ============================================================================
 * 
 * Memory Pool adalah teknik alokasi memory dengan pre-allocated blocks
 * berukuran sama. Menghindari fragmentasi dan memberikan O(1) allocation.
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                     MEMORY POOL LAYOUT                              │
 *    │                                                                     │
 *    │   Pool: 10 blocks × 32 bytes = 320 bytes total                     │
 *    │                                                                     │
 *    │   ┌────────┬────────┬────────┬────────┬────────┐                   │
 *    │   │Block 0 │Block 1 │Block 2 │Block 3 │Block 4 │ ...              │
 *    │   │ USED   │ FREE   │ USED   │ FREE   │ FREE   │                   │
 *    │   └────────┴────────┴────────┴────────┴────────┘                   │
 *    │                                                                     │
 *    │   Free List (linked):                                               │
 *    │   Head → Block 1 → Block 3 → Block 4 → ... → NULL                  │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * KEUNTUNGAN MEMORY POOL
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │  ✓ O(1) Allocation dan Free - sangat cepat                         │
 *    │  ✓ Zero fragmentation - semua blocks sama ukuran                   │
 *    │  ✓ Deterministic timing - ideal untuk real-time                    │
 *    │  ✓ Bounded memory usage - tidak bisa exceed pool size              │
 *    │  ✓ Thread-safe dengan mutex                                        │
 *    │                                                                     │
 *    │  ✗ Hanya untuk fixed-size objects                                  │
 *    │  ✗ Memory waste jika object lebih kecil dari block                 │
 *    │  ✗ Harus size pool correctly beforehand                            │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * IMPLEMENTATION
 * ============================================================================
 * 
 *    typedef struct MemoryPool {
 *        void *pxFreeList;      // Linked list of free blocks
 *        size_t xBlockSize;     // Size of each block
 *        size_t xBlockCount;    // Total blocks
 *        size_t xFreeCount;     // Available blocks
 *        SemaphoreHandle_t xMutex;  // Thread protection
 *    } MemoryPool_t;
 *    
 *    void *MemoryPool_Alloc(MemoryPool_t *pxPool):
 *      1. Take mutex
 *      2. Get head of free list
 *      3. Update head to next free
 *      4. Return block
 *    
 *    void MemoryPool_Free(MemoryPool_t *pxPool, void *pxBlock):
 *      1. Take mutex
 *      2. Add block to head of free list
 *      3. Release mutex
 * 
 * ============================================================================
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configUSE_TICKLESS_IDLE                 0
#define configCPU_CLOCK_HZ                      72000000UL
#define configTICK_RATE_HZ                      1000
#define configMAX_PRIORITIES                    5
#define configMINIMAL_STACK_SIZE                128
#define configMAX_TASK_NAME_LEN                 16
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_TASK_NOTIFICATIONS            1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES   1
#define configUSE_MUTEXES                       1
#define configUSE_COUNTING_SEMAPHORES           1
#define configQUEUE_REGISTRY_SIZE               8

#define configSUPPORT_STATIC_ALLOCATION         0
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configTOTAL_HEAP_SIZE                   ((size_t)10240)

#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configCHECK_FOR_STACK_OVERFLOW          2
#define configUSE_MALLOC_FAILED_HOOK            1

#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            256

#ifdef __NVIC_PRIO_BITS
    #define configPRIO_BITS __NVIC_PRIO_BITS
#else
    #define configPRIO_BITS 4
#endif

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5
#define configKERNEL_INTERRUPT_PRIORITY         (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_xResumeFromISR                  1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_uxTaskGetStackHighWaterMark     1
#define INCLUDE_xTimerPendFunctionCall          1

#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#define LED_GPIO_PORT       GPIOC
#define LED_GPIO_PIN        GPIO_PIN_13

#define DEBUG_UART_INSTANCE   USART1
#define DEBUG_UART_BAUDRATE   115200
#define DEBUG_UART_TX_PORT    GPIOA
#define DEBUG_UART_TX_PIN     GPIO_PIN_9
#define DEBUG_UART_RX_PORT    GPIOA
#define DEBUG_UART_RX_PIN     GPIO_PIN_10

/* Memory Pool config */
#define POOL_BLOCK_SIZE     32    /* Each block is 32 bytes */
#define POOL_BLOCK_COUNT    10    /* 10 blocks total */

#define TASK_STACK          256

#endif /* FREERTOS_CONFIG_H */
