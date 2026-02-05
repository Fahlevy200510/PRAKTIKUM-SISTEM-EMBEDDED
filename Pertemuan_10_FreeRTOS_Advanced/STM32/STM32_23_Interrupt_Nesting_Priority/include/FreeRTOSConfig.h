/**
 * ============================================================================
 * FILE: FreeRTOSConfig.h
 * PROJECT: 25-Interrupt_Nesting_Priority
 * 
 * JUDUL: Interrupt Nesting dan Priority pada ARM Cortex-M3
 * 
 * ============================================================================
 * KONSEP INTERRUPT PRIORITY CORTEX-M3
 * ============================================================================
 * 
 * NVIC (Nested Vectored Interrupt Controller):
 * - Mendukung interrupt nesting (interrupt dalam interrupt)
 * - Priority-based preemption
 * - STM32F103: 4 bit priority → 16 level (0-15)
 * - LOWER number = HIGHER priority
 * 
 *    Priority Level:    0    1    2    3    4    5 ... 15
 *    Precedence:       HIGHEST ─────────────────► LOWEST
 * 
 * ============================================================================
 * INTERRUPT NESTING
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                                                                     │
 *    │   ISR dengan Priority 6 sedang berjalan:                           │
 *    │                                                                     │
 *    │   Priority 3 interrupt? ──► PREEMPT! (higher priority)              │
 *    │   Priority 6 interrupt? ──► WAIT (same priority)                    │
 *    │   Priority 9 interrupt? ──► WAIT (lower priority)                   │
 *    │                                                                     │
 *    │   ┌───────────────────────────────────────────────────────────────┐ │
 *    │   │                                                               │ │
 *    │   │  Main ─────┐                                                  │ │
 *    │   │            │ ISR Pri-6                                        │ │
 *    │   │            └───────┐                                          │ │
 *    │   │                    │ ISR Pri-3 (preempts!)                    │ │
 *    │   │                    └──────┬───┘                               │ │
 *    │   │                           │   ISR Pri-6 resumes               │ │
 *    │   │            ┌──────────────┘                                   │ │
 *    │   │            │ ISR Pri-6 completes                              │ │
 *    │   │  ──────────┘                                                  │ │
 *    │   │  Main resumes                                                 │ │
 *    │   │                                                               │ │
 *    │   └───────────────────────────────────────────────────────────────┘ │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * FreeRTOS INTERRUPT SAFE API
 * ============================================================================
 * 
 * configMAX_SYSCALL_INTERRUPT_PRIORITY:
 * Menentukan batas priority untuk memanggil FreeRTOS API dari ISR.
 * 
 *    Priority:     0  1  2  3  4  [5]  6  7  8  9  10  11  12  13  14  15
 *                  ↑              ↑
 *                  │              │
 *                  │    configMAX_SYSCALL_INTERRUPT_PRIORITY = 5
 *                  │              │
 *                  │              ├─────────────────────────────────────────┐
 *                  │              │ ISR bisa panggil FreeRTOS FromISR API   │
 *                  │              │ (xQueueSendFromISR, xSemaphoreGiveFromISR, dll)
 *                  │              └─────────────────────────────────────────┘
 *                  │
 *                  ├─────────────────────────────────────────────────────────┐
 *                  │ ISR dengan priority 0-4:                                │
 *                  │ - TIDAK BOLEH panggil FreeRTOS API!                     │
 *                  │ - Digunakan untuk super critical interrupt              │
 *                  │ - Contoh: Motor control, ADC sampling sangat cepat     │
 *                  └─────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * KONFIGURASI PRIORITY
 * ============================================================================
 * 
 *    STM32F103 menggunakan 4 bit priority (0-15)
 *    FreeRTOS menggunakan format 8-bit, shift left.
 *    
 *    configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY = 5
 *    → configMAX_SYSCALL_INTERRUPT_PRIORITY = 5 << 4 = 80 (0x50)
 *    
 *    Saat set NVIC priority untuk ISR yang memanggil FreeRTOS:
 *    NVIC_SetPriority(USART1_IRQn, 5);  // OK - >= 5
 *    NVIC_SetPriority(TIM2_IRQn, 6);    // OK
 *    NVIC_SetPriority(ADC_IRQn, 3);     // BAHAYA jika panggil FreeRTOS API!
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
 * 
 * PENTING: Konfigurasi priority interrupt!
 * ============================================================================ */

#ifdef __NVIC_PRIO_BITS
    #define configPRIO_BITS __NVIC_PRIO_BITS
#else
    #define configPRIO_BITS 4
#endif

/*
 * configLIBRARY_LOWEST_INTERRUPT_PRIORITY = 15
 * Priority terendah (IDLE level) untuk interrupt
 */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         15

/*
 * configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY = 5
 * 
 * ISR dengan priority 5-15 BOLEH panggil FreeRTOS FromISR API
 * ISR dengan priority 0-4 TIDAK BOLEH panggil FreeRTOS API
 */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5

/* Konversi ke format 8-bit untuk NVIC */
#define configKERNEL_INTERRUPT_PRIORITY         (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/* ============================================================================
 * ASSERT CONFIGURATION
 * ============================================================================ */

#define configASSERT( x ) if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for(;;); }

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

/* Interrupt priorities untuk demo */
#define HIGH_PRIORITY_INT     6   /* Bisa panggil FreeRTOS API */
#define LOW_PRIORITY_INT      10  /* Bisa panggil FreeRTOS API */

#define TASK_STACK            256

#endif /* FREERTOS_CONFIG_H */
