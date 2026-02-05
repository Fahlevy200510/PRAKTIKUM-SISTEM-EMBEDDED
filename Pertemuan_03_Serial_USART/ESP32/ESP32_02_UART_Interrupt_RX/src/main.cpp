#include <Arduino.h>

#define RX_BUF_SIZE 64
volatile char rxBuffer[RX_BUF_SIZE];
volatile int rxIndex = 0;
volatile bool lineReady = false;

void onReceive() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            rxBuffer[rxIndex] = '\0';
            lineReady = true;
            rxIndex = 0;
        } else if (rxIndex < RX_BUF_SIZE - 1) {
            rxBuffer[rxIndex++] = c;
        }
    }
}

void setup() {
    Serial.begin(115200);
    Serial.onReceive(onReceive);
    delay(1000);
    Serial.println("Program 02: UART Interrupt RX - ESP32\n");
}

void loop() {
    if (lineReady) {
        lineReady = false;
        Serial.printf("Received: %s\n", (char*)rxBuffer);
    }
}
