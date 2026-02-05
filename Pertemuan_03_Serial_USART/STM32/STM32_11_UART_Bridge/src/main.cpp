#include <Arduino.h>

HardwareSerial Serial2(PA3, PA2);  // RX, TX

void setup() {
    Serial.begin(115200);   // USB/UART1
    Serial2.begin(9600);    // UART2
    delay(2000);
    Serial.println("Program 11: UART Bridge - STM32\n");
}

void loop() {
    while (Serial.available()) Serial2.write(Serial.read());
    while (Serial2.available()) Serial.write(Serial2.read());
}
