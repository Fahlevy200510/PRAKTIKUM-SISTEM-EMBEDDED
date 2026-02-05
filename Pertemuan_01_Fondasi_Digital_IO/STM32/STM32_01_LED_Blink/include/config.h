/**
 * @file config.h
 * @brief Konfigurasi Pin STM32F103C8T6 - Program 01 LED Blink
 */

#ifndef CONFIG_H
#define CONFIG_H

// ==================== PIN DEFINITIONS ====================
// STM32F103C8T6 Blue Pill
#define LED_BUILTIN_PIN PC13    // On-board LED (active LOW!)
#define LED_EXTERNAL    PA0     // External LED

// ==================== TIMING ====================
#define BLINK_DELAY_MS  500
#define SERIAL_BAUD     115200

// ==================== CATATAN PENTING ====================
// PC13 LED on Blue Pill adalah ACTIVE LOW!
// digitalWrite(PC13, LOW)  = LED ON
// digitalWrite(PC13, HIGH) = LED OFF

#endif // CONFIG_H
