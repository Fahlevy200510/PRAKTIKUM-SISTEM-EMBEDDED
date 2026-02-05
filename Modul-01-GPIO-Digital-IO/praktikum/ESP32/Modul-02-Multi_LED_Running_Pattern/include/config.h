/**
 * @file config.h
 * @brief Konfigurasi Pin - Program 02 Multi-LED Running
 */

#ifndef CONFIG_H
#define CONFIG_H

// ==================== LED PINS ====================
#define NUM_LEDS        4           // Jumlah LED
#define LED_1           2           // GPIO2
#define LED_2           4           // GPIO4
#define LED_3           5           // GPIO5
#define LED_4           18          // GPIO18

// Array of LED pins
const uint8_t LED_PINS[] = {LED_1, LED_2, LED_3, LED_4};

// ==================== TIMING ====================
#define PATTERN_DELAY   150         // Delay antar pattern (ms)
#define SERIAL_BAUD     115200

// ==================== PATTERNS ====================
#define PATTERN_RUNNING     0       // LED berjalan satu arah
#define PATTERN_BOUNCE      1       // LED bolak-balik
#define PATTERN_FILL        2       // LED mengisi satu per satu
#define PATTERN_BLINK_ALL   3       // Semua LED blink bersamaan

#endif // CONFIG_H
