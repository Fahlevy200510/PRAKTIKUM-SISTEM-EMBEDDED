/**
 * ============================================================================
 * FILE: FreeRTOSConfig.h
 * PROJECT: 33-Message_Buffer_Variable
 * 
 * JUDUL: Variable Length Messages dengan Message Buffer
 * 
 * ============================================================================
 * KONSEP VARIABLE LENGTH MESSAGES
 * ============================================================================
 * 
 * Message Buffer mendukung VARIABLE LENGTH messages secara native.
 * Setiap message disimpan dengan 4-byte length prefix, sehingga
 * receiver tahu persis berapa bytes yang harus dibaca.
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                  VARIABLE LENGTH MESSAGES                          │
 *    │                                                                     │
 *    │   Message 1         Message 2         Message 3                    │
 *    │   ┌───┬─────────┐  ┌───┬───────────────┐  ┌───┬───────┐            │
 *    │   │ 5 │ H E L L O│  │12 │ L O N G   M E S│  │ 2 │ H I │            │
 *    │   └───┴─────────┘  └───┴───────────────┘  └───┴───────┘            │
 *    │    5 bytes          12 bytes               2 bytes                  │
 *    │                                                                     │
 *    │   Receiver gets:                                                    │
 *    │   1. "HELLO" (5 bytes)                                             │
 *    │   2. "LONG MESSAGE" (12 bytes)                                     │
 *    │   3. "HI" (2 bytes)                                                │
 *    │                                                                     │
 *    │   Each receive = complete message, no matter the size!             │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * BUFFER SIZING
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │  Buffer Size Calculation:                                          │
 *    │                                                                     │
 *    │  Per message overhead = sizeof(size_t) = 4 bytes                   │
 *    │                                                                     │
 *    │  Example: 3 messages of 10, 20, 30 bytes                           │
 *    │                                                                     │
 *    │  Required buffer size:                                             │
 *    │    Message 1: 4 + 10 = 14 bytes                                    │
 *    │    Message 2: 4 + 20 = 24 bytes                                    │
 *    │    Message 3: 4 + 30 = 34 bytes                                    │
 *    │    Total: 14 + 24 + 34 = 72 bytes                                  │
 *    │                                                                     │
 *    │  Formula: Sum(4 + msg_size) for all concurrent messages            │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * RECEIVE BUFFER SIZING
 * ============================================================================
 * 
 *    PENTING: Receive buffer HARUS cukup besar untuk message terbesar!
 *    
 *    Jika receive buffer terlalu kecil:
 *    - xMessageBufferReceive() returns 0
 *    - Message tetap di buffer (tidak hilang)
 *    - Anda perlu call lagi dengan buffer lebih besar
 *    
 *    Best practice: Allocate buffer = max expected message size
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
#define MESSAGE_BUFFER_SIZE   256
#define MAX_MESSAGE_SIZE      80    /* Largest message we'll send */

#define TASK_STACK            256

#endif /* FREERTOS_CONFIG_H */
