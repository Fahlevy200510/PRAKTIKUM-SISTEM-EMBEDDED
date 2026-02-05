/**
 * @file main.cpp
 * @brief Program 08: ESP32 Light Sleep Mode
 * 
 * Demonstrasi light sleep dengan FreeRTOS tickless idle.
 * ESP32 masuk light sleep saat tidak ada task aktif.
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "esp_pm.h"
#include "driver/gpio.h"

#define LED_PIN         2
#define WAKEUP_PIN      GPIO_NUM_0  // Boot button

TaskHandle_t xWorkerTask = NULL;
TaskHandle_t xMonitorTask = NULL;

volatile uint32_t sleepCount = 0;
volatile uint32_t wakeupCount = 0;
volatile uint64_t totalSleepUs = 0;

void printPowerInfo() {
    Serial.println("\n╔═══════════════════════════════════════════════════════╗");
    Serial.println("║           ESP32 LIGHT SLEEP INFORMATION               ║");
    Serial.println("╠═══════════════════════════════════════════════════════╣");
    Serial.println("║ ESP32 Power Modes:                                    ║");
    Serial.println("║   Active:     ~80-240mA (depends on freq/peripherals) ║");
    Serial.println("║   Modem Sleep: ~20mA (WiFi/BT off, CPU running)       ║");
    Serial.println("║   Light Sleep: ~0.8mA (CPU paused, RTC running)       ║");
    Serial.println("║   Deep Sleep:  ~10µA (only RTC, wake = reset)         ║");
    Serial.println("╠═══════════════════════════════════════════════════════╣");
    Serial.printf("║ Sleep events:   %8lu                              ║\n", sleepCount);
    Serial.printf("║ Wakeup events:  %8lu                              ║\n", wakeupCount);
    Serial.printf("║ Total sleep:    %8.2f seconds                     ║\n", 
                 totalSleepUs / 1000000.0);
    Serial.println("╚═══════════════════════════════════════════════════════╝");
}

void enterLightSleep(uint32_t durationMs) {
    // Configure timer wakeup
    esp_sleep_enable_timer_wakeup(durationMs * 1000ULL);
    
    // Configure GPIO wakeup (button)
    esp_sleep_enable_ext0_wakeup(WAKEUP_PIN, 0);  // Wake on low
    
    Serial.printf("[Sleep] Entering light sleep for %lu ms...\n", durationMs);
    Serial.flush();
    
    sleepCount++;
    uint64_t sleepStart = esp_timer_get_time();
    
    // Enter light sleep
    esp_light_sleep_start();
    
    // We're awake now
    uint64_t sleepEnd = esp_timer_get_time();
    totalSleepUs += (sleepEnd - sleepStart);
    wakeupCount++;
    
    // Check wakeup cause
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    
    switch (cause) {
        case ESP_SLEEP_WAKEUP_TIMER:
            Serial.println("[Wakeup] Timer wakeup");
            break;
        case ESP_SLEEP_WAKEUP_EXT0:
            Serial.println("[Wakeup] Button press wakeup");
            break;
        default:
            Serial.printf("[Wakeup] Unknown cause: %d\n", cause);
    }
}

void vWorkerTask(void *pvParameters) {
    Serial.println("[Worker] Task started");
    
    int workCycle = 0;
    
    for (;;) {
        workCycle++;
        
        // Do some work
        Serial.printf("\n[Worker] ===== Work cycle %d =====\n", workCycle);
        
        // Simulate sensor reading
        Serial.println("[Worker] Reading sensors...");
        for (int i = 0; i < 3; i++) {
            digitalWrite(LED_PIN, HIGH);
            vTaskDelay(pdMS_TO_TICKS(100));
            digitalWrite(LED_PIN, LOW);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        
        float temp = 20.0 + random(0, 150) / 10.0;
        float hum = 40.0 + random(0, 400) / 10.0;
        Serial.printf("[Worker] Sensor data: T=%.1f°C, H=%.1f%%\n", temp, hum);
        
        // Process data
        Serial.println("[Worker] Processing...");
        vTaskDelay(pdMS_TO_TICKS(200));
        
        // Now enter light sleep for power saving
        Serial.println("[Worker] Work done. Entering light sleep...");
        enterLightSleep(5000);  // Sleep for 5 seconds
        
        Serial.println("[Worker] Woke up! Starting next cycle...");
    }
}

void vMonitorTask(void *pvParameters) {
    Serial.println("[Monitor] Task started");
    
    for (;;) {
        printPowerInfo();
        
        // Calculate efficiency
        float uptime = millis() / 1000.0;
        float sleepPercent = (totalSleepUs / 1000000.0) / uptime * 100;
        
        Serial.printf("\nPower Efficiency: %.1f%% time in sleep\n", sleepPercent);
        
        if (sleepPercent > 80) {
            Serial.println("Status: EXCELLENT - Very efficient");
        } else if (sleepPercent > 50) {
            Serial.println("Status: GOOD - Moderate efficiency");
        } else {
            Serial.println("Status: FAIR - Room for improvement");
        }
        
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n═══════════════════════════════════════════════════════");
    Serial.println("          ESP32 LIGHT SLEEP MODE DEMONSTRATION");
    Serial.println("═══════════════════════════════════════════════════════\n");
    
    pinMode(LED_PIN, OUTPUT);
    
    // Configure wakeup pin
    pinMode(WAKEUP_PIN, INPUT_PULLUP);
    
    Serial.println("Light Sleep Configuration:");
    Serial.println("  - Timer wakeup: Automatic after duration");
    Serial.println("  - GPIO wakeup: Press BOOT button");
    Serial.println("  - CPU state: Preserved (unlike deep sleep)\n");
    
    // Optional: Configure power management
    #if CONFIG_PM_ENABLE
    esp_pm_config_esp32_t pm_config = {
        .max_freq_mhz = 240,
        .min_freq_mhz = 80,
        .light_sleep_enable = true
    };
    esp_pm_configure(&pm_config);
    Serial.println("Power Management: Enabled with auto light sleep");
    #else
    Serial.println("Power Management: Manual light sleep mode");
    #endif
    
    // Create tasks
    xTaskCreatePinnedToCore(vWorkerTask, "Worker", 4096, NULL, 1, &xWorkerTask, 0);
    xTaskCreatePinnedToCore(vMonitorTask, "Monitor", 4096, NULL, 1, &xMonitorTask, 1);
    
    Serial.println("\nTasks created. System will alternate work and sleep.\n");
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}

/**
 * LIGHT SLEEP VS DEEP SLEEP:
 * 
 * Light Sleep:
 * + CPU state preserved
 * + Fast wakeup (~1ms)
 * + All GPIOs maintain state
 * + RAM contents preserved
 * - Higher power consumption than deep sleep
 * 
 * Deep Sleep:
 * + Lowest power (~10µA)
 * - CPU reset on wakeup (like reboot)
 * - Only RTC memory preserved
 * - Slower wakeup
 */
