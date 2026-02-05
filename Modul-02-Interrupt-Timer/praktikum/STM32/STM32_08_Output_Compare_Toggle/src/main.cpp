#include <Arduino.h>
#include <HardwareTimer.h>

#define OC_PIN PA0  // TIM2_CH1

HardwareTimer *timer = new HardwareTimer(TIM2);

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 08: Output Compare Toggle - STM32\n");
    
    timer->setMode(1, TIMER_OUTPUT_COMPARE_TOGGLE, OC_PIN);
    timer->setOverflow(1, HERTZ_FORMAT);  // 1 Hz toggle = 0.5 Hz square wave
    timer->resume();
    
    Serial.println("Output compare on PA0: 0.5Hz square wave");
}

void loop() {
    delay(1000);
}
