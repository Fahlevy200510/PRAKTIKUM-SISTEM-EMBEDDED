#include <Arduino.h>

#define ADC_PIN     34
#define LED_PIN     2
#define THRESHOLD   2048  // Mid-scale (1.65V)

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Program 10: Analog Comparator - ESP32\n");
    
    pinMode(LED_PIN, OUTPUT);
    analogReadResolution(12);
    
    Serial.printf("Threshold: %d (%.2fV)\n", THRESHOLD, THRESHOLD * 3.3 / 4095);
}

void loop() {
    int adc = analogRead(ADC_PIN);
    bool above = adc > THRESHOLD;
    
    digitalWrite(LED_PIN, above);
    Serial.printf("ADC: %4d %s threshold\n", adc, above ? ">" : "<");
    delay(200);
}
