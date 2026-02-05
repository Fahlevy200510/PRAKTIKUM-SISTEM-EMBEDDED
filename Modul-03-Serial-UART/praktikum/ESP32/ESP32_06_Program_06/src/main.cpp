#include <Arduino.h>

#define LED_PIN 2
String cmdBuffer = "";

void printHelp() {
    Serial.println("\n=== SIMPLE CLI ===");
    Serial.println("Commands:");
    Serial.println("  help     - Show this help");
    Serial.println("  led on   - Turn LED on");
    Serial.println("  led off  - Turn LED off");
    Serial.println("  info     - Show system info");
    Serial.println("  reboot   - Restart ESP32");
    Serial.println("================\n");
}

void processCmd(String cmd) {
    cmd.trim();
    cmd.toLowerCase();
    
    if (cmd == "help") {
        printHelp();
    }
    else if (cmd == "led on") {
        digitalWrite(LED_PIN, HIGH);
        Serial.println("OK: LED ON");
    }
    else if (cmd == "led off") {
        digitalWrite(LED_PIN, LOW);
        Serial.println("OK: LED OFF");
    }
    else if (cmd == "info") {
        Serial.printf("Chip: %s\n", ESP.getChipModel());
        Serial.printf("Cores: %d\n", ESP.getChipCores());
        Serial.printf("CPU Freq: %d MHz\n", ESP.getCpuFreqMHz());
        Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
        Serial.printf("Uptime: %lu ms\n", millis());
    }
    else if (cmd == "reboot") {
        Serial.println("Rebooting...");
        delay(100);
        ESP.restart();
    }
    else if (cmd.length() > 0) {
        Serial.printf("Unknown: %s\n", cmd.c_str());
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    delay(1000);
    printHelp();
    Serial.print("> ");
}

void loop() {
    while (Serial.available()) {
        char c = Serial.read();
        Serial.print(c);  // Echo
        
        if (c == '\n' || c == '\r') {
            Serial.println();
            processCmd(cmdBuffer);
            cmdBuffer = "";
            Serial.print("> ");
        } else if (c == 127 || c == 8) {  // Backspace
            if (cmdBuffer.length() > 0) {
                cmdBuffer.remove(cmdBuffer.length() - 1);
                Serial.print("\b \b");
            }
        } else {
            cmdBuffer += c;
        }
    }
}
