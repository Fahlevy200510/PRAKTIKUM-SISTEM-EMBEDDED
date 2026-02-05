#include <Arduino.h>

uint32_t rxBytes = 0, txBytes = 0, frames = 0;

void printStats() {
    Serial.printf("RX: %lu, TX: %lu, Frames: %lu\n", rxBytes, txBytes, frames);
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 12: Error Stats - STM32\n");
    Serial.println("Commands: stats, reset, send <text>");
}

void loop() {
    if (Serial.available()) {
        String s = Serial.readStringUntil('\n');
        s.trim();
        rxBytes += s.length();
        
        if (s == "stats") printStats();
        else if (s == "reset") { rxBytes = txBytes = frames = 0; }
        else if (s.startsWith("send ")) {
            Serial.println(s.substring(5));
            txBytes += s.length() - 5;
            frames++;
        }
    }
}
