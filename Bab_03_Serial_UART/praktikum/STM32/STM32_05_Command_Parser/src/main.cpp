#include <Arduino.h>

#define LED_PIN PC13
String cmd = "";

void process(String c) {
    c.trim(); c.toUpperCase();
    if (c == "ON") { digitalWrite(LED_PIN, LOW); Serial.println("LED ON"); }
    else if (c == "OFF") { digitalWrite(LED_PIN, HIGH); Serial.println("LED OFF"); }
    else if (c == "TOGGLE") { digitalWrite(LED_PIN, !digitalRead(LED_PIN)); Serial.println("TOGGLED"); }
    else Serial.printf("Unknown: %s\n", c.c_str());
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    delay(2000);
    Serial.println("Program 05: Command Parser - STM32\n");
    Serial.println("Commands: ON, OFF, TOGGLE");
}

void loop() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n') { process(cmd); cmd = ""; }
        else cmd += c;
    }
}
