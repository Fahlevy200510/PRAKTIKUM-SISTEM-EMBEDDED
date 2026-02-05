/*
 * =============================================================================
 * PROGRAM 47: PUB/SUB (PUBLISH-SUBSCRIBE) PATTERN
 * =============================================================================
 * 
 * KONSEP DASAR:
 * =============
 * Publish-Subscribe (Pub/Sub) adalah design pattern untuk decoupled
 * communication antara komponen software. Publisher mengirim pesan ke
 * "topic" tanpa perlu tahu siapa yang menerimanya. Subscriber mendaftar
 * ke topic yang diminati dan menerima semua pesan dari topic tersebut.
 * 
 * KEUNTUNGAN PUB/SUB:
 * ===================
 * 1. Loose Coupling: Publisher dan subscriber tidak perlu saling kenal
 * 2. Scalability: Mudah menambah publisher atau subscriber baru
 * 3. Flexibility: Subscriber bisa subscribe ke multiple topics
 * 4. Asynchronous: Publisher tidak perlu menunggu subscriber selesai
 * 
 * ARSITEKTUR SISTEM:
 * ==================
 * 
 *   ┌─────────────────────────────────────────────────────────────────────┐
 *   │                     PUB/SUB SYSTEM ARCHITECTURE                     │
 *   └─────────────────────────────────────────────────────────────────────┘
 *   
 *   PUBLISHERS                      BROKER                    SUBSCRIBERS
 *   ──────────                      ──────                    ───────────
 *   
 *   ┌──────────┐                ┌───────────────┐            ┌──────────┐
 *   │ TEMP     │───►TEMP───────►│               │────────────►│ DISPLAY  │
 *   │ SENSOR   │                │               │            │ TASK     │
 *   └──────────┘                │               │            └──────────┘
 *                               │               │                  │
 *   ┌──────────┐                │    MESSAGE    │            ┌──────────┐
 *   │ HUMIDITY │───►HUMID──────►│    BROKER     │────────────►│ LOGGER   │
 *   │ SENSOR   │                │               │            │ TASK     │
 *   └──────────┘                │   ┌───────┐   │            └──────────┘
 *                               │   │TOPICS │   │                  │
 *   ┌──────────┐                │   ├───────┤   │            ┌──────────┐
 *   │ BUTTON   │───►EVENT──────►│   │ TEMP  │   │────────────►│ ALARM    │
 *   │ HANDLER  │                │   │ HUMID │   │            │ TASK     │
 *   └──────────┘                │   │ EVENT │   │            └──────────┘
 *                               │   │ SYSTEM│   │
 *   ┌──────────┐                │   └───────┘   │            ┌──────────┐
 *   │ SYSTEM   │───►SYSTEM─────►│               │────────────►│ CONTROL  │
 *   │ MONITOR  │                │               │            │ TASK     │
 *   └──────────┘                └───────────────┘            └──────────┘
 * 
 * 
 * SUBSCRIPTION MODEL:
 * ===================
 * 
 *   ┌────────────────────────────────────────────────────────────────────┐
 *   │                      SUBSCRIPTION MATRIX                           │
 *   ├─────────────────┬────────┬────────┬────────┬────────┬─────────────┤
 *   │    SUBSCRIBER   │  TEMP  │ HUMID  │ EVENT  │ SYSTEM │   WILDCARD  │
 *   ├─────────────────┼────────┼────────┼────────┼────────┼─────────────┤
 *   │ Display Task    │   ✓    │   ✓    │        │        │             │
 *   │ Logger Task     │   ✓    │   ✓    │   ✓    │   ✓    │      ✓      │
 *   │ Alarm Task      │   ✓    │        │   ✓    │        │             │
 *   │ Control Task    │   ✓    │   ✓    │        │   ✓    │             │
 *   └─────────────────┴────────┴────────┴────────┴────────┴─────────────┘
 * 
 * 
 * MESSAGE FORMAT:
 * ===============
 * 
 *   PubSubMessage_t:
 *   ┌──────────────────────────────────────────────────────────────────┐
 *   │ Offset │ Field       │ Size   │ Description                     │
 *   ├──────────────────────────────────────────────────────────────────┤
 *   │ 0x00   │ ucTopicId   │ 1 byte │ Topic identifier                │
 *   │ 0x01   │ ucPublisher │ 1 byte │ Publisher ID                    │
 *   │ 0x02   │ ucPriority  │ 1 byte │ Message priority (0-3)          │
 *   │ 0x04   │ ulTimestamp │ 4 byte │ Tick count when published       │
 *   │ 0x08   │ ulData      │ 4 byte │ Message payload                 │
 *   └──────────────────────────────────────────────────────────────────┘
 * 
 * 
 * BROKER OPERATION:
 * =================
 * 
 *   ┌────────────────────────────────────────────────────────────────────┐
 *   │                     MESSAGE FLOW                                   │
 *   └────────────────────────────────────────────────────────────────────┘
 *   
 *   1. PUBLISH:
 *      Publisher ──► Broker Queue ──► Broker Task
 *   
 *   2. DISPATCH:
 *      Broker Task ──► Lookup Subscription List
 *                  ──► For each subscriber:
 *                      ──► Send to subscriber's queue
 *   
 *   3. RECEIVE:
 *      Subscriber Task ◄── Subscriber's Queue
 *   
 *   
 *   SEQUENCE DIAGRAM:
 *   
 *   TempSensor          Broker           Display        Logger
 *       │                  │                │              │
 *       │  publish(TEMP)   │                │              │
 *       ├─────────────────►│                │              │
 *       │                  │  dispatch      │              │
 *       │                  ├───────────────►│              │
 *       │                  │                │              │
 *       │                  ├──────────────────────────────►│
 *       │                  │                │              │
 *       │                  │  (queued)      │              │
 *       │                  │◄───────────────┤              │
 *       │                  │◄──────────────────────────────┤
 *       │                  │                │              │
 * 
 * 
 * TOPIC FILTERING:
 * ================
 * 
 *   ┌────────────────────────────────────────────────────────────────────┐
 *   │                     FILTERING OPTIONS                              │
 *   ├────────────────────────────────────────────────────────────────────┤
 *   │                                                                    │
 *   │  1. EXACT MATCH:                                                   │
 *   │     subscribe(TOPIC_TEMP) ──► Only TEMP messages                   │
 *   │                                                                    │
 *   │  2. WILDCARD:                                                      │
 *   │     subscribe(TOPIC_ALL) ──► All messages from all topics          │
 *   │                                                                    │
 *   │  3. MULTIPLE:                                                      │
 *   │     subscribe(TOPIC_TEMP)                                          │
 *   │     subscribe(TOPIC_HUMID) ──► TEMP and HUMID messages             │
 *   │                                                                    │
 *   └────────────────────────────────────────────────────────────────────┘
 * 
 * 
 * EXPECTED SERIAL OUTPUT:
 * =======================
 * 
 *   === PUB/SUB PATTERN DEMO ===
 *   Creating broker...
 *   Registering subscribers...
 *   Starting publishers...
 *   
 *   [BROKER] Message received: Topic=TEMP, Data=250
 *   [BROKER] Dispatching to 3 subscribers...
 *   [DISPLAY] Temp: 25.0°C
 *   [LOGGER] [TEMP] Value: 250 @ 1000ms
 *   [ALARM] Temp check: 25.0°C - OK
 *   
 *   [BROKER] Message received: Topic=EVENT, Data=1
 *   [BROKER] Dispatching to 2 subscribers...
 *   [LOGGER] [EVENT] Value: 1 @ 1050ms
 *   [ALARM] Event received: Button 1 pressed
 *   
 *   === BROKER STATISTICS ===
 *   Total Published: 45
 *   Total Dispatched: 135
 *   Active Subscribers: 4
 *   Topics: 4
 * 
 * 
 * PENGGUNAAN PRAKTIS:
 * ===================
 * 1. Event-driven architectures
 * 2. IoT sensor networks
 * 3. GUI event handling
 * 4. Logging and monitoring systems
 * 5. Message bus / Event bus
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
 * PUB/SUB CONFIGURATION
 * ==========================================*/
#define PUBSUB_MAX_SUBSCRIBERS                   4
#define PUBSUB_MAX_TOPICS                        4
#define PUBSUB_BROKER_QUEUE_SIZE                 10
#define PUBSUB_SUBSCRIBER_QUEUE_SIZE             5

/* Topic IDs */
#define TOPIC_TEMPERATURE                        0
#define TOPIC_HUMIDITY                           1
#define TOPIC_EVENT                              2
#define TOPIC_SYSTEM                             3
#define TOPIC_ALL                                0xFF  /* Wildcard */

/* Publisher IDs */
#define PUB_TEMP_SENSOR                          0
#define PUB_HUMID_SENSOR                         1
#define PUB_BUTTON                               2
#define PUB_SYSTEM                               3

/* Subscriber IDs */
#define SUB_DISPLAY                              0
#define SUB_LOGGER                               1
#define SUB_ALARM                                2
#define SUB_CONTROL                              3

#endif /* FREERTOS_CONFIG_H */
