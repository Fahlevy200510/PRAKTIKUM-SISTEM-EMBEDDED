#include <Arduino.h>

#define LED_PIN PC13
String cmd = "";

void help() {
    Serial.println("\n=== CLI ===");
    Serial.println("help, led on/off, info, reboot");
}

void process(String c) {
    c.trim(); c.toLowerCase();
    if (c == "help") help();
    else if (c == "led on") { digitalWrite(LED_PIN, LOW); Serial.println("OK"); }
    else if (c == "led off") { digitalWrite(LED_PIN, HIGH); Serial.println("OK"); }
    else if (c == "info") {
        Serial.printf("Uptime: %lu ms\n", millis());
    }
    else if (c == "reboot") { NVIC_SystemReset(); }
    else if (c.length()) Serial.println("?");
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    delay(2000);
    help();
    Serial.print("> ");
}

void loop() {
    while (Serial.available()) {
        char c = Serial.read();
        Serial.write(c);
        if (c == '\n') { process(cmd); cmd = ""; Serial.print("> "); }
        else if (c != '\r') cmd += c;
    }
}
