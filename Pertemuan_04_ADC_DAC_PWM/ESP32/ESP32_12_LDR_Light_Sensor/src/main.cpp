#include <Arduino.h>

#define LDR_PIN     34
#define LED_PIN     2

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Program 12: LDR Light Sensor - ESP32\n");
    
    pinMode(LED_PIN, OUTPUT);
    analogReadResolution(12);
}

void loop() {
    int ldr = analogRead(LDR_PIN);
    int brightness = map(ldr, 0, 4095, 255, 0);  // Invert: dark = bright LED
    
    #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        ledcAttach(LED_PIN, 5000, 8);
        ledcWrite(LED_PIN, brightness);
    #else
        analogWrite(LED_PIN, brightness);
    #endif
    
    const char* level;
    if (ldr < 1000) level = "DARK";
    else if (ldr < 2500) level = "DIM";
    else if (ldr < 3500) level = "NORMAL";
    else level = "BRIGHT";
    
    Serial.printf("LDR: %4d (%s) -> LED: %3d\n", ldr, level, brightness);
    delay(200);
}
