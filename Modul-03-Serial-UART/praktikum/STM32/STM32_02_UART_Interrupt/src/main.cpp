#include <Arduino.h>

volatile char rxBuf[64];
volatile int rxIdx = 0;
volatile bool ready = false;

void serialEvent() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n') {
            rxBuf[rxIdx] = '\0';
            ready = true;
            rxIdx = 0;
        } else if (rxIdx < 63) {
            rxBuf[rxIdx++] = c;
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 02: UART Interrupt - STM32\n");
}

void loop() {
    serialEvent();
    if (ready) {
        ready = false;
        Serial.printf("Received: %s\n", (char*)rxBuf);
    }
}
