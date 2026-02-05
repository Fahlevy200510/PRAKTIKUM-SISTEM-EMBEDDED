#include <Arduino.h>

#define ENC_A   PB0
#define ENC_B   PB1
#define LED_PIN PC13

volatile int32_t position = 0;
volatile bool updated = false;

void encISR() {
    bool a = digitalRead(ENC_A);
    bool b = digitalRead(ENC_B);
    
    if (a == b) {
        position++;
    } else {
        position--;
    }
    updated = true;
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 10: Encoder Simulation - STM32\n");
    
    pinMode(ENC_A, INPUT_PULLUP);
    pinMode(ENC_B, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    
    attachInterrupt(digitalPinToInterrupt(ENC_A), encISR, CHANGE);
    
    Serial.println("Encoder on PB0/PB1");
}

void loop() {
    if (updated) {
        updated = false;
        Serial.printf("Position: %ld\n", position);
    }
}
