/**
 * ============================================================================
 * FILE: FreeRTOSConfig.h
 * PROJECT: 22-Timer_ID_Callback
 * 
 * JUDUL: Timer ID dan Shared Callback
 * 
 * ============================================================================
 * KONSEP TIMER ID
 * ============================================================================
 * 
 * Timer ID adalah nilai yang bisa di-attach ke software timer.
 * Kegunaan utama:
 *   1. Identifikasi timer di shared callback
 *   2. Menyimpan pointer ke data (context)
 *   3. Counter untuk tracking jumlah trigger
 * 
 * ============================================================================
 * SHARED CALLBACK PATTERN
 * ============================================================================
 * 
 *    TANPA Timer ID - Butuh callback terpisah:
 *    ══════════════════════════════════════════════════════════════════════
 *    
 *    Timer1 ──────► vCallback1()  ──┐
 *    Timer2 ──────► vCallback2()  ──┼──► Banyak fungsi serupa!
 *    Timer3 ──────► vCallback3()  ──┘
 *    
 *    
 *    DENGAN Timer ID - Satu callback untuk semua:
 *    ══════════════════════════════════════════════════════════════════════
 *    
 *    Timer1 (ID=0) ──┐
 *    Timer2 (ID=1) ──┼──► vSharedCallback(xTimer)
 *    Timer3 (ID=2) ──┘           │
 *                                ▼
 *                      pvTimerGetTimerID(xTimer)
 *                                │
 *                    ┌───────────┼───────────┐
 *                    ▼           ▼           ▼
 *                  ID=0        ID=1        ID=2
 *                  Handle      Handle      Handle
 *                  Timer1      Timer2      Timer3
 * 
 * ============================================================================
 * TIMER ID SEBAGAI DATA POINTER
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │  typedef struct {                                                   │
 *    │      const char *name;                                              │
 *    │      uint32_t    count;                                             │
 *    │      GPIO_TypeDef *port;                                            │
 *    │      uint16_t    pin;                                               │
 *    │  } TimerContext_t;                                                  │
 *    │                                                                     │
 *    │  TimerContext_t led1_ctx = { "LED1", 0, GPIOC, GPIO_PIN_13 };       │
 *    │  TimerContext_t led2_ctx = { "LED2", 0, GPIOB, GPIO_PIN_12 };       │
 *    │                                                                     │
 *    │  // Set ID sebagai pointer ke context                               │
 *    │  xTimer1 = xTimerCreate(..., (void*)&led1_ctx, vBlinkCallback);     │
 *    │  xTimer2 = xTimerCreate(..., (void*)&led2_ctx, vBlinkCallback);     │
 *    │                                                                     │
 *    │  void vBlinkCallback(TimerHandle_t xTimer)                          │
 *    │  {                                                                  │
 *    │      // Dapatkan context dari Timer ID                              │
 *    │      TimerContext_t *ctx = (TimerContext_t*)pvTimerGetTimerID(xTimer);│
 *    │      ctx->count++;                                                  │
 *    │      HAL_GPIO_TogglePin(ctx->port, ctx->pin);                       │
 *    │  }                                                                  │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * API TIMER ID
 * ============================================================================
 * 
 *    // Set Timer ID saat create
 *    xTimer = xTimerCreate("Name", period, pdTRUE, 
 *                          (void*)timerID,   // <-- Timer ID
 *                          callback);
 *    
 *    // Get Timer ID di callback
 *    void *pvTimerGetTimerID(TimerHandle_t xTimer);
 *    
 *    // Update Timer ID runtime
 *    void vTimerSetTimerID(TimerHandle_t xTimer, void *pvNewID);
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
#define configUSE_COUNTING_SEMAPHORES           0
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

#define TASK_STACK            256

/* Jumlah timer yang dibuat */
#define NUM_TIMERS            3

#endif /* FREERTOS_CONFIG_H */
