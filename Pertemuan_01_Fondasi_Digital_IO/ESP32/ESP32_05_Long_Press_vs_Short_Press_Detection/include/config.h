/**
 * @file config.h
 * @brief Konfigurasi - Program 05 Long/Short Press
 */

#ifndef CONFIG_H
#define CONFIG_H

// ==================== PIN DEFINITIONS ====================
#define BUTTON_PIN      0       // GPIO0 - Boot button
#define LED_1           2       // LED untuk short press
#define LED_2           4       // LED untuk long press

// ==================== TIMING CONFIG ====================
#define DEBOUNCE_MS     50      // Debounce time
#define SHORT_PRESS_MIN 50      // Minimum untuk short press
#define SHORT_PRESS_MAX 500     // Maximum untuk short press
#define LONG_PRESS_MIN  1000    // Minimum untuk long press
#define VERY_LONG_MS    3000    // Very long press threshold

// ==================== SERIAL ====================
#define SERIAL_BAUD     115200

#endif // CONFIG_H
