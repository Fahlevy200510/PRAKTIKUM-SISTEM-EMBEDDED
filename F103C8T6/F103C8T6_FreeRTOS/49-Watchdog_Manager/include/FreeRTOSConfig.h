/*
 * =============================================================================
 * PROGRAM 49: WATCHDOG MANAGER
 * =============================================================================
 * 
 * KONSEP DASAR:
 * =============
 * Watchdog Timer (WDT) adalah mekanisme keselamatan untuk mendeteksi dan
 * memulihkan sistem dari kondisi hang atau malfunction. Watchdog Manager
 * mengelola multiple task monitoring dengan satu watchdog timer hardware.
 * 
 * MENGAPA PERLU WATCHDOG MANAGER?
 * ================================
 * 1. Hardware WDT hanya satu, tapi task banyak
 * 2. Tidak cukup hanya feed WDT di satu tempat
 * 3. Perlu deteksi task mana yang hang
 * 4. Graceful degradation jika satu task bermasalah
 * 
 * ARSITEKTUR SISTEM:
 * ==================
 * 
 *   ┌─────────────────────────────────────────────────────────────────────┐
 *   │                     WATCHDOG MANAGER SYSTEM                         │
 *   └─────────────────────────────────────────────────────────────────────┘
 *   
 *   ┌─────────────────────────────────────────────────────────────────────┐
 *   │                        MONITORED TASKS                              │
 *   │                                                                     │
 *   │  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐           │
 *   │  │  TASK A  │  │  TASK B  │  │  TASK C  │  │  TASK D  │           │
 *   │  │          │  │          │  │          │  │          │           │
 *   │  │ CheckIn()│  │ CheckIn()│  │ CheckIn()│  │ CheckIn()│           │
 *   │  │ every    │  │ every    │  │ every    │  │ every    │           │
 *   │  │ 100ms    │  │ 200ms    │  │ 500ms    │  │ 1000ms   │           │
 *   │  └────┬─────┘  └────┬─────┘  └────┬─────┘  └────┬─────┘           │
 *   │       │             │             │             │                  │
 *   └───────┼─────────────┼─────────────┼─────────────┼──────────────────┘
 *           │             │             │             │
 *           └─────────────┴─────────────┴─────────────┘
 *                                │
 *                                ▼
 *           ┌────────────────────────────────────────┐
 *           │          WATCHDOG MANAGER              │
 *           │                                        │
 *           │  ┌────────────────────────────────┐    │
 *           │  │     TASK STATUS TABLE          │    │
 *           │  ├────────┬──────────┬───────────┤    │
 *           │  │ Task   │ Timeout  │ LastCheck │    │
 *           │  ├────────┼──────────┼───────────┤    │
 *           │  │ A      │ 200ms    │ OK        │    │
 *           │  │ B      │ 400ms    │ OK        │    │
 *           │  │ C      │ 1000ms   │ LATE!     │◄───┼── Masalah!
 *           │  │ D      │ 2000ms   │ OK        │    │
 *           │  └────────┴──────────┴───────────┘    │
 *           │                                        │
 *           │  If ALL OK ─────► Feed Hardware WDT    │
 *           │  If ANY LATE ───► DON'T Feed (reset)   │
 *           │                                        │
 *           └─────────────────────────────────┬──────┘
 *                                             │
 *                                             ▼
 *                          ┌──────────────────────────────┐
 *                          │      HARDWARE WATCHDOG       │
 *                          │          (IWDG)              │
 *                          │                              │
 *                          │  Timeout: ~3 seconds         │
 *                          │  If not fed: SYSTEM RESET    │
 *                          └──────────────────────────────┘
 * 
 * 
 * CHECK-IN MECHANISM:
 * ===================
 * 
 *   TIME ─────────────────────────────────────────────────────────────────►
 *   
 *   TASK A (100ms period, 200ms timeout):
 *   
 *   ┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐
 *   │ ✓   │ │ ✓   │ │ ✓   │ │ ✓   │ │ ✓   │ │ ✓   │  ◄── Check-in
 *   └──┬──┘ └──┬──┘ └──┬──┘ └──┬──┘ └──┬──┘ └──┬──┘
 *      0     100    200    300    400    500    ms
 *   
 *   Status: [OK]   [OK]   [OK]   [OK]   [OK]   [OK]
 *   
 *   
 *   TASK C (500ms period, 1000ms timeout) DENGAN HANG:
 *   
 *   ┌─────┐              ┌─────┐              ✗ HANG!
 *   │ ✓   │              │ ✓   │              │
 *   └──┬──┘              └──┬──┘              │
 *      0                 500               1000    ms
 *   
 *   Status: [OK]        [OK]              [TIMEOUT!]
 *                                              │
 *                                              ▼
 *                                        WDT not fed
 *                                        System resets
 * 
 * 
 * WATCHDOG MANAGER STATE MACHINE:
 * ================================
 * 
 *          ┌──────────────────────────────────────────────┐
 *          │                                              │
 *          ▼                                              │
 *     ┌─────────┐                                         │
 *     │  INIT   │                                         │
 *     │         │                                         │
 *     └────┬────┘                                         │
 *          │ All tasks registered                         │
 *          ▼                                              │
 *     ┌─────────┐         Timeout detected                │
 *     │ RUNNING │────────────────────────────────────────►│
 *     │         │                                         │
 *     │ Feed WDT│◄────────────────────────────────────────│
 *     │ if OK   │         All tasks OK again              │
 *     └────┬────┘                                         │
 *          │ Any task timeout                             │
 *          ▼                                              │
 *     ┌─────────┐                                         │
 *     │ WARNING │                                         │
 *     │         │─────────────────────────────────────────┘
 *     │ Log err │         Recovery possible
 *     └────┬────┘
 *          │ Still timeout after grace period
 *          ▼
 *     ┌─────────┐
 *     │ FAILURE │
 *     │         │
 *     │ No feed │───► Hardware WDT expires ───► RESET
 *     └─────────┘
 * 
 * 
 * TASK REGISTRATION:
 * ==================
 * 
 *   WDM_Register(taskId, "TaskName", timeoutMs)
 *   
 *   ┌────────────────────────────────────────────────────────────────────┐
 *   │ Task ID │ Name        │ Timeout  │ Last Check-in │ Status         │
 *   ├─────────┼─────────────┼──────────┼───────────────┼────────────────┤
 *   │ 0       │ Sensor      │ 200 ms   │ 12450 ticks   │ OK             │
 *   │ 1       │ Display     │ 500 ms   │ 12400 ticks   │ OK             │
 *   │ 2       │ Comm        │ 1000 ms  │ 12000 ticks   │ OK             │
 *   │ 3       │ Logger      │ 2000 ms  │ 11500 ticks   │ OK             │
 *   └─────────┴─────────────┴──────────┴───────────────┴────────────────┘
 *   
 *   WDM_CheckIn(taskId)
 *   - Update last check-in time to current tick
 *   - Reset timeout counter for that task
 * 
 * 
 * TIMEOUT HANDLING:
 * =================
 * 
 *   ┌────────────────────────────────────────────────────────────────────┐
 *   │                     TIMEOUT RESPONSE OPTIONS                       │
 *   ├────────────────────────────────────────────────────────────────────┤
 *   │                                                                    │
 *   │  1. IMMEDIATE RESET (default):                                     │
 *   │     - Stop feeding hardware WDT                                    │
 *   │     - System resets in ~3 seconds                                  │
 *   │                                                                    │
 *   │  2. TASK RESTART:                                                  │
 *   │     - Delete and recreate the hanging task                         │
 *   │     - Continue feeding WDT                                         │
 *   │                                                                    │
 *   │  3. GRACEFUL DEGRADATION:                                          │
 *   │     - Mark task as failed                                          │
 *   │     - Continue with reduced functionality                          │
 *   │                                                                    │
 *   └────────────────────────────────────────────────────────────────────┘
 * 
 * 
 * EXPECTED SERIAL OUTPUT:
 * =======================
 * 
 *   === WATCHDOG MANAGER DEMO ===
 *   Initializing IWDG (timeout: ~3s)...
 *   IWDG initialized
 *   
 *   Registering tasks...
 *   [WDM] Task 0 "Sensor" registered (timeout: 200ms)
 *   [WDM] Task 1 "Display" registered (timeout: 500ms)
 *   [WDM] Task 2 "Comm" registered (timeout: 1000ms)
 *   
 *   [WDM] All tasks OK - Feeding WDT
 *   [WDM] All tasks OK - Feeding WDT
 *   ...
 *   
 *   [Comm] Simulating hang...
 *   [WDM] Task 2 "Comm" TIMEOUT!
 *   [WDM] WARNING: Task failed, grace period started
 *   [WDM] Task 2 "Comm" still TIMEOUT!
 *   [WDM] FAILURE: Not feeding WDT, reset imminent!
 *   
 *   (System resets)
 * 
 * 
 * STM32 IWDG CONFIGURATION:
 * =========================
 * 
 *   IWDG Clock: LSI ~40kHz
 *   Prescaler: /256
 *   Reload: 0xFFF (4095)
 *   Timeout: 4095 × 256 / 40000 ≈ 26 seconds (max)
 *   
 *   For ~3 seconds: Reload ≈ 468
 *   Timeout = 468 × 256 / 40000 ≈ 3 seconds
 * 
 * 
 * PENGGUNAAN PRAKTIS:
 * ===================
 * 1. Safety-critical systems (automotive, medical)
 * 2. Remote/unattended devices (IoT sensors)
 * 3. Industrial controllers
 * 4. Aerospace systems
 * 5. Network equipment
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
 * WATCHDOG MANAGER CONFIGURATION
 * ==========================================*/
#define WDM_MAX_TASKS                            4
#define WDM_CHECK_PERIOD_MS                      100
#define WDM_GRACE_PERIOD_MS                      500
#define WDM_USE_HARDWARE_WDT                     0   /* Set to 1 to enable real IWDG */

/* Task timeout values (in ms) */
#define TASK_SENSOR_TIMEOUT                      300
#define TASK_DISPLAY_TIMEOUT                     600
#define TASK_COMM_TIMEOUT                        1200

/* Hang simulation control */
#define SIMULATE_HANG_AFTER_MS                   15000  /* Simulate hang after 15s */

#endif /* FREERTOS_CONFIG_H */
