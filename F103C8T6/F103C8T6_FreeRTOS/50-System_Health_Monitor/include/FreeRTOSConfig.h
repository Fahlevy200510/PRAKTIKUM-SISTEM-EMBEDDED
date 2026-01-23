/*
 * =============================================================================
 * PROGRAM 50: SYSTEM HEALTH MONITOR
 * =============================================================================
 * 
 * KONSEP DASAR:
 * =============
 * System Health Monitor adalah komponen yang mengawasi berbagai aspek
 * kesehatan sistem embedded secara real-time. Ini mencakup monitoring
 * resource usage (RAM, CPU), task health, peripheral status, dan
 * menyediakan diagnostic capabilities.
 * 
 * ASPEK YANG DIMONITOR:
 * =====================
 * 1. Memory Health: Heap usage, fragmentation, allocation failures
 * 2. Task Health: Stack watermarks, execution times, deadlock detection
 * 3. System Resources: CPU load, queue fill levels, timer health
 * 4. Peripheral Status: UART, GPIO, ADC, timers
 * 5. Power/Thermal: Voltage levels, temperature (if available)
 * 
 * ARSITEKTUR SISTEM:
 * ==================
 * 
 *   ┌─────────────────────────────────────────────────────────────────────┐
 *   │                    SYSTEM HEALTH MONITOR                            │
 *   └─────────────────────────────────────────────────────────────────────┘
 *   
 *   ┌───────────────────────────────────────────────────────────────────┐
 *   │                        DATA COLLECTORS                            │
 *   │                                                                   │
 *   │  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐         │
 *   │  │ MEMORY   │  │  TASK    │  │ RESOURCE │  │PERIPHERAL│         │
 *   │  │ MONITOR  │  │ MONITOR  │  │ MONITOR  │  │ MONITOR  │         │
 *   │  │          │  │          │  │          │  │          │         │
 *   │  │ -Heap    │  │ -Stack   │  │ -Queues  │  │ -UART    │         │
 *   │  │ -Alloc   │  │ -State   │  │ -Timers  │  │ -GPIO    │         │
 *   │  │ -Frag    │  │ -Timing  │  │ -CPU     │  │ -ADC     │         │
 *   │  └────┬─────┘  └────┬─────┘  └────┬─────┘  └────┬─────┘         │
 *   │       │             │             │             │                │
 *   └───────┼─────────────┼─────────────┼─────────────┼────────────────┘
 *           │             │             │             │
 *           └─────────────┴─────────────┴─────────────┘
 *                                │
 *                                ▼
 *           ┌────────────────────────────────────────┐
 *           │          HEALTH AGGREGATOR             │
 *           │                                        │
 *           │  ┌────────────────────────────────┐    │
 *           │  │     OVERALL HEALTH SCORE       │    │
 *           │  │                                │    │
 *           │  │   0%        50%        100%    │    │
 *           │  │   │▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░│       │    │
 *           │  │            78%                 │    │
 *           │  └────────────────────────────────┘    │
 *           │                                        │
 *           │  Component Scores:                     │
 *           │  • Memory:     85%  ✓                  │
 *           │  • Tasks:      90%  ✓                  │
 *           │  • Resources:  65%  ⚠                  │
 *           │  • Peripherals: 72% ✓                  │
 *           │                                        │
 *           └─────────────────────────────────┬──────┘
 *                                             │
 *                ┌────────────────────────────┼───────────────────────┐
 *                │                            │                       │
 *                ▼                            ▼                       ▼
 *         ┌──────────┐               ┌──────────┐              ┌──────────┐
 *         │  UART    │               │   LED    │              │  ALARM   │
 *         │ REPORT   │               │ STATUS   │              │ HANDLER  │
 *         └──────────┘               └──────────┘              └──────────┘
 * 
 * 
 * HEALTH SCORE CALCULATION:
 * =========================
 * 
 *   ┌────────────────────────────────────────────────────────────────────┐
 *   │                   HEALTH SCORE ALGORITHM                           │
 *   ├────────────────────────────────────────────────────────────────────┤
 *   │                                                                    │
 *   │  Memory Score (25% weight):                                        │
 *   │    = 100 - (heapUsed / heapTotal × 100)                           │
 *   │    Penalty: -10% if near threshold, -25% if critical              │
 *   │                                                                    │
 *   │  Task Score (30% weight):                                          │
 *   │    = Σ(taskHealthScore) / numTasks                                 │
 *   │    Where taskHealthScore = 100 - (stackUsed / stackTotal × 100)   │
 *   │    Penalty: -20% per task in Blocked/Suspended too long           │
 *   │                                                                    │
 *   │  Resource Score (25% weight):                                      │
 *   │    = Average of queue fill scores + timer health                   │
 *   │    QueueScore = 100 - (itemsInQueue / queueSize × 100)            │
 *   │                                                                    │
 *   │  Peripheral Score (20% weight):                                    │
 *   │    = Σ(peripheralStatus) / numPeripherals                          │
 *   │    Where: OK=100, Degraded=50, Failed=0                           │
 *   │                                                                    │
 *   │  OVERALL = Mem×0.25 + Task×0.30 + Res×0.25 + Periph×0.20          │
 *   │                                                                    │
 *   └────────────────────────────────────────────────────────────────────┘
 * 
 * 
 * HEALTH STATUS LEVELS:
 * =====================
 * 
 *   ┌──────────────────────────────────────────────────────────────────┐
 *   │ Score     │ Status    │ LED       │ Action                      │
 *   ├───────────┼───────────┼───────────┼─────────────────────────────┤
 *   │ 80-100%   │ HEALTHY   │ Solid ON  │ Normal operation            │
 *   │ 60-79%    │ DEGRADED  │ Slow blink│ Monitor closely             │
 *   │ 40-59%    │ WARNING   │ Fast blink│ Take preventive action      │
 *   │ 0-39%     │ CRITICAL  │ Very fast │ Immediate intervention      │
 *   └───────────┴───────────┴───────────┴─────────────────────────────┘
 * 
 * 
 * DASHBOARD DISPLAY:
 * ==================
 * 
 *   ╔════════════════════════════════════════════════════════════════╗
 *   ║              SYSTEM HEALTH MONITOR DASHBOARD                   ║
 *   ╠════════════════════════════════════════════════════════════════╣
 *   ║                                                                ║
 *   ║  OVERALL HEALTH: [████████████████████░░░░] 82% - HEALTHY     ║
 *   ║                                                                ║
 *   ╠════════════════════════════════════════════════════════════════╣
 *   ║  MEMORY                          │  TASKS                      ║
 *   ║  ────────────────────────────────│──────────────────────────── ║
 *   ║  Heap Used:  4520/10240 (44%)    │  Active: 5                  ║
 *   ║  Heap Free:  5720 bytes          │  Blocked: 1                 ║
 *   ║  Min Free:   3200 bytes          │  Suspended: 0               ║
 *   ║  Alloc Fails: 0                  │  Stack Low: 0               ║
 *   ║  Score: [████████░░] 85%         │  Score: [█████████░] 92%    ║
 *   ║                                  │                             ║
 *   ╠════════════════════════════════════════════════════════════════╣
 *   ║  RESOURCES                       │  PERIPHERALS                ║
 *   ║  ────────────────────────────────│──────────────────────────── ║
 *   ║  SensorQueue: 2/10 (20%)         │  UART1: OK                  ║
 *   ║  LogQueue:    8/10 (80%) ⚠       │  GPIO: OK                   ║
 *   ║  Timers: 3 active, 0 failed      │  I2C: OK                    ║
 *   ║  Uptime: 01:23:45                │  SPI: N/A                   ║
 *   ║  Score: [██████░░░░] 68%         │  Score: [████████░░] 80%    ║
 *   ║                                  │                             ║
 *   ╠════════════════════════════════════════════════════════════════╣
 *   ║  RECENT EVENTS                                                 ║
 *   ║  [12:34:56] Memory warning: Heap usage >70%                    ║
 *   ║  [12:35:01] Queue near full: LogQueue at 80%                   ║
 *   ║  [12:35:15] Task "Sensor" stack usage high: 85%                ║
 *   ╚════════════════════════════════════════════════════════════════╝
 * 
 * 
 * TASK STATUS TABLE:
 * ==================
 * 
 *   ┌────────────────────────────────────────────────────────────────┐
 *   │ Task       │ State     │ Stack    │ Priority │ Health         │
 *   ├────────────┼───────────┼──────────┼──────────┼────────────────┤
 *   │ Sensor     │ Running   │ 45%      │ 2        │ ████████░░ 85% │
 *   │ Display    │ Blocked   │ 38%      │ 1        │ █████████░ 90% │
 *   │ Logger     │ Blocked   │ 52%      │ 1        │ ████████░░ 82% │
 *   │ HealthMon  │ Running   │ 41%      │ 3        │ █████████░ 88% │
 *   │ IDLE       │ Ready     │ 15%      │ 0        │ ██████████ 100%│
 *   └────────────┴───────────┴──────────┴──────────┴────────────────┘
 * 
 * 
 * EXPECTED SERIAL OUTPUT:
 * =======================
 * 
 *   === SYSTEM HEALTH MONITOR DEMO ===
 *   Starting health monitoring...
 *   
 *   ══════════════════════════════════════════════
 *   SYSTEM HEALTH REPORT @ 5000ms
 *   ══════════════════════════════════════════════
 *   
 *   OVERALL HEALTH: 85% [HEALTHY]
 *   
 *   [MEMORY]
 *     Heap: 4520/10240 used (44%)
 *     Free: 5720 bytes
 *     Min Free: 3200 bytes
 *     Score: 88%
 *   
 *   [TASKS]
 *     Active: 5, Blocked: 1
 *     Stack concerns: 0
 *     Score: 92%
 *   
 *   [RESOURCES]
 *     Queues: 2 OK, 1 Warning
 *     Uptime: 00:05:00
 *     Score: 75%
 *   
 *   [PERIPHERALS]
 *     All peripherals OK
 *     Score: 100%
 *   
 *   ══════════════════════════════════════════════
 * 
 * 
 * PENGGUNAAN PRAKTIS:
 * ===================
 * 1. Industrial systems monitoring
 * 2. Automotive ECU diagnostics
 * 3. Medical device health checks
 * 4. IoT device fleet management
 * 5. Server/infrastructure monitoring
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
#define configUSE_IDLE_HOOK                      1
#define configUSE_TICK_HOOK                      0
#define configUSE_MALLOC_FAILED_HOOK             1
#define configCHECK_FOR_STACK_OVERFLOW           2

/* Debug/Stats - ENABLED for health monitoring */
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
 * HEALTH MONITOR CONFIGURATION
 * ==========================================*/
#define HEALTH_REPORT_PERIOD_MS                  5000
#define HEALTH_MAX_TRACKED_QUEUES                4
#define HEALTH_MAX_EVENTS                        5

/* Threshold percentages */
#define HEALTH_MEMORY_WARNING_PERCENT            70
#define HEALTH_MEMORY_CRITICAL_PERCENT           90
#define HEALTH_STACK_WARNING_PERCENT             75
#define HEALTH_QUEUE_WARNING_PERCENT             80

/* Health status levels */
#define HEALTH_STATUS_HEALTHY                    0
#define HEALTH_STATUS_DEGRADED                   1
#define HEALTH_STATUS_WARNING                    2
#define HEALTH_STATUS_CRITICAL                   3

#endif /* FREERTOS_CONFIG_H */
