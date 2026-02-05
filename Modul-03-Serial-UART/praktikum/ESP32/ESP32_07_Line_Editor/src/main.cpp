#include <Arduino.h>

#define MAX_LINE 80
char lineBuffer[MAX_LINE];
int cursorPos = 0;
int lineLen = 0;

void clearLine() {
    Serial.print("\r");
    for (int i = 0; i < lineLen + 3; i++) Serial.print(" ");
    Serial.print("\r> ");
}

void refreshLine() {
    clearLine();
    Serial.print(lineBuffer);
    // Move cursor back if not at end
    for (int i = lineLen; i > cursorPos; i--) {
        Serial.print("\b");
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Program 07: Line Editor - ESP32\n");
    Serial.println("Features: Backspace, Arrow keys");
    memset(lineBuffer, 0, MAX_LINE);
    Serial.print("> ");
}

void loop() {
    if (Serial.available()) {
        char c = Serial.read();
        
        if (c == '\n' || c == '\r') {
            Serial.println();
            Serial.printf("You entered: %s\n", lineBuffer);
            memset(lineBuffer, 0, MAX_LINE);
            cursorPos = 0;
            lineLen = 0;
            Serial.print("> ");
        }
        else if (c == 127 || c == 8) {  // Backspace
            if (cursorPos > 0) {
                memmove(&lineBuffer[cursorPos-1], &lineBuffer[cursorPos], lineLen - cursorPos + 1);
                cursorPos--;
                lineLen--;
                refreshLine();
            }
        }
        else if (c == 27) {  // Escape sequence
            if (Serial.available()) {
                char c2 = Serial.read();
                if (c2 == '[' && Serial.available()) {
                    char c3 = Serial.read();
                    if (c3 == 'D' && cursorPos > 0) {  // Left
                        cursorPos--;
                        Serial.print("\b");
                    }
                    else if (c3 == 'C' && cursorPos < lineLen) {  // Right
                        cursorPos++;
                        Serial.print(lineBuffer[cursorPos-1]);
                    }
                }
            }
        }
        else if (lineLen < MAX_LINE - 1) {
            memmove(&lineBuffer[cursorPos+1], &lineBuffer[cursorPos], lineLen - cursorPos + 1);
            lineBuffer[cursorPos] = c;
            cursorPos++;
            lineLen++;
            refreshLine();
        }
    }
}
