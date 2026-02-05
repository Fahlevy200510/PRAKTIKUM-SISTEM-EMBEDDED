#include <Arduino.h>

#define PWM_PIN PA0

int brightness = 0;
int fadeAmount = 5;

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 07: PWM Breathing - STM32\n");
    
    pinMode(PWM_PIN, OUTPUT);
    analogWriteFrequency(1000);
}

void loop() {
    analogWrite(PWM_PIN, brightness);
    brightness += fadeAmount;
    
    if (brightness <= 0 || brightness >= 255) {
        fadeAmount = -fadeAmount;
        Serial.printf("Brightness: %d\n", brightness);
    }
    
    delay(20);
}
