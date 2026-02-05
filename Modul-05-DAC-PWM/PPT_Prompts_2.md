# Prompt untuk Pembuatan PPT - Bagian 2
## Modul 05: DAC & PWM - Lanjutan & Aplikasi

### 📌 Informasi
- **Slide:** 28-50 (Lanjutan dari PPT_Prompts_1)
- **Fokus:** Aplikasi lanjutan, troubleshooting, dan studi kasus
- **Durasi:** 30-40 menit

---

## SLIDE 28: Waveform Generation dengan DAC
**Prompt:**
"Buatkan slide tentang waveform generation:
- Judul: 'DAC Waveform Generation'
- Jenis waveform yang dapat dihasilkan:
  - Sine Wave (lookup table)
  - Triangle Wave
  - Sawtooth Wave
  - Square Wave
  - Custom Waveform
- Diagram lookup table untuk sine:
```
Index:  0   1   2   3   ...  99
Value: 2048 2176 2303 2426 ... 2048
```
- Formula: value = 2048 + 2000 × sin(2πi/N)
- Ilustrasi DMA circular buffer untuk continuous output"

---

## SLIDE 29: DAC dengan DMA
**Prompt:**
"Buatkan slide DMA-driven DAC:
- Judul: 'DAC + DMA: Efisiensi Maksimal'
- Diagram aliran data:
  Memory [Waveform Table] → DMA → DAC → Analog Out
- Keuntungan:
  ✓ CPU free saat generate waveform
  ✓ Timing presisi
  ✓ Continuous output
- Kode konfigurasi:
```c
HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, 
                  (uint32_t*)sine_table, 
                  SAMPLES, DAC_ALIGN_12B_R);
```
- Perbandingan CPU load: Polling vs DMA"

---

## SLIDE 30: Audio DAC
**Prompt:**
"Buatkan slide aplikasi audio:
- Judul: 'DAC untuk Audio Output'
- Spesifikasi audio standar:
  - Sample Rate: 44.1 kHz (CD quality)
  - Bit Depth: 16-bit ideal, 12-bit acceptable
- Diagram sistem audio DAC:
  Audio Data → Buffer → DAC → Amplifier → Speaker
- Challenge pada mikrokontroler:
  - STM32: 12-bit cukup untuk voice
  - ESP32: 8-bit, perlu dithering
- Tips quality improvement:
  - Oversampling
  - Interpolation
  - External DAC (I2S)"

---

## SLIDE 31: Multi-Channel PWM
**Prompt:**
"Buatkan slide multi-channel PWM:
- Judul: 'PWM Multi-Channel: RGB LED & Motor'
- Diagram 3 channel PWM untuk RGB:
  - Channel 0 → Red LED
  - Channel 1 → Green LED  
  - Channel 2 → Blue LED
- Color mixing visualization
- Kode ESP32:
```cpp
ledcSetup(0, 5000, 8);  // Red
ledcSetup(1, 5000, 8);  // Green
ledcSetup(2, 5000, 8);  // Blue

void setColor(uint8_t r, uint8_t g, uint8_t b) {
    ledcWrite(0, r);
    ledcWrite(1, g);
    ledcWrite(2, b);
}
```
- HSV to RGB conversion concept"

---

## SLIDE 32: Complementary PWM
**Prompt:**
"Buatkan slide PWM komplementer:
- Judul: 'Complementary PWM untuk H-Bridge'
- Diagram H-Bridge dengan:
  - PWM_H (High-side)
  - PWM_L (Low-side)
  - Dead-time protection
- Waveform dengan dead-time:
```
PWM_H: ▁▁█████▁▁▁▁▁▁▁▁█████▁▁
PWM_L: ▁▁▁▁▁▁▁██████▁▁▁▁▁▁▁██
       ↑      ↑
    Dead-time gaps
```
- STM32 TIM1/TIM8 support
- Konfigurasi dead-time:
```c
TIM_BDTRInitStruct.DeadTime = 100;
```
- Mencegah shoot-through current"

---

## SLIDE 33: PWM Input Capture
**Prompt:**
"Buatkan slide PWM input:
- Judul: 'PWM Input Capture & Measurement'
- Diagram pengukuran PWM:
  - Rising edge → Start capture
  - Falling edge → High time
  - Next rising → Period
- Formula:
  - Frequency = Timer_Clock / Captured_Period
  - Duty = High_Time / Period × 100%
- Use case:
  - RC receiver signal decode
  - Fan speed feedback (tachometer)
  - Sensor dengan output PWM"

---

## SLIDE 34: Servo Advanced
**Prompt:**
"Buatkan slide servo lanjutan:
- Judul: 'Servo Motor: Beyond Basic Control'
- Jenis servo:
  - Standard (180°): 0.5-2.5ms
  - Continuous rotation: Speed control
  - High-precision digital: Feedback
- Multi-servo control:
  - Max 8-16 servo per timer
  - Software PWM untuk lebih banyak
- Diagram 6-DOF robot arm dengan servo
- Library ESP32Servo features:
  - attach(pin, min, max)
  - writeMicroseconds(us)
- Tips: External power untuk multiple servo"

---

## SLIDE 35: PWM untuk Power Control
**Prompt:**
"Buatkan slide power control:
- Judul: 'PWM Power Control Applications'
- Aplikasi:
  1. LED Driver (Buck converter)
  2. Motor Speed Control
  3. Heater Control (SSR)
  4. Battery Charging (CC/CV)
- Diagram Buck Converter dengan PWM:
```
Vin → [MOSFET] → L → Vout
         ↑        ↓
       PWM       C
```
- Frekuensi switching: 100kHz - 1MHz
- Duty cycle vs output voltage:
  Vout = Vin × Duty_Cycle"

---

## SLIDE 36: Studi Kasus - LED Dimmer
**Prompt:**
"Buatkan slide studi kasus:
- Judul: 'Studi Kasus: Smart LED Dimmer'
- Spesifikasi project:
  - Input: Potentiometer / Button
  - Output: LED brightness
  - Feature: Fade effect, memory
- Block diagram sistem
- Perbandingan implementasi:
  | Method | Pro | Con |
  |--------|-----|-----|
  | DAC | Smooth | Limited pins |
  | PWM | Any GPIO | Flicker risk |
- Kode dengan smooth transition
- Demo video placeholder"

---

## SLIDE 37: Studi Kasus - Motor Speed
**Prompt:**
"Buatkan slide motor control project:
- Judul: 'Studi Kasus: DC Motor Speed Controller'
- Requirements:
  - Speed: 0-100% with soft start
  - Direction: Forward/Reverse
  - Display: Speed percentage
- Hardware diagram:
  MCU → PWM → H-Bridge → Motor
  ↓
  OLED Display
- PID control concept (basic):
  - Setpoint vs Actual speed
  - Tachometer feedback
- Safety: Current limiting, thermal protection"

---

## SLIDE 38: Studi Kasus - Function Generator
**Prompt:**
"Buatkan slide function generator:
- Judul: 'Mini Project: Function Generator'
- Fitur:
  - Waveform: Sine, Square, Triangle, Sawtooth
  - Frequency: 1Hz - 10kHz
  - Amplitude: 0-3.3V
- Block diagram:
  UI → MCU → DAC → Buffer → Output
- Waveform selection dengan rotary encoder
- Display: Waveform preview pada OLED
- STM32 lebih cocok (12-bit DAC)
- ESP32: Gunakan external DAC untuk quality"

---

## SLIDE 39: Perbandingan Platform Lengkap
**Prompt:**
"Buatkan slide perbandingan komprehensif:
- Judul: 'STM32 vs ESP32: DAC & PWM'
- Tabel besar:
```
| Feature | STM32F103 | ESP32 |
|---------|-----------|-------|
| DAC Resolution | 12-bit | 8-bit |
| DAC Channels | 2 | 2 |
| DAC DMA | Yes | Yes |
| Built-in Wave | Triangle/Noise | Cosine |
| PWM Channels | 32+ | 16 |
| PWM Resolution | 16-bit | 1-20 bit |
| Hardware Fade | No | Yes |
| Complementary | TIM1/8 | No |
| Max PWM Freq | MHz | 40MHz |
```
- Rekomendasi use case untuk masing-masing"

---

## SLIDE 40: Troubleshooting Guide
**Prompt:**
"Buatkan slide troubleshooting:
- Judul: 'Troubleshooting DAC & PWM'
- Tabel problem-solution:
  | Problem | Cause | Solution |
  |---------|-------|----------|
  | DAC stuck at 0V | Pin not analog | Check GPIO mode |
  | PWM no output | Timer not started | Call Start() |
  | Servo jitter | Interrupt timing | Hardware timer |
  | Motor noise | Low PWM freq | Increase to 20kHz |
  | LED flicker | Freq < 100Hz | Increase frequency |
  | DAC stepping | Low resolution | Use dithering |
- Flowchart debugging untuk setiap issue"

---

## SLIDE 41: Best Practices Checklist
**Prompt:**
"Buatkan slide best practices:
- Judul: 'Best Practices: DAC & PWM'
- Checklist dengan ikon:
  DAC:
  ☑ Enable output buffer untuk load handling
  ☑ Gunakan DMA untuk waveform kontinyu
  ☑ Synchronize dengan timer untuk timing presisi
  ☑ Pertimbangkan settling time
  
  PWM:
  ☑ Pilih frekuensi sesuai aplikasi
  ☑ Gunakan dead-time untuk H-bridge
  ☑ Implementasi soft-start untuk motor
  ☑ Hardware PWM lebih presisi dari software
  
- Warning box untuk common mistakes"

---

## SLIDE 42: Pengukuran dengan Oscilloscope
**Prompt:**
"Buatkan slide teknik pengukuran:
- Judul: 'Verifikasi dengan Oscilloscope'
- Setup pengukuran:
  - Probe attenuation: 10X
  - Timebase sesuai frekuensi
  - Trigger: Edge, rising
- Parameter yang diukur:
  - Frequency & Period
  - Duty Cycle
  - Rise/Fall time
  - Ripple (filtered PWM)
- Screenshot contoh waveform:
  - DAC ramp
  - PWM 50%
  - Servo signal
- Tip: Gunakan MATH function untuk FFT"

---

## SLIDE 43: Integrasi dengan Sensor
**Prompt:**
"Buatkan slide integrasi:
- Judul: 'DAC/PWM + Sensor Integration'
- Contoh aplikasi terintegrasi:
  1. Temperature → PWM → Fan Speed
  2. Light Sensor → PWM → LED Brightness
  3. Distance → DAC → Analog Meter
  4. Potentiometer → Servo Position
- Block diagram closed-loop system
- Kode contoh:
```cpp
void loop() {
    int temp = readTemperature();
    int fanSpeed = map(temp, 25, 50, 0, 255);
    ledcWrite(FAN_CHANNEL, fanSpeed);
}
```
- Concept: Sensor → Process → Actuator"

---

## SLIDE 44: Project Ideas
**Prompt:**
"Buatkan slide ide project:
- Judul: 'Project Ideas: DAC & PWM'
- Level Easy:
  - LED Breathing effect
  - Simple servo tester
  - PWM fan controller
- Level Medium:
  - RGB mood lamp
  - DC motor speed control
  - Audio tone generator
- Level Advanced:
  - Function generator
  - Robot arm controller
  - Audio player dengan DAC
- Setiap project dengan complexity rating ⭐"

---

## SLIDE 45: Tugas dan Deliverables
**Prompt:**
"Buatkan slide tugas:
- Judul: 'Tugas & Deliverables'
- Tugas Praktikum:
  1. ✏️ Implementasi semua program (40%)
  2. 📝 Laporan dengan analisis (30%)
  3. 🎥 Video demonstrasi 3-5 menit (20%)
  4. 💡 Modifikasi kreatif (10%)
- Format laporan:
  - Teori singkat
  - Prosedur
  - Hasil & Screenshot
  - Analisis
  - Kesimpulan
- Deadline dan submission method"

---

## SLIDE 46: Rubrik Penilaian
**Prompt:**
"Buatkan slide rubrik:
- Judul: 'Rubrik Penilaian'
- Tabel rubrik detail:
  | Kriteria | 4 (Excellent) | 3 (Good) | 2 (Fair) | 1 (Poor) |
  |----------|---------------|----------|----------|----------|
  | Program | Semua jalan | 80% jalan | 60% jalan | <60% |
  | Analisis | Mendalam | Cukup | Dangkal | Tidak ada |
  | Video | Profesional | Baik | Cukup | Kurang |
  | Kreativitas | Inovatif | Ada mod | Copy | Tidak |
- Bobot: Program 40%, Laporan 30%, Video 20%, Kreatif 10%"

---

## SLIDE 47: Pertanyaan Evaluasi
**Prompt:**
"Buatkan slide evaluasi:
- Judul: 'Pertanyaan Evaluasi'
- Soal pilihan:
  1. Resolusi DAC ESP32 adalah... (8-bit)
  2. Frekuensi PWM untuk motor sebaiknya... (>20kHz)
  3. Servo standard membutuhkan frekuensi... (50Hz)
  4. Untuk audio output, lebih baik... (DAC)
- Soal analisis:
  - Hitung tegangan output DAC 12-bit, input 2048, Vref 3.3V
  - Hitung duty cycle untuk pulse 1.5ms pada 50Hz PWM
- Space untuk jawaban singkat"

---

## SLIDE 48: Referensi Lengkap
**Prompt:**
"Buatkan slide referensi:
- Judul: 'Referensi & Resources'
- Dokumentasi Resmi:
  - STM32F103 Reference Manual (RM0008)
  - ESP32 Technical Reference Manual
  - ESP-IDF LEDC Documentation
- Application Notes:
  - AN3126: Audio and waveform generation
  - AN4013: STM32 Timer cookbook
- Books:
  - 'Mastering STM32' - Carmine Noviello
- Online:
  - GitHub repository course
  - Forum diskusi
- QR codes untuk quick access"

---

## SLIDE 49: Preview Modul Berikutnya
**Prompt:**
"Buatkan slide preview:
- Judul: 'Coming Up: I2C Sensor Communication'
- Teaser Modul 06:
  - Protokol I2C (Two-Wire Interface)
  - Sensor BME280 (Temperature, Humidity, Pressure)
  - OLED Display SSD1306
  - RTC DS3231
  - EEPROM 24LC256
- Keterkaitan:
  'DAC output dapat ditampilkan ke OLED via I2C'
  'Sensor reading dapat mengontrol PWM'
- Diagram integrasi modul"

---

## SLIDE 50: Terima Kasih
**Prompt:**
"Buatkan slide penutup:
- Judul: 'Terima Kasih'
- Quote inspiratif tentang embedded systems
- Summary icons:
  ✅ DAC untuk output analog presisi
  ✅ PWM untuk kontrol daya efisien
  ✅ Pilih sesuai kebutuhan aplikasi
- Contact information
- Social media / repository links
- Animasi sederhana untuk closing"

---

## 🎨 Panduan Desain Lanjutan

### Animasi yang Disarankan
1. **Waveform Animation**
   - PWM duty cycle berubah
   - Sine wave generation step-by-step
   - RC filter smoothing effect

2. **Build-up Animation**
   - Block diagram muncul satu-per-satu
   - Code highlighting line-by-line
   - Comparison table cell-by-cell

3. **Transition**
   - Slide ke slide: Fade/Push
   - Section change: Zoom
   - Demo section: Cut to video

### Assets yang Dibutuhkan
- [ ] Foto oscilloscope waveform
- [ ] Video demo LED dimming
- [ ] Video demo motor control
- [ ] Video demo servo sweep
- [ ] Diagram skematik H-bridge
- [ ] Foto setup praktikum

### Template Consistency
- Gunakan header yang sama di semua slide
- Footer: Nomor slide, nama modul
- Warna code block konsisten
- Icon set yang unified
