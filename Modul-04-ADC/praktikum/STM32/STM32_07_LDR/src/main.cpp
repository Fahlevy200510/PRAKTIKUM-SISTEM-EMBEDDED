#include <Arduino.h>

#define LDR_PIN PA0
#define LED_PIN PA1

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 12: LDR - STM32\n");
    pinMode(LED_PIN, OUTPUT);
}

void loop() {
    int ldr = analogRead(LDR_PIN);
    int led = map(ldr, 0, 4095, 255, 0);
    analogWrite(LED_PIN, led);
    Serial.printf("LDR: %4d -> LED: %3d\n", ldr, led);
    delay(200);
}
