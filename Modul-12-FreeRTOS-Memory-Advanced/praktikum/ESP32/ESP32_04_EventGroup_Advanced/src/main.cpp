/**
 * @file main.cpp
 * @brief Program 04: ESP32 Event Groups Advanced
 * 
 * Event Groups untuk sinkronisasi multi-task kompleks.
 * Demonstrasi wait for all/any bits dan event-driven design.
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#define LED_PIN     2

// Event Bits Definition
#define EVT_SENSOR_A_READY      (1 << 0)
#define EVT_SENSOR_B_READY      (1 << 1)
#define EVT_SENSOR_C_READY      (1 << 2)
#define EVT_ALL_SENSORS         (EVT_SENSOR_A_READY | EVT_SENSOR_B_READY | EVT_SENSOR_C_READY)
#define EVT_PROCESSOR_DONE      (1 << 3)
#define EVT_STORAGE_READY       (1 << 4)
#define EVT_NETWORK_CONNECTED   (1 << 5)
#define EVT_ERROR_OCCURRED      (1 << 6)
#define EVT_SYSTEM_SHUTDOWN     (1 << 7)

EventGroupHandle_t xSystemEvents = NULL;
EventGroupHandle_t xSensorEvents = NULL;

TaskHandle_t xSensorATask, xSensorBTask, xSensorCTask;
TaskHandle_t xAggregatorTask, xUploaderTask, xMonitorTask;

// Sensor data
volatile float sensorA_value = 0, sensorB_value = 0, sensorC_value = 0;

void vSensorATask(void *pvParameters) {
    Serial.println("[SensorA] Started on Core 0");
    
    for (;;) {
        // Simulate sensor reading
        vTaskDelay(pdMS_TO_TICKS(random(500, 1500)));
        
        sensorA_value = random(200, 350) / 10.0;
        Serial.printf("[SensorA] Reading: %.1f°C\n", sensorA_value);
        
        // Signal ready
        xEventGroupSetBits(xSensorEvents, EVT_SENSOR_A_READY);
        
        // Wait for processor to consume
        xEventGroupWaitBits(xSensorEvents, EVT_PROCESSOR_DONE, pdTRUE, pdFALSE, portMAX_DELAY);
    }
}

void vSensorBTask(void *pvParameters) {
    Serial.println("[SensorB] Started on Core 0");
    
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(random(600, 1400)));
        
        sensorB_value = random(400, 800) / 10.0;
        Serial.printf("[SensorB] Reading: %.1f%%\n", sensorB_value);
        
        xEventGroupSetBits(xSensorEvents, EVT_SENSOR_B_READY);
        xEventGroupWaitBits(xSensorEvents, EVT_PROCESSOR_DONE, pdTRUE, pdFALSE, portMAX_DELAY);
    }
}

void vSensorCTask(void *pvParameters) {
    Serial.println("[SensorC] Started on Core 1");
    
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(random(700, 1300)));
        
        sensorC_value = random(9800, 10500) / 10.0;
        Serial.printf("[SensorC] Reading: %.1f hPa\n", sensorC_value);
        
        xEventGroupSetBits(xSensorEvents, EVT_SENSOR_C_READY);
        xEventGroupWaitBits(xSensorEvents, EVT_PROCESSOR_DONE, pdTRUE, pdFALSE, portMAX_DELAY);
    }
}

void vAggregatorTask(void *pvParameters) {
    Serial.println("[Aggregator] Started - waiting for ALL sensors");
    
    for (;;) {
        // Wait for ALL sensors to be ready
        EventBits_t bits = xEventGroupWaitBits(
            xSensorEvents,
            EVT_ALL_SENSORS,    // Wait for all 3 sensors
            pdTRUE,             // Clear bits on exit
            pdTRUE,             // Wait for ALL bits
            pdMS_TO_TICKS(5000) // Timeout
        );
        
        if ((bits & EVT_ALL_SENSORS) == EVT_ALL_SENSORS) {
            Serial.println("\n╔════════════════════════════════════════╗");
            Serial.println("║   ALL SENSORS READY - AGGREGATING      ║");
            Serial.println("╠════════════════════════════════════════╣");
            Serial.printf("║  Temperature: %6.1f °C               ║\n", sensorA_value);
            Serial.printf("║  Humidity:    %6.1f %%                ║\n", sensorB_value);
            Serial.printf("║  Pressure:    %6.1f hPa              ║\n", sensorC_value);
            Serial.println("╚════════════════════════════════════════╝\n");
            
            // Signal uploader that data is ready
            xEventGroupSetBits(xSystemEvents, EVT_STORAGE_READY);
            
            // Signal sensors they can continue
            xEventGroupSetBits(xSensorEvents, EVT_PROCESSOR_DONE);
            
        } else {
            Serial.println("[Aggregator] Timeout - not all sensors ready!");
            // Set error event
            xEventGroupSetBits(xSystemEvents, EVT_ERROR_OCCURRED);
        }
    }
}

void vUploaderTask(void *pvParameters) {
    Serial.println("[Uploader] Started - waiting for data and network");
    
    // Simulate network connection after a while
    vTaskDelay(pdMS_TO_TICKS(3000));
    xEventGroupSetBits(xSystemEvents, EVT_NETWORK_CONNECTED);
    Serial.println("[Uploader] Network connected!");
    
    for (;;) {
        // Wait for EITHER storage ready OR error
        EventBits_t bits = xEventGroupWaitBits(
            xSystemEvents,
            EVT_STORAGE_READY | EVT_ERROR_OCCURRED,
            pdTRUE,             // Clear bits
            pdFALSE,            // Wait for ANY bit (not all)
            pdMS_TO_TICKS(10000)
        );
        
        if (bits & EVT_ERROR_OCCURRED) {
            Serial.println("[Uploader] ⚠️  Error event received!");
            continue;
        }
        
        if (bits & EVT_STORAGE_READY) {
            // Check if network is connected
            EventBits_t netBits = xEventGroupGetBits(xSystemEvents);
            
            if (netBits & EVT_NETWORK_CONNECTED) {
                Serial.println("[Uploader] 📤 Uploading data to cloud...");
                vTaskDelay(pdMS_TO_TICKS(500));
                Serial.println("[Uploader] ✓ Upload complete!");
            } else {
                Serial.println("[Uploader] ⚠️  Network not connected - data queued");
            }
        }
    }
}

void vMonitorTask(void *pvParameters) {
    Serial.println("[Monitor] Started");
    
    for (;;) {
        EventBits_t sensorBits = xEventGroupGetBits(xSensorEvents);
        EventBits_t systemBits = xEventGroupGetBits(xSystemEvents);
        
        Serial.println("\n┌────────── EVENT STATUS ──────────┐");
        Serial.printf("│ Sensor Events: 0x%02X              │\n", sensorBits);
        Serial.printf("│   A Ready: %s                    │\n", 
                     (sensorBits & EVT_SENSOR_A_READY) ? "✓" : "✗");
        Serial.printf("│   B Ready: %s                    │\n",
                     (sensorBits & EVT_SENSOR_B_READY) ? "✓" : "✗");
        Serial.printf("│   C Ready: %s                    │\n",
                     (sensorBits & EVT_SENSOR_C_READY) ? "✓" : "✗");
        Serial.printf("│ System Events: 0x%02X              │\n", systemBits);
        Serial.printf("│   Network: %s                    │\n",
                     (systemBits & EVT_NETWORK_CONNECTED) ? "✓" : "✗");
        Serial.printf("│   Error: %s                      │\n",
                     (systemBits & EVT_ERROR_OCCURRED) ? "⚠️" : "✗");
        Serial.println("└──────────────────────────────────┘");
        
        // LED heartbeat
        digitalWrite(LED_PIN, HIGH);
        vTaskDelay(pdMS_TO_TICKS(100));
        digitalWrite(LED_PIN, LOW);
        
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n═══════════════════════════════════════════════════════");
    Serial.println("       ESP32 EVENT GROUPS ADVANCED DEMONSTRATION");
    Serial.println("═══════════════════════════════════════════════════════\n");
    
    pinMode(LED_PIN, OUTPUT);
    
    // Create event groups
    xSensorEvents = xEventGroupCreate();
    xSystemEvents = xEventGroupCreate();
    
    if (xSensorEvents == NULL || xSystemEvents == NULL) {
        Serial.println("Failed to create event groups!");
        while (1);
    }
    
    Serial.println("Event Groups:");
    Serial.println("  • xSensorEvents: Sensor synchronization");
    Serial.println("  • xSystemEvents: System-wide events\n");
    
    // Create tasks distributed across cores
    xTaskCreatePinnedToCore(vSensorATask, "SensorA", 4096, NULL, 2, &xSensorATask, 0);
    xTaskCreatePinnedToCore(vSensorBTask, "SensorB", 4096, NULL, 2, &xSensorBTask, 0);
    xTaskCreatePinnedToCore(vSensorCTask, "SensorC", 4096, NULL, 2, &xSensorCTask, 1);
    xTaskCreatePinnedToCore(vAggregatorTask, "Aggregator", 4096, NULL, 1, &xAggregatorTask, 0);
    xTaskCreatePinnedToCore(vUploaderTask, "Uploader", 4096, NULL, 1, &xUploaderTask, 1);
    xTaskCreatePinnedToCore(vMonitorTask, "Monitor", 4096, NULL, 1, &xMonitorTask, 1);
    
    Serial.println("Tasks created. System running...\n");
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
