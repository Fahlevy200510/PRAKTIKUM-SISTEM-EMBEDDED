#include <Arduino.h>

#define SERVO_PIN   18
#define POT_PIN     34

// Servo typically: 500-2500us pulse, 50Hz (20ms period)
#define SERVO_MIN   26   // ~500us at 16-bit, 50Hz
#define SERVO_MAX   128  // ~2500us

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Program 08: Servo Control - ESP32\n");
    
    #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        ledcAttach(SERVO_PIN, 50, 8);  // 50Hz, 8-bit
    #else
        ledcSetup(0, 50, 8);
        ledcAttachPin(SERVO_PIN, 0);
    #endif
}

void loop() {
    int pot = analogRead(POT_PIN);
    int duty = map(pot, 0, 4095, SERVO_MIN, SERVO_MAX);
    int angle = map(pot, 0, 4095, 0, 180);
    
    #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        ledcWrite(SERVO_PIN, duty);
    #else
        ledcWrite(0, duty);
    #endif
    
    Serial.printf("Angle: %3d deg (duty: %d)\n", angle, duty);
    delay(100);
}
