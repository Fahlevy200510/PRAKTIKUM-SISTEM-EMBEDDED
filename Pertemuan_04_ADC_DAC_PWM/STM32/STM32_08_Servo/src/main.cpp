#include <Arduino.h>
#include <Servo.h>

Servo myServo;
#define SERVO_PIN PA0
#define POT_PIN   PA1

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 08: Servo - STM32\n");
    myServo.attach(SERVO_PIN);
}

void loop() {
    int pot = analogRead(POT_PIN);
    int angle = map(pot, 0, 4095, 0, 180);
    myServo.write(angle);
    Serial.printf("Angle: %d deg\n", angle);
    delay(100);
}
