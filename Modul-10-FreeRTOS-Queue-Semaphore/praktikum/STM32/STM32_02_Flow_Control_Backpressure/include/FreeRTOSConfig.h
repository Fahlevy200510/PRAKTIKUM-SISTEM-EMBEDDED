/**
 * ============================================================================
 * FILE: FreeRTOSConfig.h
 * PROJECT: 08-Flow_Control_Backpressure
 * 
 * JUDUL: Flow Control dan Backpressure dengan Queue
 * 
 * DESKRIPSI:
 * Demo mekanisme flow control menggunakan queue space monitoring.
 * Producer memperlambat laju produksi ketika queue hampir penuh,
 * mencegah data loss dan memberikan kesempatan consumer mengejar.
 * 
 * ============================================================================
 * KONSEP FLOW CONTROL & BACKPRESSURE
 * ============================================================================
 * 
 *    MASALAH: Producer Lebih Cepat dari Consumer
 *    ═══════════════════════════════════════════════════════════════════════
 *    
 *    Producer: ████████████████████████████████  (cepat: 100ms)
 *    Consumer: ████░░░░████░░░░████░░░░████░░░░  (lambat: 500ms)
 *    
 *    Queue:  [D1][D2][D3][D4][D5] ← PENUH!
 *            Producer tidak bisa kirim → data HILANG!
 *    
 *    ═══════════════════════════════════════════════════════════════════════
 * 
 *    SOLUSI: Backpressure / Flow Control
 *    ═══════════════════════════════════════════════════════════════════════
 *    
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                                                                     │
 *    │   Producer ──► Queue ──► Consumer                                  │
 *    │       ▲           │                                                 │
 *    │       │           │                                                 │
 *    │       └───────────┘                                                 │
 *    │       "Queue hampir penuh!"                                        │
 *    │       → Producer slow down                                         │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 *    
 *    Cara kerja:
 *    1. Producer cek: uxQueueSpacesAvailable(queue)
 *    2. Jika space < threshold → tambah delay
 *    3. Consumer mulai mengejar
 *    4. Space bertambah → Producer kembali normal
 *    
 *    ═══════════════════════════════════════════════════════════════════════
 * 
 *    LEVEL BACKPRESSURE:
 *    ═══════════════════════════════════════════════════════════════════════
 *    
 *    Queue Depth = 10 slots
 *    
 *    Space Available   Action              Delay
 *    ───────────────   ──────────────────  ─────────
 *    8-10 (80-100%)    NORMAL              100ms
 *    5-7  (50-80%)     SLOW                200ms
 *    2-4  (20-50%)     VERY SLOW           500ms
 *    0-1  (0-20%)      CRITICAL/STOP       1000ms
 *    
 *    Visualisasi:
 *    
 *    ┌────────────────────────────────────────────────────────────────┐
 *    │ Queue Level Bar                                                │
 *    ├────────────────────────────────────────────────────────────────┤
 *    │ ████████░░ 80% → NORMAL                                        │
 *    │ ██████████ 100% → CRITICAL!                                    │
 *    │ ████░░░░░░ 40% → SLOW                                          │
 *    │ ██░░░░░░░░ 20% → VERY SLOW                                     │
 *    └────────────────────────────────────────────────────────────────┘
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
#define INCLUDE_xEventGroupSetBitFromISR        0
#define INCLUDE_xTimerPendFunctionCall          0
#define INCLUDE_xTaskAbortDelay                 0
#define INCLUDE_xTaskGetHandle                  0
#define INCLUDE_xTaskResumeFromISR              0

/* ============================================================================
 * KONFIGURASI HARDWARE
 * ============================================================================ */

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
 * KONFIGURASI FLOW CONTROL
 * ============================================================================ */

#define QUEUE_DEPTH                             10

/* Backpressure thresholds (dalam slot tersedia) */
#define THRESHOLD_NORMAL                        8       /* >= 8 slot kosong */
#define THRESHOLD_SLOW                          5       /* >= 5 slot kosong */
#define THRESHOLD_VERY_SLOW                     2       /* >= 2 slot kosong */

/* Delay untuk setiap level (ms) */
#define DELAY_NORMAL_MS                         100
#define DELAY_SLOW_MS                           200
#define DELAY_VERY_SLOW_MS                      500
#define DELAY_CRITICAL_MS                       1000

/* Task konfigurasi */
#define PRODUCER_STACK                          256
#define PRODUCER_PRIO                           (tskIDLE_PRIORITY + 2)

#define CONSUMER_STACK                          256
#define CONSUMER_PRIO                           (tskIDLE_PRIORITY + 1)
#define CONSUMER_PROCESS_MS                     300     /* Simulasi proses lambat */

#endif /* FREERTOS_CONFIG_H */
