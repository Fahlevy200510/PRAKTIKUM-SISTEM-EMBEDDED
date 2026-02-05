/**
 * ============================================================================
 * FILE: FreeRTOSConfig.h
 * PROJECT: 09-Queue_Peek_Operations
 * 
 * JUDUL: Queue Peek - Membaca Tanpa Menghapus Data
 * 
 * DESKRIPSI:
 * Demo penggunaan xQueuePeek() untuk membaca data dari queue tanpa
 * menghapusnya. Berguna untuk preview, validation, atau shared reading.
 * 
 * ============================================================================
 * KONSEP QUEUE PEEK
 * ============================================================================
 * 
 *    xQueueReceive() vs xQueuePeek():
 *    ═══════════════════════════════════════════════════════════════════════
 *    
 *    xQueueReceive():           xQueuePeek():
 *    ────────────────           ─────────────
 *    BACA + HAPUS               BACA SAJA
 *    
 *    Before: [A][B][C]          Before: [A][B][C]
 *    Read:    A                 Read:    A
 *    After:  [B][C][ ]          After:  [A][B][C]  ← A masih ada!
 *    
 *    ═══════════════════════════════════════════════════════════════════════
 * 
 *    USE CASES UNTUK PEEK:
 *    ═══════════════════════════════════════════════════════════════════════
 *    
 *    1. PREVIEW SEBELUM PROSES
 *       ───────────────────────
 *       Lihat dulu data apa yang akan diproses.
 *       Jika tidak valid → skip tanpa hilang.
 *       
 *       ┌─────────────────┐
 *       │ Peek → Validate │
 *       │      ↓          │
 *       │   Valid?        │
 *       │   ├─ Yes → Receive & Process
 *       │   └─ No  → Log, tetap di queue
 *       └─────────────────┘
 *    
 *    2. MULTIPLE READERS
 *       ─────────────────
 *       Beberapa task bisa baca data yang sama.
 *       Hanya satu task yang "own" dan Receive.
 *       
 *       Task A: Peek ──────► [Data] ◄────── Peek :Task B
 *                            [Data]
 *       Task C: Receive ────► (Data removed)
 *    
 *    3. CONDITIONAL PROCESSING
 *       ───────────────────────
 *       Peek untuk cek kondisi.
 *       Receive hanya jika kondisi terpenuhi.
 *       
 *       if(Peek && data.priority == HIGH)
 *           Receive & ProcessNow();
 *       else
 *           Wait for higher priority task
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
 * KONFIGURASI TASK
 * ============================================================================ */

#define QUEUE_LENGTH                            5

#define PRODUCER_STACK                          256
#define PRODUCER_PRIO                           (tskIDLE_PRIORITY + 2)

#define VALIDATOR_STACK                         256
#define VALIDATOR_PRIO                          (tskIDLE_PRIORITY + 3)

#define PROCESSOR_STACK                         256
#define PROCESSOR_PRIO                          (tskIDLE_PRIORITY + 1)

#endif /* FREERTOS_CONFIG_H */
