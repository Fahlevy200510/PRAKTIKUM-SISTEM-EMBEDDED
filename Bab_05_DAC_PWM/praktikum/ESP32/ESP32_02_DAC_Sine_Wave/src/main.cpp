#include <Arduino.h>
#include <math.h>

#define DAC_PIN 25
#define SAMPLES 100

uint8_t sineTable[SAMPLES];

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Program 05: DAC Sine Wave - ESP32\n");
    
    // Generate sine lookup table
    for (int i = 0; i < SAMPLES; i++) {
        float angle = 2.0 * PI * i / SAMPLES;
        sineTable[i] = (uint8_t)(127.5 * (1 + sin(angle)));
    }
}

void loop() {
    for (int i = 0; i < SAMPLES; i++) {
        dacWrite(DAC_PIN, sineTable[i]);
        delayMicroseconds(100);  // ~100Hz sine
    }
}
