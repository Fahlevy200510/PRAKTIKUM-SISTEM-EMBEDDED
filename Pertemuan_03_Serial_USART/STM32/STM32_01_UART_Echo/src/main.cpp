#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 01: UART Echo - STM32\n");
}

void loop() {
    if (Serial.available()) {
        Serial.write(Serial.read());
    }
}
