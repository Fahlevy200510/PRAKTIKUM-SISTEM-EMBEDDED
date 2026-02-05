/**
 * ============================================================================
 * FILE: FreeRTOSConfig.h
 * PROJECT: 24-Deferred_ISR_Processing
 * 
 * JUDUL: Deferred Interrupt Processing
 * 
 * ============================================================================
 * KONSEP DEFERRED INTERRUPT PROCESSING
 * ============================================================================
 * 
 * MASALAH ISR yang Terlalu Lama:
 * 
 *    ISR harus CEPAT! Kenapa?
 *    - Interrupt lain tidak bisa diproses (jika lower/same priority)
 *    - Scheduler tidak berjalan
 *    - Real-time response terganggu
 *    - Watchdog bisa timeout
 * 
 *    BURUK - Processing di ISR:
 *    ══════════════════════════════════════════════════════════════════════
 *    
 *    void UART_IRQHandler(void)
 *    {
 *        uint8_t data = ReadData();        // ✓ OK
 *        
 *        ParseProtocol(data);              // ✗ LAMA!
 *        ValidateCRC(data);                // ✗ LAMA!
 *        StoreToFlash(data);               // ✗ SANGAT LAMA!
 *        SendResponse();                   // ✗ LAMA!
 *    }
 *    
 *    Total waktu: 10ms+ → Sistem tidak responsif!
 * 
 * ============================================================================
 * SOLUSI: DEFERRED PROCESSING
 * ============================================================================
 * 
 *    Prinsip:
 *    1. ISR hanya ambil data dan notify task
 *    2. Task melakukan processing berat
 *    3. ISR selesai CEPAT!
 *    
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                                                                     │
 *    │   INTERRUPT                                                         │
 *    │   ┌─────────────────────────────────────────────────────────────┐   │
 *    │   │ void UART_IRQHandler(void)                                  │   │
 *    │   │ {                                                           │   │
 *    │   │     data = ReadData();              // ~1μs                 │   │
 *    │   │     xQueueSendFromISR(queue, &data) // ~5μs                 │   │
 *    │   │ }                                   // Total: ~6μs ✓        │   │
 *    │   └─────────────────────────────────────────────────────────────┘   │
 *    │                           │                                         │
 *    │                           │ Send to queue                           │
 *    │                           ▼                                         │
 *    │                    ┌──────────────┐                                 │
 *    │                    │    QUEUE     │                                 │
 *    │                    └──────┬───────┘                                 │
 *    │                           │                                         │
 *    │                           ▼                                         │
 *    │   DEFERRED HANDLER TASK                                             │
 *    │   ┌─────────────────────────────────────────────────────────────┐   │
 *    │   │ void vDeferredHandler(void *p)                              │   │
 *    │   │ {                                                           │   │
 *    │   │     while(1) {                                              │   │
 *    │   │         xQueueReceive(queue, &data, MAX_DELAY);             │   │
 *    │   │                                                             │   │
 *    │   │         // Processing berat - OK di sini!                   │   │
 *    │   │         ParseProtocol(data);      // 1ms                    │   │
 *    │   │         ValidateCRC(data);        // 2ms                    │   │
 *    │   │         StoreToFlash(data);       // 5ms                    │   │
 *    │   │         SendResponse();           // 1ms                    │   │
 *    │   │     }                             // Total: 9ms, tapi OK!   │   │
 *    │   │ }                                                           │   │
 *    │   └─────────────────────────────────────────────────────────────┘   │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * METODE DEFERRED PROCESSING
 * ============================================================================
 * 
 *    1. QUEUE (paling umum)
 *       ISR → xQueueSendFromISR() → Task receives
 *       
 *    2. SEMAPHORE (untuk signaling saja)
 *       ISR → xSemaphoreGiveFromISR() → Task takes
 *       
 *    3. TASK NOTIFICATION (paling efisien)
 *       ISR → xTaskNotifyFromISR() → Task waits
 *       
 *    4. TIMER DAEMON (xTimerPendFunctionCallFromISR)
 *       ISR → Pend function to timer task → Timer task calls function
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
#define configUSE_NEWLIB_REENTRANT              0
#define configENABLE_BACKWARD_COMPATIBILITY     0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 5

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
 * INTERRUPT CONFIGURATION (Cortex-M3)
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
#define INCLUDE_xTaskGetIdleTaskHandle          0
#define INCLUDE_eTaskGetState                   0
#define INCLUDE_xEventGroupSetBitFromISR        0
#define INCLUDE_xTimerPendFunctionCall          1
#define INCLUDE_xTaskAbortDelay                 0
#define INCLUDE_xTaskGetHandle                  0
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

/* Queue untuk deferred processing */
#define EVENT_QUEUE_LENGTH    10

/* Simulasi period untuk "interrupt" */
#define SIMULATE_ISR_PERIOD_MS   500

#define TASK_STACK            256

#endif /* FREERTOS_CONFIG_H */
