#include <Arduino.h>

#define PWM_PIN PA0  // TIM2_CH1

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 05: PWM 50% - STM32\n");
    
    pinMode(PWM_PIN, OUTPUT);
    analogWriteFrequency(1000);  // 1kHz
    analogWrite(PWM_PIN, 127);   // 50% duty
    
    Serial.println("PWM on PA0: 1kHz, 50% duty");
}

void loop() {
    delay(1000);
}
