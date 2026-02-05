#include <Arduino.h>

#define ADC_PIN PA0
#define SAMPLES 64
uint16_t adcBuffer[SAMPLES];

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 09: ADC DMA - STM32\n");
    analogReadResolution(12);
}

void loop() {
    // Fill buffer with ADC readings
    for (int i = 0; i < SAMPLES; i++) {
        adcBuffer[i] = analogRead(ADC_PIN);
    }
    
    // Calculate average
    uint32_t sum = 0;
    for (int i = 0; i < SAMPLES; i++) sum += adcBuffer[i];
    
    Serial.printf("DMA Avg: %lu\n", sum / SAMPLES);
    delay(500);
}
