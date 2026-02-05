#include <stdio.h>
#include "esp_system.h"
#include "config.h"

// SPIFFS/LittleFS partition usage
// Key features: LittleFS, Partition mount, File storage

void app_main(void)
{
    printf("Starting 11_SPIFFS_LittleFS_Partition\n");
    
    // TODO: Implement 11_SPIFFS_LittleFS_Partition
    
    while (1) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
