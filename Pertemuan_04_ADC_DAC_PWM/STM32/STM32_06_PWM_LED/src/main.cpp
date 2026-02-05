#include <Arduino.h>

#define LED_PIN PA0
#define ADC_PIN PA1

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 06: PWM LED - STM32\n");
    pinMode(LED_PIN, OUTPUT);
}

void loop() {
    int adc = analogRead(ADC_PIN);
    int pwm = map(adc, 0, 4095, 0, 255);
    analogWrite(LED_PIN, pwm);
    Serial.printf("ADC: %4d -> PWM: %3d\n", adc, pwm);
    delay(100);
}
