#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Program 01: UART Echo - ESP32\n");
    Serial.println("Type something and press Enter...");
}

void loop() {
    if (Serial.available()) {
        char c = Serial.read();
        Serial.print(c);  // Echo back
    }
}
