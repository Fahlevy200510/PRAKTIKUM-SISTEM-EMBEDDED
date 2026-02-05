#include <Arduino.h>

#define BTN_HIGH    PB0   // Higher priority
#define BTN_LOW     PB1   // Lower priority
#define LED_PIN     PC13

volatile uint32_t highCount = 0, lowCount = 0;

void highPriorityISR() {
    highCount++;
    Serial.printf("HIGH priority ISR #%lu\n", highCount);
}

void lowPriorityISR() {
    lowCount++;
    Serial.printf("LOW priority ISR #%lu\n", lowCount);
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 12: NVIC Priority - STM32\n");
    
    pinMode(BTN_HIGH, INPUT_PULLUP);
    pinMode(BTN_LOW, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    
    attachInterrupt(digitalPinToInterrupt(BTN_HIGH), highPriorityISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(BTN_LOW), lowPriorityISR, FALLING);
    
    // Set priorities (lower number = higher priority)
    NVIC_SetPriority(EXTI0_IRQn, 1);   // PB0 - high priority
    NVIC_SetPriority(EXTI1_IRQn, 15);  // PB1 - low priority
    
    Serial.println("PB0: High priority (1)");
    Serial.println("PB1: Low priority (15)");
}

void loop() {
    delay(100);
}
