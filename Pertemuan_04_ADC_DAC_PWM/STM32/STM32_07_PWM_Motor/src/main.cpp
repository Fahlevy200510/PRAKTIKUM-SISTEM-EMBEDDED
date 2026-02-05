#include <Arduino.h>

#define MOTOR_PIN PA0
#define POT_PIN   PA1

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 07: PWM Motor - STM32\n");
    analogWriteFrequency(1000);
}

void loop() {
    int pot = analogRead(POT_PIN);
    int speed = map(pot, 0, 4095, 0, 255);
    analogWrite(MOTOR_PIN, speed);
    Serial.printf("Speed: %d%%\n", speed * 100 / 255);
    delay(200);
}
