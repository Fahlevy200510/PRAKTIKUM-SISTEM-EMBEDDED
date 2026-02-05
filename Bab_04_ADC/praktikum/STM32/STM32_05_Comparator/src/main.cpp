#include <Arduino.h>

#define ADC_PIN PA0
#define LED_PIN PC13
#define THRESHOLD 2048

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 10: Comparator - STM32\n");
    pinMode(LED_PIN, OUTPUT);
}

void loop() {
    int adc = analogRead(ADC_PIN);
    bool above = adc > THRESHOLD;
    digitalWrite(LED_PIN, !above);  // Active LOW
    Serial.printf("ADC: %4d %s threshold\n", adc, above ? ">" : "<");
    delay(200);
}
