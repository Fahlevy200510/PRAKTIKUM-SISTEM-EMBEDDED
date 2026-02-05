#include <Arduino.h>

#define UART1_TX    17
#define UART1_RX    16

void setup() {
    Serial.begin(115200);   // USB Serial
    Serial1.begin(9600, SERIAL_8N1, UART1_RX, UART1_TX);
    delay(1000);
    
    Serial.println("Program 11: UART Bridge - ESP32\n");
    Serial.println("USB (115200) <-> UART1 (9600)");
    Serial.printf("UART1: TX=GPIO%d, RX=GPIO%d\n\n", UART1_TX, UART1_RX);
}

void loop() {
    // USB -> UART1
    while (Serial.available()) {
        char c = Serial.read();
        Serial1.write(c);
    }
    
    // UART1 -> USB
    while (Serial1.available()) {
        char c = Serial1.read();
        Serial.write(c);
    }
}
