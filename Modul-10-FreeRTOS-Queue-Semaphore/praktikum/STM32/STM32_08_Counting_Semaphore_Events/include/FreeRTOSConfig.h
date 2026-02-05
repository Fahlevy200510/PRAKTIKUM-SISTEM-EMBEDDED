/**
 * ============================================================================
 * FILE: FreeRTOSConfig.h
 * PROJECT: 14-Counting_Semaphore_Events
 * 
 * DESKRIPSI:
 * Konfigurasi FreeRTOS untuk demo Counting Semaphore.
 * 
 * APA ITU COUNTING SEMAPHORE?
 * ===========================
 * Counting semaphore seperti "penghitung tiket":
 * - Nilainya bisa lebih dari 1 (berbeda dengan binary semaphore)
 * - Give() = menambah count
 * - Take() = mengurangi count (block jika count = 0)
 * 
 * KEGUNAAN COUNTING SEMAPHORE:
 * ============================
 * 
 * 1. MENGHITUNG EVENT
 *    ┌──────────────────────────────────────────────────────────┐
 *    │  ISR               Counting Semaphore         Task       │
 *    │                    ┌─────────────┐                       │
 *    │  Event! ──Give()──►│  count=1    │                       │
 *    │  Event! ──Give()──►│  count=2    │                       │
 *    │  Event! ──Give()──►│  count=3    │──Take()──► Process 1  │
 *    │                    │  count=2    │──Take()──► Process 2  │
 *    │                    │  count=1    │──Take()──► Process 3  │
 *    │                    │  count=0    │                       │
 *    │                    └─────────────┘            Block...   │
 *    └──────────────────────────────────────────────────────────┘
 *    
 *    Semua event TERCATAT dan DIPROSES, tidak ada yang hilang!
 * 
 * 2. RESOURCE POOL
 *    ┌──────────────────────────────────────────────────────────┐
 *    │                     Pool: 3 Buffer                       │
 *    │                     ┌─────────────┐                      │
 *    │  Task A ──Take()───►│  count=2    │  (ambil 1 buffer)    │
 *    │  Task B ──Take()───►│  count=1    │  (ambil 1 buffer)    │
 *    │  Task C ──Take()───►│  count=0    │  (ambil 1 buffer)    │
 *    │  Task D ──Take()───►│  BLOCK!     │  (tidak ada buffer)  │
 *    │                     │             │                      │
 *    │  Task A ──Give()───►│  count=1    │  (kembalikan buffer) │
 *    │  Task D             │  dapat!     │  (dapat buffer)      │
 *    │                     └─────────────┘                      │
 *    └──────────────────────────────────────────────────────────┘
 * 
 * PERBANDINGAN:
 * =============
 * 
 *    Binary Semaphore  │  Counting Semaphore
 *    ──────────────────┼─────────────────────
 *    count = 0 atau 1  │  count = 0 s/d MAX
 *    1 event = 1 take  │  N event = N take
 *    Untuk signaling   │  Untuk event/resource
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
#define configUSE_COUNTING_SEMAPHORES           1   /* WAJIB AKTIF! */

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

/* Parameter Counting Semaphore */
#define EVENT_MAX_COUNT                         10      /* Kapasitas maksimal */
#define EVENT_INITIAL_COUNT                     0       /* Mulai kosong */

#define PRODUCER_STACK                          256
#define PRODUCER_PRIO                           (tskIDLE_PRIORITY + 2)
#define PRODUCER_PERIOD_MS                      100     /* Generate event cepat */

#define CONSUMER_STACK                          256
#define CONSUMER_PRIO                           (tskIDLE_PRIORITY + 1)
#define CONSUMER_PROCESS_MS                     300     /* Proses lebih lambat */

#define MONITOR_STACK                           256
#define MONITOR_PRIO                            (tskIDLE_PRIORITY + 1)
#define MONITOR_PERIOD_MS                       500

#endif /* FREERTOS_CONFIG_H */
