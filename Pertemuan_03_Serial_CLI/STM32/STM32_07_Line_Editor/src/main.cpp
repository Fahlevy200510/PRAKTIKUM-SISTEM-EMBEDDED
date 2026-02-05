#include <Arduino.h>

#define MAX_LINE 64
char line[MAX_LINE];
int pos = 0, len = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 07: Line Editor - STM32\n");
    Serial.print("> ");
}

void loop() {
    if (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            Serial.println();
            line[len] = '\0';
            Serial.printf("Input: %s\n> ", line);
            pos = len = 0;
        } else if (c == 8 || c == 127) {
            if (pos > 0) { pos--; len--; Serial.print("\b \b"); }
        } else if (len < MAX_LINE-1) {
            line[pos++] = c; len++;
            Serial.write(c);
        }
    }
}
