/*
 * =============================================================================
 * PROGRAM 45: DATA LOGGER
 * =============================================================================
 * 
 * KONSEP DASAR:
 * =============
 * Data Logger adalah sistem yang mencatat data secara periodik dan menyimpannya
 * untuk analisis di kemudian hari. Dalam FreeRTOS, data logger diimplementasikan
 * dengan producer-consumer pattern di mana task sensor menghasilkan data
 * dan task logger menyimpannya ke storage (dalam demo ini: RAM buffer dan UART).
 * 
 * FITUR UTAMA DATA LOGGER:
 * ========================
 * 1. Circular Buffer: Penyimpanan efisien dengan overwrite data lama
 * 2. Timestamping: Setiap record memiliki timestamp akurat
 * 3. Multi-channel: Bisa log dari berbagai sumber data
 * 4. Configurable Rate: Bisa diatur frekuensi logging
 * 5. Data Export: Bisa export data via UART
 * 
 * ARSITEKTUR SISTEM:
 * ==================
 * 
 *   ┌─────────────────────────────────────────────────────────────────────┐
 *   │                        DATA LOGGER SYSTEM                           │
 *   └─────────────────────────────────────────────────────────────────────┘
 *                                     │
 *   ┌─────────────────────────────────┼─────────────────────────────────┐
 *   │                                 │                                 │
 *   ▼                                 ▼                                 ▼
 * ┌──────────┐                 ┌──────────┐                     ┌──────────┐
 * │ ADC TASK │                 │ COUNTER  │                     │ SYSTEM   │
 * │          │                 │   TASK   │                     │ STATS    │
 * │ Analog   │                 │          │                     │   TASK   │
 * │ Sampling │                 │ Event    │                     │          │
 * │ @ 10Hz   │                 │ Counter  │                     │ Heap/CPU │
 * └────┬─────┘                 └────┬─────┘                     └────┬─────┘
 *      │                            │                                │
 *      │    ┌───────────────────────┼────────────────────────────┐   │
 *      └───►│                                                    │◄──┘
 *           │              LOG QUEUE                             │
 *           │         (LogEntry_t messages)                      │
 *           │                                                    │
 *           └───────────────────────┬────────────────────────────┘
 *                                   │
 *                                   ▼
 *                           ┌──────────────┐
 *                           │    LOGGER    │
 *                           │     TASK     │
 *                           │              │
 *                           │ - Buffer mgmt│
 *                           │ - Write      │
 *                           │ - Export     │
 *                           └──────┬───────┘
 *                                  │
 *                ┌─────────────────┼─────────────────┐
 *                │                 │                 │
 *                ▼                 ▼                 ▼
 *         ┌──────────┐      ┌──────────┐      ┌──────────┐
 *         │ CIRCULAR │      │   UART   │      │  STATUS  │
 *         │  BUFFER  │      │  OUTPUT  │      │   LED    │
 *         │ (RAM)    │      │(Export)  │      │          │
 *         └──────────┘      └──────────┘      └──────────┘
 * 
 * 
 * CIRCULAR BUFFER OPERATION:
 * ==========================
 * 
 *   Buffer Size: 100 entries
 *   Head = Write position
 *   Tail = Read position
 *   
 *   EMPTY STATE:              AFTER 5 WRITES:
 *   ┌───┬───┬───┬───┬───┐     ┌───┬───┬───┬───┬───┬───┐
 *   │   │   │   │   │   │     │ A │ B │ C │ D │ E │   │
 *   └───┴───┴───┴───┴───┘     └───┴───┴───┴───┴───┴───┘
 *   H=T=0                      T=0             H=5
 *   
 *   WRAP AROUND:              OVERWRITE OLD DATA:
 *   ┌───┬───┬───┬───┬───┐     ┌───┬───┬───┬───┬───┐
 *   │ Z │ Y │ C │ D │ E │     │ Z │ Y │ X │ D │ E │
 *   └───┴───┴───┴───┴───┘     └───┴───┴───┴───┴───┘
 *   H=2     T=2                H=3   T=3
 *                              C di-overwrite oleh X
 * 
 * 
 * LOG ENTRY STRUCTURE:
 * ====================
 * 
 *   ┌────────────────────────────────────────────────────────────────┐
 *   │                        LOG ENTRY FORMAT                        │
 *   ├────────────────────────────────────────────────────────────────┤
 *   │ Offset │ Field       │ Size   │ Description                   │
 *   ├────────┼─────────────┼────────┼───────────────────────────────┤
 *   │ 0x00   │ ulTimestamp │ 4 byte │ Tick count (ms since boot)    │
 *   │ 0x04   │ ucChannel   │ 1 byte │ Source channel (0-7)          │
 *   │ 0x05   │ ucType      │ 1 byte │ Data type (ADC/EVENT/SYS)     │
 *   │ 0x06   │ usValue     │ 2 byte │ Actual data value             │
 *   │ 0x08   │ ucFlags     │ 1 byte │ Status flags                  │
 *   └────────┴─────────────┴────────┴───────────────────────────────┘
 *   Total: 12 bytes per entry (with padding)
 * 
 * 
 * DATA EXPORT FORMAT (CSV):
 * =========================
 * 
 *   ┌─────────────────────────────────────────────────────────────────┐
 *   │ TIMESTAMP,CHANNEL,TYPE,VALUE,FLAGS                             │
 *   ├─────────────────────────────────────────────────────────────────┤
 *   │ 1000,0,ADC,2048,0                                               │
 *   │ 1000,1,ADC,1896,0                                               │
 *   │ 1050,2,EVENT,5,0                                                │
 *   │ 2000,3,SYS,8500,0  ← Heap free                                  │
 *   │ 2000,0,ADC,2100,1  ← Flag: threshold exceeded                  │
 *   │ ...                                                             │
 *   └─────────────────────────────────────────────────────────────────┘
 * 
 * 
 * LOGGING TIMELINE:
 * =================
 * 
 *   Time ─────────────────────────────────────────────────────────►
 *   
 *   ADC Task     ║ S ║   ║ S ║   ║ S ║   ║ S ║   ║ S ║   ║ S ║
 *   (100ms)      ╠═══╣   ╠═══╣   ╠═══╣   ╠═══╣   ╠═══╣   ╠═══╣
 *                0  100 200 300 400 500 600 700 800 900 1000 ms
 *   
 *   Event Task   ║   ║  E  ║       ║       E       ║   ║  E  ║
 *   (async)      ║   ╠═════╣       ║       ╠═══════╣   ╠═════╣
 *                0  150   250     400     550     700 800   950
 *   
 *   System Task  ║         S         ║         S         ║
 *   (500ms)      ╠═══════════════════╣═══════════════════╣
 *                0                  500                 1000
 *   
 *   Logger Task  ║W║ ║W║ ║W║ ║W║ ║W║ ║W║ ║W║ ║W║ ║W║ ║W║ ║W║
 *   (50ms)       ╠═╣ ╠═╣ ╠═╣ ╠═╣ ╠═╣ ╠═╣ ╠═╣ ╠═╣ ╠═╣ ╠═╣ ╠═╣
 *   
 *   Legend: S=Sample, E=Event, W=Write to buffer
 * 
 * 
 * BUFFER STATISTICS:
 * ==================
 * 
 *   ┌─────────────────────────────────────────────────────────────────┐
 *   │                      BUFFER STATUS                              │
 *   ├─────────────────────────────────────────────────────────────────┤
 *   │                                                                 │
 *   │  Total Entries: 100                                             │
 *   │  Used: 78        [████████████████████████████████░░░░░░░░░░] 78%│
 *   │  Free: 22                                                       │
 *   │                                                                 │
 *   │  Write Position (Head): 78                                      │
 *   │  Read Position (Tail): 0                                        │
 *   │                                                                 │
 *   │  Total Writes: 578                                              │
 *   │  Overwrites: 478                                                │
 *   │  Dropped: 3 (queue full)                                        │
 *   │                                                                 │
 *   │  Channel Stats:                                                 │
 *   │    CH0 (ADC): 320 entries                                       │
 *   │    CH1 (ADC): 320 entries                                       │
 *   │    CH2 (EVENT): 45 entries                                      │
 *   │    CH3 (SYS): 12 entries                                        │
 *   │                                                                 │
 *   └─────────────────────────────────────────────────────────────────┘
 * 
 * 
 * EXPECTED SERIAL OUTPUT:
 * =======================
 * 
 *   === DATA LOGGER DEMO ===
 *   Initializing circular buffer...
 *   Buffer size: 100 entries (1200 bytes)
 *   Creating tasks...
 *   Starting scheduler...
 *   
 *   [LOG] Entry #1: CH0 ADC=2048 @ 100ms
 *   [LOG] Entry #2: CH1 ADC=1896 @ 100ms
 *   [LOG] Entry #3: CH2 EVENT=1 @ 150ms
 *   [LOG] Entry #4: CH0 ADC=2100 @ 200ms
 *   ...
 *   
 *   === BUFFER STATUS ===
 *   Used: 45/100 (45%)
 *   Total writes: 145
 *   Overwrites: 45
 *   
 *   [EXPORT] Starting data export...
 *   TIMESTAMP,CHANNEL,TYPE,VALUE,FLAGS
 *   100,0,ADC,2048,0
 *   100,1,ADC,1896,0
 *   150,2,EVENT,1,0
 *   ...
 *   [EXPORT] Complete. 45 entries exported.
 * 
 * 
 * PENGGUNAAN PRAKTIS:
 * ===================
 * 1. Environmental Monitoring: Log suhu/kelembaban untuk analisis cuaca
 * 2. Industrial Data Acquisition: Recording sensor produksi
 * 3. Vehicle Black Box: Log parameter kendaraan
 * 4. Medical Devices: Patient vital sign recording
 * 5. Research/Lab Equipment: Experiment data collection
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
 * DATA LOGGER CONFIGURATION
 * ==========================================*/
#define LOG_BUFFER_SIZE                          50      /* Number of entries */
#define LOG_QUEUE_LENGTH                         10      /* Queue size */
#define ADC_SAMPLE_PERIOD_MS                     100     /* ADC sampling rate */
#define SYSTEM_LOG_PERIOD_MS                     1000    /* System stats logging */
#define EXPORT_TRIGGER_ENTRIES                   30      /* Auto export threshold */

/* Channel definitions */
#define LOG_CHANNEL_ADC0                         0
#define LOG_CHANNEL_ADC1                         1
#define LOG_CHANNEL_EVENT                        2
#define LOG_CHANNEL_SYSTEM                       3

/* Log types */
#define LOG_TYPE_ADC                             0
#define LOG_TYPE_EVENT                           1
#define LOG_TYPE_SYSTEM                          2

#endif /* FREERTOS_CONFIG_H */
