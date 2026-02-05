# JOBSHEET BAB 02: Interrupt dan Timer

## 📋 Informasi Praktikum

| Item | Keterangan |
|------|------------|
| **Topik** | External Interrupt dan Hardware Timer |
| **Platform** | STM32F103C8T6 (Blue Pill), ESP32 DevKitC |
| **Framework** | Arduino (PlatformIO) |
| **Jumlah Program STM32** | 12 |
| **Jumlah Program ESP32** | 12 |
| **Durasi** | 3 x 50 menit |

---

## 🎯 Tujuan Praktikum

Setelah menyelesaikan praktikum ini, mahasiswa mampu:

1. Memahami perbedaan mekanisme polling dan interrupt
2. Mengkonfigurasi External Interrupt pada STM32 dan ESP32
3. Mengimplementasikan Hardware Timer dengan interrupt
4. Menerapkan teknik debouncing berbasis interrupt
5. Mengembangkan aplikasi real-time dengan timer
6. Melakukan debugging interrupt dan timer

---

## 🔧 Alat dan Bahan

### Hardware

| No | Komponen | Jumlah | Keterangan |
|----|----------|--------|------------|
| 1 | STM32F103C8T6 (Blue Pill) | 1 | ARM Cortex-M3, 72MHz |
| 2 | ESP32 DevKitC | 1 | Dual-core, 240MHz |
| 3 | ST-Link V2 | 1 | Programmer STM32 |
| 4 | USB Cable Micro | 2 | Power & programming |
| 5 | Push Button | 4 | Tactile switch |
| 6 | LED 5mm | 4 | Merah, Kuning, Hijau, Biru |
| 7 | Resistor 330Ω | 4 | Current limiting LED |
| 8 | Resistor 10kΩ | 4 | Pull-up/pull-down |
| 9 | Breadboard | 1 | 830 tie-points |
| 10 | Kabel Jumper | 20 | Male-Male |
| 11 | Multimeter | 1 | Opsional, untuk debugging |
| 12 | Oscilloscope | 1 | Opsional, untuk timing analysis |

### Software

| No | Software | Versi | Keterangan |
|----|----------|-------|------------|
| 1 | VS Code | Latest | IDE utama |
| 2 | PlatformIO | Latest | Build system |
| 3 | STM32 Platform | ststm32 | Platform STM32 |
| 4 | ESP32 Platform | espressif32 | Platform ESP32 |
| 5 | Serial Monitor | Built-in | Debugging output |

---

## 📐 Konfigurasi Pin

### STM32F103C8T6 Pin Assignment

```
┌─────────────────────────────────────────────────────────────┐
│                    STM32F103C8T6 Pinout                      │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│    GND ─┤ 1      │USB│     40 ├─ 5V                         │
│   PC13 ─┤ 2     │PORT│     39 ├─ GND                        │
│   PC14 ─┤ 3      └───┘     38 ├─ 3.3V                       │
│   PC15 ─┤ 4                37 ├─ PB9                        │
│    PA0 ─┤ 5  ◄── BTN1      36 ├─ PB8                        │
│    PA1 ─┤ 6  ◄── BTN2      35 ├─ PB7                        │
│    PA2 ─┤ 7                34 ├─ PB6                        │
│    PA3 ─┤ 8                33 ├─ PB5 ──► LED3               │
│    PA4 ─┤ 9                32 ├─ PB4 ──► LED2               │
│    PA5 ─┤ 10               31 ├─ PB3 ──► LED1               │
│    PA6 ─┤ 11               30 ├─ PA15                       │
│    PA7 ─┤ 12               29 ├─ PA12                       │
│    PB0 ─┤ 13 ◄── BTN3      28 ├─ PA11                       │
│    PB1 ─┤ 14 ◄── BTN4      27 ├─ PA10                       │
│   PB10 ─┤ 15               26 ├─ PA9                        │
│   PB11 ─┤ 16               25 ├─ PA8                        │
│    RST ─┤ 17               24 ├─ PB15                       │
│   3.3V ─┤ 18               23 ├─ PB14                       │
│    GND ─┤ 19               22 ├─ PB13                       │
│    GND ─┤ 20               21 ├─ PB12                       │
│                                                             │
│   PC13 = Built-in LED (Active LOW)                          │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

**Tabel Pin Assignment STM32:**

| Fungsi | Pin | Mode | Keterangan |
|--------|-----|------|------------|
| LED_BUILTIN | PC13 | OUTPUT | Active LOW |
| LED1 | PB3 | OUTPUT | Active HIGH |
| LED2 | PB4 | OUTPUT | Active HIGH |
| LED3 | PB5 | OUTPUT | Active HIGH |
| BTN1 | PA0 | INPUT_PULLUP | EXTI0 |
| BTN2 | PA1 | INPUT_PULLUP | EXTI1 |
| BTN3 | PB0 | INPUT_PULLUP | EXTI0 (alternate) |
| BTN4 | PB1 | INPUT_PULLUP | EXTI1 (alternate) |

### ESP32 DevKitC Pin Assignment

```
┌─────────────────────────────────────────────────────────────┐
│                    ESP32 DevKitC Pinout                      │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│     3.3V ─┤ 1                              38 ├─ GND        │
│       EN ─┤ 2                              37 ├─ GPIO23     │
│  VP (36) ─┤ 3     ┌────────────────┐       36 ├─ GPIO22     │
│  VN (39) ─┤ 4     │                │       35 ├─ TX0        │
│   GPIO34 ─┤ 5     │     ESP32      │       34 ├─ RX0        │
│   GPIO35 ─┤ 6     │                │       33 ├─ GPIO21     │
│   GPIO32 ─┤ 7     │   DevKitC      │       32 ├─ GND        │
│   GPIO33 ─┤ 8     │                │       31 ├─ GPIO19     │
│   GPIO25 ─┤ 9     └────────────────┘       30 ├─ GPIO18──►LED3│
│   GPIO26 ─┤ 10                             29 ├─ GPIO5 ──►LED2│
│   GPIO27 ─┤ 11                             28 ├─ GPIO17     │
│   GPIO14 ─┤ 12                             27 ├─ GPIO16     │
│   GPIO12 ─┤ 13                             26 ├─ GPIO4 ──►LED1│
│    GND   ─┤ 14                             25 ├─ GPIO0◄──BTN_BOOT│
│   GPIO13 ─┤ 15◄──BTN3                      24 ├─ GPIO2 ──►LED_BUILTIN│
│    SD2   ─┤ 16                             23 ├─ GPIO15◄──BTN4│
│    SD3   ─┤ 17                             22 ├─ SD1        │
│    CMD   ─┤ 18                             21 ├─ SD0        │
│    5V    ─┤ 19                             20 ├─ CLK        │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

**Tabel Pin Assignment ESP32:**

| Fungsi | Pin | Mode | Keterangan |
|--------|-----|------|------------|
| LED_BUILTIN | GPIO2 | OUTPUT | Active HIGH |
| LED1 | GPIO4 | OUTPUT | Active HIGH |
| LED2 | GPIO5 | OUTPUT | Active HIGH |
| LED3 | GPIO18 | OUTPUT | Active HIGH |
| BTN1 | GPIO0 | INPUT_PULLUP | BOOT button |
| BTN2 | GPIO13 | INPUT_PULLUP | External |
| BTN3 | GPIO15 | INPUT_PULLUP | External |
| BTN4 | GPIO14 | INPUT_PULLUP | External |

---

## 🔌 Skema Rangkaian

### Rangkaian Button dengan Pull-up Internal

```
STM32/ESP32:
                    ┌─────────────────────┐
                    │     MCU             │
    ┌───────────────┤ GPIO (INPUT_PULLUP) │
    │               └─────────────────────┘
    │                        │
    │                       ─┴─ Internal Pull-up
    │                       ═══ (10kΩ - 50kΩ)
    │                        │
   ─┴─                       │
   │ │ Push Button          ─┴─
   ─┬─                      GND
    │
   ─┴─
   GND

Saat tidak ditekan: GPIO = HIGH (pulled up)
Saat ditekan: GPIO = LOW (connected to GND)
Trigger: FALLING edge
```

### Rangkaian LED dengan Current Limiting Resistor

```
                    ┌─────────────────┐
                    │     MCU         │
                    │ GPIO (OUTPUT)   ├────┬───────┐
                    └─────────────────┘    │       │
                                          ─┴─     ─┴─
                                          │ │     │ │
                                          ─┬─     ─┬─
                                           │ R1    │ R2
                                          330Ω   330Ω
                                           │       │
                                          ▼│      ▼│
                                         ────    ────
                                         LED1    LED2
                                          │       │
                                         ─┴─     ─┴─
                                         GND     GND
```

---

## 📝 Daftar Program Praktikum

### Program ESP32

| No | Nama Program | Topik | Tingkat |
|----|--------------|-------|---------|
| 1 | Modul-01 | Basic External Interrupt | Dasar |
| 2 | Modul-02 | Multiple Button Interrupt | Dasar |
| 3 | Modul-03 | Interrupt dengan Debounce Software | Menengah |
| 4 | Modul-04 | Basic Hardware Timer | Dasar |
| 5 | Modul-05 | Timer dengan LED Blink | Dasar |
| 6 | Modul-06 | Timer Interrupt Counter | Menengah |
| 7 | Modul-07 | Stopwatch dengan Timer | Menengah |
| 8 | Modul-08 | Frequency Counter | Lanjut |
| 9 | Modul-09 | PWM dengan Timer | Menengah |
| 10 | Modul-10 | Timer + External Interrupt | Lanjut |
| 11 | Modul-11 | Event Counter | Menengah |
| 12 | Modul-12 | Multi-Timer Application | Lanjut |

### Program STM32

| No | Nama Program | Topik | Tingkat |
|----|--------------|-------|---------|
| 1 | STM32_01 | Basic EXTI Interrupt | Dasar |
| 2 | STM32_02 | Multiple EXTI Lines | Dasar |
| 3 | STM32_03 | EXTI dengan Debounce | Menengah |
| 4 | STM32_04 | Basic TIM2 Timer | Dasar |
| 5 | STM32_05 | Timer LED Blink | Dasar |
| 6 | STM32_06 | Timer Update Interrupt | Menengah |
| 7 | STM32_07 | SysTick Timer | Menengah |
| 8 | STM32_08 | Input Capture Mode | Lanjut |
| 9 | STM32_09 | Output Compare Mode | Menengah |
| 10 | STM32_10 | Timer + EXTI Combined | Lanjut |
| 11 | STM32_11 | Pulse Counter | Menengah |
| 12 | STM32_12 | Multi-Timer System | Lanjut |

---

## 📚 Tugas Praktikum

### Tugas 1: External Interrupt (30 menit)

**Tujuan:** Memahami dan mengimplementasikan external interrupt

**Langkah Kerja:**

1. **Persiapan Hardware (10 menit)**
   - Hubungkan button ke pin PA0 (STM32) atau GPIO0 (ESP32)
   - Hubungkan LED ke pin PB3 (STM32) atau GPIO4 (ESP32)
   - Verifikasi koneksi dengan multimeter

2. **Implementasi STM32 (10 menit)**
   - Buka program `STM32_01`
   - Compile dan upload
   - Tekan button, amati LED toggle
   - Catat waktu respons

3. **Implementasi ESP32 (10 menit)**
   - Buka program `Modul-01`
   - Compile dan upload
   - Tekan button, amati LED toggle
   - Bandingkan dengan STM32

**Pertanyaan Analisis:**
1. Apa perbedaan konfigurasi interrupt antara STM32 dan ESP32?
2. Mengapa ESP32 memerlukan `IRAM_ATTR`?
3. Apa yang terjadi jika tidak clear interrupt flag di STM32?

---

### Tugas 2: Hardware Timer (30 menit)

**Tujuan:** Mengkonfigurasi dan menggunakan hardware timer

**Langkah Kerja:**

1. **Timer Basic Setup (15 menit)**
   - **STM32:** Buka `STM32_04`
     - Hitung prescaler dan ARR untuk periode 1 detik
     - Verifikasi dengan stopwatch
   - **ESP32:** Buka `Modul-04`
     - Konfigurasi timer 1MHz (prescaler 80)
     - Set alarm 1 detik

2. **Timer LED Blink (15 menit)**
   - **STM32:** Buka `STM32_05`
     - Modifikasi periode menjadi 500ms
   - **ESP32:** Buka `Modul-05`
     - Modifikasi periode menjadi 500ms

**Perhitungan Timer STM32:**
```
Clock = 72 MHz
Target Period = 1 second

Timer_Freq = 72MHz / (PSC + 1)
Period = (ARR + 1) / Timer_Freq

Jika PSC = 7199:
Timer_Freq = 72MHz / 7200 = 10kHz
ARR = 10kHz × 1s - 1 = 9999
```

**Pertanyaan Analisis:**
1. Bagaimana menghitung nilai PSC dan ARR untuk periode 250ms?
2. Apa keuntungan timer 64-bit ESP32 dibanding 16-bit STM32?
3. Mengapa perlu `portENTER_CRITICAL()` di ESP32?

---

### Tugas 3: Debouncing (20 menit)

**Tujuan:** Implementasi debouncing menggunakan timer

**Langkah Kerja:**

1. **Tanpa Debounce**
   - Buka `STM32_01` atau `Modul-01`
   - Tekan button cepat, amati counter
   - Catat jumlah "false trigger"

2. **Dengan Debounce**
   - Buka `STM32_03` atau `Modul-03`
   - Tekan button dengan cara yang sama
   - Bandingkan hasil dengan langkah 1

**Dokumentasi:**
| Percobaan | Tanpa Debounce | Dengan Debounce |
|-----------|----------------|-----------------|
| Press 1 | | |
| Press 2 | | |
| Press 3 | | |
| Press 4 | | |
| Press 5 | | |
| **Total False** | | |

---

### Tugas 4: Integrasi (30 menit)

**Tujuan:** Menggabungkan interrupt dan timer

**Langkah Kerja:**

1. **Event Counter dengan Timer Gate**
   - Buka `STM32_10` atau `Modul-10`
   - Sistem akan menghitung pulse input selama periode tertentu
   - Gunakan button press sebagai input

2. **Stopwatch Implementation**
   - Buka `STM32_07` atau `Modul-07`
   - Start/Stop dengan button interrupt
   - Display time via Serial

**Tugas Pengembangan:**
Modifikasi program untuk membuat:
- Reaction Time Tester
  - LED menyala random setelah 1-5 detik
  - Ukur waktu user menekan button
  - Tampilkan hasil di Serial Monitor

---

## 📊 Rubrik Penilaian Praktikum

| Komponen | Bobot | Kriteria |
|----------|-------|----------|
| **Implementasi** | 40% | Semua program berjalan dengan benar |
| | | - Excellent (36-40): Semua program + modifikasi |
| | | - Good (28-35): Semua program dasar |
| | | - Fair (20-27): Sebagian besar program |
| | | - Poor (<20): Sedikit program berhasil |
| **Laporan** | 30% | Dokumentasi lengkap dan analisis mendalam |
| | | - Excellent (27-30): Analisis komprehensif |
| | | - Good (21-26): Analisis memadai |
| | | - Fair (15-20): Dokumentasi minimal |
| | | - Poor (<15): Tidak lengkap |
| **Pemahaman** | 20% | Menjawab pertanyaan dengan benar |
| | | - Excellent (18-20): Jawaban detail |
| | | - Good (14-17): Jawaban benar |
| | | - Fair (10-13): Sebagian benar |
| | | - Poor (<10): Banyak kesalahan |
| **Keaktifan** | 10% | Partisipasi dan inisiatif |
| | | - Excellent (9-10): Sangat aktif |
| | | - Good (7-8): Aktif |
| | | - Fair (5-6): Cukup aktif |
| | | - Poor (<5): Pasif |

---

## 📝 Format Laporan Praktikum

### Struktur Laporan

1. **Cover** (1 halaman)
   - Judul: Praktikum 02 - Interrupt dan Timer
   - Nama dan NIM
   - Tanggal praktikum

2. **Tujuan** (0.5 halaman)
   - List tujuan dari jobsheet

3. **Dasar Teori** (1-2 halaman)
   - Ringkasan konsep interrupt
   - Ringkasan konsep timer
   - Perbandingan STM32 vs ESP32

4. **Metodologi** (1 halaman)
   - Alat dan bahan
   - Diagram rangkaian
   - Prosedur kerja

5. **Hasil dan Analisis** (3-4 halaman)
   - Screenshot hasil setiap tugas
   - Tabel pengukuran
   - Analisis perbandingan
   - Jawaban pertanyaan

6. **Kesimpulan** (0.5 halaman)
   - Ringkasan pembelajaran
   - Tantangan yang dihadapi
   - Saran pengembangan

7. **Lampiran**
   - Source code modifikasi
   - Data mentah
   - Screenshot tambahan

---

## ⚠️ Troubleshooting

### Masalah Umum Interrupt

| Masalah | Penyebab | Solusi |
|---------|----------|--------|
| ISR tidak terpanggil | NVIC tidak di-enable | Cek `NVIC_EnableIRQ()` |
| ISR terpanggil terus | Flag tidak di-clear | Tambahkan clear flag |
| Button tidak responsif | Pull-up tidak aktif | Cek `INPUT_PULLUP` |
| ESP32 crash | IRAM_ATTR hilang | Tambahkan `IRAM_ATTR` |
| Double trigger | Bouncing | Implementasi debounce |

### Masalah Umum Timer

| Masalah | Penyebab | Solusi |
|---------|----------|--------|
| Timer tidak jalan | Clock tidak enabled | Enable RCC clock |
| Periode tidak akurat | PSC/ARR salah | Hitung ulang nilai |
| Interrupt tidak terjadi | Update interrupt disable | Set `UIE` bit |
| Counter overflow | ARR terlalu kecil | Gunakan prescaler lebih besar |

---

## 📚 Referensi Tambahan

1. **STM32F103 Reference Manual** - Chapter 10: General-purpose timers
2. **ESP32 Technical Reference Manual** - Chapter 17: Timer Group
3. **ARM Cortex-M3 Definitive Guide** - Chapter 7: Exceptions and Interrupts
4. [STM32 EXTI Application Note AN4228](https://www.st.com/resource/en/application_note/an4228.pdf)
5. [ESP-IDF Timer Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gptimer.html)

