#include <Arduino.h>
#include <stdio.h>

// Retarget printf to Serial
extern "C" int _write(int fd, char *ptr, int len) {
    Serial.write((uint8_t*)ptr, len);
    return len;
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    printf("Program 04: Printf Retarget - STM32\n\n");
    printf("printf() is working!\n");
}

void loop() {
    static int count = 0;
    printf("Count: %d, millis: %lu\n", count++, millis());
    delay(1000);
}
