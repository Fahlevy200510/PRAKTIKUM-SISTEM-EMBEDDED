#include <Arduino.h>
#include "esp_adc_cal.h"

#define ADC_PIN ADC1_CHANNEL_6  // GPIO34

esp_adc_cal_characteristics_t adc_chars;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Program 09: ADC Calibration - ESP32\n");
    
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_PIN, ADC_ATTEN_DB_11);
    
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
}

void loop() {
    int raw = adc1_get_raw(ADC_PIN);
    uint32_t voltage = esp_adc_cal_raw_to_voltage(raw, &adc_chars);
    
    Serial.printf("Raw: %4d, Calibrated: %lu mV\n", raw, voltage);
    delay(500);
}
