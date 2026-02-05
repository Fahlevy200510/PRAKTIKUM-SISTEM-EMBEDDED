#include <Arduino.h>
#include <HardwareTimer.h>

HardwareTimer *tim2 = new HardwareTimer(TIM2);
HardwareTimer *tim3 = new HardwareTimer(TIM3);

volatile uint32_t t2Count = 0, t3Count = 0;

void tim2Callback() { t2Count++; }
void tim3Callback() { t3Count++; }

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 11: Multiple Timers - STM32\n");
    
    pinMode(PA0, OUTPUT);
    pinMode(PA1, OUTPUT);
    
    tim2->setOverflow(2, HERTZ_FORMAT);  // 2 Hz
    tim2->attachInterrupt(tim2Callback);
    tim2->resume();
    
    tim3->setOverflow(5, HERTZ_FORMAT);  // 5 Hz
    tim3->attachInterrupt(tim3Callback);
    tim3->resume();
    
    Serial.println("TIM2: 2Hz, TIM3: 5Hz");
}

void loop() {
    static uint32_t last2 = 0, last3 = 0;
    
    if (t2Count != last2) {
        last2 = t2Count;
        digitalWrite(PA0, !digitalRead(PA0));
        Serial.printf("TIM2: %lu\n", t2Count);
    }
    
    if (t3Count != last3) {
        last3 = t3Count;
        digitalWrite(PA1, !digitalRead(PA1));
    }
}
