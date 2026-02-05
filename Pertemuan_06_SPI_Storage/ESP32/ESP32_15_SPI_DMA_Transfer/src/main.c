#include <stdio.h>
#include "esp_system.h"
#include "config.h"

// DMA-based SPI transfer for ESP32
// Key features: SPI DMA, Async transfer, Performance

void app_main(void)
{
    printf("Starting 03_SPI_DMA_Transfer\n");
    
    // TODO: Implement 03_SPI_DMA_Transfer
    
    while (1) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
