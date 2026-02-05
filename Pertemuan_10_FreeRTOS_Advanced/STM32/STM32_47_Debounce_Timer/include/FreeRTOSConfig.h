/**
 * ============================================================================
 * FILE: FreeRTOSConfig.h
 * PROJECT: 23-Debounce_Timer
 * 
 * JUDUL: Debounce Button dengan One-Shot Timer
 * 
 * ============================================================================
 * KONSEP DEBOUNCE
 * ============================================================================
 * 
 * MASALAH BOUNCE pada Switch/Button:
 * 
 *    Ketika button ditekan, kontak fisik tidak langsung stabil.
 *    Terjadi beberapa bounce (oscillation) sebelum stabil.
 *    
 *    Tanpa Debounce:
 *    ══════════════════════════════════════════════════════════════════════
 *    
 *    Physical button press:
 *    
 *    VCC ─────┐     ┌─┐ ┌─┐ ┌────────────────────
 *             │     │ │ │ │ │
 *    GND      └─────┘ └─┘ └─┘
 *                   ↑
 *                Bounce region (~5-50ms)
 *    
 *    Software sees:
 *      Press #1  │  Press #2  │  Press #3  │  Stable press
 *      ─────────►│───────────►│───────────►│──────────────►
 *    
 *    HASIL: 1 tekan fisik = 3+ interrupt!
 * 
 * ============================================================================
 * SOLUSI: ONE-SHOT TIMER DEBOUNCE
 * ============================================================================
 * 
 *    Prinsip:
 *    - Saat edge pertama detected, start one-shot timer
 *    - Abaikan semua edge selama timer berjalan (debounce period)
 *    - Setelah timer expire, baca state final button
 *    
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                                                                     │
 *    │   Button Press           Bounce                 Stable              │
 *    │        │                  │ │                     │                 │
 *    │        ▼                  ▼ ▼                     ▼                 │
 *    │   ─────┬───────────────────┬───────────────────────────────►        │
 *    │        │                   │                                        │
 *    │        │◄─── Debounce ────►│                                        │
 *    │        │     Timer (50ms)  │                                        │
 *    │        │                   │                                        │
 *    │        │                   ▼                                        │
 *    │        │           Timer Callback                                   │
 *    │        │           Read button state                                │
 *    │        │           Process if still pressed                         │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 *    
 *    Timeline dengan xTimerReset():
 *    ══════════════════════════════════════════════════════════════════════
 *    
 *    Edge1   Edge2   Edge3              Callback
 *      │       │       │                    │
 *      ▼       ▼       ▼                    ▼
 *    ──┼───────┼───────┼────────────────────┼──────────►
 *      │       │       │                    │
 *      │       │       │◄───── 50ms ───────►│
 *      │       │                            │
 *      │       │ xTimerReset() - restart!   │
 *      │                                    │
 *      │ xTimerReset() - restart!           │
 *      │◄───────────────────────────────────┤
 *      Timer start                     Only triggered once!
 * 
 * ============================================================================
 * IMPLEMENTASI
 * ============================================================================
 * 
 *    // ISR saat button interrupt
 *    void EXTI_IRQHandler(void)
 *    {
 *        // Reset timer - restart debounce period
 *        xTimerResetFromISR(xDebounceTimer, &xHigherPriorityTaskWoken);
 *        
 *        // JANGAN process button di sini!
 *    }
 *    
 *    // Timer callback - setelah debounce period
 *    void vDebounceCallback(TimerHandle_t xTimer)
 *    {
 *        // Baca state button sekarang
 *        if(HAL_GPIO_ReadPin(BTN_PORT, BTN_PIN) == GPIO_PIN_RESET)
 *        {
 *            // Button masih ditekan - valid press!
 *            ProcessButtonPress();
 *        }
 *    }
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

/* Simulasi button dengan ADC threshold */
#define ADC_BUTTON_PORT     GPIOA
#define ADC_BUTTON_PIN      GPIO_PIN_0

#define DEBUG_UART_INSTANCE   USART1
#define DEBUG_UART_BAUDRATE   115200
#define DEBUG_UART_TX_PORT    GPIOA
#define DEBUG_UART_TX_PIN     GPIO_PIN_9
#define DEBUG_UART_RX_PORT    GPIOA
#define DEBUG_UART_RX_PIN     GPIO_PIN_10

/* Debounce period in ms */
#define DEBOUNCE_PERIOD_MS    50

/* Polling period untuk simulasi (tanpa EXTI) */
#define POLL_PERIOD_MS        10

#define TASK_STACK            256

#endif /* FREERTOS_CONFIG_H */
