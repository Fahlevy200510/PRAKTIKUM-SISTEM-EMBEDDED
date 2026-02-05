#include <stdio.h>
#include "esp_system.h"
#include "config.h"

// External EEPROM read/write operations
// Key features: 24LC EEPROM, Sequential access, Page write

void app_main(void)
{
    printf("Starting 06_EEPROM_24LC_ReadWrite\n");
    
    // TODO: Implement 06_EEPROM_24LC_ReadWrite
    
    while (1) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
