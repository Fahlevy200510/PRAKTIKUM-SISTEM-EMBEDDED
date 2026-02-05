/**
 * ============================================================================
 * PROJECT: 29-Stream_Buffer_Basics
 * PLATFORM: ESP32
 * PERTEMUAN: 10 - FreeRTOS Advanced
 * 
 * DESKRIPSI:
 * ESP32 version of Stream_Buffer_Basics using ESP-IDF framework.
 * ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/stream_buffer.h"
#include "freertos/message_buffer.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"

#define LED_GPIO    GPIO_NUM_2
#define TAG         "Program29"

/* Task prototypes */
static void vMainTask(void *pvParameters);

/* ============================================================================
 * MAIN APPLICATION
 * ============================================================================ */

void app_main(void)
{
    ESP_LOGI(TAG, "Starting Program 29: Stream_Buffer_Basics");
    
    /* Configure LED */
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    
    /* Create main task */
    xTaskCreate(vMainTask, "MainTask", 2048, NULL, 5, NULL);
}

static void vMainTask(void *pvParameters)
{
    (void)pvParameters;
    
    for(;;)
    {
        ESP_LOGI(TAG, "Program 29 running...");
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
