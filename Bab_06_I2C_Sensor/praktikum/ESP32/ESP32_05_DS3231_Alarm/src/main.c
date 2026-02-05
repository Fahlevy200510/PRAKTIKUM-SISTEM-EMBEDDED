#include <stdio.h>
#include "esp_system.h"
#include "config.h"

// RTC alarm configuration and interrupt
// Key features: Alarm setting, ISR handling, Event detection

void app_main(void)
{
    printf("Starting 05_DS3231_Alarm\n");
    
    // TODO: Implement 05_DS3231_Alarm
    
    while (1) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
