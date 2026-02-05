#include <Arduino.h>

uint8_t crc8(uint8_t* d, int len) {
    uint8_t crc = 0xFF;
    for (int i = 0; i < len; i++) {
        crc ^= d[i];
        for (int j = 0; j < 8; j++)
            crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : crc << 1;
    }
    return crc;
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 09: CRC Checksum - STM32\n");
    
    uint8_t data[] = "Hello";
    Serial.printf("CRC-8 of 'Hello': 0x%02X\n", crc8(data, 5));
}

void loop() {
    if (Serial.available()) {
        String s = Serial.readStringUntil('\n');
        Serial.printf("CRC-8: 0x%02X\n", crc8((uint8_t*)s.c_str(), s.length()));
    }
}
