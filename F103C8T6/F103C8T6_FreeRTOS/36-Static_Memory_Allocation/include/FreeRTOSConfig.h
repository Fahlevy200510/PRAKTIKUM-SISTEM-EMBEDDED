/**
 * ============================================================================
 * FILE: FreeRTOSConfig.h
 * PROJECT: 36-Static_Memory_Allocation
 * 
 * JUDUL: Static Memory Allocation - No Dynamic Memory
 * 
 * ============================================================================
 * KONSEP STATIC ALLOCATION
 * ============================================================================
 * 
 * Secara default FreeRTOS menggunakan dynamic memory (pvPortMalloc).
 * Static allocation memungkinkan semua memory dialokasikan compile-time.
 * 
 *    DYNAMIC vs STATIC:
 *    ══════════════════════════════════════════════════════════════════════
 *    
 *    DYNAMIC (default):
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │  + Fleksibel - create/delete runtime                               │
 *    │  + Heap shared by all objects                                      │
 *    │  - Risk of fragmentation                                           │
 *    │  - Risk of allocation failure                                      │
 *    │  - Slightly less deterministic                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 *    
 *    STATIC:
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │  + Deterministic - memory guaranteed at compile time               │
 *    │  + No fragmentation                                                │
 *    │  + No allocation failures (unless you don't have enough RAM)       │
 *    │  + Required for safety-critical systems (DO-178C, IEC 61508)       │
 *    │  - Less flexible - can't create at runtime                         │
 *    │  - Must size everything beforehand                                 │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * STATIC ALLOCATION APIs
 * ============================================================================
 * 
 *    // Static task
 *    xTaskCreateStatic(pxTaskCode, pcName, ulStackDepth, 
 *                      pvParameters, uxPriority, 
 *                      puxStackBuffer,  // You provide stack
 *                      pxTaskBuffer);   // You provide TCB
 *    
 *    // Static queue
 *    xQueueCreateStatic(uxQueueLength, uxItemSize, 
 *                       pucQueueStorage,  // You provide storage
 *                       pxQueueBuffer);   // You provide structure
 *    
 *    // Static semaphore
 *    xSemaphoreCreateBinaryStatic(pxSemaphoreBuffer);
 *    xSemaphoreCreateMutexStatic(pxMutexBuffer);
 * 
 * ============================================================================
 * REQUIRED CALLBACKS
 * ============================================================================
 * 
 *    Saat configSUPPORT_STATIC_ALLOCATION = 1, Anda HARUS implement:
 *    
 *    vApplicationGetIdleTaskMemory():
 *      - Provide stack and TCB for Idle task
 *    
 *    vApplicationGetTimerTaskMemory() [if configUSE_TIMERS=1]:
 *      - Provide stack and TCB for Timer task
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

/* Enable BOTH static and dynamic for this demo */
#define configSUPPORT_STATIC_ALLOCATION         1
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configTOTAL_HEAP_SIZE                   ((size_t)8192)

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

#define STATIC_TASK_STACK     256

#endif /* FREERTOS_CONFIG_H */
