#include <Arduino.h>

#define RING_SIZE 128

class RingBuffer {
public:
    char buffer[RING_SIZE];
    volatile int head = 0;
    volatile int tail = 0;
    
    bool push(char c) {
        int next = (head + 1) % RING_SIZE;
        if (next == tail) return false;  // Full
        buffer[head] = c;
        head = next;
        return true;
    }
    
    bool pop(char &c) {
        if (head == tail) return false;  // Empty
        c = buffer[tail];
        tail = (tail + 1) % RING_SIZE;
        return true;
    }
    
    int available() {
        return (head - tail + RING_SIZE) % RING_SIZE;
    }
};

RingBuffer rxRing;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Program 03: Ring Buffer - ESP32\n");
}

void loop() {
    // Read from Serial to ring buffer
    while (Serial.available()) {
        char c = Serial.read();
        rxRing.push(c);
    }
    
    // Process ring buffer
    char c;
    while (rxRing.pop(c)) {
        Serial.print(c);
    }
}
