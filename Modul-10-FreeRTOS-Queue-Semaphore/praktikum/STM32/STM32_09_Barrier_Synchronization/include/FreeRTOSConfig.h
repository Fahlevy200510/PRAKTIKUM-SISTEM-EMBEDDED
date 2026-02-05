/**
 * ============================================================================
 * FILE: FreeRTOSConfig.h
 * PROJECT: 15-Barrier_Synchronization
 * 
 * DESKRIPSI:
 * Konfigurasi FreeRTOS untuk demo Barrier Synchronization menggunakan
 * Event Groups.
 * 
 * APA ITU BARRIER SYNCHRONIZATION?
 * ================================
 * Barrier adalah titik sinkronisasi dimana SEMUA task harus tiba
 * sebelum ada yang boleh melanjutkan.
 * 
 * ILUSTRASI BARRIER:
 * ==================
 * 
 *    Task A    Task B    Task C         BARRIER
 *      │         │         │               ║
 *      │         │         │               ║
 *      ▼         │         │               ║
 *    Selesai    │         │               ║
 *    (tunggu)   │         │               ║
 *      ░         ▼         │               ║
 *      ░       Selesai    │               ║
 *      ░       (tunggu)   │               ║
 *      ░         ░         ▼               ║
 *      ░         ░       Selesai          ║
 *      ░         ░                        ║
 *    ═══════════════════════════════════  ║  SEMUA SUDAH TIBA!
 *      │         │         │               ║
 *      ▼         ▼         ▼               ║
 *    Lanjut    Lanjut    Lanjut           ║
 *      │         │         │               ║
 * 
 * CONTOH PENGGUNAAN:
 * ==================
 * 1. Inisialisasi multi-subsistem
 *    - Tunggu semua sensor ready sebelum mulai pengukuran
 * 
 * 2. Paralel computation
 *    - Tunggu semua thread selesai bagiannya sebelum gabungkan hasil
 * 
 * 3. Game/simulasi
 *    - Tunggu semua player ready sebelum mulai game
 * 
 * IMPLEMENTASI DENGAN EVENT GROUP:
 * ================================
 * 
 *    Event Group Bits:
 *    ┌────┬────┬────┬────┬────┬────┬────┬────┐
 *    │ 7  │ 6  │ 5  │ 4  │ 3  │ 2  │ 1  │ 0  │
 *    └────┴────┴────┴────┴────┴────┴────┴────┘
 *           Task C    Task B    Task A
 *             │         │         │
 *    Set bit ketika task tiba di barrier
 *    Tunggu sampai SEMUA bit = 1
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
#define INCLUDE_xEventGroupSetBitFromISR        1   /* Untuk event group dari ISR */
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

/* Parameter Barrier */
#define NUM_WORKER_TASKS                        3       /* Jumlah task di barrier */

#define WORKER_STACK                            256
#define WORKER_PRIO                             (tskIDLE_PRIORITY + 1)

/* Event bits untuk setiap task */
#define BARRIER_BIT_TASK_A                      (1 << 0)
#define BARRIER_BIT_TASK_B                      (1 << 1)
#define BARRIER_BIT_TASK_C                      (1 << 2)
#define BARRIER_ALL_BITS                        (BARRIER_BIT_TASK_A | BARRIER_BIT_TASK_B | BARRIER_BIT_TASK_C)

#endif /* FREERTOS_CONFIG_H */
