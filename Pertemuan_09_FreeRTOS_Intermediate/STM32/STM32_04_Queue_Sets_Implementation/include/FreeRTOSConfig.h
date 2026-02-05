/**
 * ============================================================================
 * FILE: FreeRTOSConfig.h
 * PROJECT: 10-Queue_Sets_Implementation
 * 
 * JUDUL: Queue Sets - Menunggu Multiple Queues Sekaligus
 * 
 * DESKRIPSI:
 * Demo penggunaan Queue Sets untuk menunggu data dari beberapa
 * queue sekaligus dalam satu blocking call. Mirip select() di Unix.
 * 
 * ============================================================================
 * KONSEP QUEUE SETS
 * ============================================================================
 * 
 *    MASALAH: Menunggu Data dari Banyak Sumber
 *    ═══════════════════════════════════════════════════════════════════════
 *    
 *    Tanpa Queue Sets (BURUK):
 *    ─────────────────────────
 *    
 *    Task Handler:
 *    ┌───────────────────────────────────────────────────────────────────┐
 *    │ while(1) {                                                        │
 *    │     xQueueReceive(queueA, &data, 10ms);  // Cek A                 │
 *    │     xQueueReceive(queueB, &data, 10ms);  // Cek B                 │
 *    │     xQueueReceive(queueC, &data, 10ms);  // Cek C                 │
 *    │     // Polling terus, buang CPU time!                             │
 *    │ }                                                                 │
 *    └───────────────────────────────────────────────────────────────────┘
 *    
 *    Masalah:
 *    - Busy polling (boros CPU)
 *    - Latency tinggi (harus tunggu timeout)
 *    - Kompleks jika banyak queue
 *    
 *    ═══════════════════════════════════════════════════════════════════════
 *    
 *    Dengan Queue Sets (BAIK):
 *    ─────────────────────────
 *    
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                                                                     │
 *    │     Queue A ────┐                                                   │
 *    │                 │                                                   │
 *    │     Queue B ────┼────► QUEUE SET ────► xQueueSelectFromSet()       │
 *    │                 │         │                     │                   │
 *    │     Queue C ────┘         │                     ▼                   │
 *    │                           │            "Queue mana yang ready?"    │
 *    │                           │                     │                   │
 *    │                           └─────────────────────┘                   │
 *    │                                                                     │
 *    │   Task Handler:                                                     │
 *    │   ┌───────────────────────────────────────────────────────────────┐│
 *    │   │ QueueHandle = xQueueSelectFromSet(set, portMAX_DELAY);        ││
 *    │   │ // BLOCK sampai salah satu queue ada data!                    ││
 *    │   │ // Return handle queue yang ready                             ││
 *    │   │ xQueueReceive(QueueHandle, &data, 0);                         ││
 *    │   └───────────────────────────────────────────────────────────────┘│
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 *    
 *    Keuntungan:
 *    - Blocking (hemat CPU)
 *    - Zero latency (langsung bangun saat data masuk)
 *    - Skalabel (mudah tambah queue baru)
 * 
 * ============================================================================
 * ILUSTRASI ALUR
 * ============================================================================
 * 
 *    Time ────────────────────────────────────────────────────────────►
 *    
 *    Queue A:      ░░░░░░░░░░░░░░░██░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
 *                                  ↑ data masuk
 *    
 *    Queue B:      ░░░░░░░░░░░░░░░░░░░░░░░░░░░░██░░░░░░░░░░░░░░░░░░░
 *                                              ↑ data masuk
 *    
 *    Queue C:      ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░██░░░░░░░
 *                                                          ↑ data masuk
 *    
 *    Handler:      ████████████████↓█████████████↓████████████↓████
 *                  [BLOCKED]       [A]           [B]          [C]
 *                                  wake          wake         wake
 *                                  process A     process B    process C
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

#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             0
#define configUSE_COUNTING_SEMAPHORES           0

/* PENTING: Aktifkan Queue Sets! */
#define configUSE_QUEUE_SETS                    1

#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configUSE_MALLOC_FAILED_HOOK            1
#define configCHECK_FOR_STACK_OVERFLOW          2

#define configGENERATE_RUN_TIME_STATS           0
#define configUSE_TRACE_FACILITY                0
#define configUSE_STATS_FORMATTING_FUNCTIONS    0

#define configUSE_CO_ROUTINES                   0
#define configMAX_CO_ROUTINE_PRIORITIES         2

/* Timer diperlukan untuk demo */
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               (configMAX_PRIORITIES - 1)
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
 * KONFIGURASI QUEUE SETS
 * ============================================================================ */

#define QUEUE_A_LENGTH                          3
#define QUEUE_B_LENGTH                          3
#define QUEUE_C_LENGTH                          3

/* Total item dalam set = sum of all queue lengths */
#define QUEUE_SET_SIZE                          (QUEUE_A_LENGTH + QUEUE_B_LENGTH + QUEUE_C_LENGTH)

#define TASK_STACK                              256

#endif /* FREERTOS_CONFIG_H */
