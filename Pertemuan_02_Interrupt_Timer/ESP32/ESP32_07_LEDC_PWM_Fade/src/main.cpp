/**
 * @file main.cpp
 * @brief Program 07: LEDC PWM Fade (Hardware) - ESP32
 */

#include <Arduino.h>

#define LED_PIN     2
#define PWM_FREQ    5000
#define PWM_RES     13

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Program 07: LEDC Fade - ESP32\n");
    
    #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        ledcAttach(LED_PIN, PWM_FREQ, PWM_RES);
        ledcFadeWithTime(LED_PIN, 0, 8191, 2000);
    #else
        ledcSetup(0, PWM_FREQ, PWM_RES);
        ledcAttachPin(LED_PIN, 0);
        ledcWrite(0, 0);
    #endif
    
    Serial.println("Hardware fade: 0 to 100% in 2 seconds");
}

void loop() {
    static bool fadeUp = true;
    static unsigned long lastFade = 0;
    
    if (millis() - lastFade > 2500) {
        lastFade = millis();
        
        #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
            if (fadeUp) {
                ledcFadeWithTime(LED_PIN, 8191, 0, 2000);
            } else {
                ledcFadeWithTime(LED_PIN, 0, 8191, 2000);
            }
        #else
            // Manual fade for older versions
            static int duty = 0;
            duty = fadeUp ? 0 : 8191;
            ledcWrite(0, duty);
        #endif
        
        fadeUp = !fadeUp;
        Serial.printf("Fade %s\n", fadeUp ? "UP" : "DOWN");
    }
}
