#include <Arduino.h>

#define DAC_PIN 25  // DAC1 = GPIO25, DAC2 = GPIO26

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Program 04: DAC Output - ESP32\n");
    Serial.println("DAC on GPIO25 (0-255 = 0-3.3V)");
}

void loop() {
    // Ramp up
    for (int i = 0; i < 256; i++) {
        dacWrite(DAC_PIN, i);
        delay(10);
    }
    
    // Ramp down
    for (int i = 255; i >= 0; i--) {
        dacWrite(DAC_PIN, i);
        delay(10);
    }
}
