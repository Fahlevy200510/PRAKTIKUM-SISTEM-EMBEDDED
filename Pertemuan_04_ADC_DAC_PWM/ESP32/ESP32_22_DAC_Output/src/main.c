#include <stdio.h>
#include "esp_system.h"
#include "config.h"

// DAC output on GPIO25 and GPIO26
// Key features: DAC output, Two channels, Analog generation

void app_main(void)
{
    printf("Starting 10_DAC_Output\n");
    
    // TODO: Implement 10_DAC_Output
    
    while (1) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
