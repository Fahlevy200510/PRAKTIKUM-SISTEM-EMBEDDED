#include <Arduino.h>

#define MOTOR_PIN   25
#define POT_PIN     34

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Program 07: PWM Motor Control - ESP32\n");
    
    #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        ledcAttach(MOTOR_PIN, 1000, 10);  // 1kHz, 10-bit
    #else
        ledcSetup(0, 1000, 10);
        ledcAttachPin(MOTOR_PIN, 0);
    #endif
    
    analogReadResolution(12);
}

void loop() {
    int pot = analogRead(POT_PIN);
    int speed = map(pot, 0, 4095, 0, 1023);
    
    #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        ledcWrite(MOTOR_PIN, speed);
    #else
        ledcWrite(0, speed);
    #endif
    
    Serial.printf("Speed: %d%% (%d/1023)\n", speed * 100 / 1023, speed);
    delay(200);
}
