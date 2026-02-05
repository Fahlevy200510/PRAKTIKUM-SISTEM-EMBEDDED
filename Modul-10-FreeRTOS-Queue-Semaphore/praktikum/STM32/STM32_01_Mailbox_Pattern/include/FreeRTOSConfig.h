/**
 * ============================================================================
 * FILE: FreeRTOSConfig.h
 * PROJECT: 07-Mailbox_Pattern
 * 
 * JUDUL: Mailbox Pattern - Queue dengan Depth 1
 * 
 * DESKRIPSI:
 * Mailbox adalah queue dengan depth=1, digunakan untuk menyimpan
 * "nilai terbaru" yang bisa dibaca kapan saja. Berbeda dengan queue
 * biasa yang FIFO, mailbox selalu menyimpan data paling baru.
 * 
 * ============================================================================
 * KONSEP MAILBOX PATTERN
 * ============================================================================
 * 
 *    QUEUE NORMAL (FIFO) vs MAILBOX:
 *    ════════════════════════════════════════════════════════════════════
 *    
 *    QUEUE NORMAL (depth=5):
 *    ┌──────┬──────┬──────┬──────┬──────┐
 *    │ D1   │ D2   │ D3   │ D4   │ D5   │  Data masuk berurutan
 *    └──────┴──────┴──────┴──────┴──────┘  Consumer harus baca semua!
 *        ↑                           ↑
 *      HEAD                        TAIL
 *    
 *    - Consumer HARUS baca D1 dulu, baru D2, dst
 *    - Jika consumer lambat, data lama menumpuk
 *    - Cocok untuk: semua data penting, tidak boleh hilang
 *    
 *    MAILBOX (depth=1):
 *    ┌──────────────────┐
 *    │ DATA TERBARU     │  Hanya 1 slot, selalu yang paling baru
 *    └──────────────────┘
 *    
 *    - Producer selalu OVERWRITE data lama
 *    - Consumer bisa baca kapan saja
 *    - Consumer bisa PEEK tanpa mengambil
 *    - Cocok untuk: sensor terbaru, status system, shared state
 *    
 *    ════════════════════════════════════════════════════════════════════
 * 
 *    ILUSTRASI OVERWRITE:
 *    ────────────────────
 *    
 *    t=0:  Producer kirim "A"    Mailbox: [A]
 *    t=1:  Producer kirim "B"    Mailbox: [B]  ← A hilang!
 *    t=2:  Producer kirim "C"    Mailbox: [C]  ← B hilang!
 *    t=3:  Consumer baca         Consumer dapat: C (data terbaru)
 *    
 *    CATATAN: Ini adalah FITUR, bukan bug! Kita memang hanya
 *             butuh data paling baru untuk sensor reading.
 * 
 * ============================================================================
 * USE CASE MAILBOX
 * ============================================================================
 * 
 *    1. SENSOR READING TERBARU
 *       ─────────────────────
 *       Sensor task update suhu setiap 100ms.
 *       Display task baca suhu untuk tampilkan (tidak perlu semua history).
 *       
 *    2. SYSTEM STATUS
 *       ─────────────────────
 *       Main task update status: IDLE, RUNNING, ERROR
 *       Multiple task bisa peek status tanpa mempengaruhi.
 *       
 *    3. COMMAND REGISTER
 *       ─────────────────────
 *       UI task tulis command terbaru.
 *       Worker task baca dan eksekusi command.
 * 
 * ============================================================================
 * API YANG DIGUNAKAN
 * ============================================================================
 * 
 *    // Buat mailbox (queue depth=1)
 *    xMailbox = xQueueCreate(1, sizeof(DataType));
 *    
 *    // Tulis ke mailbox (OVERWRITE jika sudah ada)
 *    xQueueOverwrite(xMailbox, &newData);
 *    
 *    // Baca dan hapus dari mailbox
 *    xQueueReceive(xMailbox, &data, timeout);
 *    
 *    // Baca tanpa hapus (PEEK)
 *    xQueuePeek(xMailbox, &data, timeout);
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

#define SENSOR_TASK_STACK                       256
#define SENSOR_TASK_PRIO                        (tskIDLE_PRIORITY + 2)
#define SENSOR_UPDATE_MS                        200     /* Update tiap 200ms */

#define DISPLAY_TASK_STACK                      256
#define DISPLAY_TASK_PRIO                       (tskIDLE_PRIORITY + 1)
#define DISPLAY_REFRESH_MS                      500     /* Refresh tiap 500ms */

#endif /* FREERTOS_CONFIG_H */
