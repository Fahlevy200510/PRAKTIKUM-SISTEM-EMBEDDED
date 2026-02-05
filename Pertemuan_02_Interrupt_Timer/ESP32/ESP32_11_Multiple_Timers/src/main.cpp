/**
 * @file main.cpp
 * @brief Program 11: Multiple Timers + Priority - ESP32
 */

#include <Arduino.h>

hw_timer_t *timer1 = NULL;
hw_timer_t *timer2 = NULL;

volatile uint32_t timer1Count = 0;
volatile uint32_t timer2Count = 0;

void IRAM_ATTR onTimer1() { timer1Count++; }
void IRAM_ATTR onTimer2() { timer2Count++; }

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Program 11: Multiple Timers - ESP32\n");
    
    pinMode(2, OUTPUT);
    pinMode(4, OUTPUT);
    
    #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        // ESP32 Arduino Core 3.x API
        timer1 = timerBegin(1000000);
        timerAttachInterrupt(timer1, &onTimer1);
        timerAlarm(timer1, 500000, true, 0);
        
        timer2 = timerBegin(1000000);
        timerAttachInterrupt(timer2, &onTimer2);
        timerAlarm(timer2, 1000000, true, 0);
    #else
        // ESP32 Arduino Core 2.x API
        timer1 = timerBegin(0, 80, true);
        timerAttachInterrupt(timer1, &onTimer1, true);
        timerAlarmWrite(timer1, 500000, true);
        timerAlarmEnable(timer1);
        
        timer2 = timerBegin(1, 80, true);
        timerAttachInterrupt(timer2, &onTimer2, true);
        timerAlarmWrite(timer2, 1000000, true);
        timerAlarmEnable(timer2);
    #endif
    
    Serial.println("Timer1: 500ms, Timer2: 1000ms");
}

void loop() {
    static uint32_t last1 = 0, last2 = 0;
    
    if (timer1Count != last1) {
        last1 = timer1Count;
        digitalWrite(2, !digitalRead(2));
        Serial.printf("Timer1: %lu\n", timer1Count);
    }
    
    if (timer2Count != last2) {
        last2 = timer2Count;
        digitalWrite(4, !digitalRead(4));
        Serial.printf("Timer2: %lu\n", timer2Count);
    }
}
