#include <Arduino.h>

#define STX 0x02
#define ETX 0x03
char frame[64];
int idx = 0;
bool inFrame = false;

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 08: STX/ETX Framing - STM32\n");
}

void loop() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == STX) { inFrame = true; idx = 0; }
        else if (c == ETX && inFrame) {
            inFrame = false;
            frame[idx] = '\0';
            Serial.printf("Frame: %s\n", frame);
        } else if (inFrame && idx < 63) {
            frame[idx++] = c;
        }
    }
}
