/**
 * @file main.cpp
 * @brief Program 08: Low Power Tickless Idle pada STM32
 * 
 * Deskripsi:
 * Demonstrasi fitur low power dengan tickless idle mode.
 * MCU masuk sleep mode saat tidak ada task aktif.
 * 
 * Tickless Idle:
 * - Menghentikan tick interrupt saat idle
 * - MCU masuk deep sleep
 * - Timer peripheral digunakan untuk wake-up
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include "stm32f1xx_hal.h"

// ==================== KONFIGURASI ====================
#define LED_PIN             PC13
#define BUTTON_PIN          PA0
#define SERIAL_BAUD         115200

// Power modes
#define USE_SLEEP_MODE      1   // Basic sleep
#define USE_STOP_MODE       0   // Deep sleep (more power savings)

// ==================== VARIABEL GLOBAL ====================
TaskHandle_t xWorkerTask = NULL;
TaskHandle_t xMonitorTask = NULL;

volatile uint32_t sleepCount = 0;
volatile uint32_t wakeCount = 0;
volatile uint32_t totalSleepTicks = 0;
volatile uint32_t lastSleepDuration = 0;

// ==================== LOW POWER CALLBACKS ====================

/**
 * @brief Dipanggil sebelum masuk sleep mode
 * 
 * Gunakan untuk:
 * - Mematikan peripheral tidak diperlukan
 * - Menyimpan state
 * - Konfigurasi wake-up source
 * 
 * @param xExpectedIdleTime Estimasi waktu idle (dalam ticks)
 */
void vPreSleepProcessing(uint32_t xExpectedIdleTime) {
    // Record sleep duration
    lastSleepDuration = xExpectedIdleTime;
    totalSleepTicks += xExpectedIdleTime;
    sleepCount++;
    
    // Disable peripherals to save power
    // (In real application, disable unused peripherals)
    
    // Turn off LED
    digitalWrite(LED_PIN, HIGH);  // OFF (active low)
    
    // Optional: Print debug info (tidak disarankan untuk production)
    // Serial.printf("[SLEEP] Entering for %lu ticks\n", xExpectedIdleTime);
    // Serial.flush();  // Ensure data is sent before sleep
}

/**
 * @brief Dipanggil setelah bangun dari sleep
 * 
 * Gunakan untuk:
 * - Menghidupkan kembali peripheral
 * - Restore state
 * - Re-konfigurasi clocks jika perlu
 * 
 * @param xExpectedIdleTime Estimasi waktu yang sudah tidur
 */
void vPostSleepProcessing(uint32_t xExpectedIdleTime) {
    wakeCount++;
    
    // Re-enable peripherals
    // (In real application, re-enable needed peripherals)
    
    // Restore system clock if using STOP mode
    #if USE_STOP_MODE
    // SystemClock_Config();  // Re-configure clocks
    #endif
    
    // Optional debug
    // Serial.printf("[WAKE] Slept for %lu ticks\n", xExpectedIdleTime);
}

// ==================== POWER MANAGEMENT FUNCTIONS ====================

/**
 * @brief Menghitung estimasi konsumsi daya
 */
void calculatePowerMetrics() {
    uint32_t uptime = millis() / 1000;
    if (uptime == 0) uptime = 1;
    
    uint32_t sleepTimeMs = totalSleepTicks * portTICK_PERIOD_MS;
    float sleepPercentage = (float)sleepTimeMs / millis() * 100;
    
    Serial.println("\n┌─────────────────────────────────────────────┐");
    Serial.println("│         POWER CONSUMPTION ANALYSIS          │");
    Serial.println("├─────────────────────────────────────────────┤");
    Serial.printf("│ Uptime:         %6lu seconds              │\n", uptime);
    Serial.printf("│ Sleep events:   %6lu                      │\n", sleepCount);
    Serial.printf("│ Wake events:    %6lu                      │\n", wakeCount);
    Serial.printf("│ Sleep time:     %6lu ms (~%.1f%%)          │\n", 
                 sleepTimeMs, sleepPercentage);
    Serial.println("├─────────────────────────────────────────────┤");
    
    // Power estimation (typical values)
    float activeCurrentmA = 20.0;   // ~20mA in run mode
    float sleepCurrentmA = 2.0;     // ~2mA in sleep mode
    float stopCurrentmA = 0.02;     // ~20uA in stop mode
    
    float avgCurrent;
    #if USE_SLEEP_MODE
    avgCurrent = (sleepPercentage/100 * sleepCurrentmA) + 
                 ((100-sleepPercentage)/100 * activeCurrentmA);
    #else
    avgCurrent = (sleepPercentage/100 * stopCurrentmA) + 
                 ((100-sleepPercentage)/100 * activeCurrentmA);
    #endif
    
    Serial.printf("│ Estimated avg current: %.2f mA             │\n", avgCurrent);
    Serial.printf("│ Power savings vs always-on: %.1f%%          │\n",
                 (1 - avgCurrent/activeCurrentmA) * 100);
    Serial.println("└─────────────────────────────────────────────┘");
}

// ==================== TASKS ====================

/**
 * @brief Worker task - melakukan kerja periodik
 * 
 * Task ini aktif sebentar, lalu tidur lama.
 * Saat semua task tidur, scheduler akan masuk tickless idle.
 */
void vWorkerTask(void *pvParameters) {
    Serial.println("[Worker] Task started");
    
    uint32_t workCount = 0;
    
    for (;;) {
        // Do some work
        Serial.printf("\n[Worker] ===== Work cycle %lu =====\n", ++workCount);
        Serial.println("[Worker] Processing data...");
        
        // Simulate work (LED blink)
        for (int i = 0; i < 5; i++) {
            digitalWrite(LED_PIN, LOW);   // ON
            vTaskDelay(pdMS_TO_TICKS(100));
            digitalWrite(LED_PIN, HIGH);  // OFF
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        
        Serial.println("[Worker] Work complete. Sleeping for 10 seconds...");
        Serial.println("[Worker] (System will enter tickless idle)");
        
        // Long sleep - this triggers tickless idle
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

/**
 * @brief Monitor task - menampilkan status power
 */
void vMonitorTask(void *pvParameters) {
    Serial.println("[Monitor] Task started");
    
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    for (;;) {
        calculatePowerMetrics();
        
        Serial.println("\n┌─────────────────────────────────────────────┐");
        Serial.println("│           TICKLESS IDLE STATUS              │");
        Serial.println("├─────────────────────────────────────────────┤");
        
        #if USE_SLEEP_MODE
        Serial.println("│ Mode: SLEEP (WFI instruction)               │");
        Serial.println("│ - CPU stopped, peripherals running          │");
        Serial.println("│ - Fast wake-up (~1us)                       │");
        Serial.println("│ - Moderate power savings                    │");
        #elif USE_STOP_MODE
        Serial.println("│ Mode: STOP (Deep sleep)                     │");
        Serial.println("│ - CPU & peripherals stopped                 │");
        Serial.println("│ - Slower wake-up (~6us)                     │");
        Serial.println("│ - Maximum power savings                     │");
        #endif
        
        Serial.printf("│ Expected idle threshold: %d ticks            │\n",
                     configEXPECTED_IDLE_TIME_BEFORE_SLEEP);
        Serial.printf("│ Last sleep duration: %lu ticks               │\n",
                     lastSleepDuration);
        
        Serial.println("└─────────────────────────────────────────────┘");
        
        // Long delay to allow worker to trigger tickless
        vTaskDelay(pdMS_TO_TICKS(15000));
    }
}

// ==================== SETUP & LOOP ====================

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(2000);
    
    Serial.println("\n════════════════════════════════════════════════════════════");
    Serial.println("          MODUL 12: LOW POWER TICKLESS IDLE (STM32)         ");
    Serial.println("════════════════════════════════════════════════════════════\n");
    
    Serial.println("Tickless Idle Mode:");
    Serial.println("  Normal idle: Tick interrupt setiap 1ms - wastes power");
    Serial.println("  Tickless idle: No tick during sleep - saves power");
    Serial.println("");
    Serial.println("STM32 Power Modes:");
    Serial.println("  Run:   ~20mA @ 72MHz");
    Serial.println("  Sleep: ~2mA (CPU stopped)");
    Serial.println("  Stop:  ~20uA (all clocks stopped)");
    Serial.println("");
    
    // Initialize pins
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);  // OFF
    
    Serial.println("Configuration:");
    Serial.printf("  configUSE_TICKLESS_IDLE = 1\n");
    Serial.printf("  configEXPECTED_IDLE_TIME_BEFORE_SLEEP = %d\n",
                 configEXPECTED_IDLE_TIME_BEFORE_SLEEP);
    Serial.println("");
    
    // Create tasks with long delays (to trigger tickless)
    xTaskCreate(vWorkerTask, "Worker", 256, NULL, 1, &xWorkerTask);
    xTaskCreate(vMonitorTask, "Monitor", 256, NULL, 1, &xMonitorTask);
    
    Serial.println("Tasks created. Starting scheduler...");
    Serial.println("Watch for sleep/wake events in power analysis.\n");
    
    vTaskStartScheduler();
    
    Serial.println("[ERROR] Scheduler failed!");
}

void loop() {
    // Empty
}

/**
 * PENJELASAN TICKLESS IDLE:
 * 
 * 1. Konfigurasi (FreeRTOSConfig.h):
 *    configUSE_TICKLESS_IDLE = 1
 *    configEXPECTED_IDLE_TIME_BEFORE_SLEEP = N (minimum ticks)
 * 
 * 2. Cara Kerja:
 *    - Saat semua tasks blocked lebih dari N ticks
 *    - Scheduler menghentikan tick interrupt
 *    - MCU masuk sleep mode
 *    - Timer peripheral untuk wake-up
 *    - Tick count di-adjust saat wake-up
 * 
 * 3. Hook Functions:
 *    - configPRE_SLEEP_PROCESSING: Sebelum sleep
 *    - configPOST_SLEEP_PROCESSING: Setelah wake-up
 * 
 * 4. Wake-up Sources:
 *    - Timer peripheral (scheduled wake-up)
 *    - External interrupts (GPIO, EXTI)
 *    - Communication peripherals (UART, I2C, etc.)
 * 
 * BEST PRACTICES:
 * 
 * 1. Disable unused peripherals sebelum sleep
 * 2. Configure wake-up sources dengan benar
 * 3. Test wake-up timing untuk accuracy
 * 4. Consider battery life vs responsiveness trade-off
 * 
 * EXPECTED OUTPUT:
 * 
 * [Worker] ===== Work cycle 1 =====
 * [Worker] Processing data...
 * [Worker] Work complete. Sleeping for 10 seconds...
 * [Worker] (System will enter tickless idle)
 * 
 * ┌─────────────────────────────────────────────┐
 * │         POWER CONSUMPTION ANALYSIS          │
 * │ Sleep events:      15                       │
 * │ Sleep time:      8500 ms (~85%)             │
 * │ Estimated avg current: 4.70 mA              │
 * │ Power savings vs always-on: 76.5%           │
 * └─────────────────────────────────────────────┘
 */
