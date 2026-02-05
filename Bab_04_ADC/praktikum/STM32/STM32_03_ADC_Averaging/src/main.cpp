#include <Arduino.h>

#define ADC_PIN PA0
#define SAMPLES 16

int readAvg(int pin, int n) {
    long sum = 0;
    for (int i = 0; i < n; i++) sum += analogRead(pin);
    return sum / n;
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 03: ADC Averaging - STM32\n");
}

void loop() {
    Serial.printf("Raw: %4d, Avg: %4d\n", analogRead(ADC_PIN), readAvg(ADC_PIN, SAMPLES));
    delay(500);
}
