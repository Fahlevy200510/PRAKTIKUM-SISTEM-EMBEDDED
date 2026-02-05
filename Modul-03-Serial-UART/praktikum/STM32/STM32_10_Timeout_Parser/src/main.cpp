#include <Arduino.h>

#define TIMEOUT 100
char buf[128];
int len = 0;
unsigned long lastChar = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 10: Timeout Parser - STM32\n");
}

void loop() {
    while (Serial.available()) {
        buf[len++] = Serial.read();
        lastChar = millis();
    }
    
    if (len > 0 && millis() - lastChar > TIMEOUT) {
        buf[len] = '\0';
        Serial.printf("Message: %s\n", buf);
        len = 0;
    }
}
