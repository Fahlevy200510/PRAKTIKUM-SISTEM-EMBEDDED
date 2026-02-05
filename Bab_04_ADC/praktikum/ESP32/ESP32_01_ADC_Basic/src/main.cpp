#include <Arduino.h>

#define ADC_PIN 34

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Program 01: ADC Basic - ESP32\n");
    
    analogReadResolution(12);  // 12-bit (0-4095)
    analogSetAttenuation(ADC_11db);  // 0-3.3V
}

void loop() {
    int raw = analogRead(ADC_PIN);
    float voltage = raw * 3.3 / 4095.0;
    
    Serial.printf("ADC: %4d (%.2fV)\n", raw, voltage);
    delay(500);
}
