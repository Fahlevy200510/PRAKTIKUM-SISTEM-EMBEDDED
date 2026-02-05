#include <Arduino.h>

#define NUM_CH 4
const int channels[] = {34, 35, 32, 33};

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Program 02: ADC Multi-Channel - ESP32\n");
    analogReadResolution(12);
}

void loop() {
    Serial.print("ADC: ");
    for (int i = 0; i < NUM_CH; i++) {
        int val = analogRead(channels[i]);
        Serial.printf("CH%d=%4d  ", i, val);
    }
    Serial.println();
    delay(500);
}
