# Program 01: LED Blink - Dasar GPIO Output

## Tujuan Pembelajaran
- Memahami konfigurasi GPIO sebagai output
- Menggunakan fungsi `pinMode()` dan `digitalWrite()`
- Implementasi non-blocking delay dengan `millis()`
- Dasar debugging dengan Serial Monitor

## Komponen yang Dibutuhkan
| Komponen | Jumlah | Keterangan |
|----------|--------|------------|
| ESP32 DevKitC | 1 | Board utama |
| LED | 1 | Built-in atau external |
| Resistor 220Ω | 1 | Untuk LED external |
| Breadboard | 1 | Opsional |
| Kabel Jumper | Secukupnya | Opsional |

## Wiring Diagram
```
   ESP32 DevKitC
   ┌─────────────┐
   │             │
   │        GPIO2├───┬───[220Ω]───[LED+]
   │             │   │              │
   │         GND ├───┴──────────[LED-]
   │             │
   └─────────────┘

   Note: GPIO2 = LED Built-in (tidak perlu external LED)
```

## Cara Upload
1. Buka folder ini di VS Code dengan PlatformIO
2. Hubungkan ESP32 ke komputer via USB
3. Klik tombol Upload (→) atau tekan `Ctrl+Alt+U`
4. Buka Serial Monitor (`Ctrl+Alt+S`) untuk melihat output

## Expected Output
```
========================================
Program 01: LED Blink - ESP32
Praktikum Sistem Embedded
========================================

LED Pin: GPIO2
Blink Interval: 500 ms
Program dimulai...

[500 ms] LED: ON  | Blink #1
[1000 ms] LED: OFF | Blink #2
[1500 ms] LED: ON  | Blink #3
```

## Penjelasan Kode

### 1. Konfigurasi Pin
```cpp
pinMode(LED_PIN, OUTPUT);    // Set GPIO sebagai output
digitalWrite(LED_PIN, LOW);  // Inisialisasi LED mati
```

### 2. Non-Blocking Blink
```cpp
if (currentMillis - previousMillis >= BLINK_DELAY_MS) {
    previousMillis = currentMillis;
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
}
```
Menggunakan `millis()` instead of `delay()` agar program tidak blocking.

## Troubleshooting
| Masalah | Solusi |
|---------|--------|
| LED tidak menyala | Cek polaritas LED dan nilai resistor |
| Upload gagal | Tekan tombol BOOT saat upload |
| Serial tidak tampil | Pastikan baud rate = 115200 |

## Eksperimen Lanjutan
1. Ubah `BLINK_DELAY_MS` ke 100 atau 1000
2. Tambahkan LED kedua di GPIO lain
3. Buat pola blink SOS (... --- ...)
