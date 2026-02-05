# Prompt untuk Pembuatan PPT - Bagian 1
## Modul 01: GPIO Digital I/O - Teori dan Konsep Dasar

> **Instruksi untuk AI/Designer:** Gunakan prompt di bawah ini untuk membuat slide presentasi. Setiap section mewakili 1-3 slide. Total target: 25-30 slide untuk Bagian 1.

---

## Slide 1: Judul

```
Buatkan slide judul dengan:
- Judul: "GPIO dan Digital I/O"
- Subtitle: "Modul 01 - Praktikum Sistem Embedded"
- Visual: Gambar mikrokontroler STM32 dan ESP32 berdampingan
- Warna tema: Biru teknologi dengan aksen hijau
- Logo institusi di pojok
- Nama pengajar dan tanggal
```

---

## Slide 2: Agenda/Outline

```
Buatkan slide agenda dengan ikon untuk setiap topik:
1. 📚 Pengenalan GPIO
2. 🔧 Arsitektur GPIO pada STM32 dan ESP32
3. ⚡ Mode Operasi GPIO
4. 🔘 Input: Button dan Debouncing
5. 💡 Output: LED Control
6. 🛡️ Best Practices dan Safety
7. 🔬 Demo dan Praktikum
```

---

## Slide 3-4: Apa itu GPIO?

```
Slide 3 - Definisi GPIO:
- Judul: "Apa itu GPIO?"
- Definisi: General Purpose Input/Output - pin serbaguna pada mikrokontroler
- Analogi: "GPIO seperti saklar dan sensor universal pada robot"
- Gambar: Diagram blok sederhana MCU dengan GPIO pins
- Highlight bahwa GPIO adalah building block paling dasar

Slide 4 - Aplikasi GPIO:
- Judul: "Aplikasi GPIO dalam Dunia Nyata"
- Grid 2x3 dengan gambar dan caption:
  * LED indicator (lampu status)
  * Push button (input user)
  * Relay control (on/off beban besar)
  * Sensor digital (PIR, limit switch)
  * Motor driver enable
  * Communication bit-banging
```

---

## Slide 5-6: Perbandingan STM32 vs ESP32

```
Slide 5 - Spesifikasi:
- Judul: "STM32F103C8T6 vs ESP32 - GPIO Comparison"
- Tabel perbandingan side-by-side:
  | Fitur | STM32F103 | ESP32 |
  |-------|-----------|-------|
  | Core | ARM Cortex-M3 | Xtensa LX6 |
  | GPIO Count | 37 pins | 34 pins |
  | Voltage | 3.3V (5V tolerant*) | 3.3V only |
  | Max Current | 25mA/pin | 40mA/pin |
  | Built-in LED | PC13 (Active LOW) | GPIO2 (Active HIGH) |

Slide 6 - Visualisasi Pinout:
- Judul: "Pinout Diagram"
- Gambar pinout STM32 Blue Pill di kiri
- Gambar pinout ESP32 DevKitC di kanan
- Highlight pin-pin penting dengan warna berbeda
- Catatan: "Perhatikan pin yang tidak boleh digunakan"
```

---

## Slide 7-8: Struktur Internal GPIO

```
Slide 7 - Blok Diagram GPIO:
- Judul: "Struktur Internal GPIO Pin"
- Diagram blok menunjukkan:
  * Input buffer
  * Output driver (P-MOS dan N-MOS)
  * Pull-up/Pull-down resistor internal
  * Protection diodes
- Gunakan animasi untuk menjelaskan aliran sinyal

Slide 8 - Register GPIO:
- Judul: "GPIO Register pada STM32"
- Tabel register dengan fungsi:
  * CRL/CRH - Configuration
  * IDR - Input Data
  * ODR - Output Data
  * BSRR - Bit Set/Reset
- Contoh kode akses register:
  GPIOC->ODR |= (1 << 13);
```

---

## Slide 9-11: Mode GPIO

```
Slide 9 - Input Modes:
- Judul: "GPIO Input Modes"
- Diagram 3 mode input:
  1. Floating (Hi-Z) - Tidak disarankan!
  2. Pull-up - Default HIGH
  3. Pull-down - Default LOW
- Warning box: "Jangan biarkan input floating!"

Slide 10 - Output Modes:
- Judul: "GPIO Output Modes"  
- Diagram perbandingan:
  1. Push-Pull: Dapat drive HIGH dan LOW
  2. Open-Drain: Hanya dapat pull LOW
- Use case masing-masing mode
- Gambar transistor untuk visualisasi

Slide 11 - Memilih Mode yang Tepat:
- Judul: "Kapan Menggunakan Mode Apa?"
- Flowchart decision tree:
  * LED → Push-Pull
  * I2C → Open-Drain
  * Button → Input Pull-up
  * Sensor Active-Low → Input Pull-up
```

---

## Slide 12-14: LED Control

```
Slide 12 - Dasar LED:
- Judul: "Mengendalikan LED"
- Diagram rangkaian LED dengan resistor
- Formula: R = (Vcc - Vf) / If
- Contoh perhitungan untuk LED merah

Slide 13 - Sourcing vs Sinking:
- Judul: "Current Sourcing vs Sinking"
- Diagram side-by-side:
  * Sourcing: GPIO → Resistor → LED → GND
  * Sinking: VCC → Resistor → LED → GPIO
- Kapan menggunakan masing-masing metode

Slide 14 - Active HIGH vs Active LOW:
- Judul: "Memahami Active HIGH dan Active LOW"
- Tabel truth table untuk kedua kasus
- Contoh: STM32 PC13 adalah Active LOW
- Tips: "Selalu cek datasheet untuk polaritas"
```

---

## Slide 15-17: Push Button dan Debouncing

```
Slide 15 - Masalah Bouncing:
- Judul: "Fenomena Bouncing pada Button"
- Grafik oscilloscope showing bounce
- Penjelasan mengapa terjadi (kontak mekanis)
- Durasi typical: 5-50ms

Slide 16 - Solusi Hardware:
- Judul: "Hardware Debouncing"
- Rangkaian RC filter
- Perhitungan time constant: τ = R × C
- Kelebihan dan kekurangan

Slide 17 - Solusi Software:
- Judul: "Software Debouncing"
- Pseudocode state machine:
  if (reading != lastState):
      debounceTimer = now
  if (now - debounceTimer > DEBOUNCE_TIME):
      buttonState = reading
- Kelebihan: Fleksibel, tidak butuh komponen tambahan
```

---

## Slide 18-19: GPIO pada ESP32 - Fitur Khusus

```
Slide 18 - ESP32 Special Features:
- Judul: "Fitur Unik GPIO ESP32"
- List dengan ikon:
  * 🖐️ Touch Sensing (10 pin)
  * 💤 RTC GPIO (deep sleep wake-up)
  * 🔌 GPIO Matrix (flexible routing)
  * ⚡ Configurable drive strength

Slide 19 - Pin Restrictions:
- Judul: "Pin yang Perlu Diperhatikan"
- Warning boxes:
  * GPIO6-11: Flash SPI - JANGAN GUNAKAN
  * GPIO34-39: INPUT ONLY
  * GPIO0: Boot mode
  * GPIO2: LED + boot indicator
```

---

## Slide 20-21: GPIO pada STM32 - Fitur Khusus

```
Slide 20 - STM32 GPIO Features:
- Judul: "Fitur GPIO STM32F103"
- List:
  * 5V tolerant pada banyak pin
  * Alternate function mapping
  * Lock register untuk keamanan
  * Atomic bit operations (BSRR)

Slide 21 - BSRR Register:
- Judul: "Operasi Atomic dengan BSRR"
- Penjelasan bit 0-15 (SET) dan 16-31 (RESET)
- Contoh kode:
  // Set bit
  GPIOC->BSRR = (1 << pin);
  // Reset bit  
  GPIOC->BSRR = (1 << (pin + 16));
- Mengapa lebih baik dari ODR untuk concurrent access
```

---

## Slide 22-24: Best Practices

```
Slide 22 - Do's and Don'ts:
- Judul: "Best Practices GPIO"
- Two columns:
  ✅ DO:
  - Gunakan resistor untuk LED
  - Enable pull-up untuk button
  - Debounce semua input mekanis
  
  ❌ DON'T:
  - Hubungkan 5V ke ESP32
  - Biarkan input floating
  - Exceed max current

Slide 23 - Safety Considerations:
- Judul: "Pertimbangan Keselamatan"
- ESD protection
- Current limiting
- Voltage level matching
- Short circuit prevention

Slide 24 - Coding Standards:
- Judul: "Standar Penulisan Kode"
- Contoh struktur kode yang baik:
  * Pin definitions di config.h
  * Meaningful variable names
  * Non-blocking code dengan millis()
  * Proper commenting
```

---

## Slide 25-26: Rangkuman

```
Slide 25 - Key Takeaways:
- Judul: "Rangkuman Materi"
- 5 poin kunci dengan checkmark:
  ☑ GPIO adalah interface fundamental embedded
  ☑ STM32 dan ESP32 memiliki karakteristik berbeda
  ☑ Selalu gunakan pull resistor untuk input
  ☑ Debouncing wajib untuk button mekanis
  ☑ Perhatikan current limits dan voltage levels

Slide 26 - Preview Praktikum:
- Judul: "Apa yang Akan Kita Pelajari di Lab"
- Grid preview 4 program:
  * LED Blink
  * Button Debounce
  * Running LED
  * Serial Control
- "Mari kita praktikkan!"
```

---

## Slide 27: Q&A

```
Buatkan slide Q&A:
- Judul besar: "Pertanyaan?"
- Gambar ilustrasi mahasiswa bertanya
- Kontak info dosen/asisten
- QR code ke material online (opsional)
```

---

## Catatan untuk Presenter

```
Sertakan speaker notes untuk setiap slide:
- Slide 7: "Gunakan animasi untuk menjelaskan aliran sinyal step-by-step"
- Slide 15: "Tunjukkan video oscilloscope jika tersedia"
- Slide 17: "Live coding demo debouncing"
- Setiap slide teknis: "Pause untuk pertanyaan"
```

---

## Design Guidelines

```
Panduan desain untuk keseluruhan PPT:
- Font: Sans-serif (Calibri, Arial, atau Roboto)
- Ukuran minimum: 24pt untuk body text
- Warna primary: #1E3A8A (dark blue)
- Warna secondary: #10B981 (green)
- Warna accent: #F59E0B (amber untuk warning)
- Maksimal 6 bullet points per slide
- Setiap slide teknis wajib ada diagram/gambar
- Gunakan ikon dari Flaticon atau similar
- Animasi: Subtle, entrance dari kiri
```
