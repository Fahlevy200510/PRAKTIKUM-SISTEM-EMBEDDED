#include <stdio.h>
#include "esp_system.h"
#include "config.h"

// ADC single channel from potentiometer GPIO36
// Key features: Single channel, 12-bit ADC, 1100mV attenuation

void app_main(void)
{
    printf("Starting 01_ADC_Single_Channel\n");
    
    // TODO: Implement 01_ADC_Single_Channel
    
    while (1) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
