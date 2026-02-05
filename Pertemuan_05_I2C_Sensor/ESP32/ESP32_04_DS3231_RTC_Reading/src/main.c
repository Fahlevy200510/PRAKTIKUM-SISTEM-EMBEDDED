#include <stdio.h>
#include "esp_system.h"
#include "config.h"

// RTC real-time clock reading
// Key features: RTC time, I2C master, Time sync

void app_main(void)
{
    printf("Starting 04_DS3231_RTC_Reading\n");
    
    // TODO: Implement 04_DS3231_RTC_Reading
    
    while (1) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
