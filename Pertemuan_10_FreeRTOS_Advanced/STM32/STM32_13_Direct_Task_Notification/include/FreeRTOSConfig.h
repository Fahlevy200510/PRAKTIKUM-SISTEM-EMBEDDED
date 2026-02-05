/**
 * ============================================================================
 * FILE: FreeRTOSConfig.h
 * PROJECT: 16-Direct_Task_Notification
 * 
 * DESKRIPSI:
 * Konfigurasi FreeRTOS untuk demo Task Notification sebagai pengganti
 * Binary Semaphore yang LEBIH EFISIEN.
 * 
 * APA ITU TASK NOTIFICATION?
 * ==========================
 * Setiap task di FreeRTOS punya "notification value" bawaan.
 * Tidak perlu membuat semaphore/queue terpisah!
 * 
 * PERBANDINGAN DENGAN BINARY SEMAPHORE:
 * =====================================
 * 
 *    Binary Semaphore           │    Task Notification
 *    ───────────────────────────┼─────────────────────────────
 *    Perlu xSemaphoreCreate()   │  Otomatis ada di setiap task
 *    Butuh RAM untuk struct     │  RAM sudah termasuk di TCB
 *    Lebih lambat               │  45% lebih cepat!
 *    Bisa Give dari mana saja   │  Notify ke task tertentu
 *    Multi producer             │  Biasanya 1:1
 * 
 * ILUSTRASI:
 * ==========
 * 
 *    ISR / Task Lain                      Target Task
 *    ─────────────────                    ───────────
 *    
 *    xTaskNotifyGive(handle) ─────────► Notification Value++
 *                                             │
 *                                             ▼
 *                              ulTaskNotifyTake(pdTRUE, ...)
 *                                             │
 *                                             ▼
 *                              Return count & reset to 0
 * 
 * KEGUNAAN:
 * =========
 * 1. Pengganti binary semaphore (lebih cepat, hemat RAM)
 * 2. Pengganti counting semaphore (dengan batasan)
 * 3. Pengganti event flags (dengan xTaskNotify)
 * 4. Pengganti mailbox ringan
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

/* Task Notification tersedia otomatis, tidak perlu konfigurasi khusus */
#define configUSE_TASK_NOTIFICATIONS            1  /* Default 1 di FreeRTOS v10+ */

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

/* Parameter Demo */
#define PRODUCER_STACK                          256
#define PRODUCER_PRIO                           (tskIDLE_PRIORITY + 2)
#define PRODUCER_PERIOD_MS                      300

#define CONSUMER_STACK                          256
#define CONSUMER_PRIO                           (tskIDLE_PRIORITY + 1)

#endif /* FREERTOS_CONFIG_H */
