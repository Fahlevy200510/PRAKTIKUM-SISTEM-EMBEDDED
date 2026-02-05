#include <Arduino.h>

#define ADC_PIN PA0

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 01: ADC Basic - STM32\n");
    analogReadResolution(12);
}

void loop() {
    int raw = analogRead(ADC_PIN);
    float voltage = raw * 3.3 / 4095.0;
    Serial.printf("ADC: %4d (%.2fV)\n", raw, voltage);
    delay(500);
}
