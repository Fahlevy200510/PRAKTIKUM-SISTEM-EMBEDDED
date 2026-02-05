#include <Arduino.h>

#define ADC_PIN 34
#define SAMPLES 16

int readAverage(int pin, int samples) {
    long sum = 0;
    for (int i = 0; i < samples; i++) {
        sum += analogRead(pin);
        delayMicroseconds(100);
    }
    return sum / samples;
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Program 03: ADC Averaging - ESP32\n");
    analogReadResolution(12);
}

void loop() {
    int raw = analogRead(ADC_PIN);
    int avg = readAverage(ADC_PIN, SAMPLES);
    
    Serial.printf("Raw: %4d, Avg(%d): %4d\n", raw, SAMPLES, avg);
    delay(500);
}
