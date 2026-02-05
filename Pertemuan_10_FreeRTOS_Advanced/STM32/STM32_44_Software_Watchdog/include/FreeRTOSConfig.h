/**
 * ============================================================================
 * FILE: FreeRTOSConfig.h
 * PROJECT: 20-Software_Watchdog
 * 
 * DESKRIPSI:
 * Implementasi Software Watchdog menggunakan FreeRTOS Timers.
 * Memonitor task-task penting dan me-reset sistem jika ada yang hang.
 * 
 * KONSEP SOFTWARE WATCHDOG:
 * =========================
 * 
 *    Hardware Watchdog (WDG):
 *    ┌──────────────────────────────────────────────────────────┐
 *    │ Counter yang harus di-refresh sebelum timeout           │
 *    │ Jika tidak di-refresh → MCU RESET                       │
 *    │ Biasanya hanya 1 watchdog untuk seluruh sistem          │
 *    └──────────────────────────────────────────────────────────┘
 *    
 *    Software Watchdog (per-task):
 *    ┌──────────────────────────────────────────────────────────┐
 *    │ Timer software yang memantau SETIAP task secara mandiri │
 *    │ Jika salah satu task tidak "check in" → action handler  │
 *    │ Bisa me-restart hanya task yang bermasalah              │
 *    │ Atau memicu hardware reset jika critical                │
 *    └──────────────────────────────────────────────────────────┘
 * 
 * ILUSTRASI:
 * ═════════════════════════════════════════════════════════════════════
 *    
 *    ┌─────────────┐  ┌─────────────┐  ┌─────────────┐
 *    │   Task A    │  │   Task B    │  │   Task C    │
 *    │   Normal    │  │   Normal    │  │  ★ HANG ★   │
 *    └──────┬──────┘  └──────┬──────┘  └──────┬──────┘
 *           │                │                │
 *           │ "check in"     │ "check in"     │ (tidak check in!)
 *           ▼                ▼                ▼
 *    ┌──────────────────────────────────────────────────────────┐
 *    │                  SOFTWARE WATCHDOG TIMER                 │
 *    │                                                          │
 *    │   ┌─────────┐     ┌─────────┐     ┌─────────┐            │
 *    │   │Task A   │     │Task B   │     │Task C   │            │
 *    │   │Count: ✓ │     │Count: ✓ │     │Count: ✗ │ TIMEOUT!  │
 *    │   └─────────┘     └─────────┘     └─────────┘            │
 *    │                                                          │
 *    │   Action: Log error, restart Task C, atau system reset  │
 *    └──────────────────────────────────────────────────────────┘
 * 
 * ═════════════════════════════════════════════════════════════════════
 * 
 * MEKANISME CHECK-IN:
 * ===================
 * 
 *    Normal Flow:
 *    ────────────────────────────────────────────────────
 *    
 *    Task A: ████████░░░░████████░░░░████████░░░░████████
 *                   │           │           │
 *                   └──►WDG_CheckIn()      └──►WDG_CheckIn()
 *    
 *    WDG Timer:     ────╫─────────────╫─────────────╫────
 *                       │ Check       │ Check       │ Check
 *                       │ A: OK       │ A: OK       │ A: OK
 *    
 *    
 *    Hang/Deadlock Scenario:
 *    ────────────────────────────────────────────────────
 *    
 *    Task A: ████████████████████████░░░░░░░░░░░░░░░░░░░
 *                                   ▲ STUCK di blocking call
 *                                   │ atau infinite loop
 *    
 *    WDG Timer:     ────╫─────────────╫──────► TIMEOUT!
 *                       │ Check       │ Check
 *                       │ A: OK       │ A: FAIL!
 *                                     │
 *                                     └──► Trigger recovery
 * 
 * ============================================================================
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "stm32f1xx.h"

#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
#define configUSE_TICKLESS_IDLE                 0
#define configCPU_CLOCK_HZ                      (SystemCoreClock)
#define configTICK_RATE_HZ                      ((TickType_t)1000)
#define configMAX_PRIORITIES                    8
#define configMINIMAL_STACK_SIZE                ((uint16_t)128)
#define configTOTAL_HEAP_SIZE                   ((size_t)(10 * 1024))
#define configMAX_TASK_NAME_LEN                 16
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1

#define configUSE_TASK_NOTIFICATIONS            1

#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             0
#define configUSE_COUNTING_SEMAPHORES           0

#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configUSE_MALLOC_FAILED_HOOK            1
#define configCHECK_FOR_STACK_OVERFLOW          2

#define configGENERATE_RUN_TIME_STATS           0
#define configUSE_TRACE_FACILITY                0
#define configUSE_STATS_FORMATTING_FUNCTIONS    0

#define configUSE_CO_ROUTINES                   0
#define configMAX_CO_ROUTINE_PRIORITIES         2

/* Timer Service Task - REQUIRED untuk Software Watchdog */
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               (configMAX_PRIORITIES - 1)  /* Highest priority */
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            (configMINIMAL_STACK_SIZE * 2)

#ifdef __NVIC_PRIO_BITS
    #define configPRIO_BITS                     __NVIC_PRIO_BITS
#else
    #define configPRIO_BITS                     4
#endif
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5
#define configKERNEL_INTERRUPT_PRIORITY         (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

#define xPortPendSVHandler                      PendSV_Handler
#define vPortSVCHandler                         SVC_Handler

#define INCLUDE_vTaskPrioritySet                0
#define INCLUDE_uxTaskPriorityGet               0
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_xResumeFromISR                  0
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_uxTaskGetStackHighWaterMark     1
#define INCLUDE_xTaskGetIdleTaskHandle          0
#define INCLUDE_eTaskGetState                   1
#define INCLUDE_xEventGroupSetBitFromISR        0
#define INCLUDE_xTimerPendFunctionCall          0
#define INCLUDE_xTaskAbortDelay                 0
#define INCLUDE_xTaskGetHandle                  0
#define INCLUDE_xTaskResumeFromISR              0

#define LED_GPIO_PORT                           GPIOC
#define LED_GPIO_PIN                            GPIO_PIN_13
#define LED_ACTIVE_LOW                          1

#define DEBUG_UART_INSTANCE                     USART1
#define DEBUG_UART_BAUDRATE                     115200
#define DEBUG_UART_TX_PORT                      GPIOA
#define DEBUG_UART_TX_PIN                       GPIO_PIN_9
#define DEBUG_UART_RX_PORT                      GPIOA
#define DEBUG_UART_RX_PIN                       GPIO_PIN_10

/* ============================================================================
 * SOFTWARE WATCHDOG CONFIGURATION
 * ============================================================================ */

/* Jumlah maksimal task yang bisa dimonitor */
#define WDG_MAX_MONITORED_TASKS                 4

/* Periode watchdog timer check (ms) */
#define WDG_CHECK_PERIOD_MS                     1000

/* Default timeout per task (ms) - task harus check in sebelum ini */
#define WDG_DEFAULT_TIMEOUT_MS                  3000

/* Task stack sizes */
#define WORKER_TASK_STACK                       256
#define WORKER_TASK_PRIO                        (tskIDLE_PRIORITY + 2)

#endif /* FREERTOS_CONFIG_H */
