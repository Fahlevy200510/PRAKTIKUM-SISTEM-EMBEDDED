/*
 * =============================================================================
 * PROGRAM 48: ACTIVE OBJECT PATTERN
 * =============================================================================
 * 
 * KONSEP DASAR:
 * =============
 * Active Object adalah design pattern concurrency yang mengenkapsulasi
 * thread-nya sendiri beserta execution context-nya. Setiap Active Object
 * memiliki event queue dan menjalankan event handling dalam konteks
 * thread-nya sendiri, sehingga tidak perlu sinkronisasi eksternal.
 * 
 * KEUNTUNGAN ACTIVE OBJECT:
 * =========================
 * 1. Encapsulation: Object mengontrol eksekusi sendiri
 * 2. No Shared State: Tidak perlu mutex untuk state internal
 * 3. Asynchronous: Caller tidak di-block saat mengirim event
 * 4. Predictable: Satu thread per object, mudah di-debug
 * 5. Scalable: Mudah menambah Active Object baru
 * 
 * ARSITEKTUR ACTIVE OBJECT:
 * =========================
 * 
 *   ┌─────────────────────────────────────────────────────────────────────┐
 *   │                    ACTIVE OBJECT STRUCTURE                          │
 *   └─────────────────────────────────────────────────────────────────────┘
 *   
 *   ┌─────────────────────────────────────────────────────────────────────┐
 *   │                      ACTIVE OBJECT                                  │
 *   │  ┌───────────────────────────────────────────────────────────────┐  │
 *   │  │                    EVENT QUEUE                                │  │
 *   │  │   ┌─────┬─────┬─────┬─────┬─────┐                            │  │
 *   │  │   │ E1  │ E2  │ E3  │ E4  │ ... │                            │  │
 *   │  │   └─────┴─────┴─────┴─────┴─────┘                            │  │
 *   │  └───────────────────────────┬───────────────────────────────────┘  │
 *   │                              │                                      │
 *   │                              ▼                                      │
 *   │  ┌───────────────────────────────────────────────────────────────┐  │
 *   │  │                  PRIVATE THREAD                               │  │
 *   │  │                                                               │  │
 *   │  │   while (1) {                                                 │  │
 *   │  │       event = receive_from_queue();                           │  │
 *   │  │       dispatch(event);  // No mutex needed!                   │  │
 *   │  │   }                                                           │  │
 *   │  │                                                               │  │
 *   │  └───────────────────────────────────────────────────────────────┘  │
 *   │                              │                                      │
 *   │                              ▼                                      │
 *   │  ┌───────────────────────────────────────────────────────────────┐  │
 *   │  │                  PRIVATE STATE                                │  │
 *   │  │                                                               │  │
 *   │  │   - Internal variables                                        │  │
 *   │  │   - State machine state                                       │  │
 *   │  │   - Accumulated data                                          │  │
 *   │  │                                                               │  │
 *   │  └───────────────────────────────────────────────────────────────┘  │
 *   └─────────────────────────────────────────────────────────────────────┘
 * 
 * 
 * CONTOH APLIKASI: LED CONTROLLER
 * ================================
 * 
 *   ┌─────────────────────────────────────────────────────────────────────┐
 *   │                   LED CONTROLLER ACTIVE OBJECT                      │
 *   └─────────────────────────────────────────────────────────────────────┘
 *   
 *                           ┌──────────────┐
 *          BUTTON_PRESS ───►│              │
 *          TIMER_TICK   ───►│  LED CTRL    │───► LED ON/OFF
 *          SET_MODE     ───►│  Active Obj  │───► Status Report
 *          GET_STATUS   ───►│              │
 *                           └──────────────┘
 *   
 *   INTERNAL STATE MACHINE:
 *   
 *              ┌─────────────────────────────────────────┐
 *              │                                         │
 *              ▼                                         │
 *         ┌─────────┐    TIMER_TICK     ┌─────────┐     │
 *         │  OFF    │──────────────────►│ BLINK   │─────┘
 *         │         │◄──────────────────│   ON    │
 *         └────┬────┘    BUTTON_PRESS   └────┬────┘
 *              │                             │
 *   BUTTON_PRESS                        TIMER_TICK
 *              │                             │
 *              ▼                             ▼
 *         ┌─────────┐                  ┌─────────┐
 *         │ SOLID   │◄─────────────────│ BLINK   │
 *         │   ON    │   BUTTON_PRESS   │   OFF   │
 *         └─────────┘                  └─────────┘
 * 
 * 
 * EVENT DISPATCHING:
 * ==================
 * 
 *   ┌────────────────────────────────────────────────────────────────────┐
 *   │                     EVENT DISPATCH FLOW                            │
 *   └────────────────────────────────────────────────────────────────────┘
 *   
 *   Client                Queue                    Active Object
 *      │                    │                           │
 *      │  post(EVENT)       │                           │
 *      ├───────────────────►│                           │
 *      │  (non-blocking)    │                           │
 *      │◄───────────────────┤                           │
 *      │  return            │                           │
 *      │                    │   receive()               │
 *      │                    ├──────────────────────────►│
 *      │                    │                           │
 *      │                    │   dispatch(event)         │
 *      │                    │                    ┌──────┴──────┐
 *      │                    │                    │  Process    │
 *      │                    │                    │  Event      │
 *      │                    │                    │  (no mutex) │
 *      │                    │                    └──────┬──────┘
 *      │                    │                           │
 *      │                    │   wait for next           │
 *      │                    │◄──────────────────────────┤
 *      │                    │                           │
 * 
 * 
 * MULTIPLE ACTIVE OBJECTS:
 * ========================
 * 
 *   ┌──────────────┐     ┌──────────────┐     ┌──────────────┐
 *   │    LED       │     │   BUTTON     │     │   TEMP       │
 *   │ Controller   │     │  Monitor     │     │  Monitor     │
 *   │              │     │              │     │              │
 *   │  ┌────────┐  │     │  ┌────────┐  │     │  ┌────────┐  │
 *   │  │ Queue  │  │     │  │ Queue  │  │     │  │ Queue  │  │
 *   │  └───┬────┘  │     │  └───┬────┘  │     │  └───┬────┘  │
 *   │      │       │     │      │       │     │      │       │
 *   │  ┌───▼────┐  │     │  ┌───▼────┐  │     │  ┌───▼────┐  │
 *   │  │ Thread │  │     │  │ Thread │  │     │  │ Thread │  │
 *   │  └────────┘  │     │  └────────┘  │     │  └────────┘  │
 *   │              │     │              │     │              │
 *   │  ┌────────┐  │     │  ┌────────┐  │     │  ┌────────┐  │
 *   │  │ State  │  │     │  │ State  │  │     │  │ State  │  │
 *   │  └────────┘  │     │  └────────┘  │     │  └────────┘  │
 *   └──────────────┘     └──────────────┘     └──────────────┘
 *           │                   │                    │
 *           │                   │                    │
 *           └───────────────────┴────────────────────┘
 *                               │
 *                    (Communication via events)
 * 
 * 
 * EVENT STRUCTURE:
 * ================
 * 
 *   AO_Event_t:
 *   ┌──────────────────────────────────────────────────────────────────┐
 *   │ Offset │ Field       │ Size   │ Description                     │
 *   ├──────────────────────────────────────────────────────────────────┤
 *   │ 0x00   │ eSignal     │ 4 byte │ Event type (enum)               │
 *   │ 0x04   │ ulParam1    │ 4 byte │ Event parameter 1               │
 *   │ 0x08   │ ulParam2    │ 4 byte │ Event parameter 2               │
 *   │ 0x0C   │ pxSender    │ 4 byte │ Sender Active Object (optional) │
 *   └──────────────────────────────────────────────────────────────────┘
 * 
 * 
 * EXPECTED SERIAL OUTPUT:
 * =======================
 * 
 *   === ACTIVE OBJECT PATTERN DEMO ===
 *   Creating Active Objects...
 *   
 *   [LED_AO] Created, initial state: OFF
 *   [BTN_AO] Created, monitoring started
 *   [TEMP_AO] Created, sampling started
 *   
 *   [BTN_AO] Button press detected
 *   [BTN_AO] Posting BUTTON_PRESS to LED_AO
 *   [LED_AO] Event: BUTTON_PRESS
 *   [LED_AO] State: OFF -> SOLID_ON
 *   [LED_AO] LED ON
 *   
 *   [TEMP_AO] Temperature: 25.3°C
 *   [TEMP_AO] Posting TEMP_UPDATE to LED_AO
 *   [LED_AO] Event: TEMP_UPDATE (253)
 *   [LED_AO] Temperature in range, no action
 *   
 *   [BTN_AO] Button press detected
 *   [LED_AO] Event: BUTTON_PRESS
 *   [LED_AO] State: SOLID_ON -> BLINK
 *   [LED_AO] Blink mode started (500ms)
 *   
 *   === AO STATISTICS ===
 *   LED_AO: 15 events processed
 *   BTN_AO: 8 events processed
 *   TEMP_AO: 20 events processed
 * 
 * 
 * PERBANDINGAN DENGAN THREAD BIASA:
 * =================================
 * 
 *   REGULAR THREAD:               ACTIVE OBJECT:
 *   ───────────────               ──────────────
 *   - Shared state perlu mutex    - No shared state
 *   - Complex synchronization     - Simple event-based
 *   - Race conditions possible    - No race conditions
 *   - Harder to debug             - Easy to debug
 *   - Blocking calls              - Async event posting
 * 
 * 
 * PENGGUNAAN PRAKTIS:
 * ===================
 * 1. GUI event handling (Qt, GTK)
 * 2. Actor model (Erlang, Akka)
 * 3. Game engines (Unity ECS)
 * 4. Embedded systems with state machines
 * 5. Telecommunications (protocol handlers)
 * 
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* STM32F103 @ 72MHz */
#define configCPU_CLOCK_HZ                       72000000UL
#define configTICK_RATE_HZ                       1000
#define configSYSTICK_CLOCK_HZ                   configCPU_CLOCK_HZ

/* Scheduler */
#define configUSE_PREEMPTION                     1
#define configUSE_TIME_SLICING                   1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION  0

/* Memory */
#define configMINIMAL_STACK_SIZE                 128
#define configTOTAL_HEAP_SIZE                    10240
#define configMAX_TASK_NAME_LEN                  12

/* Task configuration */
#define configMAX_PRIORITIES                     5
#define configIDLE_SHOULD_YIELD                  1

/* Feature flags */
#define configUSE_MUTEXES                        1
#define configUSE_COUNTING_SEMAPHORES            1
#define configUSE_RECURSIVE_MUTEXES              0
#define configUSE_QUEUE_SETS                     0
#define configUSE_TASK_NOTIFICATIONS             1
#define configUSE_TIMERS                         1
#define configUSE_16_BIT_TICKS                   0

/* Timer configuration */
#define configTIMER_TASK_PRIORITY                3
#define configTIMER_QUEUE_LENGTH                 5
#define configTIMER_TASK_STACK_DEPTH             128

/* Hook functions */
#define configUSE_IDLE_HOOK                      0
#define configUSE_TICK_HOOK                      0
#define configUSE_MALLOC_FAILED_HOOK             1
#define configCHECK_FOR_STACK_OVERFLOW           2

/* Debug/Stats */
#define configUSE_TRACE_FACILITY                 1
#define configGENERATE_RUN_TIME_STATS            0

/* Co-routines disabled */
#define configUSE_CO_ROUTINES                    0

/* API functions */
#define INCLUDE_vTaskPrioritySet                 1
#define INCLUDE_uxTaskPriorityGet                1
#define INCLUDE_vTaskDelete                      1
#define INCLUDE_vTaskSuspend                     1
#define INCLUDE_vTaskDelayUntil                  1
#define INCLUDE_vTaskDelay                       1
#define INCLUDE_xTaskGetSchedulerState           1
#define INCLUDE_xTaskGetCurrentTaskHandle        1
#define INCLUDE_uxTaskGetStackHighWaterMark      1

/* ARM Cortex-M3 specific */
#ifdef __NVIC_PRIO_BITS
    #define configPRIO_BITS __NVIC_PRIO_BITS
#else
    #define configPRIO_BITS 4
#endif

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY      15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5
#define configKERNEL_INTERRUPT_PRIORITY         (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/* Interrupt handlers */
#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

/* Assertion */
#define configASSERT(x) if((x) == 0) { taskDISABLE_INTERRUPTS(); for(;;); }

/* ==========================================
 * ACTIVE OBJECT CONFIGURATION
 * ==========================================*/
#define AO_MAX_OBJECTS                           4
#define AO_QUEUE_SIZE                            8
#define AO_STACK_SIZE                            (configMINIMAL_STACK_SIZE + 50)

/* LED Controller States */
#define LED_STATE_OFF                            0
#define LED_STATE_SOLID_ON                       1
#define LED_STATE_BLINK                          2

/* Blink period in ms */
#define LED_BLINK_PERIOD_MS                      500

#endif /* FREERTOS_CONFIG_H */
