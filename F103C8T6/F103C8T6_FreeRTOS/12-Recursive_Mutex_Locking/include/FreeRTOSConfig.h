/**
 * ============================================================================
 * FILE: FreeRTOSConfig.h
 * PROJECT: 12-Recursive_Mutex_Locking
 * 
 * DESKRIPSI:
 * Konfigurasi FreeRTOS untuk demo Recursive Mutex.
 * 
 * APA ITU RECURSIVE MUTEX?
 * ========================
 * Recursive mutex adalah mutex yang BOLEH diambil berkali-kali oleh TASK
 * YANG SAMA tanpa menyebabkan deadlock.
 * 
 * MASALAH DENGAN MUTEX BIASA:
 * ===========================
 * 
 *    Task A ambil mutex  ──►  Task A panggil fungsi_x()
 *                             │
 *                             ▼
 *                         fungsi_x() coba ambil mutex lagi
 *                             │
 *                             ▼
 *                         ╔═══════════════════════╗
 *                         ║   DEADLOCK!           ║
 *                         ║   Task A menunggu     ║
 *                         ║   mutex yang sudah    ║
 *                         ║   dia pegang sendiri  ║
 *                         ╚═══════════════════════╝
 * 
 * SOLUSI DENGAN RECURSIVE MUTEX:
 * ==============================
 * 
 *    Task A ambil mutex (count=1)  ──►  Task A panggil fungsi_x()
 *                                       │
 *                                       ▼
 *                                   fungsi_x() ambil mutex lagi
 *                                   (count=2) ✓ BERHASIL!
 *                                       │
 *                                       ▼
 *                                   fungsi_x() lepas mutex
 *                                   (count=1)
 *                                       │
 *                                       ▼
 *                                   Task A lepas mutex
 *                                   (count=0) - mutex bebas
 * 
 * ATURAN PENTING:
 * ===============
 * 1. Setiap xSemaphoreTakeRecursive() HARUS dipasangkan dengan
 *    xSemaphoreGiveRecursive()
 * 2. Jumlah Take == Jumlah Give agar mutex benar-benar terlepas
 * 3. Hanya task yang mengambil mutex yang boleh melepaskannya
 * 
 * ============================================================================
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "stm32f1xx.h"

/* ==========================================================================
 * KONFIGURASI KERNEL DASAR
 * ========================================================================== */
#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
#define configUSE_TICKLESS_IDLE                 0
#define configCPU_CLOCK_HZ                      (SystemCoreClock)
#define configTICK_RATE_HZ                      ((TickType_t)1000)
#define configMAX_PRIORITIES                    8
#define configMINIMAL_STACK_SIZE                ((uint16_t)128)
#define configTOTAL_HEAP_SIZE                   ((size_t)(8 * 1024))
#define configMAX_TASK_NAME_LEN                 16
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1

/* ==========================================================================
 * FITUR SINKRONISASI - AKTIFKAN RECURSIVE MUTEX
 * ========================================================================== */
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1   /* WAJIB 1 untuk project ini! */
#define configUSE_COUNTING_SEMAPHORES           0

/* ==========================================================================
 * HOOK FUNCTIONS
 * ========================================================================== */
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configUSE_MALLOC_FAILED_HOOK            1
#define configCHECK_FOR_STACK_OVERFLOW          2

/* ==========================================================================
 * FITUR DEBUG & TRACE
 * ========================================================================== */
#define configGENERATE_RUN_TIME_STATS           0
#define configUSE_TRACE_FACILITY                0
#define configUSE_STATS_FORMATTING_FUNCTIONS    0

/* ==========================================================================
 * CO-ROUTINES (tidak dipakai)
 * ========================================================================== */
#define configUSE_CO_ROUTINES                   0
#define configMAX_CO_ROUTINE_PRIORITIES         2

/* ==========================================================================
 * SOFTWARE TIMER
 * ========================================================================== */
#define configUSE_TIMERS                        0
#define configTIMER_TASK_PRIORITY               3
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            configMINIMAL_STACK_SIZE

/* ==========================================================================
 * KONFIGURASI INTERRUPT
 * ========================================================================== */
#ifdef __NVIC_PRIO_BITS
    #define configPRIO_BITS                     __NVIC_PRIO_BITS
#else
    #define configPRIO_BITS                     4
#endif
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5
#define configKERNEL_INTERRUPT_PRIORITY         (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/* ==========================================================================
 * HANDLER MAPPING
 * ========================================================================== */
#define xPortPendSVHandler                      PendSV_Handler
#define vPortSVCHandler                         SVC_Handler

/* ==========================================================================
 * OPTIONAL FUNCTIONS
 * ========================================================================== */
#define INCLUDE_vTaskPrioritySet                0
#define INCLUDE_uxTaskPriorityGet               0
#define INCLUDE_vTaskDelete                     0
#define INCLUDE_vTaskSuspend                    0
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

/* ==========================================================================
 * KONFIGURASI HARDWARE
 * ========================================================================== */
#define LED_GPIO_PORT                           GPIOC
#define LED_GPIO_PIN                            GPIO_PIN_13
#define LED_ACTIVE_LOW                          1

#define DEBUG_UART_INSTANCE                     USART1
#define DEBUG_UART_BAUDRATE                     115200
#define DEBUG_UART_TX_PORT                      GPIOA
#define DEBUG_UART_TX_PIN                       GPIO_PIN_9
#define DEBUG_UART_RX_PORT                      GPIOA
#define DEBUG_UART_RX_PIN                       GPIO_PIN_10

/* ==========================================================================
 * PARAMETER RECURSIVE MUTEX (dapat diubah sesuai kebutuhan)
 * ========================================================================== */
#define WORKER_STACK                            256
#define WORKER_PRIO                             (tskIDLE_PRIORITY + 2)
#define WORKER_PERIOD_MS                        500

#define LOGGER_STACK                            256
#define LOGGER_PRIO                             (tskIDLE_PRIORITY + 1)
#define LOGGER_PERIOD_MS                        1000

#define MAX_RECURSION_DEPTH                     3   /* Kedalaman rekursi demo */

#endif /* FREERTOS_CONFIG_H */
