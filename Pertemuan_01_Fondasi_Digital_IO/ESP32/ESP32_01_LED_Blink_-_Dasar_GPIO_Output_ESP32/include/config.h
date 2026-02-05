/**
 * @file config.h
 * @brief Konfigurasi Pin dan Konstanta - Program 01 LED Blink
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#ifndef CONFIG_H
#define CONFIG_H

// ==================== PIN DEFINITIONS ====================
#define LED_PIN         2       // LED Built-in ESP32 DevKitC (GPIO2)
#define LED_EXTERNAL    4       // LED External (opsional)

// ==================== TIMING CONSTANTS ====================
#define BLINK_DELAY_MS  500     // Delay blink dalam milidetik
#define BLINK_FAST_MS   100     // Blink cepat
#define BLINK_SLOW_MS   1000    // Blink lambat

// ==================== DEBUG ====================
#define DEBUG_SERIAL    1       // 1 = aktifkan serial debug
#define SERIAL_BAUD     115200  // Baud rate serial

#endif // CONFIG_H
