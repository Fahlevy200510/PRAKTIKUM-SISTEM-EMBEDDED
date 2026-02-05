#include <Arduino.h>

#define PWM_PIN PA0

uint8_t duty = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 06: PWM Ramp - STM32\n");
    
    pinMode(PWM_PIN, OUTPUT);
    analogWriteFrequency(1000);
}

void loop() {
    analogWrite(PWM_PIN, duty++);
    
    if (duty == 0) {
        Serial.println("Ramp complete");
    }
    
    delay(10);
}
