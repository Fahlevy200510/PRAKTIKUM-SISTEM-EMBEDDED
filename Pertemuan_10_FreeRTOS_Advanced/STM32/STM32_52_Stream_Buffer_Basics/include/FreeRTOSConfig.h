/**
 * ============================================================================
 * FILE: FreeRTOSConfig.h
 * PROJECT: 29-Stream_Buffer_Basics
 * 
 * JUDUL: Stream Buffer - Byte Stream Communication
 * 
 * ============================================================================
 * KONSEP STREAM BUFFER
 * ============================================================================
 * 
 * Stream Buffer adalah mekanisme untuk transfer BYTE STREAM
 * dari satu task/ISR ke task lain. Berbeda dengan Queue yang
 * transfer discrete items, Stream Buffer transfer continuous bytes.
 * 
 *    QUEUE vs STREAM BUFFER:
 *    ══════════════════════════════════════════════════════════════════════
 *    
 *    QUEUE:
 *    - Transfer fixed-size items
 *    - Setiap item independen
 *    - Overhead per-item (header)
 *    
 *    ┌─────┬─────┬─────┬─────┬─────┐
 *    │Item1│Item2│Item3│Item4│ ... │
 *    └─────┴─────┴─────┴─────┴─────┘
 *    
 *    STREAM BUFFER:
 *    - Transfer continuous byte stream
 *    - Tidak ada batas item
 *    - Minimal overhead
 *    
 *    ┌─────────────────────────────────┐
 *    │ A B C D E F G H I J K L M N O P │
 *    └─────────────────────────────────┘
 *      └─── continuous byte stream ───┘
 * 
 * ============================================================================
 * KAPAN GUNAKAN STREAM BUFFER
 * ============================================================================
 * 
 *    COCOK untuk:
 *    ✓ UART data reception
 *    ✓ Audio streaming
 *    ✓ Log messages
 *    ✓ Continuous sensor data
 *    
 *    TIDAK COCOK untuk:
 *    ✗ Discrete messages/events
 *    ✗ Structured data packets (gunakan Message Buffer)
 *    ✗ Multiple writers (Stream Buffer = 1 writer only!)
 * 
 * ============================================================================
 * TRIGGER LEVEL
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                                                                     │
 *    │   Stream Buffer: size=100 bytes, trigger=10 bytes                  │
 *    │                                                                     │
 *    │   ┌──────────────────────────────────────────────────────────────┐  │
 *    │   │░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░│  │
 *    │   └──────────────────────────────────────────────────────────────┘  │
 *    │   ↑         ↑                                                       │
 *    │   0         10 (trigger level)                                      │
 *    │                                                                     │
 *    │   Receiver akan UNBLOCK jika:                                       │
 *    │   - Buffer memiliki >= 10 bytes (trigger level), ATAU               │
 *    │   - Timeout tercapai                                                │
 *    │                                                                     │
 *    │   Ini memungkinkan "batch processing" untuk efisiensi               │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * API STREAM BUFFER
 * ============================================================================
 * 
 *    // Create
 *    StreamBufferHandle_t xStreamBufferCreate(size_t xBufferSizeBytes,
 *                                              size_t xTriggerLevelBytes);
 *    
 *    // Send (Task context)
 *    size_t xStreamBufferSend(StreamBufferHandle_t xStreamBuffer,
 *                             const void *pvTxData,
 *                             size_t xDataLengthBytes,
 *                             TickType_t xTicksToWait);
 *    
 *    // Receive (Task context)
 *    size_t xStreamBufferReceive(StreamBufferHandle_t xStreamBuffer,
 *                                void *pvRxData,
 *                                size_t xBufferLengthBytes,
 *                                TickType_t xTicksToWait);
 *    
 *    // FromISR versions
 *    size_t xStreamBufferSendFromISR(...)
 *    size_t xStreamBufferReceiveFromISR(...)
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
#define INCLUDE_xTaskResumeFromISR              1

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
#define STREAM_BUFFER_SIZE    100   /* Total buffer size */
#define TRIGGER_LEVEL         10    /* Unblock when this many bytes available */

#define TASK_STACK            256

#endif /* FREERTOS_CONFIG_H */
