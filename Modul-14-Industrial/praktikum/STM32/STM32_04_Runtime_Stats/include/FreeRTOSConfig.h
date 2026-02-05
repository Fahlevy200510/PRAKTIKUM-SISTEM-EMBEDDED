/**
 * ============================================================================
 * FILE: FreeRTOSConfig.h
 * PROJECT: 42-Runtime_Stats
 * 
 * DESKRIPSI:
 * Demonstrasi runtime statistics untuk mengukur CPU time yang digunakan
 * oleh setiap task. Berguna untuk profiling dan optimasi performa.
 * 
 * ============================================================================
 * KONSEP RUNTIME STATISTICS
 * ============================================================================
 * 
 * Runtime Stats mengukur berapa banyak waktu CPU yang digunakan setiap task.
 * Ini memerlukan high-resolution timer (lebih cepat dari SysTick).
 * 
 * CARA KERJA:
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                    RUNTIME STATS MECHANISM                          │
 *    │                                                                     │
 *    │   High-Resolution Timer (e.g., TIM2 @ 10x tick rate)               │
 *    │   ┌───────────────────────────────────────────────────────────────┐ │
 *    │   │ ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓ │ │
 *    │   │ 0                                                     10000   │ │
 *    │   └───────────────────────────────────────────────────────────────┘ │
 *    │                                                                     │
 *    │   Pada setiap context switch:                                      │
 *    │   • Baca timer counter                                             │
 *    │   • Hitung selisih dari context switch sebelumnya                  │
 *    │   • Tambahkan ke total runtime task yang keluar                    │
 *    │                                                                     │
 *    │   Task A       Task B       Task C       Idle                      │
 *    │   ┌────────┐  ┌────────┐  ┌────────┐  ┌────────┐                   │
 *    │   │ 450    │  │ 200    │  │ 150    │  │ 200    │  = 1000 counts    │
 *    │   │  45%   │  │  20%   │  │  15%   │  │  20%   │  = 100%           │
 *    │   └────────┘  └────────┘  └────────┘  └────────┘                   │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * KONFIGURASI YANG DIPERLUKAN:
 * 
 *    1. configGENERATE_RUN_TIME_STATS = 1
 *    2. portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() - init timer
 *    3. portGET_RUN_TIME_COUNTER_VALUE() - baca counter
 * 
 * ============================================================================
 * OUTPUT FORMAT
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                                                                     │
 *    │   vTaskGetRunTimeStats() output:                                   │
 *    │                                                                     │
 *    │   Task Name       Abs Time      % Time                              │
 *    │   ─────────────────────────────────────                            │
 *    │   TaskA           1250000       45%                                │
 *    │   TaskB           550000        20%                                │
 *    │   TaskC           412500        15%                                │
 *    │   IDLE            550000        20%                                │
 *    │   ─────────────────────────────────────                            │
 *    │   Total           2762500       100%                               │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * CONTOH OUTPUT SERIAL
 * ============================================================================
 * 
 *    === RUNTIME STATS DEMO ===
 *    
 *    [CPU Task] Running CPU-intensive work...
 *    [Light Task] Light periodic work...
 *    [IO Task] Simulated I/O operations...
 *    
 *    ┌─── RUNTIME STATISTICS ─────────────────────────────────────────────┐
 *    │  Task Name          Abs Time       % Time                         │
 *    │  ─────────────────────────────────────────                        │
 *    │  CPUIntensive       1250034        42%                            │
 *    │  LightTask          298123         10%                            │
 *    │  IOTask             446234         15%                            │
 *    │  Monitor            178234         6%                             │
 *    │  IDLE               805234         27%                            │
 *    └───────────────────────────────────────────────────────────────────┘
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
 * RUNTIME STATS - KEY CONFIGURATION
 * ============================================================================ */
#define configGENERATE_RUN_TIME_STATS           1
#define configUSE_TRACE_FACILITY                1
#define configUSE_STATS_FORMATTING_FUNCTIONS    1

/* 
 * Timer macros for runtime stats
 * Menggunakan software counter yang di-increment di tick hook
 * (Lebih sederhana dari hardware timer untuk demo)
 */
extern volatile uint32_t ulHighFrequencyTimerTicks;
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()    (ulHighFrequencyTimerTicks = 0UL)
#define portGET_RUN_TIME_COUNTER_VALUE()            ulHighFrequencyTimerTicks

/* ============================================================================
 * HOOK FUNCTION CONFIGURATION
 * ============================================================================ */
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     1  /* Enable untuk runtime counter */
#define configUSE_DAEMON_TASK_STARTUP_HOOK      0

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
#define STATS_BUFFER_SIZE                       512

#endif /* FREERTOS_CONFIG_H */
