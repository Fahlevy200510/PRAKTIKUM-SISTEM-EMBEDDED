#include <Arduino.h>
#include <HardwareTimer.h>

#define LED_PIN PC13

HardwareTimer *timer = new HardwareTimer(TIM2);
volatile uint32_t timerCount = 0;
volatile bool timerFlag = false;

void timerCallback() {
    timerCount++;
    timerFlag = true;
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 03: TIM2 Periodic - STM32\n");
    
    pinMode(LED_PIN, OUTPUT);
    
    timer->setOverflow(1, HERTZ_FORMAT);  // 1 Hz
    timer->attachInterrupt(timerCallback);
    timer->resume();
    
    Serial.println("TIM2 configured: 1 second interval");
}

void loop() {
    if (timerFlag) {
        timerFlag = false;
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        Serial.printf("Timer tick #%lu\n", timerCount);
    }
}
