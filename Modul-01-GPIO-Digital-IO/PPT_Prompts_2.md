# Prompt untuk Pembuatan PPT - Bagian 2
## Modul 01: GPIO Digital I/O - Praktikum dan Implementasi

> **Instruksi untuk AI/Designer:** Gunakan prompt di bawah ini untuk membuat slide presentasi. Bagian 2 fokus pada implementasi praktikum. Total target: 25-30 slide.

---

## Slide 1: Judul Bagian 2

```
Buatkan slide judul dengan:
- Judul: "GPIO Digital I/O - Praktikum"
- Subtitle: "Implementasi pada STM32 dan ESP32"
- Visual: Foto breadboard dengan LED dan button
- Badge: "Hands-On Session"
```

---

## Slide 2: Tujuan Praktikum

```
Buatkan slide tujuan dengan format checklist:
- Judul: "Apa yang Akan Kita Capai"
□ Mengkonfigurasi GPIO sebagai output untuk LED
□ Mengkonfigurasi GPIO sebagai input untuk button
□ Mengimplementasikan software debouncing
□ Membuat berbagai pola LED
□ Memahami perbedaan STM32 vs ESP32
□ Menggunakan Serial Monitor untuk debugging
```

---

## Slide 3-4: Tools dan Setup

```
Slide 3 - Hardware Setup:
- Judul: "Peralatan yang Diperlukan"
- Foto annotated:
  * STM32 Blue Pill + ST-Link
  * ESP32 DevKitC + USB cable
  * Breadboard
  * LED (4 warna)
  * Resistor 220Ω
  * Push button (2)
  * Jumper wires

Slide 4 - Software Setup:
- Judul: "Persiapan Software"
- Steps dengan screenshot:
  1. Install VS Code
  2. Install PlatformIO Extension
  3. Clone repository praktikum
  4. Verify board detection
```

---

## Slide 5-6: Skema Rangkaian

```
Slide 5 - Rangkaian LED:
- Judul: "Wiring Diagram - LED"
- Diagram Fritzing style untuk STM32:
  * PA0-PA3 → Resistor 220Ω → LED → GND
  * PC13 (built-in LED)
- Diagram untuk ESP32:
  * GPIO2, 4, 5, 18 → Resistor 220Ω → LED → GND
- Color coding untuk kabel

Slide 6 - Rangkaian Button:
- Judul: "Wiring Diagram - Button"
- Diagram dengan internal pull-up enabled:
  * Button antara GPIO dan GND
  * Tidak perlu resistor eksternal
- Catatan: "Pull-up internal ~40kΩ"
```

---

## Slide 7-9: Program 1 - LED Blink

```
Slide 7 - Konsep LED Blink:
- Judul: "Program 1: LED Blink"
- Flowchart sederhana:
  START → Set GPIO OUTPUT → Toggle LED → Delay → Loop
- Foto expected result: LED berkedip

Slide 8 - Kode ESP32:
- Judul: "Kode: LED Blink ESP32"
- Syntax highlighted code:
  #define LED_PIN 2
  
  void setup() {
      pinMode(LED_PIN, OUTPUT);
  }
  
  void loop() {
      digitalWrite(LED_PIN, HIGH);
      delay(500);
      digitalWrite(LED_PIN, LOW);
      delay(500);
  }

Slide 9 - Kode STM32:
- Judul: "Kode: LED Blink STM32"
- Highlight perbedaan dengan ESP32:
  * PC13 adalah Active LOW
  * digitalWrite(PC13, LOW) = LED ON
- Tabel perbandingan logic
```

---

## Slide 10-12: Program 2 - Button Debounce

```
Slide 10 - Demo Bouncing:
- Judul: "Mengapa Perlu Debouncing?"
- Screenshot Serial output TANPA debouncing:
  Button pressed!
  Button pressed!
  Button pressed!  ← Multiple triggers!
- Grafik bouncing

Slide 11 - State Machine Debouncing:
- Judul: "Implementasi State Machine"
- Diagram state:
  IDLE → PRESSED_PENDING → PRESSED → RELEASED_PENDING → IDLE
- Kode snippet:
  if (millis() - lastDebounce > DEBOUNCE_MS) {
      buttonState = currentReading;
  }

Slide 12 - Hasil Setelah Debouncing:
- Judul: "Hasil Debouncing"
- Screenshot Serial output DENGAN debouncing:
  Button pressed!  ← Single clean trigger
- Perbandingan before/after
```

---

## Slide 13-15: Program 3 - Running LED

```
Slide 13 - Konsep Running LED:
- Judul: "Program 3: Running LED Pattern"
- Animasi GIF atau sequence diagram:
  LED1 → LED2 → LED3 → LED4 → repeat
- Aplikasi: Scanner effect, progress indicator

Slide 14 - Implementasi dengan Array:
- Judul: "Menggunakan Array untuk LED"
- Kode:
  const int leds[] = {4, 5, 18, 19};
  const int numLeds = 4;
  
  for (int i = 0; i < numLeds; i++) {
      digitalWrite(leds[i], HIGH);
      delay(100);
      digitalWrite(leds[i], LOW);
  }

Slide 15 - Variasi Pattern:
- Judul: "Variasi Pola LED"
- Grid 2x2 dengan pattern:
  * Forward: → → → →
  * Reverse: ← ← ← ←
  * Ping-pong: → ← → ←
  * All blink: ● ● ● ●
```

---

## Slide 16-18: Program 4 - LED Breathing

```
Slide 16 - Konsep PWM:
- Judul: "LED Breathing dengan PWM"
- Diagram PWM waveform
- Penjelasan duty cycle dan perceived brightness
- Grafik brightness vs duty cycle (tidak linear!)

Slide 17 - Implementasi ESP32 (LEDC):
- Judul: "LEDC PWM pada ESP32"
- Kode:
  ledcSetup(channel, freq, resolution);
  ledcAttachPin(pin, channel);
  ledcWrite(channel, dutyCycle);
- Highlight: ESP32 punya 16 channel PWM hardware!

Slide 18 - Implementasi STM32 (Software PWM):
- Judul: "Software PWM pada STM32"
- Kode dengan millis() based PWM
- Penjelasan trade-off software vs hardware PWM
```

---

## Slide 19-21: Long Press vs Short Press

```
Slide 19 - Konsep:
- Judul: "Deteksi Long Press vs Short Press"
- Timeline diagram:
  |----SHORT----|
  |----------LONG----------|
  0ms        500ms       1500ms

Slide 20 - Implementasi:
- Judul: "Kode Deteksi Press Duration"
- Logic flowchart:
  if (buttonPressed) {
      pressStartTime = millis();
  }
  if (buttonReleased) {
      duration = millis() - pressStartTime;
      if (duration < 500) shortPress();
      else longPress();
  }

Slide 21 - Aplikasi:
- Judul: "Aplikasi dalam Dunia Nyata"
- Contoh:
  * Short press: Next track
  * Long press: Power off
  * Double press: Fast forward
```

---

## Slide 22-23: Serial Control

```
Slide 22 - Konsep:
- Judul: "Kontrol LED via Serial Monitor"
- Diagram komunikasi:
  PC (Serial Monitor) → USB → ESP32/STM32 → LED
- Contoh command: "LED1 ON", "LED2 OFF", "BLINK 5"

Slide 23 - Implementasi Command Parser:
- Judul: "Parsing Perintah Serial"
- Kode:
  if (Serial.available()) {
      String cmd = Serial.readStringUntil('\n');
      if (cmd == "ON") digitalWrite(LED, HIGH);
      else if (cmd == "OFF") digitalWrite(LED, LOW);
  }
- Tips: Gunakan trim() untuk menghapus whitespace
```

---

## Slide 24-26: GPIO Matrix (ESP32)

```
Slide 24 - Konsep Keypad Matrix:
- Judul: "Scanning Keypad Matrix"
- Diagram 4x4 keypad matrix
- Penjelasan row-column scanning
- Mengapa menghemat pin GPIO

Slide 25 - Implementasi:
- Judul: "Kode Scanning Matrix"
- Pseudocode:
  for each row:
      set row LOW
      for each column:
          if column == LOW:
              key = matrix[row][col]
      set row HIGH

Slide 26 - Optimasi:
- Judul: "Tips Optimasi Matrix"
- Interrupt-based detection
- Ghost key prevention
- Debouncing untuk matrix
```

---

## Slide 27-28: Emergency Stop Logic

```
Slide 27 - Safety First:
- Judul: "Implementasi Emergency Stop"
- Diagram safety interlock:
  E-Stop → Interrupt → Stop All Outputs
- Real-world importance dalam industri

Slide 28 - Implementasi:
- Judul: "Kode Safety Interlock"
- Kode dengan interrupt:
  attachInterrupt(E_STOP_PIN, emergencyStop, FALLING);
  
  void emergencyStop() {
      disableAllOutputs();
      systemState = EMERGENCY;
  }
- Warning: "Jangan hanya andalkan software untuk safety!"
```

---

## Slide 29-31: Live Demo

```
Slide 29 - Demo Checklist:
- Judul: "Demo Live"
- Checklist yang akan didemonstrasikan:
  □ LED Blink pada kedua platform
  □ Button dengan dan tanpa debouncing
  □ Running LED pattern
  □ Serial control
  □ Perbedaan aktif HIGH vs LOW

Slide 30 - Hands-On Time:
- Judul: "Giliran Anda! 🔧"
- Instruksi:
  1. Rangkai LED dan button
  2. Upload program LED Blink
  3. Modifikasi delay menjadi 250ms
  4. Upload program button debounce
  5. Eksperimen dengan program lain

Slide 31 - Troubleshooting:
- Judul: "Jika Ada Masalah..."
- Tabel troubleshooting:
  | Gejala | Penyebab | Solusi |
  | LED tidak nyala | Polaritas terbalik | Balik LED |
  | Upload gagal | Port salah | Pilih port correct |
  | Button tidak responsif | Pull-up tidak aktif | INPUT_PULLUP |
```

---

## Slide 32-33: Tugas Praktikum

```
Slide 32 - Tugas Individu:
- Judul: "Tugas Praktikum"
- Tugas 1: Dokumentasi (screenshot, foto)
- Tugas 2: Modifikasi 3 program
- Tugas 3: Analisis perbedaan STM32 vs ESP32
- Deadline dan format pengumpulan

Slide 33 - Mini Project:
- Judul: "Mini Project GPIO"
- Opsi project (pilih salah satu):
  * Traffic Light Controller
  * Binary Counter Display
  * Simon Says Game
  * Morse Code Transmitter
- Deliverables: Kode + Video + Laporan
```

---

## Slide 34: Kesimpulan

```
Buatkan slide kesimpulan:
- Judul: "Apa yang Sudah Kita Pelajari"
- Summary dengan ikon:
  ✅ GPIO output untuk mengendalikan LED
  ✅ GPIO input dengan debouncing
  ✅ Perbedaan karakteristik STM32 dan ESP32
  ✅ Multiple LED patterns
  ✅ Serial communication integration
  ✅ Safety considerations
```

---

## Slide 35: Preview Modul Berikutnya

```
Buatkan slide preview:
- Judul: "Coming Up Next..."
- Preview Modul 02: Interrupt dan Timer
- Gambar:
  * External interrupt diagram
  * Timer block diagram
- Teaser: "Bagaimana membuat program yang responsive tanpa polling?"
```

---

## Slide 36: Q&A dan Kontak

```
Buatkan slide penutup:
- Judul: "Pertanyaan & Diskusi"
- Informasi kontak dosen/asisten
- Link repository GitHub
- QR code untuk feedback form
- Jam konsultasi
```

---

## Catatan Tambahan untuk Presenter

```
Tips presentasi:
1. Slide 7-9: Lakukan live coding bersama mahasiswa
2. Slide 10-12: Tunjukkan oscilloscope jika tersedia
3. Slide 29-31: Siapkan backup hardware jika demo gagal
4. Setiap demo: Minta volunteer mahasiswa

Durasi estimasi:
- Teori (Bagian 1): 45 menit
- Praktikum (Bagian 2): 90 menit
- Q&A: 15 menit
- Total: 150 menit (2.5 jam)
```

---

## Resource List untuk Slide

```
Gambar yang perlu disiapkan:
1. Foto STM32 Blue Pill (high-res)
2. Foto ESP32 DevKitC (high-res)
3. Diagram Fritzing rangkaian
4. Screenshot PlatformIO
5. Screenshot Serial Monitor
6. GIF animasi LED blink
7. Diagram oscilloscope bouncing
8. Flowchart berbagai program

Video untuk embed/link:
1. Demo LED blink
2. Demo bouncing vs debounced
3. Demo running LED
4. Time-lapse rangkaian
```
