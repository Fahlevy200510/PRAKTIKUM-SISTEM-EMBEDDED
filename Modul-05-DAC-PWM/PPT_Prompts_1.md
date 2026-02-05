# Prompt untuk Pembuatan PPT - Bagian 1
## Modul 05: DAC & PWM Output

### 📌 Informasi Umum
- **Total Slide:** 25-30 slide
- **Durasi Presentasi:** 45-50 menit
- **Target Audiens:** Mahasiswa Teknik Elektro/Informatika semester 4-5

---

## SLIDE 1: Judul
**Prompt:**
"Buatkan slide judul dengan desain modern dan profesional untuk materi kuliah 'BAB 05: DAC dan PWM Output'. Sertakan:
- Judul utama: 'DAC (Digital-to-Analog Converter) dan PWM (Pulse Width Modulation)'
- Subtitle: 'Praktikum Sistem Embedded'
- Logo institusi (placeholder)
- Informasi: 'Pertemuan 5 | Platform: STM32F103 & ESP32'
- Warna tema: Biru elektrik dan oranye
- Gambar ilustrasi: Waveform analog dan digital"

---

## SLIDE 2: Capaian Pembelajaran
**Prompt:**
"Buatkan slide Capaian Pembelajaran dengan layout yang jelas dan icon untuk setiap poin:
1. 🎯 Memahami prinsip kerja DAC dan arsitekturnya
2. 📊 Memahami konsep PWM dan aplikasinya
3. 🔧 Mengkonfigurasi DAC pada STM32 (12-bit) dan ESP32 (8-bit)
4. ⚡ Mengimplementasikan PWM untuk LED dimming, motor, servo
5. 📈 Membandingkan DAC vs PWM untuk output analog
6. 🔬 Menerapkan teknik filtering PWM
Gunakan desain dengan progress bar atau checklist visual"

---

## SLIDE 3: Outline Materi
**Prompt:**
"Buatkan slide outline/daftar isi dengan timeline visual:
1. Teori Dasar DAC (15 menit)
   - Prinsip Konversi D/A
   - Arsitektur DAC (R-2R, Weighted)
2. DAC pada Mikrokontroler (10 menit)
   - STM32 DAC 12-bit
   - ESP32 DAC 8-bit
3. Teori Dasar PWM (10 menit)
   - Konsep Duty Cycle
   - Frekuensi dan Resolusi
4. PWM pada Mikrokontroler (10 menit)
   - Timer STM32
   - LEDC ESP32
5. Aplikasi & Praktikum (15 menit)
Desain dengan roadmap atau flowchart horizontal"

---

## SLIDE 4: Mengapa Output Analog?
**Prompt:**
"Buatkan slide pengantar dengan ilustrasi perbandingan:
- Judul: 'Mengapa Perlu Output Analog?'
- Tampilkan diagram mikrokontroler digital yang perlu mengontrol:
  - 💡 Kecerahan LED (0-100%)
  - ⚙️ Kecepatan Motor (0-max RPM)
  - 🔊 Audio/Speaker (waveform)
  - 🎚️ Posisi Servo (0°-180°)
- Dua solusi: DAC (True Analog) vs PWM (Pseudo Analog)
- Gunakan animasi perbandingan sinyal digital vs analog"

---

## SLIDE 5: Prinsip Kerja DAC
**Prompt:**
"Buatkan slide dengan diagram blok DAC:
- Judul: 'Prinsip Kerja DAC'
- Diagram konversi: Digital Input (Binary) → DAC → Analog Output (Voltage)
- Formula dengan penjelasan visual:
  ```
  Vout = Vref × (Digital_Value / 2^n)
  ```
- Contoh konkret: DAC 8-bit, Vref=3.3V
  - Input 0 → 0V
  - Input 127 → 1.64V
  - Input 255 → 3.3V
- Gunakan grafik tangga (staircase) untuk menunjukkan level diskrit"

---

## SLIDE 6: Arsitektur R-2R Ladder DAC
**Prompt:**
"Buatkan slide dengan diagram skematik R-2R Ladder:
- Judul: 'R-2R Ladder DAC'
- Gambar rangkaian R-2R 4-bit lengkap dengan:
  - Resistor R dan 2R
  - Input D0-D3
  - Output Vout
- Penjelasan keuntungan:
  ✓ Hanya 2 nilai resistor
  ✓ Akurasi tinggi
  ✓ Mudah dikaskade
- Animasi step-by-step aliran arus"

---

## SLIDE 7: Spesifikasi DAC
**Prompt:**
"Buatkan slide tabel spesifikasi DAC dengan visualisasi:
| Parameter | Simbol | Deskripsi | Ilustrasi |
|-----------|--------|-----------|-----------|
| Resolution | n-bit | Jumlah level output | Grafik step |
| INL | ±LSB | Linearity error | Kurva deviasi |
| DNL | ±LSB | Step size error | Diagram step |
| Settling Time | µs | Waktu stabilisasi | Waveform |
| Glitch Energy | nV·s | Transient energy | Spike |
Gunakan ikon dan mini-grafik untuk setiap parameter"

---

## SLIDE 8: DAC pada STM32F103
**Prompt:**
"Buatkan slide dengan diagram blok DAC STM32:
- Judul: 'DAC STM32F103C8T6'
- Spesifikasi:
  - 2 Channel DAC
  - Resolusi 12-bit (0-4095)
  - Pin: PA4 (DAC1), PA5 (DAC2)
  - Output Buffer terintegrasi
  - DMA Support
  - Timer Trigger
- Diagram internal: Data Register → Converter → Buffer → Pin
- Highlight fitur utama dengan badge/label"

---

## SLIDE 9: Register DAC STM32
**Prompt:**
"Buatkan slide dengan tabel register DAC:
- Judul: 'Register DAC STM32'
- Tabel dengan highlight warna:
  | Register | Alamat | Fungsi |
  |----------|--------|--------|
  | DAC_CR | 0x400x | Control (Enable, Trigger) |
  | DAC_DHR12R1 | 0x400x | Data 12-bit right-aligned |
  | DAC_DHR12L1 | 0x400x | Data 12-bit left-aligned |
  | DAC_DOR1 | 0x400x | Data Output |
- Diagram bit-field untuk DAC_CR
- Warna berbeda untuk read/write register"

---

## SLIDE 10: Kode DAC STM32 (HAL)
**Prompt:**
"Buatkan slide dengan code snippet dan penjelasan:
- Judul: 'Konfigurasi DAC STM32 dengan HAL'
- Kode dengan syntax highlighting:
```c
// 1. Enable Clock
__HAL_RCC_DAC_CLK_ENABLE();

// 2. Configure GPIO sebagai Analog
GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;

// 3. Configure DAC Channel
sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;

// 4. Set Value (0-4095)
HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, 
                 DAC_ALIGN_12B_R, value);
```
- Annotasi pada setiap bagian penting
- Flow diagram di samping kode"

---

## SLIDE 11: DAC pada ESP32
**Prompt:**
"Buatkan slide perbandingan DAC ESP32:
- Judul: 'DAC ESP32'
- Spesifikasi:
  - 2 Channel DAC 8-bit
  - DAC1: GPIO25
  - DAC2: GPIO26
  - Cosine Wave Generator built-in
  - DMA untuk audio
- Diagram pin mapping ESP32
- Perbandingan vs STM32:
  | Feature | STM32 | ESP32 |
  |---------|-------|-------|
  | Resolution | 12-bit | 8-bit |
  | Channels | 2 | 2 |
  | Wave Gen | Triangle/Noise | Cosine |"

---

## SLIDE 12: Kode DAC ESP32
**Prompt:**
"Buatkan slide dengan code snippet ESP32:
- Judul: 'DAC ESP32 - Arduino Framework'
- Kode dengan syntax highlighting:
```cpp
#include <Arduino.h>

#define DAC_PIN 25  // GPIO25 = DAC1

void setup() {
    // DAC tidak perlu konfigurasi khusus
}

void loop() {
    // Ramp up 0V -> 3.3V
    for (int i = 0; i < 256; i++) {
        dacWrite(DAC_PIN, i);  // 8-bit: 0-255
        delay(10);
    }
}
```
- Bandingkan dengan ESP-IDF native API
- Note: Keterbatasan 8-bit resolution"

---

## SLIDE 13: Cosine Wave Generator ESP32
**Prompt:**
"Buatkan slide fitur unik ESP32:
- Judul: 'Hardware Cosine Wave Generator'
- Diagram blok cosine generator
- Kode konfigurasi:
```cpp
dac_cw_config_t config = {
    .en_ch = DAC_CHANNEL_1,
    .scale = DAC_CW_SCALE_1,
    .phase = DAC_CW_PHASE_0,
    .freq = 1000,  // 1 kHz
};
dac_cw_generator_config(&config);
dac_cw_generator_enable();
```
- Visualisasi parameter: scale, phase, frequency
- Use case: Signal generator, audio synthesis"

---

## SLIDE 14: Konsep PWM
**Prompt:**
"Buatkan slide penjelasan PWM dengan animasi:
- Judul: 'Pulse Width Modulation (PWM)'
- Diagram waveform untuk berbagai duty cycle:
  - 25% duty: ▁█▁▁▁█▁▁
  - 50% duty: ▁██▁▁██▁
  - 75% duty: ▁███▁███
- Formula:
  - Duty Cycle (%) = (Ton / T) × 100%
  - Vavg = Vmax × Duty Cycle
- Visualisasi tegangan rata-rata dengan area shading
- Animasi perubahan duty cycle"

---

## SLIDE 15: Frekuensi dan Resolusi PWM
**Prompt:**
"Buatkan slide dengan tabel aplikasi:
- Judul: 'Memilih Frekuensi PWM'
- Tabel aplikasi:
  | Aplikasi | Frekuensi | Alasan |
  |----------|-----------|--------|
  | LED | 1-10 kHz | Anti-flicker |
  | Motor DC | 10-20 kHz | Above audible |
  | Servo | 50 Hz | Standard |
  | Audio | >44 kHz | CD quality |
- Grafik trade-off: Resolusi vs Frekuensi
- Formula: Max_Freq = Clock / 2^Resolution
- Diagram visual resolusi 8-bit vs 16-bit"

---

## SLIDE 16: PWM pada STM32
**Prompt:**
"Buatkan slide dengan diagram timer STM32:
- Judul: 'PWM STM32 - Timer Architecture'
- Diagram blok lengkap:
  - Clock Source → Prescaler (PSC)
  - Counter (CNT) → Compare (CCR)
  - Output Compare → PWM Pin
- Formula:
  - PWM_Freq = Clock / ((PSC+1) × (ARR+1))
- Contoh perhitungan:
  - 72MHz / (72 × 1000) = 1 kHz
- Highlight timer yang support PWM: TIM1-4, TIM5, TIM8"

---

## SLIDE 17: Konfigurasi PWM STM32
**Prompt:**
"Buatkan slide dengan code dan diagram:
- Judul: 'Konfigurasi PWM STM32 HAL'
```c
// Timer Configuration
htim3.Init.Prescaler = 72 - 1;    // 1MHz
htim3.Init.Period = 1000 - 1;     // 1kHz PWM
HAL_TIM_PWM_Init(&htim3);

// Channel Configuration
sConfigOC.OCMode = TIM_OCMODE_PWM1;
sConfigOC.Pulse = 500;            // 50% duty
HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1);

// Start PWM
HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

// Update duty cycle
__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, new_duty);
```
- Diagram timing dengan ARR dan CCR"

---

## SLIDE 18: LEDC ESP32
**Prompt:**
"Buatkan slide LEDC peripheral ESP32:
- Judul: 'ESP32 LEDC (LED Controller)'
- Diagram arsitektur:
  - High Speed (8 channel)
  - Low Speed (8 channel)
  - 4 Timer per mode
- Fitur unggulan:
  - ✓ 16 channel independen
  - ✓ Resolusi 1-20 bit
  - ✓ Hardware fade
  - ✓ Frekuensi hingga 40MHz
- Perbandingan dengan STM32 timer"

---

## SLIDE 19: Kode PWM ESP32
**Prompt:**
"Buatkan slide dengan code ESP32:
- Judul: 'PWM ESP32 - LEDC'
```cpp
#define PWM_PIN       25
#define PWM_CHANNEL   0
#define PWM_FREQ      5000   // 5 kHz
#define PWM_RESOLUTION 8     // 8-bit

void setup() {
    // Setup
    ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(PWM_PIN, PWM_CHANNEL);
}

void loop() {
    // Fade
    for (int duty = 0; duty <= 255; duty++) {
        ledcWrite(PWM_CHANNEL, duty);
        delay(10);
    }
}
```
- Highlight kemudahan dibanding STM32
- Note tentang ESP32 Arduino Core API"

---

## SLIDE 20: Hardware Fade ESP32
**Prompt:**
"Buatkan slide fitur hardware fade:
- Judul: 'ESP32 Hardware Fade'
- Penjelasan: CPU-free fade operation
- Kode:
```cpp
// Install fade function
ledc_fade_func_install(0);

// Fade to target
ledc_set_fade_time_and_start(
    LEDC_HIGH_SPEED_MODE,
    LEDC_CHANNEL_0,
    target_duty,
    fade_time_ms,
    LEDC_FADE_WAIT_DONE
);
```
- Diagram timing fade linear
- Use case: Smooth LED transitions, motor soft-start"

---

## SLIDE 21: Aplikasi - Motor Control
**Prompt:**
"Buatkan slide aplikasi motor:
- Judul: 'PWM Motor DC Control'
- Diagram H-Bridge dengan PWM:
  - PWM → Enable
  - DIR_A, DIR_B → Direction
- Kode pseudo:
```
if (speed >= 0) {
    DIR_A = HIGH, DIR_B = LOW
    PWM = speed
} else {
    DIR_A = LOW, DIR_B = HIGH
    PWM = -speed
}
```
- Waveform PWM 20kHz untuk motor
- Tips: Soft-start untuk mengurangi inrush current"

---

## SLIDE 22: Aplikasi - Servo Control
**Prompt:**
"Buatkan slide servo control:
- Judul: 'PWM Servo Motor Control'
- Diagram timing servo:
  - 50Hz (20ms period)
  - 0.5ms = 0°
  - 1.5ms = 90°
  - 2.5ms = 180°
- Ilustrasi posisi servo vs pulse width
- Kode konversi angle ke pulse:
```c
pulse = 500 + (angle * 2000 / 180);  // µs
```
- Warning tentang timing accuracy"

---

## SLIDE 23: DAC vs PWM Comparison
**Prompt:**
"Buatkan slide perbandingan komprehensif:
- Judul: 'DAC vs PWM: Kapan Menggunakan?'
- Tabel perbandingan dengan ikon:
  | Aspek | DAC | PWM |
  |-------|-----|-----|
  | Resolution | 8-12 bit | Unlimited |
  | Ripple | ✓ Low | ✗ High |
  | Pin | Dedicated | Any GPIO |
  | Efficiency | Medium | High |
  | Audio | ✓ Better | Needs filter |
  | Motor | ✗ | ✓ Best |
- Diagram use case untuk masing-masing"

---

## SLIDE 24: PWM sebagai Pseudo-DAC
**Prompt:**
"Buatkan slide teknik filtering:
- Judul: 'PWM + RC Filter = Pseudo-DAC'
- Diagram rangkaian:
  PWM → R (10kΩ) → [Output] → C (100nF) → GND
- Formula cutoff:
  fc = 1 / (2πRC) = 159 Hz
- Waveform sebelum dan sesudah filter
- Tips:
  - PWM freq >> cutoff freq
  - Multiple RC stages untuk quality lebih baik
- Perbandingan quality vs True DAC"

---

## SLIDE 25: Demo & Praktikum
**Prompt:**
"Buatkan slide overview praktikum:
- Judul: 'Praktikum: DAC & PWM'
- Daftar percobaan dengan progress tracker:
  □ DAC Ramp Output (STM32 & ESP32)
  □ DAC Sine Wave Generator
  □ PWM LED Dimming
  □ PWM Motor Control
  □ Servo Control
  □ PWM Pseudo-DAC
  □ RGB LED Color Mixing
- Deliverables:
  - Laporan dengan screenshot waveform
  - Video demonstrasi
  - Analisis perbandingan"

---

## SLIDE 26: Kesimpulan
**Prompt:**
"Buatkan slide kesimpulan dengan summary visual:
- Judul: 'Kesimpulan'
- Key takeaways dengan ikon:
  1. 📊 DAC: Konversi digital → analog langsung
  2. 📈 PWM: Duty cycle untuk kontrol daya
  3. 🔧 STM32: DAC 12-bit, Timer untuk PWM
  4. ⚡ ESP32: DAC 8-bit, LEDC dengan hardware fade
  5. 🎯 Pilihan tergantung aplikasi
- Diagram decision tree: Kapan DAC vs PWM
- QR code ke referensi tambahan"

---

## SLIDE 27: Q&A
**Prompt:**
"Buatkan slide Q&A yang interaktif:
- Judul: 'Pertanyaan & Diskusi'
- Beberapa pertanyaan pemicu:
  - 'Mengapa servo perlu tepat 50Hz?'
  - 'Bagaimana meningkatkan resolusi DAC 8-bit?'
  - 'Kapan PWM filtering tidak cukup?'
- Space untuk catatan
- Ikon tangan terangkat dan speech bubble
- Contact info untuk pertanyaan lanjutan"

---

## 🎨 Panduan Desain

### Warna Tema
- Primary: #2196F3 (Biru)
- Secondary: #FF9800 (Oranye)
- Accent: #4CAF50 (Hijau)
- Background: #FAFAFA (Light gray)
- Text: #212121 (Dark gray)

### Font
- Heading: Roboto Bold, 32-44pt
- Body: Open Sans Regular, 18-24pt
- Code: Fira Code / JetBrains Mono, 14-16pt

### Elemen Visual
- Gunakan diagram waveform untuk setiap konsep
- Animasi untuk proses konversi
- Code dengan syntax highlighting
- Perbandingan side-by-side STM32 vs ESP32

### Tips Presentasi
1. Demo langsung dengan oscilloscope jika tersedia
2. Tunjukkan waveform real-time
3. Biarkan mahasiswa mengubah parameter
4. Bandingkan output DAC vs PWM filtered
