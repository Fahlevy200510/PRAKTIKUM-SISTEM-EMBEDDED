#include <Arduino.h>

uint8_t calculateCRC8(uint8_t* data, int len) {
    uint8_t crc = 0xFF;
    for (int i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x80) crc = (crc << 1) ^ 0x31;
            else crc <<= 1;
        }
    }
    return crc;
}

uint16_t calculateCRC16(uint8_t* data, int len) {
    uint16_t crc = 0xFFFF;
    for (int i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) crc = (crc >> 1) ^ 0xA001;
            else crc >>= 1;
        }
    }
    return crc;
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Program 09: CRC Checksum - ESP32\n");
    
    uint8_t testData[] = "Hello World";
    int len = strlen((char*)testData);
    
    uint8_t crc8 = calculateCRC8(testData, len);
    uint16_t crc16 = calculateCRC16(testData, len);
    
    Serial.printf("Data: %s\n", testData);
    Serial.printf("CRC-8: 0x%02X\n", crc8);
    Serial.printf("CRC-16: 0x%04X\n", crc16);
}

void loop() {
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        uint8_t crc8 = calculateCRC8((uint8_t*)input.c_str(), input.length());
        uint16_t crc16 = calculateCRC16((uint8_t*)input.c_str(), input.length());
        
        Serial.printf("Input: %s\n", input.c_str());
        Serial.printf("CRC-8: 0x%02X, CRC-16: 0x%04X\n", crc8, crc16);
    }
}
