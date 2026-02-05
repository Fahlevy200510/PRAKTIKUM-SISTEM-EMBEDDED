/**
 * ============================================================================
 * FILE: FreeRTOSConfig.h
 * PROJECT: 35-Stack_Overflow_Detection
 * 
 * JUDUL: Stack Overflow Detection Methods
 * 
 * ============================================================================
 * KONSEP STACK OVERFLOW DETECTION
 * ============================================================================
 * 
 * FreeRTOS menyediakan 2 metode deteksi stack overflow:
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                    STACK MEMORY LAYOUT                              │
 *    │                                                                     │
 *    │   ┌───────────────────────────────────────────────────────────────┐│
 *    │   │                         TASK STACK                            ││
 *    │   │                                                               ││
 *    │   │   Low Address                              High Address       ││
 *    │   │   ┌─────────┬─────────────────────────────────────────┐       ││
 *    │   │   │SENTINEL │        Stack Space          │ SP ↓      │       ││
 *    │   │   └─────────┴─────────────────────────────────────────┘       ││
 *    │   │   ↑                                       ↑                   ││
 *    │   │   Stack Bottom                     Stack Top (initial SP)     ││
 *    │   │   (overflow zone)                                             ││
 *    │   │                                                               ││
 *    │   └───────────────────────────────────────────────────────────────┘│
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * METHOD 1 (configCHECK_FOR_STACK_OVERFLOW = 1)
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │  Check at context switch:                                          │
 *    │                                                                     │
 *    │  - Verifies SP is within stack bounds                              │
 *    │  - Fast, low overhead                                              │
 *    │  - May miss overflow between context switches                      │
 *    │                                                                     │
 *    │  ┌──────────────────────────────────────────┐                      │
 *    │  │            STACK                          │                      │
 *    │  │  ┌─────┬──────────────────────┬─────┐    │                      │
 *    │  │  │     │░░░░░░░░░░░░░░░░░░░░░░│ SP  │    │                      │
 *    │  │  └─────┴──────────────────────┴─────┘    │                      │
 *    │  │        ↑                       ↑         │                      │
 *    │  │        Bottom              Current SP    │                      │
 *    │  │                                          │                      │
 *    │  │  CHECK: Is SP > Bottom?                  │                      │
 *    │  └──────────────────────────────────────────┘                      │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * METHOD 2 (configCHECK_FOR_STACK_OVERFLOW = 2)
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │  Watermark pattern check:                                          │
 *    │                                                                     │
 *    │  - First 20 bytes filled with 0xA5A5A5A5 pattern                   │
 *    │  - Check if pattern corrupted at context switch                    │
 *    │  - More reliable detection                                         │
 *    │  - Slightly higher overhead                                        │
 *    │                                                                     │
 *    │  ┌──────────────────────────────────────────┐                      │
 *    │  │            STACK                          │                      │
 *    │  │  ┌─────────────┬──────────────────────┐  │                      │
 *    │  │  │A5 A5 A5 A5 A5│░░░░░░░░░░░░░░░░░░░░│  │                      │
 *    │  │  └─────────────┴──────────────────────┘  │                      │
 *    │  │  │← 20 bytes →│                          │                      │
 *    │  │                                          │                      │
 *    │  │  CHECK: Are first 20 bytes still 0xA5?   │                      │
 *    │  └──────────────────────────────────────────┘                      │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * HIGH WATER MARK
 * ============================================================================
 * 
 *    uxTaskGetStackHighWaterMark():
 *      - Returns MINIMUM free stack ever (in words)
 *      - Lower = closer to overflow
 *      - Use during development to size stacks appropriately
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

/* IMPORTANT: Stack overflow detection method 2 (watermark pattern) */
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

/* Stack sizes for testing */
#define SAFE_TASK_STACK       256   /* Normal size */
#define TIGHT_TASK_STACK      128   /* Minimal size */

#endif /* FREERTOS_CONFIG_H */
