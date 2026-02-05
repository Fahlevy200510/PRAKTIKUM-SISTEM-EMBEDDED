# JOBSHEET BAB 01: GPIO dan Digital I/O

## 📋 Informasi Praktikum

| Item | Keterangan |
|------|------------|
| **Topik** | GPIO dan Digital I/O |
| **Platform** | STM32F103C8T6 (Blue Pill), ESP32 DevKitC |
| **Jumlah Program STM32** | 12 program |
| **Jumlah Program ESP32** | 12 program |
| **Durasi** | 3 x 50 menit |
| **Tools** | PlatformIO, VS Code, Serial Monitor |

---

## 🎯 Tujuan Praktikum

Setelah menyelesaikan praktikum ini, mahasiswa mampu:

1. Memahami konsep dasar GPIO (General Purpose Input/Output)
2. Mengkonfigurasi pin sebagai input dan output pada STM32 dan ESP32
3. Mengimplementasikan teknik debouncing untuk push button
4. Membuat berbagai pola LED menggunakan GPIO
5. Memahami perbedaan karakteristik GPIO antara STM32 dan ESP32
6. Menerapkan best practices dalam pemrograman embedded systems

---

## 🔧 Peralatan yang Diperlukan

### Hardware
| No | Komponen | Jumlah | Keterangan |
|----|----------|--------|------------|
| 1 | STM32F103C8T6 Blue Pill | 1 | Development board |
| 2 | ESP32 DevKitC | 1 | Development board |
| 3 | ST-Link V2 | 1 | Programmer STM32 |
| 4 | LED 5mm (berbagai warna) | 8 | Merah, Hijau, Kuning |
| 5 | Resistor 220Ω | 8 | Untuk LED |
| 6 | Push Button | 4 | Tactile switch |
| 7 | DIP Switch 4-bit | 1 | Untuk input multiple |
| 8 | Breadboard | 1 | Full-size |
| 9 | Kabel Jumper | 20 | Male-Male |
| 10 | Kabel USB | 2 | Micro USB / USB-C |

### Software
- Visual Studio Code dengan PlatformIO Extension
- Serial Monitor (PlatformIO atau Putty)
- Git (opsional, untuk version control)

---

## 📐 Skema Rangkaian

### Rangkaian LED (untuk kedua platform)

```
                    LED OUTPUT CIRCUIT
                    
    GPIO Pin ────┬────[220Ω]────[LED]────┐
                 │                       │
                 │      Anode   Cathode  │
                 │        (+)     (-)    │
                 │                       │
                 └───────────────────────┴──── GND
                 
    Catatan:
    - Resistor 220Ω membatasi arus ke ~10mA
    - LED memerlukan ~2V (merah) sampai ~3V (biru/putih)
```

### Rangkaian Button dengan Pull-up Internal

```
                    BUTTON INPUT CIRCUIT
                    
    3.3V ────[Internal Pull-up ~40kΩ]────┬──── GPIO Pin
                                         │
                                    ┌────┴────┐
                                    │  BUTTON │
                                    └────┬────┘
                                         │
                                        GND
    
    Status:
    - Button TIDAK ditekan: GPIO = HIGH (3.3V)
    - Button DITEKAN: GPIO = LOW (0V)
```

### Pin Assignment STM32F103C8T6

| Fungsi | Pin | Keterangan |
|--------|-----|------------|
| LED Built-in | PC13 | Active LOW |
| LED External 1 | PA0 | Active HIGH |
| LED External 2 | PA1 | Active HIGH |
| LED External 3 | PA2 | Active HIGH |
| LED External 4 | PA3 | Active HIGH |
| Button 1 | PB0 | Pull-up internal |
| Button 2 | PB1 | Pull-up internal |
| Serial TX | PA9 | USART1 |
| Serial RX | PA10 | USART1 |

### Pin Assignment ESP32 DevKitC

| Fungsi | Pin | Keterangan |
|--------|-----|------------|
| LED Built-in | GPIO2 | Active HIGH |
| LED External 1 | GPIO4 | Active HIGH |
| LED External 2 | GPIO5 | Active HIGH |
| LED External 3 | GPIO18 | Active HIGH |
| LED External 4 | GPIO19 | Active HIGH |
| Button 1 | GPIO21 | Pull-up internal |
| Button 2 | GPIO22 | Pull-up internal |
| Serial TX | GPIO1 | UART0 |
| Serial RX | GPIO3 | UART0 |

---

## 📝 Daftar Program Praktikum

### STM32F103C8T6 Programs

| No | Nama Program | Deskripsi | Konsep yang Dipelajari |
|----|--------------|-----------|------------------------|
| 1 | STM32_01_LED_Blink | LED berkedip dasar | GPIO output, timing |
| 2 | STM32_02_Multi-LED_Running_Pattern | Running LED | Array, sequencing |
| 3 | STM32_03_LED_Breathing_Effect | LED fade in/out | Software PWM |
| 4 | STM32_04_Button_Debounce_State_Machine | Debouncing | State machine |
| 5 | STM32_05_Long_Short_Press | Deteksi durasi tekan | Timing, detection |
| 6 | STM32_06_Toggle_Latch_Behavior | Toggle on/off | Latch logic |
| 7 | STM32_07_GPIO_Open-Drain_Mode | Mode open-drain | GPIO modes |
| 8 | STM32_08_DIP_Switch_Reader | Baca DIP switch | Multiple input |
| 9 | STM32_09_LED_Brightness_Control | Kontrol via Serial | Serial + PWM |
| 10 | STM32_10_Bit_Manipulation_BSRR | Register langsung | BSRR register |
| 11 | STM32_11_Emergency_Stop_Logic | Safety interlock | Safety systems |
| 12 | STM32_12_LED_Test_Pattern | Pola diagnostik | Testing patterns |

### ESP32 Programs

| No | Nama Program | Deskripsi | Konsep yang Dipelajari |
|----|--------------|-----------|------------------------|
| 1 | Modul-01-LED_Blink_Dasar_GPIO_Output | LED berkedip dasar | GPIO output, timing |
| 2 | Modul-02-Multi_LED_Running_Pattern | Running LED | Array, sequencing |
| 3 | Modul-03-LED_Breathing_Effect_LEDC_PWM | LED fade in/out | LEDC PWM |
| 4 | Modul-04-Button_Debounce_State_Machine | Debouncing | State machine |
| 5 | Modul-05-Long_Press_vs_Short_Press | Deteksi durasi tekan | Timing, detection |
| 6 | Modul-06-Toggle_LED_Latch_Behavior | Toggle on/off | Latch logic |
| 7 | Modul-07-GPIO_Drive_Strength | Drive strength config | GPIO capability |
| 8 | Modul-08-DIP_Switch_Reader | Baca DIP switch | Multiple input |
| 9 | Modul-09-LED_Brightness_Control | Kontrol via Serial | Serial + PWM |
| 10 | Modul-10-GPIO_Matrix | Keypad matrix | Matrix scanning |
| 11 | Modul-11-Emergency_Stop_Logic | Safety interlock | Safety systems |
| 12 | Modul-12-LED_Test_Pattern | Pola diagnostik | Testing patterns |

---

## 📋 Langkah Praktikum

### Persiapan Awal (15 menit)

1. **Clone/Download Repository**
   ```bash
   cd ~/Documents
   git clone [repository-url] praktikum-embedded
   ```

2. **Buka Project di VS Code**
   ```bash
   code praktikum-embedded
   ```

3. **Install PlatformIO Extension** (jika belum)
   - Buka Extensions (Ctrl+Shift+X)
   - Cari "PlatformIO IDE"
   - Install dan restart VS Code

4. **Rangkai Hardware**
   - Pasang LED dan resistor sesuai skema
   - Hubungkan button ke pin yang ditentukan
   - Hubungkan ST-Link ke STM32 (SWDIO, SWCLK, GND, 3.3V)
   - Hubungkan ESP32 via USB

### Praktikum 1: LED Blink (20 menit)

**Tujuan:** Memahami dasar GPIO output dan timing

**Langkah:**

1. Buka folder `praktikum/ESP32/Modul-01-LED_Blink_Dasar_GPIO_Output`

2. Perhatikan struktur project:
   ```
   ├── src/
   │   └── main.cpp      # Program utama
   ├── include/
   │   └── config.h      # Konfigurasi pin
   └── platformio.ini    # Konfigurasi PlatformIO
   ```

3. Pelajari kode `main.cpp`:
   ```cpp
   // Setup: Konfigurasi pin sebagai OUTPUT
   pinMode(LED_PIN, OUTPUT);
   
   // Loop: Toggle LED dengan delay
   digitalWrite(LED_PIN, HIGH);
   delay(500);
   digitalWrite(LED_PIN, LOW);
   delay(500);
   ```

4. Build dan Upload:
   - Klik tombol Build (✓) di status bar
   - Klik tombol Upload (→) untuk upload ke board
   - Buka Serial Monitor (baud rate: 115200)

5. **Modifikasi yang harus dilakukan:**
   - Ubah interval blink menjadi 250ms
   - Tambahkan counter untuk menghitung jumlah blink
   - Dokumentasikan hasilnya

**Untuk STM32:**
- Buka folder `praktikum/STM32/STM32_01_LED_Blink`
- Perhatikan perbedaan: PC13 adalah **active LOW**
- Build dan upload menggunakan ST-Link

### Praktikum 2: Button Debouncing (25 menit)

**Tujuan:** Memahami masalah bouncing dan solusinya

**Langkah:**

1. Buka program Button_Debounce_State_Machine

2. **Eksperimen Tanpa Debouncing:**
   - Modifikasi kode untuk menghilangkan debouncing
   - Tekan button dan perhatikan output Serial
   - Catat berapa kali button terdeteksi ditekan

3. **Eksperimen Dengan Debouncing:**
   - Kembalikan kode debouncing
   - Tekan button dan perhatikan perbedaannya
   - Catat hasilnya

4. **Analisis:**
   - Jelaskan mengapa terjadi bouncing
   - Jelaskan bagaimana state machine mengatasi bouncing
   - Gambarkan diagram state machine

### Praktikum 3: Running LED Pattern (25 menit)

**Tujuan:** Memahami array dan sequencing dalam embedded

**Langkah:**

1. Buka program Multi_LED_Running_Pattern

2. **Pelajari Konsep:**
   ```cpp
   // Array pin LED
   const int ledPins[] = {PA0, PA1, PA2, PA3};
   const int numLeds = sizeof(ledPins) / sizeof(ledPins[0]);
   
   // Running pattern
   for (int i = 0; i < numLeds; i++) {
       digitalWrite(ledPins[i], HIGH);
       delay(100);
       digitalWrite(ledPins[i], LOW);
   }
   ```

3. **Modifikasi:**
   - Buat pola ping-pong (maju-mundur)
   - Buat pola blink semua LED bersamaan
   - Buat pola random

4. **Dokumentasikan** setiap pola dalam laporan

### Praktikum 4: Serial Control LED (20 menit)

**Tujuan:** Integrasi GPIO dengan komunikasi Serial

**Langkah:**

1. Buka program LED_Brightness_Control

2. **Kirim perintah via Serial Monitor:**
   ```
   Ketik "1" → LED 1 ON
   Ketik "0" → LED 1 OFF
   Ketik "+" → Brightness naik
   Ketik "-" → Brightness turun
   ```

3. **Implementasikan command parser:**
   - Parsing perintah string
   - Response feedback ke Serial

---

## 📊 Tugas Praktikum

### Tugas 1: Dokumentasi Program (Individu)
**Deadline:** Akhir sesi praktikum

Untuk setiap program yang dijalankan, dokumentasikan:
- Screenshot Serial Monitor output
- Foto rangkaian hardware
- Penjelasan cara kerja program (dalam komentar kode)

### Tugas 2: Modifikasi dan Analisis (Individu)
**Deadline:** H+3 setelah praktikum

Pilih 3 program dan lakukan modifikasi:
1. Ubah parameter (timing, pin, pattern)
2. Tambahkan fitur baru
3. Dokumentasikan perbedaan perilaku

### Tugas 3: Mini Project GPIO (Kelompok 2-3 orang)
**Deadline:** H+7 setelah praktikum

Buat salah satu aplikasi berikut:
- **Traffic Light Controller:** 3 LED dengan timing realistic
- **Binary Counter:** 4 LED menampilkan hitungan 0-15
- **Simon Says Game:** LED pattern memory game
- **Morse Code Transmitter:** Input teks, output LED morse code

**Deliverables:**
- Kode program yang dapat dicompile
- Video demonstrasi (max 3 menit)
- Laporan singkat (max 2 halaman)

---

## 📊 Rubrik Penilaian Praktikum

| Komponen | Bobot | Kriteria |
|----------|-------|----------|
| **Kehadiran & Partisipasi** | 10% | Hadir tepat waktu, aktif bertanya |
| **Implementasi Program** | 30% | Semua program berhasil dijalankan |
| **Modifikasi & Eksperimen** | 25% | Modifikasi kreatif dan analisis benar |
| **Dokumentasi** | 20% | Lengkap, rapi, screenshot jelas |
| **Mini Project** | 15% | Fungsional, kreatif, presentasi |

### Kriteria Penilaian Detail

**A (85-100):** Semua tugas selesai dengan modifikasi kreatif, dokumentasi lengkap
**B (70-84):** Semua tugas selesai, dokumentasi baik
**C (55-69):** Sebagian besar tugas selesai, dokumentasi cukup
**D (40-54):** Beberapa tugas selesai, dokumentasi minimal
**E (<40):** Tidak menyelesaikan tugas minimum

---

## ❓ Pertanyaan Refleksi

Jawab pertanyaan berikut dalam laporan:

1. Apa perbedaan utama GPIO STM32 dan ESP32 yang Anda temukan?
2. Mengapa debouncing penting dalam membaca input button?
3. Apa kelebihan menggunakan `millis()` dibanding `delay()`?
4. Bagaimana cara menghitung nilai resistor untuk LED?
5. Apa yang terjadi jika GPIO input dibiarkan floating?

---

## 📚 Referensi Tambahan

1. [STM32 GPIO Tutorial - DeepBlue Embedded](https://deepbluembedded.com/stm32-gpio-tutorial/)
2. [ESP32 GPIO Reference - Espressif](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html)
3. [Button Debouncing - Ganssle](http://www.ganssle.com/debouncing.htm)
4. [Arduino GPIO Best Practices](https://www.arduino.cc/en/Tutorial/Foundations/DigitalPins)

---

## 🆘 Troubleshooting

| Masalah | Kemungkinan Penyebab | Solusi |
|---------|---------------------|--------|
| Upload gagal (STM32) | ST-Link tidak terdeteksi | Periksa koneksi SWDIO, SWCLK |
| Upload gagal (ESP32) | Port COM salah | Pilih port yang benar di PlatformIO |
| LED tidak menyala | Polaritas terbalik | Balik arah LED |
| Button tidak responsif | Pull-up tidak aktif | Tambahkan `INPUT_PULLUP` |
| Serial tidak muncul | Baud rate salah | Set ke 115200 |

---

*Jobsheet Praktikum Sistem Embedded - Modul 01*
*Versi 1.0 - Februari 2026*

