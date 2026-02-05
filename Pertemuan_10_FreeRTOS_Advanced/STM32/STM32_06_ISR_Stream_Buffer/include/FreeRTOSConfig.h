/**
 * ============================================================================
 * FILE: FreeRTOSConfig.h
 * PROJECT: 32-ISR_Stream_Buffer
 * 
 * JUDUL: ISR to Task Communication via Stream Buffer
 * 
 * ============================================================================
 * KONSEP ISR STREAM BUFFER
 * ============================================================================
 * 
 * Stream Buffer sangat efisien untuk menerima data dari ISR ke task:
 *   - ISR mengirim data dengan xStreamBufferSendFromISR()
 *   - Task memproses data dengan xStreamBufferReceive()
 *   - Zero-copy transfer (data langsung ke buffer)
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                  ISR-to-TASK COMMUNICATION                          │
 *    │                                                                     │
 *    │    INTERRUPT CONTEXT                    TASK CONTEXT               │
 *    │    ═══════════════════                  ════════════               │
 *    │                                                                     │
 *    │    ┌─────────────────┐                  ┌─────────────────┐         │
 *    │    │   TIM2_IRQHandler│                  │  Receiver Task  │         │
 *    │    │                  │                  │                 │         │
 *    │    │  Read sensor    │                  │  Wait for data  │         │
 *    │    │  xStreamBuffer- │    STREAM        │  Process batch  │         │
 *    │    │  SendFromISR()  │    BUFFER        │  xStreamBuffer- │         │
 *    │    │        │        │   ┌──────┐       │  Receive()      │         │
 *    │    │        ├───────────►│▓▓▓▓▓▓├──────────►│               │         │
 *    │    │        │        │   └──────┘       │        │        │         │
 *    │    │  Yield if higher │                  │  Process data  │         │
 *    │    │  priority ready │                  │                 │         │
 *    │    └─────────────────┘                  └─────────────────┘         │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * API ISR-SAFE
 * ============================================================================
 * 
 *    // Send from ISR
 *    size_t xStreamBufferSendFromISR(
 *        StreamBufferHandle_t xStreamBuffer,
 *        const void *pvTxData,
 *        size_t xDataLengthBytes,
 *        BaseType_t *pxHigherPriorityTaskWoken
 *    );
 *    
 *    // PENTING: Check pxHigherPriorityTaskWoken dan yield jika perlu!
 *    portYIELD_FROM_ISR(*pxHigherPriorityTaskWoken);
 * 
 * ============================================================================
 * TYPICAL USE CASE: UART RX
 * ============================================================================
 * 
 *    UART_IRQHandler() {
 *        uint8_t byte = USART->DR;  // Read received byte
 *        
 *        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
 *        xStreamBufferSendFromISR(xRxBuffer, &byte, 1, 
 *                                  &xHigherPriorityTaskWoken);
 *        
 *        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
 *    }
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

/* ISR Stream Buffer config */
#define ISR_BUFFER_SIZE       64
#define TRIGGER_LEVEL         8    /* Process when 8 bytes available */

/* Timer config */
#define TIM2_PRESCALER        (7200 - 1)   /* 72MHz / 7200 = 10kHz */
#define TIM2_PERIOD           (100 - 1)    /* 10kHz / 100 = 100Hz (10ms) */

#define TASK_STACK            256

#endif /* FREERTOS_CONFIG_H */
