#include <Arduino.h>

const int ch[] = {PA0, PA1, PA2, PA3};

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 02: ADC Multi-Ch - STM32\n");
    analogReadResolution(12);
}

void loop() {
    for (int i = 0; i < 4; i++) {
        Serial.printf("CH%d=%4d ", i, analogRead(ch[i]));
    }
    Serial.println();
    delay(500);
}
