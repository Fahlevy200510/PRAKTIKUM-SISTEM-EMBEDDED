#include <Arduino.h>

#define RING_SIZE 128
char ring[RING_SIZE];
volatile int head = 0, tail = 0;

void push(char c) {
    int next = (head + 1) % RING_SIZE;
    if (next != tail) { ring[head] = c; head = next; }
}

bool pop(char &c) {
    if (head == tail) return false;
    c = ring[tail]; tail = (tail + 1) % RING_SIZE;
    return true;
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 03: Ring Buffer - STM32\n");
}

void loop() {
    while (Serial.available()) push(Serial.read());
    char c;
    while (pop(c)) Serial.write(c);
}
