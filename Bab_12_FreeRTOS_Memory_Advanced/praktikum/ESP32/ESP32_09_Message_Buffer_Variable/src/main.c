/**
 * ============================================================================
 * PROJECT: 33-Message_Buffer_Variable
 * PLATFORM: ESP32
 * PERTEMUAN: 10 - FreeRTOS Advanced
 * 
 * DESKRIPSI:
 * ESP32 version of Message_Buffer_Variable using ESP-IDF framework.
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
#define TAG         "Program33"

/* Task prototypes */
static void vMainTask(void *pvParameters);

/* ============================================================================
 * MAIN APPLICATION
 * ============================================================================ */

void app_main(void)
{
    ESP_LOGI(TAG, "Starting Program 33: Message_Buffer_Variable");
    
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
        ESP_LOGI(TAG, "Program 33 running...");
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
