/**
 * Program 25: Mailbox Pattern
 * Concept: Core functionality demonstration (ESP32)
 * 
 * Learning Points:
 * - FreeRTOS integration with ESP-IDF
 * - ESP32-specific features & capabilities
 * - Hardware initialization
 * - Serial logging
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "config.h"

static const char *TAG = "PROG_25";

/* FreeRTOS objects */
QueueHandle_t queue_handle = NULL;
SemaphoreHandle_t semaphore_handle = NULL;
TimerHandle_t timer_handle = NULL;

static void task_function_1(void *pvParameters)
{
    while(1)
    {
        ESP_LOGI(TAG, "Task 1 running");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static void task_function_2(void *pvParameters)
{
    while(1)
    {
        ESP_LOGI(TAG, "Task 2 running");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void timer_callback(TimerHandle_t xTimer)
{
    ESP_LOGI(TAG, "Timer callback");
}

void app_main(void)
{
    printf("\n=== Program 25: Mailbox Pattern (ESP32) ===\n");
    
    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("Chip: %s\n", CHIP_NAME);
    printf("Cores: %d\n", chip_info.cores);
    printf("Free heap: %lu bytes\n", esp_get_free_heap_size());
    
    /* Create FreeRTOS objects */
    queue_handle = xQueueCreate(10, sizeof(uint32_t));
    semaphore_handle = xSemaphoreCreateBinary();
    timer_handle = xTimerCreate("Timer", pdMS_TO_TICKS(1000), pdTRUE, NULL, timer_callback);
    
    /* Create tasks */
    xTaskCreate(task_function_1, "Task1", 2048, NULL, 2, NULL);
    xTaskCreate(task_function_2, "Task2", 2048, NULL, 1, NULL);
    
    /* Start timer */
    if(timer_handle != NULL)
        xTimerStart(timer_handle, 0);
    
    ESP_LOGI(TAG, "FreeRTOS scheduler running");
    
    /* Main task continues */
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
