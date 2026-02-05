/**
 * @file config.h
 * @brief Konfigurasi - Program 03 LED Breathing (LEDC PWM)
 */

#ifndef CONFIG_H
#define CONFIG_H

// ==================== LED PIN ====================
#define LED_PIN             2       // GPIO untuk LED

// ==================== LEDC CONFIG ====================
#define LEDC_CHANNEL        0       // Channel LEDC (0-15)
#define LEDC_FREQ           5000    // Frekuensi PWM (Hz)
#define LEDC_RESOLUTION     8       // Resolusi 8-bit (0-255)

// ==================== BREATHING CONFIG ====================
#define BREATH_STEP         5       // Step increment/decrement
#define BREATH_DELAY        20      // Delay per step (ms)
#define BREATH_MIN          0       // Nilai minimum PWM
#define BREATH_MAX          255     // Nilai maksimum PWM

// ==================== SERIAL ====================
#define SERIAL_BAUD         115200

#endif // CONFIG_H
