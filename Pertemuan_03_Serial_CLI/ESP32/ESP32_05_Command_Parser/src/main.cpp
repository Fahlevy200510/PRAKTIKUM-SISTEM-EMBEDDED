#include <Arduino.h>

#define LED_PIN 2

String inputBuffer = "";

void processCommand(String cmd) {
    cmd.trim();
    cmd.toUpperCase();
    
    if (cmd == "ON") {
        digitalWrite(LED_PIN, HIGH);
        Serial.println("LED ON");
    }
    else if (cmd == "OFF") {
        digitalWrite(LED_PIN, LOW);
        Serial.println("LED OFF");
    }
    else if (cmd == "TOGGLE") {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        Serial.println("LED TOGGLED");
    }
    else if (cmd == "STATUS") {
        Serial.printf("LED is %s\n", digitalRead(LED_PIN) ? "ON" : "OFF");
    }
    else {
        Serial.printf("Unknown command: %s\n", cmd.c_str());
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    delay(1000);
    
    Serial.println("Program 05: Command Parser - ESP32\n");
    Serial.println("Commands: ON, OFF, TOGGLE, STATUS");
}

void loop() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            if (inputBuffer.length() > 0) {
                processCommand(inputBuffer);
                inputBuffer = "";
            }
        } else {
            inputBuffer += c;
        }
    }
}
