/**
 * @file main.cpp
 * @brief Program 03: LED Breathing Effect (Hardware PWM) - STM32
 * 
 * Menggunakan hardware timer PWM pada STM32F103C8T6.
 * Timer 2 Channel 1 (PA0) untuk PWM output.
 */

#include <Arduino.h>

// ==================== KONFIGURASI ====================
#define LED_PWM_PIN     PA0     // TIM2_CH1
#define PWM_FREQUENCY   1000    // 1kHz PWM
#define PWM_RESOLUTION  255     // 8-bit

// ==================== VARIABEL ====================
int brightness = 0;
int fadeAmount = 5;
unsigned long previousMillis = 0;

// ==================== SETUP ====================
void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n========================================");
    Serial.println("Program 03: LED Breathing (PWM) - STM32");
    Serial.println("========================================\n");
    
    // Configure PWM pin
    pinMode(LED_PWM_PIN, OUTPUT);
    
    // Arduino STM32 supports analogWrite with PWM
    // PA0 is TIM2_CH1
    analogWriteFrequency(PWM_FREQUENCY);
    analogWriteResolution(8);  // 8-bit resolution
    
    Serial.printf("PWM Pin: PA0 (TIM2_CH1)\n");
    Serial.printf("PWM Frequency: %d Hz\n", PWM_FREQUENCY);
    Serial.println("Breathing effect started...\n");
}

// ==================== LOOP ====================
void loop() {
    unsigned long currentMillis = millis();
    
    if (currentMillis - previousMillis >= 20) {
        previousMillis = currentMillis;
        
        // Set PWM duty cycle
        analogWrite(LED_PWM_PIN, brightness);
        
        // Update brightness
        brightness += fadeAmount;
        
        // Reverse at limits
        if (brightness <= 0 || brightness >= PWM_RESOLUTION) {
            fadeAmount = -fadeAmount;
            
            if (brightness >= PWM_RESOLUTION) {
                Serial.println("Peak brightness");
            } else {
                Serial.println("Minimum brightness");
            }
        }
    }
}

/**
 * STM32F103C8T6 PWM CAPABLE PINS:
 * 
 * Timer 1 (Advanced):
 *   PA8  - TIM1_CH1
 *   PA9  - TIM1_CH2 (also USART1_TX!)
 *   PA10 - TIM1_CH3 (also USART1_RX!)
 *   PA11 - TIM1_CH4
 * 
 * Timer 2 (General Purpose):
 *   PA0  - TIM2_CH1 ← Used here
 *   PA1  - TIM2_CH2
 *   PA2  - TIM2_CH3
 *   PA3  - TIM2_CH4
 * 
 * Timer 3 (General Purpose):
 *   PA6  - TIM3_CH1
 *   PA7  - TIM3_CH2
 *   PB0  - TIM3_CH3
 *   PB1  - TIM3_CH4
 * 
 * WIRING:
 *   PA0 → 220Ω → LED → GND
 */
