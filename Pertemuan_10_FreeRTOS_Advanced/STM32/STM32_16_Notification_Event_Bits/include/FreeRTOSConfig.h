/**
 * ============================================================================
 * FILE: FreeRTOSConfig.h
 * PROJECT: 18-Notification_Event_Bits
 * 
 * DESKRIPSI:
 * Demo Task Notification sebagai Event Bits (pengganti Event Group).
 * Setiap bit dalam notification value mewakili satu event.
 * 
 * KEUNTUNGAN DIBANDING EVENT GROUP:
 * =================================
 * - Lebih cepat (direct ke task)
 * - Tidak perlu membuat Event Group object
 * - Hemat RAM
 * 
 * BATASAN:
 * ========
 * - Hanya 1 task yang bisa menunggu (receiver)
 * - 32 bit = maksimal 32 event berbeda
 * - Tidak ada xEventGroupSync (barrier)
 * 
 * ILUSTRASI EVENT BITS:
 * =====================
 * 
 *    Notification Value (32-bit):
 *    ┌────┬────┬────┬────┬────┬────┬────┬────┐
 *    │ 7  │ 6  │ 5  │ 4  │ 3  │ 2  │ 1  │ 0  │
 *    └────┴────┴────┴────┴────┴────┴────┴────┘
 *           │         │         │         │
 *           │         │         │         └─ EVENT_BTN_PRESSED
 *           │         │         └─────────── EVENT_DATA_READY
 *           │         └───────────────────── EVENT_TIMEOUT
 *           └─────────────────────────────── EVENT_ERROR
 * 
 *    Producer A:  xTaskNotify(handle, EVENT_BTN_PRESSED, eSetBits)
 *    Producer B:  xTaskNotify(handle, EVENT_DATA_READY,  eSetBits)
 *    
 *    Consumer:    xTaskNotifyWait(...) untuk cek bit mana yang aktif
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

#define LED_GPIO_PORT                           GPIOC
#define LED_GPIO_PIN                            GPIO_PIN_13
#define LED_ACTIVE_LOW                          1

#define DEBUG_UART_INSTANCE                     USART1
#define DEBUG_UART_BAUDRATE                     115200
#define DEBUG_UART_TX_PORT                      GPIOA
#define DEBUG_UART_TX_PIN                       GPIO_PIN_9
#define DEBUG_UART_RX_PORT                      GPIOA
#define DEBUG_UART_RX_PIN                       GPIO_PIN_10

/* Event bit definitions */
#define EVENT_BUTTON_PRESSED                    (1 << 0)
#define EVENT_DATA_READY                        (1 << 1)
#define EVENT_TIMER_EXPIRED                     (1 << 2)
#define EVENT_ERROR_OCCURRED                    (1 << 3)
#define EVENT_ALL_BITS                          (EVENT_BUTTON_PRESSED | EVENT_DATA_READY | \
                                                 EVENT_TIMER_EXPIRED | EVENT_ERROR_OCCURRED)

#define PRODUCER_STACK                          256
#define PRODUCER_PRIO                           (tskIDLE_PRIORITY + 2)

#define HANDLER_STACK                           256
#define HANDLER_PRIO                            (tskIDLE_PRIORITY + 1)

#endif /* FREERTOS_CONFIG_H */
