#include <Arduino.h>
#include <HardwareTimer.h>

#define LED_PIN     PC13
#define BUTTON_PIN  PB0

HardwareTimer *timer = new HardwareTimer(TIM3);
volatile bool expired = false;

void timerCallback() {
    expired = true;
    timer->pause();
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 04: One-Shot Timer - STM32\n");
    
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    
    timer->setOverflow(2000000, MICROSEC_FORMAT);  // 2 seconds
    timer->attachInterrupt(timerCallback);
    
    Serial.println("Press button to start 2s one-shot timer");
}

void loop() {
    static bool lastBtn = HIGH;
    bool btn = digitalRead(BUTTON_PIN);
    
    if (btn == LOW && lastBtn == HIGH) {
        digitalWrite(LED_PIN, LOW);  // ON
        timer->refresh();
        timer->resume();
        Serial.println("One-shot started...");
    }
    lastBtn = btn;
    
    if (expired) {
        expired = false;
        digitalWrite(LED_PIN, HIGH);  // OFF
        Serial.println(">>> Timer expired!");
    }
}
