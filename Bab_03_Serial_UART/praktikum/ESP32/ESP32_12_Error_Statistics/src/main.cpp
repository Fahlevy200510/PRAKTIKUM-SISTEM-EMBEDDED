#include <Arduino.h>

struct UartStats {
    uint32_t rxBytes = 0;
    uint32_t txBytes = 0;
    uint32_t frameErrors = 0;
    uint32_t parityErrors = 0;
    uint32_t overruns = 0;
    uint32_t validFrames = 0;
} stats;

void printStats() {
    Serial.println("\n=== UART Statistics ===");
    Serial.printf("RX Bytes: %lu\n", stats.rxBytes);
    Serial.printf("TX Bytes: %lu\n", stats.txBytes);
    Serial.printf("Valid Frames: %lu\n", stats.validFrames);
    Serial.printf("Frame Errors: %lu\n", stats.frameErrors);
    Serial.printf("Parity Errors: %lu\n", stats.parityErrors);
    Serial.printf("Overruns: %lu\n", stats.overruns);
    Serial.println("=====================\n");
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Program 12: Error Statistics - ESP32\n");
    Serial.println("Commands: stats, reset, send <text>");
}

void loop() {
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        input.trim();
        stats.rxBytes += input.length();
        
        if (input == "stats") {
            printStats();
        }
        else if (input == "reset") {
            memset(&stats, 0, sizeof(stats));
            Serial.println("Statistics reset");
        }
        else if (input.startsWith("send ")) {
            String msg = input.substring(5);
            Serial.println(msg);
            stats.txBytes += msg.length();
            stats.validFrames++;
        }
    }
}
