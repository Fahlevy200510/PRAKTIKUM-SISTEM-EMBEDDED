/**
 * ============================================================================
 * FILE: FreeRTOSConfig.h
 * PROJECT: 43-Simple_CLI
 * 
 * DESKRIPSI:
 * Demonstrasi Command Line Interface (CLI) sederhana menggunakan FreeRTOS.
 * User dapat mengetik perintah via serial untuk mengontrol sistem.
 * 
 * ============================================================================
 * KONSEP CLI PADA EMBEDDED SYSTEM
 * ============================================================================
 * 
 * CLI memungkinkan interaksi dengan embedded system melalui terminal serial.
 * Berguna untuk debugging, konfigurasi, dan monitoring tanpa perlu IDE.
 * 
 * ARSITEKTUR CLI:
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                                                                     │
 *    │   UART RX                                                           │
 *    │   ┌───────────┐                                                     │
 *    │   │ h e l p   │  ← User mengetik "help"                            │
 *    │   │ \r        │    dan tekan Enter                                 │
 *    │   └─────┬─────┘                                                     │
 *    │         │                                                           │
 *    │         ▼                                                           │
 *    │   ┌───────────────────────────────────────────────────────────┐     │
 *    │   │                    RX TASK                                │     │
 *    │   │                                                           │     │
 *    │   │  • Terima karakter dari UART                             │     │
 *    │   │  • Bangun buffer command                                  │     │
 *    │   │  • Deteksi Enter (\r)                                     │     │
 *    │   │  • Kirim ke CLI task via Queue                            │     │
 *    │   └──────────────────────┬────────────────────────────────────┘     │
 *    │                          │                                          │
 *    │                          ▼                                          │
 *    │   ┌───────────────────────────────────────────────────────────┐     │
 *    │   │                    CLI TASK                               │     │
 *    │   │                                                           │     │
 *    │   │  • Parse command                                          │     │
 *    │   │  • Cari di command table                                  │     │
 *    │   │  • Execute handler                                        │     │
 *    │   │  • Return output via UART TX                              │     │
 *    │   └───────────────────────────────────────────────────────────┘     │
 *    │                                                                     │
 *    │   COMMAND TABLE:                                                    │
 *    │   ┌──────────┬────────────────────────────────────────────────┐     │
 *    │   │ Command  │ Handler                                       │     │
 *    │   ├──────────┼────────────────────────────────────────────────┤     │
 *    │   │ help     │ Show all commands                             │     │
 *    │   │ status   │ Show system status                            │     │
 *    │   │ led on   │ Turn LED on                                   │     │
 *    │   │ led off  │ Turn LED off                                  │     │
 *    │   │ tasks    │ List all tasks                                │     │
 *    │   │ heap     │ Show heap usage                               │     │
 *    │   │ reset    │ Software reset                                │     │
 *    │   └──────────┴────────────────────────────────────────────────┘     │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * CONTOH INTERAKSI
 * ============================================================================
 * 
 *    === SIMPLE CLI DEMO ===
 *    Type 'help' for available commands
 *    
 *    > help
 *    Available commands:
 *      help    - Show this help
 *      status  - Show system status
 *      led     - Control LED (led on/off)
 *      tasks   - List running tasks
 *      heap    - Show heap usage
 *      reset   - Software reset
 *    
 *    > led on
 *    LED turned ON
 *    
 *    > tasks
 *    Task Name       State   Prio    Stack
 *    CLI             R       2       150
 *    UART_RX         B       3       200
 *    IDLE            R       0       110
 *    
 *    > heap
 *    Heap free: 8452 bytes
 *    Min ever:  7234 bytes
 *    
 *    > unknown
 *    Unknown command: 'unknown'
 *    Type 'help' for available commands
 * 
 * ============================================================================
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* ============================================================================
 * STM32F103 SPECIFIC
 * ============================================================================ */
#define configENABLE_FPU                        0
#define configENABLE_MPU                        0

/* ============================================================================
 * CORE CONFIGURATION
 * ============================================================================ */
#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configUSE_TICKLESS_IDLE                 0
#define configCPU_CLOCK_HZ                      ((unsigned long)72000000)
#define configSYSTICK_CLOCK_HZ                  configCPU_CLOCK_HZ
#define configTICK_RATE_HZ                      ((TickType_t)1000)
#define configMAX_PRIORITIES                    5
#define configMINIMAL_STACK_SIZE                ((unsigned short)128)
#define configMAX_TASK_NAME_LEN                 16
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_TASK_NOTIFICATIONS            1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES   1
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configQUEUE_REGISTRY_SIZE               8
#define configUSE_QUEUE_SETS                    0
#define configUSE_TIME_SLICING                  1
#define configUSE_NEWLIB_REENTRANT              0
#define configENABLE_BACKWARD_COMPATIBILITY     0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 2
#define configSTACK_DEPTH_TYPE                  uint16_t
#define configMESSAGE_BUFFER_LENGTH_TYPE        size_t

/* ============================================================================
 * MEMORY ALLOCATION
 * ============================================================================ */
#define configSUPPORT_STATIC_ALLOCATION         0
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configTOTAL_HEAP_SIZE                   ((size_t)(10 * 1024))
#define configUSE_MALLOC_FAILED_HOOK            1
#define configCHECK_FOR_STACK_OVERFLOW          2

/* ============================================================================
 * HOOK FUNCTION CONFIGURATION
 * ============================================================================ */
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configUSE_DAEMON_TASK_STARTUP_HOOK      0

/* ============================================================================
 * RUNTIME STATS
 * ============================================================================ */
#define configGENERATE_RUN_TIME_STATS           0
#define configUSE_TRACE_FACILITY                1
#define configUSE_STATS_FORMATTING_FUNCTIONS    1

/* ============================================================================
 * CO-ROUTINES
 * ============================================================================ */
#define configUSE_CO_ROUTINES                   0
#define configMAX_CO_ROUTINE_PRIORITIES         2

/* ============================================================================
 * SOFTWARE TIMER
 * ============================================================================ */
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               3
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            configMINIMAL_STACK_SIZE

/* ============================================================================
 * INTERRUPT CONFIGURATION
 * ============================================================================ */
#ifdef __NVIC_PRIO_BITS
    #define configPRIO_BITS __NVIC_PRIO_BITS
#else
    #define configPRIO_BITS 4
#endif

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5
#define configKERNEL_INTERRUPT_PRIORITY         (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_API_CALL_INTERRUPT_PRIORITY   configMAX_SYSCALL_INTERRUPT_PRIORITY

/* ============================================================================
 * ASSERT CONFIGURATION
 * ============================================================================ */
#define configASSERT(x) if((x) == 0) { taskDISABLE_INTERRUPTS(); for(;;); }

/* ============================================================================
 * FreeRTOS API
 * ============================================================================ */
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
#define INCLUDE_xTaskGetIdleTaskHandle          1
#define INCLUDE_eTaskGetState                   1
#define INCLUDE_xEventGroupSetBitFromISR        1
#define INCLUDE_xTimerPendFunctionCall          1
#define INCLUDE_xTaskAbortDelay                 0
#define INCLUDE_xTaskGetHandle                  0
#define INCLUDE_xTaskResumeFromISR              1

/* ============================================================================
 * CORTEX-M3 HANDLERS
 * ============================================================================ */
#define xPortPendSVHandler                      PendSV_Handler
#define vPortSVCHandler                         SVC_Handler
#define xPortSysTickHandler                     SysTick_Handler

/* ============================================================================
 * APPLICATION DEFINES
 * ============================================================================ */
#define LED_GPIO_PORT                           GPIOC
#define LED_GPIO_PIN                            GPIO_PIN_13
#define DEBUG_UART_INSTANCE                     USART1
#define DEBUG_UART_BAUDRATE                     115200
#define DEBUG_UART_TX_PORT                      GPIOA
#define DEBUG_UART_TX_PIN                       GPIO_PIN_9
#define DEBUG_UART_RX_PORT                      GPIOA
#define DEBUG_UART_RX_PIN                       GPIO_PIN_10

#define TASK_STACK                              256

/* CLI configuration */
#define CLI_CMD_MAX_LEN                         64
#define CLI_PROMPT                              "> "

#endif /* FREERTOS_CONFIG_H */
