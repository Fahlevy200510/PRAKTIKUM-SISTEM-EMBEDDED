/**
 * ============================================================================
 * FILE: FreeRTOSConfig.h
 * PROJECT: 41-Graceful_Shutdown
 * 
 * DESKRIPSI:
 * Demonstrasi graceful shutdown pattern untuk mematikan sistem dengan aman.
 * Semua task diberi kesempatan untuk cleanup sebelum sistem shutdown.
 * 
 * ============================================================================
 * KONSEP GRACEFUL SHUTDOWN
 * ============================================================================
 * 
 * Graceful Shutdown memastikan sistem berhenti dengan teratur:
 * - Semua task selesai operasi yang sedang berjalan
 * - Data tersimpan dengan aman
 * - Peripheral dimatikan dengan benar
 * - Tidak ada resource yang corrupt
 * 
 * PERBANDINGAN SHUTDOWN:
 * 
 *    HARD SHUTDOWN (Berbahaya):
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                                                                     │
 *    │   Task A: [######### Writing to EEPROM ####                         │
 *    │   Task B: [######## Transmitting data ###                           │
 *    │   Task C: [######### Motor control ####                             │
 *    │                                    ↑                                │
 *    │                               POWER OFF!                            │
 *    │                                    │                                │
 *    │   ⚠️ EEPROM corrupted!           │                                 │
 *    │   ⚠️ Data loss!                  │                                 │
 *    │   ⚠️ Motor stuck in bad state!   │                                 │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 *    GRACEFUL SHUTDOWN (Aman):
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                                                                     │
 *    │   1. Shutdown request diterima                                      │
 *    │      ↓                                                              │
 *    │   2. Broadcast notification ke semua task                           │
 *    │      ↓                                                              │
 *    │   Task A: "Finishing EEPROM write... Done ✓"                        │
 *    │   Task B: "Completing transmission... Done ✓"                       │
 *    │   Task C: "Motor to safe position... Done ✓"                        │
 *    │      ↓                                                              │
 *    │   3. Semua task konfirmasi shutdown                                 │
 *    │      ↓                                                              │
 *    │   4. Peripheral cleanup                                             │
 *    │      ↓                                                              │
 *    │   5. Safe power off ✓                                               │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * MEKANISME DENGAN FreeRTOS:
 * 
 *    ┌──────────────────────────────────────────────────────────────────────┐
 *    │                                                                      │
 *    │   Shutdown                              Worker Tasks                 │
 *    │   Manager Task                                                       │
 *    │   ┌────────────┐                        ┌────────────┐              │
 *    │   │ Detect     │   Notification         │   Task A   │              │
 *    │   │ shutdown   │──────────────────────▶ │  cleanup() │              │
 *    │   │ trigger    │                        └─────┬──────┘              │
 *    │   └─────┬──────┘                              │                     │
 *    │         │                               ┌─────▼──────┐              │
 *    │         │                               │   Task B   │              │
 *    │         │                               │  cleanup() │              │
 *    │         │                               └─────┬──────┘              │
 *    │         ▼                                     │                     │
 *    │   ┌────────────┐                        ┌─────▼──────┐              │
 *    │   │ Wait for   │◀─── ACK ───────────────│   Task C   │              │
 *    │   │ all ACKs   │                        │  cleanup() │              │
 *    │   └─────┬──────┘                        └────────────┘              │
 *    │         │                                                           │
 *    │         ▼                                                           │
 *    │   ┌────────────┐                                                    │
 *    │   │ Final      │                                                    │
 *    │   │ cleanup    │                                                    │
 *    │   │ & halt     │                                                    │
 *    │   └────────────┘                                                    │
 *    │                                                                      │
 *    └──────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * CONTOH OUTPUT SERIAL
 * ============================================================================
 * 
 *    === GRACEFUL SHUTDOWN DEMO ===
 *    
 *    [Task A] Running normally...
 *    [Task B] Running normally...
 *    [Task C] Running normally...
 *    
 *    [Shutdown] Shutdown request received!
 *    [Shutdown] Notifying all tasks...
 *    
 *    [Task A] Shutdown notification received
 *    [Task A] Completing current operation... Done
 *    [Task A] Cleanup complete, sending ACK
 *    
 *    [Task B] Shutdown notification received
 *    [Task B] Saving data... Done
 *    [Task B] Cleanup complete, sending ACK
 *    
 *    [Task C] Shutdown notification received
 *    [Task C] Motor to safe position... Done
 *    [Task C] Cleanup complete, sending ACK
 *    
 *    [Shutdown] All tasks acknowledged
 *    [Shutdown] Disabling peripherals...
 *    [Shutdown] System halted safely
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
#define WORKER_TASK_COUNT                       3

/* Notification bits for shutdown */
#define NOTIFY_SHUTDOWN                         (1UL << 0)

#endif /* FREERTOS_CONFIG_H */
