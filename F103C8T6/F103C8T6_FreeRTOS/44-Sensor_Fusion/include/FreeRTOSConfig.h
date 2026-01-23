/*
 * =============================================================================
 * PROGRAM 44: SENSOR FUSION
 * =============================================================================
 * 
 * KONSEP DASAR:
 * =============
 * Sensor Fusion adalah teknik menggabungkan data dari beberapa sensor untuk
 * menghasilkan informasi yang lebih akurat dan reliable dibanding sensor
 * individual. Dalam FreeRTOS, ini diimplementasikan dengan multiple task
 * yang masing-masing menangani satu sensor, kemudian mengirim data ke
 * fusion task yang menggabungkan dan memproses semua data.
 * 
 * MENGAPA SENSOR FUSION?
 * ======================
 * 1. Redundansi: Jika satu sensor gagal, sistem masih berjalan
 * 2. Akurasi: Kompensasi noise dan error dari sensor individual
 * 3. Kelengkapan: Kombinasi sensor berbeda untuk informasi lebih kaya
 * 4. Reliability: Deteksi sensor yang memberikan data anomali
 * 
 * METODE FUSION YANG DIGUNAKAN:
 * =============================
 * - Complementary Filter: Gabungan low-pass dan high-pass filter
 * - Weighted Average: Rata-rata tertimbang berdasarkan confidence
 * - Voting: Mayoritas pembacaan dianggap benar (untuk digital)
 * 
 * ARSITEKTUR SISTEM:
 * ==================
 * 
 *   ┌─────────────────────────────────────────────────────────────────────┐
 *   │                        SENSOR FUSION SYSTEM                         │
 *   └─────────────────────────────────────────────────────────────────────┘
 *                                     │
 *       ┌────────────────────────────────────────────────────┐
 *       │                                                    │
 *       ▼                    ▼                    ▼          │
 *   ┌────────┐          ┌────────┐          ┌────────┐       │
 *   │ TEMP   │          │ TEMP   │          │ TEMP   │       │
 *   │Sensor 1│          │Sensor 2│          │Sensor 3│       │
 *   │(ADC CH1)│          │(ADC CH2)│          │(ADC CH3)│       │
 *   └───┬────┘          └───┬────┘          └───┬────┘       │
 *       │                   │                   │            │
 *       ▼                   ▼                   ▼            │
 *   ┌────────┐          ┌────────┐          ┌────────┐       │
 *   │Task    │          │Task    │          │Task    │       │
 *   │Sensor1 │          │Sensor2 │          │Sensor3 │       │
 *   │Pri: 2  │          │Pri: 2  │          │Pri: 2  │       │
 *   └───┬────┘          └───┬────┘          └───┬────┘       │
 *       │                   │                   │            │
 *       │    ┌──────────────┼──────────────┐    │            │
 *       └───►│                             │◄───┘            │
 *            │        MESSAGE QUEUE        │                 │
 *            │     (Sensor Readings)       │                 │
 *            │    [ {id,val,ts,conf} ]     │                 │
 *            └─────────────┬───────────────┘                 │
 *                          │                                 │
 *                          ▼                                 │
 *                    ┌───────────┐                           │
 *                    │  FUSION   │                           │
 *                    │   TASK    │                           │
 *                    │  Pri: 3   │                           │
 *                    │           │                           │
 *                    │ Algorithms│                           │
 *                    │ -Weighted │                           │
 *                    │  Average  │                           │
 *                    │ -Outlier  │                           │
 *                    │  Detect   │                           │
 *                    │ -Voting   │                           │
 *                    └─────┬─────┘                           │
 *                          │                                 │
 *                          ▼                                 │
 *                    ┌───────────┐                           │
 *                    │  OUTPUT   │                           │
 *                    │   TASK    │◄──────────────────────────┘
 *                    │  Pri: 1   │     (Feedback)
 *                    │           │
 *                    │ -UART     │
 *                    │ -LED      │
 *                    │ -Alarm    │
 *                    └───────────┘
 * 
 * 
 * DATA FLOW DIAGRAM:
 * ==================
 * 
 *   ┌─────────────────────────────────────────────────────────────────┐
 *   │                      SENSOR DATA PIPELINE                       │
 *   └─────────────────────────────────────────────────────────────────┘
 * 
 *   RAW DATA           FILTERED             FUSED              OUTPUT
 *   ────────           ────────             ─────              ──────
 * 
 *   Sensor1: 25.2°C ──► Low-Pass ──┐
 *                       Filter    │
 *                                 │
 *   Sensor2: 24.8°C ──► Low-Pass ──┼──► Weighted ──► Fused ──► 25.0°C
 *                       Filter    │    Average      Value     (High
 *                                 │    + Outlier              Conf)
 *   Sensor3: 25.1°C ──► Low-Pass ──┘    Detection
 *                       Filter
 * 
 * 
 *   OUTLIER DETECTION:
 *   ┌────────────────────────────────────────────────────────────────┐
 *   │                                                                │
 *   │  Sensor1 ──► 25.2   ◄── Within 2σ from mean ── VALID          │
 *   │  Sensor2 ──► 24.8   ◄── Within 2σ from mean ── VALID          │
 *   │  Sensor3 ──► 35.0   ◄── Outside 2σ ────────── OUTLIER         │
 *   │                                                                │
 *   │  Mean = 25.0°C, σ = 0.3°C                                     │
 *   │  Threshold = Mean ± 2σ = [24.4°C, 25.6°C]                     │
 *   │  35.0°C > 25.6°C → Marked as outlier, excluded from fusion    │
 *   │                                                                │
 *   └────────────────────────────────────────────────────────────────┘
 * 
 * 
 * WEIGHTED AVERAGE ALGORITHM:
 * ===========================
 * 
 *              Σ(value[i] × confidence[i])
 *   Fused = ─────────────────────────────────
 *                 Σ(confidence[i])
 * 
 *   Confidence factors:
 *   - Base confidence per sensor type
 *   - Reduced if reading is near edge of valid range
 *   - Reduced if reading changes rapidly (noise indicator)
 *   - Increased if reading is stable over time
 * 
 * 
 * TIMING DIAGRAM:
 * ===============
 * 
 *   Time ─────────────────────────────────────────────────────►
 *   
 *   Sensor1   ║  READ  ║        ║  READ  ║        ║  READ  ║
 *   (100ms)   ╠════════╣        ╠════════╣        ╠════════╣
 *             0       100      200      300      400      500 ms
 *   
 *   Sensor2   ║        ║  READ  ║        ║  READ  ║        ║
 *   (100ms)   ║        ╠════════╣        ╠════════╣        ║
 *             0       100      200      300      400      500 ms
 *   
 *   Sensor3   ║             ║  READ  ║        ║  READ  ║
 *   (100ms)   ║             ╠════════╣        ╠════════╣
 *             0       100      200      300      400      500 ms
 *   
 *   Fusion    ║    ║PROCESS ║  ║PROCESS ║  ║PROCESS ║  ║PROCESS
 *   (50ms)    ║    ╠════════╣  ╠════════╣  ╠════════╣  ╠════════
 *             0   50       150  200      250  300      350  400
 *   
 *   Note: Fusion task runs faster to process all incoming readings
 * 
 * 
 * STRUKTUR DATA:
 * ==============
 * 
 *   SensorReading_t:
 *   ┌──────────────────────────────────────────────────────────┐
 *   │ Offset │ Field      │ Size   │ Description               │
 *   ├──────────────────────────────────────────────────────────┤
 *   │ 0x00   │ ucSensorId │ 1 byte │ Sensor identifier (0-2)   │
 *   │ 0x04   │ fValue     │ 4 byte │ Reading value (float)     │
 *   │ 0x08   │ ulTimestamp│ 4 byte │ Tick count when read      │
 *   │ 0x0C   │ ucConfidence│ 1 byte│ Confidence 0-100%         │
 *   └──────────────────────────────────────────────────────────┘
 * 
 *   FusedResult_t:
 *   ┌──────────────────────────────────────────────────────────┐
 *   │ Offset │ Field        │ Size   │ Description             │
 *   ├──────────────────────────────────────────────────────────┤
 *   │ 0x00   │ fFusedValue  │ 4 byte │ Hasil fusion            │
 *   │ 0x04   │ ucConfidence │ 1 byte │ Overall confidence      │
 *   │ 0x05   │ ucValidSensors│ 1 byte│ Bitmask sensor valid    │
 *   │ 0x08   │ ulTimestamp  │ 4 byte │ Waktu fusion            │
 *   │ 0x0C   │ fMin         │ 4 byte │ Nilai minimum           │
 *   │ 0x10   │ fMax         │ 4 byte │ Nilai maximum           │
 *   │ 0x14   │ fStdDev      │ 4 byte │ Standard deviation      │
 *   └──────────────────────────────────────────────────────────┘
 * 
 * 
 * SENSOR HEALTH MONITORING:
 * =========================
 * 
 *   ┌─────────────────────────────────────────────────────────────┐
 *   │                   HEALTH STATUS                             │
 *   ├─────────────────────────────────────────────────────────────┤
 *   │                                                             │
 *   │  Sensor 1: [████████████████████] 100% ── HEALTHY          │
 *   │  Sensor 2: [██████████████░░░░░░]  70% ── DEGRADED         │
 *   │  Sensor 3: [████░░░░░░░░░░░░░░░░]  20% ── FAILING          │
 *   │                                                             │
 *   │  Overall System Health: 63%                                 │
 *   │  Fusion Confidence: MEDIUM                                  │
 *   │                                                             │
 *   │  Alerts:                                                    │
 *   │  ⚠ Sensor 2: Increased noise detected                      │
 *   │  ⚠ Sensor 3: Readings out of expected range                │
 *   │                                                             │
 *   └─────────────────────────────────────────────────────────────┘
 * 
 * 
 * EXPECTED SERIAL OUTPUT:
 * =======================
 * 
 *   === SENSOR FUSION DEMO ===
 *   Initializing 3 temperature sensors...
 *   Creating sensor tasks...
 *   Creating fusion task...
 *   Starting scheduler...
 *   
 *   --- Sensor Readings ---
 *   [S1] Temp: 25.20°C (Conf: 95%)
 *   [S2] Temp: 24.80°C (Conf: 92%)
 *   [S3] Temp: 25.10°C (Conf: 90%)
 *   
 *   --- Fusion Result ---
 *   Fused Temp: 25.04°C
 *   Confidence: 92%
 *   Valid Sensors: 3/3
 *   Std Dev: 0.17°C
 *   
 *   --- Sensor Readings ---
 *   [S1] Temp: 25.30°C (Conf: 95%)
 *   [S2] Temp: 24.90°C (Conf: 91%)
 *   [S3] Temp: 35.00°C (Conf: 30%) ← OUTLIER DETECTED!
 *   
 *   --- Fusion Result ---
 *   Fused Temp: 25.11°C
 *   Confidence: 68%
 *   Valid Sensors: 2/3  ← S3 excluded
 *   Std Dev: 0.20°C
 *   
 *   === Health Report ===
 *   Sensor 1: HEALTHY (100%)
 *   Sensor 2: HEALTHY (95%)
 *   Sensor 3: DEGRADED (30%) - Outlier count: 5
 * 
 * 
 * APLIKASI DUNIA NYATA:
 * ====================
 * 1. Drone/Quadcopter: Fusion accelerometer + gyroscope + magnetometer
 * 2. Autonomous Vehicle: LIDAR + Camera + Radar fusion
 * 3. Weather Station: Multiple temp/humidity sensor averaging
 * 4. Industrial Control: Triple modular redundancy (TMR)
 * 5. Robotics: Multi-sensor localization
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
#define configUSE_RECURSIVE_MUTEXES              1
#define configUSE_QUEUE_SETS                     0
#define configUSE_TASK_NOTIFICATIONS             1
#define configUSE_TIMERS                         1

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
#define configUSE_16_BIT_TICKS                   0

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
 * SENSOR FUSION CONFIGURATION
 * ==========================================*/
#define NUM_SENSORS                              3
#define SENSOR_SAMPLE_PERIOD_MS                  100
#define FUSION_PERIOD_MS                         50
#define OUTLIER_THRESHOLD_SIGMA                  2.0f
#define MIN_VALID_SENSORS                        2
#define SENSOR_QUEUE_LENGTH                      10

/* Simulated temperature range for demo */
#define TEMP_MIN                                 20.0f
#define TEMP_MAX                                 30.0f
#define TEMP_NOMINAL                             25.0f
#define TEMP_NOISE_MAX                           0.5f

#endif /* FREERTOS_CONFIG_H */
