#include <Arduino.h>

#define DAC_PIN PA4  // DAC1 on STM32F103

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 04: DAC Output - STM32\n");
    pinMode(DAC_PIN, OUTPUT);
    analogWriteResolution(12);
}

void loop() {
    for (int i = 0; i < 4096; i += 16) {
        analogWrite(DAC_PIN, i);
        delay(5);
    }
    for (int i = 4095; i >= 0; i -= 16) {
        analogWrite(DAC_PIN, i);
        delay(5);
    }
}
