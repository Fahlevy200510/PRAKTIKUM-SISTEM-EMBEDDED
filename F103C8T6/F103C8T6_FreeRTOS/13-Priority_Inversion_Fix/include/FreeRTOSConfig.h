/**
 * ============================================================================
 * FILE: FreeRTOSConfig.h
 * PROJECT: 13-Priority_Inversion_Fix
 * 
 * DESKRIPSI:
 * Konfigurasi FreeRTOS untuk demo Priority Inversion dan solusinya
 * menggunakan Priority Inheritance (bawaan mutex FreeRTOS).
 * 
 * APA ITU PRIORITY INVERSION?
 * ===========================
 * Priority inversion terjadi ketika task prioritas TINGGI ter-block oleh
 * task prioritas RENDAH karena mutex.
 * 
 * SKENARIO PRIORITY INVERSION:
 * ============================
 * 
 *    Prioritas
 *       ▲
 *       │   ┌─────────────────────────────────────────────────────────┐
 *    H  │   │ Task H                                                  │
 *       │   │ ████████░░░░░░░░░░░░░░░░░░░░░░░░░░████████████████████  │
 *       │   │ Running  Blocked(mutex)           Running               │
 *       │   └─────────────────────────────────────────────────────────┘
 *       │
 *       │   ┌─────────────────────────────────────────────────────────┐
 *    M  │   │ Task M                                                  │
 *       │   │         ████████████████████████████                    │
 *       │   │         Preempts Task L! (Masalah!)                     │
 *       │   └─────────────────────────────────────────────────────────┘
 *       │
 *       │   ┌─────────────────────────────────────────────────────────┐
 *    L  │   │ Task L (punya mutex)                                    │
 *       │   │ ████████                        ████                    │
 *       │   │ Holding  Preempted by M          Release mutex          │
 *       │   └─────────────────────────────────────────────────────────┘
 *       │
 *       └────────────────────────────────────────────────────────────►
 *                                   Waktu
 * 
 * MASALAH:
 * - Task H menunggu mutex yang dipegang Task L
 * - Task M (tidak butuh mutex) preempt Task L
 * - Task H TERBLOKIR oleh Task M meskipun H > M
 * - Ini disebut "unbounded priority inversion"
 * 
 * SOLUSI: PRIORITY INHERITANCE
 * ============================
 * 
 * Ketika Task H menunggu mutex dari Task L:
 * - FreeRTOS OTOMATIS menaikkan prioritas Task L = prioritas Task H
 * - Task L tidak bisa di-preempt oleh Task M
 * - Task L selesai, lepas mutex, prioritas kembali normal
 * - Task H langsung jalan
 * 
 *    Prioritas
 *       ▲
 *       │   ┌─────────────────────────────────────────────────────────┐
 *    H  │   │ Task H           Task L (inherited)  Task H             │
 *       │   │ ████████░░░░░░░░░████████████████████████████████████   │
 *       │   └─────────────────────────────────────────────────────────┘
 *       │
 *       │   ┌─────────────────────────────────────────────────────────┐
 *    M  │   │ Task M (tidak bisa preempt L yang sudah "naik pangkat") │
 *       │   │ ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░████████████████│
 *       │   └─────────────────────────────────────────────────────────┘
 *       │
 *       │   ┌─────────────────────────────────────────────────────────┐
 *    L  │   │ Task L                                                  │
 *       │   │ ████████                                       Idle     │
 *       │   └─────────────────────────────────────────────────────────┘
 *       └────────────────────────────────────────────────────────────►
 * 
 * CATATAN: FreeRTOS mutex OTOMATIS menggunakan priority inheritance!
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
#define configTOTAL_HEAP_SIZE                   ((size_t)(8 * 1024))
#define configMAX_TASK_NAME_LEN                 16
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1

/* MUTEX WAJIB AKTIF untuk priority inheritance */
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

#define configUSE_TIMERS                        0
#define configTIMER_TASK_PRIORITY               3
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            configMINIMAL_STACK_SIZE

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

#define INCLUDE_vTaskPrioritySet                1   /* Dibutuhkan untuk demo */
#define INCLUDE_uxTaskPriorityGet               1   /* Untuk cek prioritas */
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

/* Hardware */
#define LED_GPIO_PORT                           GPIOC
#define LED_GPIO_PIN                            GPIO_PIN_13
#define LED_ACTIVE_LOW                          1

#define DEBUG_UART_INSTANCE                     USART1
#define DEBUG_UART_BAUDRATE                     115200
#define DEBUG_UART_TX_PORT                      GPIOA
#define DEBUG_UART_TX_PIN                       GPIO_PIN_9
#define DEBUG_UART_RX_PORT                      GPIOA
#define DEBUG_UART_RX_PIN                       GPIO_PIN_10

/* Parameter Priority Inversion Demo */
#define TASK_LOW_PRIO                           (tskIDLE_PRIORITY + 1)
#define TASK_MED_PRIO                           (tskIDLE_PRIORITY + 2)
#define TASK_HIGH_PRIO                          (tskIDLE_PRIORITY + 3)

#define TASK_LOW_STACK                          256
#define TASK_MED_STACK                          256
#define TASK_HIGH_STACK                         256

#define RESOURCE_HOLD_TIME_MS                   200  /* Waktu Task L pegang mutex */

#endif /* FREERTOS_CONFIG_H */
