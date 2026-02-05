/**
 * Program 44: DNS
 * Platform: ESP32
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#define TAG "Prog44"

void app_main(void)
{
    ESP_LOGI(TAG, "=== Program 44: DNS ===");
    
    // Initialize NVS
    nvs_flash_init();
    
    // Initialize network stack
    esp_netif_init();
    esp_event_loop_create_default();
    
    ESP_LOGI(TAG, "Initialized. Running...");
    
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        ESP_LOGI(TAG, "Running...");
    }
}
