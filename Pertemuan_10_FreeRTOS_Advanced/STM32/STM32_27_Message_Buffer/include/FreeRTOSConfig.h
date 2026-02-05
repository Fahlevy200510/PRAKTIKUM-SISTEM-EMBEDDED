/**
 * ============================================================================
 * FILE: FreeRTOSConfig.h
 * PROJECT: 30-Message_Buffer
 * 
 * JUDUL: Message Buffer - Framed Message Communication
 * 
 * ============================================================================
 * KONSEP MESSAGE BUFFER
 * ============================================================================
 * 
 * Message Buffer adalah varian dari Stream Buffer yang menambahkan
 * FRAMING - setiap write/read adalah DISCRETE MESSAGE dengan length prefix.
 * 
 *    STREAM BUFFER vs MESSAGE BUFFER:
 *    ══════════════════════════════════════════════════════════════════════
 *    
 *    STREAM BUFFER (continuous bytes):
 *    ┌─────────────────────────────────┐
 *    │ A B C D E F G H I J K L M N O P │  <-- No message boundaries
 *    └─────────────────────────────────┘
 *    Read dapat return any number of bytes
 *    
 *    MESSAGE BUFFER (discrete messages):
 *    ┌───┬───────┬───┬─────────┬───┬───────┐
 *    │ 5 │HELLO │ 3 │BYE    │ 7 │WORLD! │  <-- Length prefixed
 *    └───┴───────┴───┴─────────┴───┴───────┘
 *     │   └─5 bytes│  └3 bytes│   └7 bytes
 *     │            │          │
 *     └──length────┴──length──┴──length
 *    
 *    Read SELALU return complete message atau nothing
 * 
 * ============================================================================
 * MENGAPA MESSAGE BUFFER?
 * ============================================================================
 * 
 *    PROBLEM dengan Stream Buffer untuk discrete messages:
 *    
 *    Sender: "HELLO"                  Receiver mungkin dapat:
 *    Sender: "BYE"                    "HELLOBYE" (merged)
 *                                     atau "HEL" + "LOBYE" (split)
 *    
 *    Message Buffer GUARANTEES:
 *    ✓ Satu xMessageBufferReceive = satu complete message
 *    ✓ Message boundaries preserved
 *    ✓ No partial messages
 * 
 * ============================================================================
 * INTERNAL STRUCTURE
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                      MESSAGE BUFFER LAYOUT                          │
 *    │                                                                     │
 *    │   4-byte    N-byte         4-byte    M-byte                        │
 *    │   ┌───────┬─────────────┬───────┬─────────────┬─────────────────┐  │
 *    │   │Len=N  │  Message 1  │Len=M  │  Message 2  │  (free space)   │  │
 *    │   └───────┴─────────────┴───────┴─────────────┴─────────────────┘  │
 *    │                                                                     │
 *    │   Overhead: sizeof(size_t) = 4 bytes per message                   │
 *    │   Max message size = BufferSize - 4                                │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * API MESSAGE BUFFER
 * ============================================================================
 * 
 *    // Create
 *    MessageBufferHandle_t xMessageBufferCreate(size_t xBufferSizeBytes);
 *    
 *    // Send complete message
 *    size_t xMessageBufferSend(MessageBufferHandle_t xMessageBuffer,
 *                              const void *pvTxData,
 *                              size_t xDataLengthBytes,
 *                              TickType_t xTicksToWait);
 *    
 *    // Receive complete message
 *    size_t xMessageBufferReceive(MessageBufferHandle_t xMessageBuffer,
 *                                 void *pvRxData,
 *                                 size_t xBufferLengthBytes,
 *                                 TickType_t xTicksToWait);
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

/* Message Buffer config */
#define MESSAGE_BUFFER_SIZE   200   /* Total buffer size */
#define MAX_MESSAGE_SIZE      50    /* Max single message */

#define TASK_STACK            256

#endif /* FREERTOS_CONFIG_H */
