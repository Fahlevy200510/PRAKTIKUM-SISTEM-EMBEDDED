/**
 * @file main.cpp
 * @brief Program 10: Bit Manipulation dengan BSRR Register - STM32
 * 
 * Demonstrasi akses register langsung untuk kontrol GPIO yang lebih cepat.
 * BSRR (Bit Set Reset Register) memungkinkan atomic bit manipulation.
 */

#include <Arduino.h>

// Direct register access for GPIOA
#define GPIOA_BASE      0x40010800
#define GPIOA_BSRR      (*(volatile uint32_t*)(GPIOA_BASE + 0x10))
#define GPIOA_ODR       (*(volatile uint32_t*)(GPIOA_BASE + 0x0C))

#define LED_1           PA0     // Bit 0
#define LED_2           PA1     // Bit 1
#define LED_3           PA2     // Bit 2
#define LED_4           PA3     // Bit 3

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("Program 10: Bit Manipulation BSRR - STM32\n");
    
    pinMode(LED_1, OUTPUT);
    pinMode(LED_2, OUTPUT);
    pinMode(LED_3, OUTPUT);
    pinMode(LED_4, OUTPUT);
    
    Serial.println("BSRR Register:");
    Serial.println("  Bits 0-15: Set bits (write 1 to set pin HIGH)");
    Serial.println("  Bits 16-31: Reset bits (write 1 to set pin LOW)");
    Serial.println("\nAtomic operation - no read-modify-write needed!\n");
}

void loop() {
    // Method 1: Using BSRR for atomic set
    Serial.println("Set PA0-PA3 using BSRR...");
    GPIOA_BSRR = 0x0000000F;  // Set bits 0-3 (PA0-PA3 HIGH)
    delay(500);
    
    // Method 2: Using BSRR for atomic reset
    Serial.println("Reset PA0-PA3 using BSRR...");
    GPIOA_BSRR = 0x000F0000;  // Reset bits 0-3 (PA0-PA3 LOW)
    delay(500);
    
    // Binary counter using BSRR
    Serial.println("\nBinary counter (0-15):");
    for (int i = 0; i <= 15; i++) {
        // Reset all first, then set needed bits
        GPIOA_BSRR = 0x000F0000 | (i & 0x0F);
        Serial.printf("  %2d = B%d%d%d%d\n", i,
                     (i>>3)&1, (i>>2)&1, (i>>1)&1, i&1);
        delay(200);
    }
    
    delay(1000);
}

/**
 * BSRR REGISTER EXPLANATION:
 * 
 * 32-bit register split into two halves:
 * 
 * Bits 31-16: BR (Bit Reset)
 *   Write 1 = Set corresponding pin to LOW
 *   Write 0 = No change
 * 
 * Bits 15-0: BS (Bit Set)
 *   Write 1 = Set corresponding pin to HIGH
 *   Write 0 = No change
 * 
 * Example: BSRR = 0x00020001
 *   - Bit 0 (BS0) = 1 → PA0 = HIGH
 *   - Bit 17 (BR1) = 1 → PA1 = LOW
 * 
 * Advantages over ODR:
 * 1. Atomic (no race condition)
 * 2. No read-modify-write cycle
 * 3. Can set and reset different pins simultaneously
 */
