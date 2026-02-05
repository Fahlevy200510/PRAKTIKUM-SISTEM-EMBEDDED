/**
 * ============================================================================
 * FILE: FreeRTOSConfig.h
 * PROJECT: 39-Tickless_Idle
 * 
 * DESKRIPSI:
 * Demonstrasi Tickless Idle mode untuk penghematan daya. Saat tidak ada task
 * yang siap jalan, MCU akan masuk sleep mode tanpa tick interrupt.
 * 
 * ============================================================================
 * KONSEP TICKLESS IDLE
 * ============================================================================
 * 
 * Normal tick interrupt: MCU bangun setiap 1ms meskipun tidak ada pekerjaan
 * Tickless idle: MCU tidur lebih lama, bangun hanya saat diperlukan
 * 
 * PERBANDINGAN NORMAL vs TICKLESS:
 * 
 *    NORMAL TICK MODE:
 *    ─────────────────────────────────────────────────────────────────────────
 *    
 *    CPU Power │    ▄▄    ▄▄    ▄▄    ▄▄    ▄▄    ▄▄    ▄▄    ▄▄    ▄▄
 *              │  ▄▄  ▄▄▄▄  ▄▄▄▄  ▄▄▄▄  ▄▄▄▄  ▄▄▄▄  ▄▄▄▄  ▄▄▄▄  ▄▄▄▄  ▄▄
 *              │▄▄  ▄▄    ▄▄    ▄▄    ▄▄    ▄▄    ▄▄    ▄▄    ▄▄    ▄▄
 *              └───┬────┬────┬────┬────┬────┬────┬────┬────┬────────────▶ time
 *                  │    │    │    │    │    │    │    │    │
 *              Tick  Tick  Tick  Tick  Tick  Tick  Tick  Tick  Tick
 *              ISR   ISR   ISR   ISR   ISR   ISR   ISR   ISR   ISR
 *    
 *    CPU wakes up EVERY tick even when idle = WASTE POWER!
 * 
 *    ─────────────────────────────────────────────────────────────────────────
 *    
 *    TICKLESS IDLE MODE:
 *    ─────────────────────────────────────────────────────────────────────────
 *    
 *    CPU Power │           ▄▄▄▄                              ▄▄▄▄
 *              │  ▄▄▄▄▄▄▄▄▄    ▄▄▄▄▄                     ▄▄▄▄    ▄▄▄▄
 *              │▄▄                  ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄              ▄▄
 *              └──┬────────────────┬───────────────────────┬───────────▶ time
 *                 │                │                       │
 *              Task 1           Task 2                  Task 3
 *              runs             runs                    runs
 *    
 *              ←────────────────────────────────────────────→
 *                  CPU sleeps during idle, wakes only when needed
 *    
 *    ─────────────────────────────────────────────────────────────────────────
 * 
 * MEKANISME TICKLESS:
 * 
 *    ┌───────────────────────────────────────────────────────────────────────┐
 *    │                                                                       │
 *    │  1. Scheduler detects: semua task blocked/delayed                    │
 *    │                     ↓                                                 │
 *    │  2. Calculate: berapa lama sampai task berikutnya ready?             │
 *    │                     ↓                                                 │
 *    │  3. Program low-power timer untuk bangun tepat waktu                 │
 *    │                     ↓                                                 │
 *    │  4. Disable SysTick interrupt                                        │
 *    │                     ↓                                                 │
 *    │  5. Enter sleep mode (WFI instruction)                               │
 *    │                     ↓                                                 │
 *    │  6. Wake up by: timer atau external interrupt                        │
 *    │                     ↓                                                 │
 *    │  7. Calculate: berapa lama kita tidur?                               │
 *    │                     ↓                                                 │
 *    │  8. Update tick count dengan waktu tidur                             │
 *    │                     ↓                                                 │
 *    │  9. Re-enable SysTick dan resume normal                              │
 *    │                                                                       │
 *    └───────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * KEUNTUNGAN DAN KERUGIAN
 * ============================================================================
 * 
 * KEUNTUNGAN:
 * ✓ Hemat daya signifikan untuk battery-powered devices
 * ✓ Mengurangi EMI dari tick interrupt
 * ✓ Extend battery life hingga 10x atau lebih
 * 
 * KERUGIAN:
 * ✗ Timing menjadi kurang presisi
 * ✗ Wake-up latency sedikit lebih lama
 * ✗ Perlu konfigurasi low-power timer
 * ✗ Debugging lebih sulit
 * 
 * ============================================================================
 * CONTOH OUTPUT SERIAL
 * ============================================================================
 * 
 *    === TICKLESS IDLE DEMO ===
 *    
 *    [Task] Work done, sleeping 5000ms
 *    [Idle] Entering tickless sleep...
 *    [Idle] Expected sleep: 4985 ticks
 *    
 *    ... (MCU dalam sleep mode, hemat daya) ...
 *    
 *    [Idle] Woke up after 4985 ticks
 *    [Task] Woke up at tick 10234
 *    [Task] Work done, sleeping 5000ms
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

/* 
 * TICKLESS IDLE MODE ENABLE
 * Nilai 1 = enable low power tickless mode
 * Nilai 0 = normal tick mode (default)
 * 
 * Saat enabled, FreeRTOS akan memanggil:
 * - portSUPPRESS_TICKS_AND_SLEEP() saat masuk idle
 * - configPRE_SLEEP_PROCESSING() sebelum sleep
 * - configPOST_SLEEP_PROCESSING() setelah wake up
 */
#define configUSE_TICKLESS_IDLE                 1

/* 
 * Minimum idle time (in ticks) before entering low power mode
 * Jika idle time < ini, tidak masuk tickless mode
 * Default is 2 ticks
 */
#define configEXPECTED_IDLE_TIME_BEFORE_SLEEP   2

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
 * HOOK FUNCTIONS
 * ============================================================================ */
#define configUSE_IDLE_HOOK                     1  /* Enable untuk tickless demo */
#define configUSE_TICK_HOOK                     0
#define configUSE_DAEMON_TASK_STARTUP_HOOK      0

/* ============================================================================
 * TICKLESS IDLE HOOKS (Optional)
 * ============================================================================ */
/* Dipanggil sebelum masuk sleep mode */
extern void vApplicationPreSleepProcessing(uint32_t ulExpectedIdleTime);
#define configPRE_SLEEP_PROCESSING(x)   vApplicationPreSleepProcessing(x)

/* Dipanggil setelah bangun dari sleep */
extern void vApplicationPostSleepProcessing(uint32_t ulExpectedIdleTime);
#define configPOST_SLEEP_PROCESSING(x)  vApplicationPostSleepProcessing(x)

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

#endif /* FREERTOS_CONFIG_H */
