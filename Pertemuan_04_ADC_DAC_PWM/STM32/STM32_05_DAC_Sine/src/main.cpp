#include <Arduino.h>
#include <math.h>

#define DAC_PIN PA4
#define SAMPLES 100
uint16_t sine[SAMPLES];

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 05: DAC Sine - STM32\n");
    
    analogWriteResolution(12);
    for (int i = 0; i < SAMPLES; i++) {
        sine[i] = (uint16_t)(2047.5 * (1 + sin(2 * PI * i / SAMPLES)));
    }
}

void loop() {
    for (int i = 0; i < SAMPLES; i++) {
        analogWrite(DAC_PIN, sine[i]);
        delayMicroseconds(100);
    }
}
