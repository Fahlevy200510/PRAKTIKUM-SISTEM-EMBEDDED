/**
 * ============================================================================
 * FILE: FreeRTOSConfig.h
 * PROJECT: 26-ISR_Safe_Queue
 * 
 * JUDUL: ISR-Safe Queue Operations
 * 
 * ============================================================================
 * KONSEP ISR-SAFE API
 * ============================================================================
 * 
 * FreeRTOS menyediakan DUA versi API:
 *   1. Normal API - untuk dipanggil dari TASK context
 *   2. FromISR API - untuk dipanggil dari INTERRUPT context
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                                                                     │
 *    │   TASK CONTEXT                   ISR CONTEXT                        │
 *    │   ═══════════                    ═══════════                        │
 *    │                                                                     │
 *    │   xQueueSend()              →    xQueueSendFromISR()                │
 *    │   xQueueReceive()           →    xQueueReceiveFromISR()             │
 *    │   xSemaphoreGive()          →    xSemaphoreGiveFromISR()            │
 *    │   xSemaphoreTake()          →    xSemaphoreTakeFromISR()            │
 *    │   xTaskNotifyGive()         →    vTaskNotifyGiveFromISR()           │
 *    │   xEventGroupSetBits()      →    xEventGroupSetBitsFromISR()        │
 *    │                                                                     │
 *    │   BOLEH block                    TIDAK BOLEH block!                 │
 *    │   (wait untuk queue/sem)         (ISR harus cepat)                  │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * MENGAPA BUTUH FromISR API?
 * ============================================================================
 * 
 *    Normal API bisa BLOCK:
 *    ══════════════════════════════════════════════════════════════════════
 *    
 *    xQueueSend(queue, &data, portMAX_DELAY);
 *                              ↑
 *                              Block sampai ada slot!
 *    
 *    Di ISR ini FATAL! ISR tidak boleh block karena:
 *    - Interrupt lain tidak bisa diproses
 *    - Scheduler tidak berjalan
 *    - Sistem hang!
 *    
 *    
 *    FromISR API TIDAK PERNAH BLOCK:
 *    ══════════════════════════════════════════════════════════════════════
 *    
 *    BaseType_t result;
 *    result = xQueueSendFromISR(queue, &data, &xHigherPriorityTaskWoken);
 *    
 *    if(result == pdPASS) {
 *        // Berhasil masuk queue
 *    } else {
 *        // Queue penuh - data hilang!
 *    }
 * 
 * ============================================================================
 * xHigherPriorityTaskWoken PARAMETER
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                                                                     │
 *    │   Parameter ini PENTING untuk proper context switch!               │
 *    │                                                                     │
 *    │   Sebelum ISR:                                                      │
 *    │   ──────────────────────────────────────────────────────────────    │
 *    │   TaskA (running, priority 1)                                       │
 *    │   TaskB (blocked on queue, priority 3)                              │
 *    │                                                                     │
 *    │   Di ISR:                                                           │
 *    │   ──────────────────────────────────────────────────────────────    │
 *    │   xQueueSendFromISR(queue, &data, &xHigherPriorityTaskWoken);       │
 *    │   //                               ↑                               │
 *    │   // TaskB unblocked! TaskB priority (3) > TaskA priority (1)      │
 *    │   // xHigherPriorityTaskWoken = pdTRUE                             │
 *    │                                                                     │
 *    │   Akhir ISR:                                                        │
 *    │   ──────────────────────────────────────────────────────────────    │
 *    │   portYIELD_FROM_ISR(xHigherPriorityTaskWoken);                     │
 *    │   // Jika pdTRUE: langsung switch ke TaskB                          │
 *    │   // Jika pdFALSE: kembali ke TaskA                                 │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* ============================================================================
 * BASIC CONFIGURATION
 * ============================================================================ */

#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configUSE_TICKLESS_IDLE                 0
#define configCPU_CLOCK_HZ                      72000000UL
#define configTICK_RATE_HZ                      1000
#define configMAX_PRIORITIES                    5
#define configMINIMAL_STACK_SIZE                128
#define configMAX_TASK_NAME_LEN                 16
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_TASK_NOTIFICATIONS            1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES   1
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             0
#define configUSE_COUNTING_SEMAPHORES           1
#define configQUEUE_REGISTRY_SIZE               8
#define configUSE_QUEUE_SETS                    0
#define configUSE_TIME_SLICING                  1

/* ============================================================================
 * MEMORY CONFIGURATION
 * ============================================================================ */

#define configSUPPORT_STATIC_ALLOCATION         0
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configTOTAL_HEAP_SIZE                   ((size_t)10240)

/* ============================================================================
 * HOOK CONFIGURATION
 * ============================================================================ */

#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configCHECK_FOR_STACK_OVERFLOW          2
#define configUSE_MALLOC_FAILED_HOOK            1

/* ============================================================================
 * SOFTWARE TIMER CONFIGURATION
 * ============================================================================ */

#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            256

/* ============================================================================
 * INTERRUPT CONFIGURATION
 * ============================================================================ */

#ifdef __NVIC_PRIO_BITS
    #define configPRIO_BITS __NVIC_PRIO_BITS
#else
    #define configPRIO_BITS 4
#endif

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5
#define configKERNEL_INTERRUPT_PRIORITY         (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/* ============================================================================
 * API FUNCTIONS
 * ============================================================================ */

#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_xResumeFromISR                  1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_uxTaskGetStackHighWaterMark     1
#define INCLUDE_xTimerPendFunctionCall          1
#define INCLUDE_xTaskResumeFromISR              1

/* ============================================================================
 * CORTEX-M3 HANDLERS
 * ============================================================================ */

#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

/* ============================================================================
 * APPLICATION DEFINITIONS
 * ============================================================================ */

#define LED_GPIO_PORT       GPIOC
#define LED_GPIO_PIN        GPIO_PIN_13

#define DEBUG_UART_INSTANCE   USART1
#define DEBUG_UART_BAUDRATE   115200
#define DEBUG_UART_TX_PORT    GPIOA
#define DEBUG_UART_TX_PIN     GPIO_PIN_9
#define DEBUG_UART_RX_PORT    GPIOA
#define DEBUG_UART_RX_PIN     GPIO_PIN_10

#define ISR_QUEUE_LENGTH      5
#define TASK_STACK            256

#endif /* FREERTOS_CONFIG_H */
