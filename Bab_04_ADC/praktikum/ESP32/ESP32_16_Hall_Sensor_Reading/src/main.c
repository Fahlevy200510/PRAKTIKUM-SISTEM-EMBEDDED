#include <stdio.h>
#include "esp_system.h"
#include "config.h"

// Hall sensor input on dedicated GPIO34
// Key features: Hall sensor, Magnetic detection, Speed sensing

void app_main(void)
{
    printf("Starting 09_Hall_Sensor_Reading\n");
    
    // TODO: Implement 09_Hall_Sensor_Reading
    
    while (1) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
