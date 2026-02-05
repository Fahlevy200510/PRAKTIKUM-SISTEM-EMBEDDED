/**
 * ============================================================================
 * FILE: FreeRTOSConfig.h
 * PROJECT: 27-Critical_Section_ISR
 * 
 * JUDUL: Critical Section dan Interrupt Masking
 * 
 * ============================================================================
 * KONSEP CRITICAL SECTION
 * ============================================================================
 * 
 * Critical Section adalah bagian kode yang TIDAK BOLEH diinterupsi.
 * Digunakan untuk protect akses ke shared resources.
 * 
 *    MASALAH: Race Condition
 *    ══════════════════════════════════════════════════════════════════════
 *    
 *    Shared variable: counter = 5
 *    
 *    Task A:                  ISR:
 *    ─────────────────        ─────────────────
 *    temp = counter;          // ISR happens here!
 *    // temp = 5              counter = counter + 10;
 *    temp = temp + 1;         // counter = 15
 *    // temp = 6              // ISR returns
 *    counter = temp;          
 *    // counter = 6  SALAH! Harusnya 16!
 * 
 * ============================================================================
 * JENIS CRITICAL SECTION
 * ============================================================================
 * 
 *    1. taskENTER_CRITICAL() / taskEXIT_CRITICAL()
 *       - Disable semua interrupt sampai configMAX_SYSCALL_INTERRUPT_PRIORITY
 *       - Untuk TASK context
 *       - Bisa nested (counter-based)
 *       
 *    2. taskENTER_CRITICAL_FROM_ISR() / taskEXIT_CRITICAL_FROM_ISR()
 *       - Untuk ISR context
 *       - Return value harus disimpan!
 *       
 *    3. vTaskSuspendAll() / xTaskResumeAll()
 *       - Suspend scheduler saja (interrupt tetap enabled)
 *       - Lebih ringan, tapi ISR bisa preempt
 * 
 * ============================================================================
 * ILUSTRASI taskENTER/EXIT_CRITICAL
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                                                                     │
 *    │   TANPA Critical Section:                                           │
 *    │   ──────────────────────────────────────────────────────────────    │
 *    │                                                                     │
 *    │   Task ───┬───────────────────────────────────────────────────►     │
 *    │           │                                                         │
 *    │   ISR     │     ┌───────┐                                           │
 *    │           └────►│  ISR  │──────────────────────────────────────►    │
 *    │                 └───────┘                                           │
 *    │                 ↑        ↑                                          │
 *    │             Preempts   Corrupts shared data!                        │
 *    │                                                                     │
 *    │                                                                     │
 *    │   DENGAN Critical Section:                                          │
 *    │   ──────────────────────────────────────────────────────────────    │
 *    │                                                                     │
 *    │   Task ───┬─[CRITICAL SECTION]──────┬────────────────────────►      │
 *    │           │                         │                               │
 *    │   ISR     │ (IRQ disabled)          │     ┌───────┐                 │
 *    │           │ ISR PENDING!            └────►│  ISR  │───────────►     │
 *    │           │                               └───────┘                 │
 *    │                                           ↑                         │
 *    │                                       ISR runs AFTER critical       │
 *    │                                       section ends                  │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * DARI ISR CONTEXT
 * ============================================================================
 * 
 *    // Di ISR - WAJIB simpan return value!
 *    UBaseType_t uxSavedInterruptStatus;
 *    uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
 *    {
 *        // Critical section code
 *        shared_data++;
 *    }
 *    taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
 * 
 * ============================================================================
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

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

#define configSUPPORT_STATIC_ALLOCATION         0
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configTOTAL_HEAP_SIZE                   ((size_t)10240)

#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configCHECK_FOR_STACK_OVERFLOW          2
#define configUSE_MALLOC_FAILED_HOOK            1

#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            256

#ifdef __NVIC_PRIO_BITS
    #define configPRIO_BITS __NVIC_PRIO_BITS
#else
    #define configPRIO_BITS 4
#endif

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5
#define configKERNEL_INTERRUPT_PRIORITY         (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

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

#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#define LED_GPIO_PORT       GPIOC
#define LED_GPIO_PIN        GPIO_PIN_13

#define DEBUG_UART_INSTANCE   USART1
#define DEBUG_UART_BAUDRATE   115200
#define DEBUG_UART_TX_PORT    GPIOA
#define DEBUG_UART_TX_PIN     GPIO_PIN_9
#define DEBUG_UART_RX_PORT    GPIOA
#define DEBUG_UART_RX_PIN     GPIO_PIN_10

#define TASK_STACK            256

#endif /* FREERTOS_CONFIG_H */
