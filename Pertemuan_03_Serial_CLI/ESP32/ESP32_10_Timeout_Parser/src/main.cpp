#include <Arduino.h>

#define TIMEOUT_MS  100
#define MAX_MSG     128

char msgBuffer[MAX_MSG];
int msgLen = 0;
unsigned long lastCharTime = 0;

void processMessage() {
    if (msgLen > 0) {
        msgBuffer[msgLen] = '\0';
        Serial.printf("Message complete: %s\n", msgBuffer);
        msgLen = 0;
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Program 10: Timeout Parser - ESP32\n");
    Serial.printf("Timeout: %d ms\n\n", TIMEOUT_MS);
}

void loop() {
    while (Serial.available()) {
        char c = Serial.read();
        
        if (msgLen < MAX_MSG - 1) {
            msgBuffer[msgLen++] = c;
            lastCharTime = millis();
        }
    }
    
    // Check timeout
    if (msgLen > 0 && millis() - lastCharTime > TIMEOUT_MS) {
        processMessage();
    }
}
