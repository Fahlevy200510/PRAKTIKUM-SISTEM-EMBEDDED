#include <Arduino.h>

#define STX 0x02
#define ETX 0x03
#define MAX_FRAME 64

enum FrameState { WAIT_STX, RECV_DATA, WAIT_ETX };
FrameState state = WAIT_STX;
char frameData[MAX_FRAME];
int frameLen = 0;

void processFrame(char* data, int len) {
    Serial.printf("Frame received (%d bytes): ", len);
    for (int i = 0; i < len; i++) {
        Serial.printf("%02X ", (uint8_t)data[i]);
    }
    Serial.println();
    
    // Echo as string if printable
    bool printable = true;
    for (int i = 0; i < len && printable; i++) {
        if (data[i] < 32 || data[i] > 126) printable = false;
    }
    if (printable) {
        data[len] = '\0';
        Serial.printf("As text: %s\n", data);
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Program 08: STX/ETX Framing - ESP32\n");
    Serial.println("Send: <STX>data<ETX>");
    Serial.println("STX=0x02, ETX=0x03\n");
}

void loop() {
    while (Serial.available()) {
        char c = Serial.read();
        
        switch (state) {
            case WAIT_STX:
                if (c == STX) {
                    state = RECV_DATA;
                    frameLen = 0;
                }
                break;
                
            case RECV_DATA:
                if (c == ETX) {
                    processFrame(frameData, frameLen);
                    state = WAIT_STX;
                } else if (frameLen < MAX_FRAME - 1) {
                    frameData[frameLen++] = c;
                }
                break;
        }
    }
}
