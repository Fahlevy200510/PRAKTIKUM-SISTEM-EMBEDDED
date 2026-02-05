#include <Arduino.h>

#define LED_PIN 2
#define ADC_PIN 34

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Program 06: PWM LED Control - ESP32\n");
    
    #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        ledcAttach(LED_PIN, 5000, 8);
    #else
        ledcSetup(0, 5000, 8);
        ledcAttachPin(LED_PIN, 0);
    #endif
    
    analogReadResolution(12);
}

void loop() {
    int adc = analogRead(ADC_PIN);
    int pwm = map(adc, 0, 4095, 0, 255);
    
    #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        ledcWrite(LED_PIN, pwm);
    #else
        ledcWrite(0, pwm);
    #endif
    
    Serial.printf("ADC: %4d -> PWM: %3d\n", adc, pwm);
    delay(100);
}
