/**
 * ============================================================================
 * FILE: FreeRTOSConfig.h
 * PROJECT: 31-Stream_Buffer_Trigger
 * 
 * JUDUL: Stream Buffer Trigger Level - Batch Processing Optimization
 * 
 * ============================================================================
 * KONSEP TRIGGER LEVEL
 * ============================================================================
 * 
 * Trigger Level menentukan KAPAN receiver task di-unblock:
 *   - Receiver akan BLOCKED sampai trigger level bytes tersedia
 *   - Ini memungkinkan "batch processing" untuk efisiensi
 *   - Mengurangi context switch overhead
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                   TRIGGER LEVEL OPERATION                          │
 *    │                                                                     │
 *    │   Stream Buffer: 100 bytes, Trigger = 20 bytes                     │
 *    │                                                                     │
 *    │   State 1: 10 bytes in buffer (below trigger)                      │
 *    │   ┌──────────┬─────────────────────────────────────────────────────┐│
 *    │   │▓▓▓▓▓▓▓▓▓▓│░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░││
 *    │   └──────────┴─────────────────────────────────────────────────────┘│
 *    │     10 bytes                                                        │
 *    │   Consumer: BLOCKED ⏸                                              │
 *    │                                                                     │
 *    │   State 2: 25 bytes in buffer (above trigger!)                     │
 *    │   ┌─────────────────────────────┬──────────────────────────────────┐│
 *    │   │▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░││
 *    │   └─────────────────────────────┴──────────────────────────────────┘│
 *    │     25 bytes (>= 20 trigger)                                        │
 *    │   Consumer: UNBLOCKED! ▶                                           │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * TRIGGER LEVEL STRATEGIES
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │   TRIGGER = 1 (minimum, immediate processing)                      │
 *    │   + Lowest latency - process immediately                           │
 *    │   - More context switches                                          │
 *    │   - Less efficient for high-rate data                              │
 *    │   → Use case: Real-time commands, urgent data                      │
 *    │                                                                     │
 *    │   TRIGGER = Buffer/2 (balanced)                                    │
 *    │   + Good balance latency vs efficiency                             │
 *    │   + Reasonable batch size                                          │
 *    │   → Use case: Most applications                                    │
 *    │                                                                     │
 *    │   TRIGGER = Buffer-margin (maximum batching)                       │
 *    │   + Maximum efficiency - minimal context switches                  │
 *    │   - Higher latency                                                 │
 *    │   → Use case: High-throughput logging, bulk data                   │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * DYNAMIC TRIGGER ADJUSTMENT
 * ============================================================================
 * 
 *    xStreamBufferSetTriggerLevel() dapat mengubah trigger runtime:
 *    
 *    // Increase batch size for bulk transfer
 *    xStreamBufferSetTriggerLevel(xBuffer, 50);
 *    
 *    // Decrease for low-latency mode
 *    xStreamBufferSetTriggerLevel(xBuffer, 1);
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

/* Stream Buffer config */
#define STREAM_BUFFER_SIZE    100

/* Different trigger levels to demonstrate */
#define TRIGGER_LOW           1    /* Immediate processing */
#define TRIGGER_MEDIUM        20   /* Balanced */
#define TRIGGER_HIGH          50   /* Maximum batching */

#define TASK_STACK            256

#endif /* FREERTOS_CONFIG_H */
