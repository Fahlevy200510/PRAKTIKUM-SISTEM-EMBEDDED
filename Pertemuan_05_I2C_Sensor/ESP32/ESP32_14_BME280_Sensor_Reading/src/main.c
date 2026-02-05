#include <stdio.h>
#include "esp_system.h"
#include "config.h"

// BME280 sensor with I2C communication
// Key features: BME280, Multi-sensor, Calibration

void app_main(void)
{
    printf("Starting 02_BME280_Sensor_Reading\n");
    
    // TODO: Implement 02_BME280_Sensor_Reading
    
    while (1) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
