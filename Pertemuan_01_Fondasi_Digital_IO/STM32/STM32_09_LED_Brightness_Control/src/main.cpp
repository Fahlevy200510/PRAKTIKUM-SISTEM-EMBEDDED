/**
 * @file main.cpp
 * @brief Program 09: LED Brightness Control - STM32
 */

#include <Arduino.h>

#define LED_PWM         PA0     // TIM2_CH1
#define BTN_UP          PB0
#define BTN_DOWN        PB1

int brightness = 50;  // 0-100%

void setBrightness(int percent) {
    int pwm = map(percent, 0, 100, 0, 255);
    analogWrite(LED_PWM, pwm);
    
    Serial.printf("Brightness: %3d%% [", percent);
    int bars = percent / 5;
    for (int i = 0; i < 20; i++) {
        Serial.print(i < bars ? "#" : "-");
    }
    Serial.println("]");
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("Program 09: LED Brightness - STM32\n");
    
    pinMode(LED_PWM, OUTPUT);
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    
    analogWriteFrequency(1000);
    setBrightness(brightness);
    
    Serial.println("Commands: 0-100, u=up, d=down\n");
}

void loop() {
    // Serial input
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        input.trim();
        
        if (input == "u" || input == "U") {
            brightness = min(100, brightness + 10);
        } else if (input == "d" || input == "D") {
            brightness = max(0, brightness - 10);
        } else if (isDigit(input[0])) {
            brightness = constrain(input.toInt(), 0, 100);
        }
        setBrightness(brightness);
    }
    
    // Button input
    static unsigned long lastBtn = 0;
    if (millis() - lastBtn > 200) {
        if (digitalRead(BTN_UP) == LOW) {
            brightness = min(100, brightness + 5);
            setBrightness(brightness);
            lastBtn = millis();
        }
        if (digitalRead(BTN_DOWN) == LOW) {
            brightness = max(0, brightness - 5);
            setBrightness(brightness);
            lastBtn = millis();
        }
    }
}
