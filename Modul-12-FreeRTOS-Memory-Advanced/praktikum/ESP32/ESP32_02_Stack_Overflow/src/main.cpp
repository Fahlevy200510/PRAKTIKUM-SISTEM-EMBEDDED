/**
 * @file main.cpp
 * @brief Program 02: ESP32 Stack Overflow Detection
 * 
 * Demonstrasi stack overflow detection menggunakan
 * hardware watchpoint ESP32 (lebih akurat dari software).
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#define LED_PIN     2

// Task handles
TaskHandle_t xSafeTask = NULL;
TaskHandle_t xDangerTask = NULL;
TaskHandle_t xMonitorTask = NULL;

volatile bool overflowDetected = false;

// Recursive function to use stack
void recursiveFunction(int depth) {
    volatile char buffer[64];
    memset((void*)buffer, depth, sizeof(buffer));
    
    if (depth > 0) {
        recursiveFunction(depth - 1);
    }
}

void vSafeTask(void *pvParameters) {
    Serial.println("[Safe] Task started with 4096 bytes stack");
    
    for (;;) {
        UBaseType_t hwm = uxTaskGetStackHighWaterMark(NULL);
        Serial.printf("[Safe] Stack HWM: %d bytes\n", hwm * 4);
        
        // Safe recursion
        recursiveFunction(5);
        
        digitalWrite(LED_PIN, HIGH);
        vTaskDelay(pdMS_TO_TICKS(100));
        digitalWrite(LED_PIN, LOW);
        
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void vDangerTask(void *pvParameters) {
    Serial.println("[DANGER] Task started with minimal stack - WILL OVERFLOW!");
    
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    int iteration = 0;
    for (;;) {
        iteration++;
        
        Serial.printf("[DANGER] Iteration %d - attempting deep recursion\n", iteration);
        
        UBaseType_t hwm = uxTaskGetStackHighWaterMark(NULL);
        Serial.printf("[DANGER] Stack HWM before: %d bytes\n", hwm * 4);
        
        // Progressively deeper - will cause overflow
        recursiveFunction(iteration * 5);
        
        Serial.println("[DANGER] Survived!");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void vMonitorTask(void *pvParameters) {
    Serial.println("[Monitor] Task started");
    
    for (;;) {
        Serial.println("\n════════ STACK MONITOR ════════");
        
        if (xSafeTask) {
            UBaseType_t hwm = uxTaskGetStackHighWaterMark(xSafeTask);
            Serial.printf("  Safe:   %5d bytes free\n", hwm * 4);
        }
        if (xDangerTask) {
            UBaseType_t hwm = uxTaskGetStackHighWaterMark(xDangerTask);
            Serial.printf("  Danger: %5d bytes free %s\n", 
                         hwm * 4, hwm < 100 ? "⚠️ LOW!" : "");
        }
        
        Serial.printf("  Total heap: %d bytes\n", ESP.getFreeHeap());
        Serial.println("═══════════════════════════════\n");
        
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n════════════════════════════════════════");
    Serial.println("  ESP32 STACK OVERFLOW DETECTION DEMO");
    Serial.println("════════════════════════════════════════");
    Serial.println("\nESP32 uses hardware watchpoint for detection");
    Serial.println("More reliable than software method\n");
    
    pinMode(LED_PIN, OUTPUT);
    
    // Safe task with adequate stack
    xTaskCreatePinnedToCore(vSafeTask, "Safe", 4096, NULL, 1, &xSafeTask, 0);
    
    // Danger task with small stack
    xTaskCreatePinnedToCore(vDangerTask, "Danger", 1024, NULL, 1, &xDangerTask, 1);
    
    // Monitor task
    xTaskCreatePinnedToCore(vMonitorTask, "Monitor", 4096, NULL, 2, &xMonitorTask, 0);
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
