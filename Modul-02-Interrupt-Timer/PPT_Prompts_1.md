# Prompt untuk Pembuatan PPT - Bagian 1 (Teori)
## Modul 02: Interrupt dan Timer

> **Instruksi Umum untuk AI Image Generator:**
> - Style: Technical illustration, clean, professional
> - Color scheme: Biru tua (#1a365d), Hijau (#38a169), Abu-abu (#718096)
> - Resolusi: 1920x1080 (16:9)
> - Font style: Modern sans-serif

---

## Slide 1: Cover

**Prompt:**
```
Buat slide cover PPT dengan judul "BAB 02: INTERRUPT DAN TIMER", subjudul 
"Pemrograman Sistem Embedded - STM32 & ESP32". Latar belakang menampilkan 
ilustrasi minimalis timing diagram dengan interrupt signals, gelombang 
clock, dan mikrokontroler. Gunakan warna gradien biru tua ke biru terang. 
Tambahkan logo universitas di pojok kanan atas dan nama institusi di bawah.
```

---

## Slide 2: Capaian Pembelajaran

**Prompt:**
```
Buat slide "Capaian Pembelajaran" dengan 6 poin dalam format numbered list:
1. Memahami konsep interrupt dan perbedaan dengan polling
2. Menguasai konfigurasi NVIC pada STM32
3. Mengimplementasikan External Interrupt pada ESP32
4. Mengkonfigurasi Hardware Timer pada kedua platform
5. Menerapkan teknik debouncing berbasis interrupt/timer
6. Mengembangkan aplikasi real-time dengan interrupt dan timer

Gunakan ikon target/crosshair di samping judul. Background dengan pattern 
subtle circuit board.
```

---

## Slide 3: Outline Materi

**Prompt:**
```
Buat slide "Outline Materi" dengan diagram mind-map sederhana:
- Pusat: "Interrupt & Timer"
- Cabang 1: "Konsep Interrupt" → Polling vs Interrupt, ISR, Priority
- Cabang 2: "External Interrupt" → NVIC STM32, GPIO ESP32
- Cabang 3: "Hardware Timer" → Counter, Prescaler, Auto-reload
- Cabang 4: "Aplikasi" → Debouncing, Timing, Event counting

Gunakan warna berbeda untuk setiap cabang. Style: clean infographic.
```

---

## Slide 4: Apa itu Interrupt?

**Prompt:**
```
Buat slide penjelasan "Apa itu Interrupt?" dengan:
- Definisi dalam box highlight: "Interrupt adalah mekanisme hardware yang 
  memungkinkan CPU menghentikan sementara program utama untuk menangani 
  event prioritas tinggi"
- Ilustrasi analogi: seseorang sedang bekerja di meja, telepon berdering, 
  dia mengangkat telepon, lalu kembali bekerja
- Poin-poin: "Responsif", "Efisien", "Real-time capable"

Gunakan ikon lightning bolt untuk menggambarkan event interrupt.
```

---

## Slide 5: Polling vs Interrupt

**Prompt:**
```
Buat slide perbandingan "Polling vs Interrupt" dengan layout dua kolom:

Kolom Kiri - POLLING:
- Ilustrasi: loop arrow checking status repeatedly
- CPU terus-menerus memeriksa
- Membuang cycle CPU
- Simple to implement
- ❌ Tidak efisien

Kolom Tengah: VS dengan panah perbandingan

Kolom Kanan - INTERRUPT:
- Ilustrasi: bell notification icon
- CPU diberitahu saat event terjadi
- CPU bebas melakukan hal lain
- More complex
- ✓ Sangat efisien

Tambahkan timeline diagram di bawah menunjukkan CPU usage comparison.
```

---

## Slide 6: Alur Proses Interrupt

**Prompt:**
```
Buat slide "Alur Proses Interrupt" dengan flowchart vertikal:

1. [Normal Execution] - Program utama berjalan
   ↓ ⚡ Interrupt Signal
2. [Save Context] - CPU menyimpan state (PC, registers)
   ↓
3. [Vector Table] - CPU mencari alamat ISR
   ↓
4. [Execute ISR] - Interrupt Service Routine dijalankan
   ↓
5. [Clear Flag] - Flag interrupt dibersihkan
   ↓
6. [Restore Context] - CPU mengembalikan state
   ↓
7. [Resume] - Program utama dilanjutkan

Gunakan warna hijau untuk normal flow, merah untuk interrupt event.
```

---

## Slide 7: Jenis-jenis Interrupt

**Prompt:**
```
Buat slide "Jenis-jenis Interrupt" dengan kategori:

1. BERDASARKAN SUMBER:
   - External Interrupt: dari pin GPIO (tombol, sensor)
   - Internal Interrupt: dari peripheral (timer, ADC, UART)
   - Software Interrupt: dari instruksi program

2. BERDASARKAN PRIORITY:
   - Non-Maskable (NMI): tidak bisa di-disable
   - Maskable: bisa di-enable/disable

3. BERDASARKAN TRIGGER:
   - Edge-triggered: Rising, Falling, Both
   - Level-triggered: High, Low

Ilustrasi dengan sinyal waveform untuk setiap trigger type.
```

---

## Slide 8: NVIC pada ARM Cortex-M

**Prompt:**
```
Buat slide "NVIC - Nested Vectored Interrupt Controller" dengan diagram blok:

┌───────────────────────────────────────┐
│           NVIC Architecture            │
├───────────────────────────────────────┤
│                                       │
│  [External Sources] ──┐               │
│  [Peripheral IRQ] ────┼──► [NVIC] ──► [Cortex-M Core]
│  [Software IRQ] ──────┘     │              │
│                             │              │
│                      ┌──────┴──────┐       │
│                      │ • Priority  │       │
│                      │ • Pending   │       │
│                      │ • Enable    │       │
│                      └─────────────┘       │
└───────────────────────────────────────────┘

Fitur NVIC:
• Nested: ISR prioritas tinggi bisa interrupt ISR prioritas rendah
• Vectored: Setiap interrupt punya alamat handler sendiri
• Low Latency: 12 clock cycles entry time
```

---

## Slide 9: External Interrupt STM32 (EXTI)

**Prompt:**
```
Buat slide "External Interrupt STM32 (EXTI)" dengan:

1. Diagram EXTI Lines:
   - 16 EXTI lines (EXTI0-EXTI15)
   - Mapping dari GPIO: PA0→EXTI0, PB0→EXTI0, etc
   - Catatan: Hanya satu pin per nomor yang bisa aktif

2. Trigger Modes (dengan waveform):
   - Rising Edge: LOW → HIGH transition
   - Falling Edge: HIGH → LOW transition
   - Both Edges: Both transitions

3. Block diagram EXTI:
   GPIO Pin → Edge Detector → Pending Register → NVIC → CPU

Warna: gunakan warna berbeda untuk setiap GPIO port.
```

---

## Slide 10: Konfigurasi EXTI STM32

**Prompt:**
```
Buat slide "Konfigurasi EXTI STM32" dengan code snippet dan diagram:

Langkah Konfigurasi:
┌────────────────────────────────────────────────┐
│ 1. Enable Clock GPIO & AFIO                    │
│    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN |       │
│                    RCC_APB2ENR_AFIOEN;         │
├────────────────────────────────────────────────┤
│ 2. Konfigurasi GPIO sebagai Input              │
│    GPIOA->CRL &= ~(GPIO_CRL_MODE0);           │
│    GPIOA->CRL |= GPIO_CRL_CNF0_1; // Pull-up  │
├────────────────────────────────────────────────┤
│ 3. Map GPIO ke EXTI Line                       │
│    AFIO->EXTICR[0] = AFIO_EXTICR1_EXTI0_PA;   │
├────────────────────────────────────────────────┤
│ 4. Konfigurasi Trigger Edge                    │
│    EXTI->FTSR |= EXTI_FTSR_TR0; // Falling    │
├────────────────────────────────────────────────┤
│ 5. Enable Interrupt Mask                       │
│    EXTI->IMR |= EXTI_IMR_MR0;                  │
├────────────────────────────────────────────────┤
│ 6. Enable di NVIC                              │
│    NVIC_EnableIRQ(EXTI0_IRQn);                 │
└────────────────────────────────────────────────┘

Highlight setiap langkah dengan nomor dalam circle.
```

---

## Slide 11: Interrupt Handler STM32

**Prompt:**
```
Buat slide "Interrupt Handler (ISR) STM32" dengan code box:

void EXTI0_IRQHandler(void) {
    // 1. Cek pending flag
    if (EXTI->PR & EXTI_PR_PR0) {
        
        // 2. Clear pending flag (WAJIB!)
        EXTI->PR = EXTI_PR_PR0;
        
        // 3. Handle interrupt
        button_flag = true;
        led_toggle();
    }
}

PENTING:
⚠️ Clear flag dengan menulis '1', bukan '0'
⚠️ ISR harus singkat (< 1ms)
⚠️ Gunakan volatile untuk shared variables
⚠️ Nama handler HARUS sesuai vector table

Gunakan syntax highlighting dan warning box untuk poin penting.
```

---

## Slide 12: Interrupt ESP32 Architecture

**Prompt:**
```
Buat slide "Interrupt ESP32 Architecture" dengan diagram:

┌─────────────────────────────────────────────────────┐
│              ESP32 Interrupt System                  │
├─────────────────────────────────────────────────────┤
│                                                     │
│   ┌───────────┐         ┌───────────┐              │
│   │ PRO_CPU   │         │ APP_CPU   │              │
│   │ (Core 0)  │         │ (Core 1)  │              │
│   │ 32 slots  │         │ 32 slots  │              │
│   └─────┬─────┘         └─────┬─────┘              │
│         │                     │                     │
│         └────────┬────────────┘                     │
│                  │                                  │
│         ┌────────▼────────┐                         │
│         │ Interrupt Matrix│ (Crossbar Switch)       │
│         └────────┬────────┘                         │
│                  │                                  │
│    ┌─────────────┼─────────────┐                    │
│   GPIO         Timer         UART    ...            │
│   (71 interrupt sources total)                      │
└─────────────────────────────────────────────────────┘

Highlight: Dual-core = interrupt bisa di-route ke core manapun
```

---

## Slide 13: GPIO Interrupt ESP32

**Prompt:**
```
Buat slide "GPIO Interrupt ESP32" dengan:

1. Available Pins:
   ✓ GPIO 0-33: Support interrupt
   ✗ GPIO 34-39: Input-only, NO interrupt output

2. Interrupt Modes (dengan waveform):
   - RISING: LOW → HIGH
   - FALLING: HIGH → LOW
   - CHANGE: Both edges
   - ONLOW: Level LOW
   - ONHIGH: Level HIGH
   - LOW_WE/HIGH_WE: Dengan wakeup dari sleep

3. Code snippet:
   pinMode(GPIO_NUM_4, INPUT_PULLUP);
   attachInterrupt(digitalPinToInterrupt(GPIO_NUM_4), 
                   myISR, FALLING);

Gunakan checkmark hijau dan X merah untuk available/unavailable.
```

---

## Slide 14: IRAM_ATTR pada ESP32

**Prompt:**
```
Buat slide "IRAM_ATTR - Mengapa Penting?" dengan:

Diagram memory:
┌─────────────────┐
│   IRAM (Fast)   │ ◄── ISR dengan IRAM_ATTR
│   Internal RAM  │     Access: ~1 clock cycle
├─────────────────┤
│   DRAM (Data)   │     Variables
├─────────────────┤
│   Flash (Slow)  │ ◄── Normal code
│   External SPI  │     Access: Variable (Cache)
└─────────────────┘

TANPA IRAM_ATTR:
❌ ISR di Flash → Cache miss → Unpredictable delay → CRASH!

DENGAN IRAM_ATTR:
✓ ISR di IRAM → Selalu tersedia → Consistent fast access

Code:
void IRAM_ATTR buttonISR() {
    // ISR code - ALWAYS in IRAM
}

⚠️ IRAM terbatas (~200KB) - gunakan bijak!
```

---

## Slide 15: Hardware Timer - Konsep Dasar

**Prompt:**
```
Buat slide "Hardware Timer - Konsep Dasar" dengan:

Definisi: "Timer adalah peripheral yang menghitung clock cycles untuk 
menghasilkan event berbasis waktu yang presisi"

Block Diagram Timer:
                    
Clock Source ──► Prescaler ──► Counter ──► Compare/Capture
   (APB)          (÷N)        (CNT)         (CCR)
                                │
                                ▼
                          Auto-Reload
                            (ARR)

Kegunaan:
• ⏱️ Delay presisi (tanpa blocking)
• 📊 Pengukuran waktu/frekuensi
• 🔄 Generasi event periodik
• 🎵 PWM generation
• 📥 Input capture (measure pulse width)
```

---

## Slide 16: Timer STM32F103

**Prompt:**
```
Buat slide "Timer pada STM32F103" dengan tabel:

┌─────────┬──────────────┬──────────┬─────────┬─────────────────┐
│ Timer   │ Tipe         │ Resolusi │ Channel │ Fitur Khusus    │
├─────────┼──────────────┼──────────┼─────────┼─────────────────┤
│ TIM1    │ Advanced     │ 16-bit   │ 4       │ PWM, Dead-time  │
│ TIM2    │ General      │ 16-bit   │ 4       │ Encoder, IC/OC  │
│ TIM3    │ General      │ 16-bit   │ 4       │ Encoder, IC/OC  │
│ TIM4    │ General      │ 16-bit   │ 4       │ Encoder, IC/OC  │
│ SysTick │ System       │ 24-bit   │ -       │ RTOS tick       │
└─────────┴──────────────┴──────────┴─────────┴─────────────────┘

Clock Tree Diagram:
APB1 (36MHz max) → TIM2, TIM3, TIM4
APB2 (72MHz max) → TIM1, SysTick
```

---

## Slide 17: Perhitungan Timer STM32

**Prompt:**
```
Buat slide "Perhitungan Timer STM32" dengan formula dan contoh:

┌────────────────────────────────────────────────────┐
│ RUMUS DASAR                                        │
├────────────────────────────────────────────────────┤
│                                                    │
│ Timer_Freq = APB_Clock / (PSC + 1)                 │
│                                                    │
│ Period = (ARR + 1) / Timer_Freq                    │
│                                                    │
│ Update_Freq = APB_Clock / ((PSC+1) × (ARR+1))     │
│                                                    │
└────────────────────────────────────────────────────┘

CONTOH: Membuat timer 1 Hz (1 detik) dengan clock 72 MHz

Langkah 1: 72,000,000 = (PSC+1) × (ARR+1)

Langkah 2: Pilih PSC = 7199
           Timer_Freq = 72MHz / 7200 = 10 kHz

Langkah 3: ARR = 10kHz / 1Hz - 1 = 9999

Verifikasi: 72MHz / (7200 × 10000) = 1 Hz ✓

Tampilkan dengan calculator-style box dan step-by-step.
```

---

## Slide 18: Konfigurasi Timer STM32

**Prompt:**
```
Buat slide "Konfigurasi Timer STM32" dengan code dan diagram:

// Timer 2: Interrupt setiap 100ms

// 1. Enable clock
RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

// 2. Set prescaler (72MHz / 7200 = 10kHz)
TIM2->PSC = 7200 - 1;

// 3. Set auto-reload (10kHz / 10Hz = 1000)
TIM2->ARR = 1000 - 1;

// 4. Enable update interrupt
TIM2->DIER |= TIM_DIER_UIE;

// 5. Enable di NVIC
NVIC_EnableIRQ(TIM2_IRQn);

// 6. Start timer
TIM2->CR1 |= TIM_CR1_CEN;

Diagram timing di bawah menunjukkan PSC counting → ARR counting → Update event.
```

---

## Slide 19: Timer ESP32

**Prompt:**
```
Buat slide "Timer pada ESP32" dengan:

Fitur Hardware Timer ESP32:
┌────────────────────────────────────────┐
│ • 4 Hardware Timers (64-bit!)          │
│ • Timer Group 0: Timer 0, Timer 1      │
│ • Timer Group 1: Timer 0, Timer 1      │
│ • Base clock: 80 MHz (APB)             │
│ • Prescaler: 2 - 65536                 │
│ • Auto-reload support                  │
└────────────────────────────────────────┘

Perbandingan dengan STM32:
┌─────────────────┬───────────────┬───────────────┐
│ Aspek           │ STM32 Timer   │ ESP32 Timer   │
├─────────────────┼───────────────┼───────────────┤
│ Resolusi        │ 16-bit        │ 64-bit        │
│ Max Count       │ 65,535        │ 18 quintillion│
│ Jumlah          │ 4             │ 4             │
│ PWM Channels    │ 4 per timer   │ Separate      │
└─────────────────┴───────────────┴───────────────┘
```

---

## Slide 20: Konfigurasi Timer ESP32

**Prompt:**
```
Buat slide "Konfigurasi Timer ESP32" dengan code:

hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
volatile int counter = 0;

void IRAM_ATTR onTimer() {
    portENTER_CRITICAL_ISR(&timerMux);
    counter++;
    portEXIT_CRITICAL_ISR(&timerMux);
}

void setup() {
    // Timer 0, prescaler 80 (1MHz = 1µs per tick)
    timer = timerBegin(0, 80, true);
    
    // Attach interrupt
    timerAttachInterrupt(timer, &onTimer, true);
    
    // Alarm setiap 1 detik (1,000,000 µs)
    timerAlarmWrite(timer, 1000000, true);
    
    // Enable alarm
    timerAlarmEnable(timer);
}

Highlight dengan callout: "80 = 80MHz/80 = 1MHz", "1000000µs = 1s"
```

---

## Slide 21: Critical Section ESP32

**Prompt:**
```
Buat slide "Critical Section pada ESP32" dengan:

MASALAH - Race Condition:
┌──────────────────────────────────────────────────┐
│ ISR:           counter++;  // Read-Modify-Write  │
│ Main:          value = counter;                  │
│                                                  │
│ ⚡ Interrupt terjadi di tengah operasi = BUG!   │
└──────────────────────────────────────────────────┘

SOLUSI - Critical Section:
┌──────────────────────────────────────────────────┐
│ portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED; │
│                                                  │
│ // Di ISR:                                       │
│ portENTER_CRITICAL_ISR(&mux);                    │
│ counter++;                                       │
│ portEXIT_CRITICAL_ISR(&mux);                     │
│                                                  │
│ // Di main loop:                                 │
│ portENTER_CRITICAL(&mux);                        │
│ localValue = counter;                            │
│ portEXIT_CRITICAL(&mux);                         │
└──────────────────────────────────────────────────┘

⚠️ Spinlock mencegah interrupt saat critical section
```

---

## Slide 22: Best Practices ISR

**Prompt:**
```
Buat slide "Best Practices ISR" dengan DO dan DON'T list:

✅ DO:
• Keep ISR short (< 100µs)
• Use volatile for shared variables
• Set flag, process in main loop
• Clear interrupt flags
• Use IRAM_ATTR on ESP32
• Use critical sections for shared data

❌ DON'T:
• Don't use delay() in ISR
• Don't use Serial.print() in ISR
• Don't do complex calculations
• Don't allocate memory
• Don't call non-reentrant functions
• Don't forget to clear flags

Code Example:
volatile bool flag = false;

void IRAM_ATTR myISR() {
    flag = true;  // ✅ Simple!
}

void loop() {
    if (flag) {
        flag = false;
        doComplexProcessing();  // ✅ In main
    }
}
```

---

## Slide 23: Debouncing dengan Timer

**Prompt:**
```
Buat slide "Debouncing dengan Timer" dengan:

MASALAH - Button Bounce:
[Waveform showing bouncing signal with multiple transitions in ~20ms]

TANPA DEBOUNCE:
"1 tekan = 5-10 interrupt!"

SOLUSI - Timer-based Debounce:
┌────────────────────────────────────────────────────┐
│ volatile unsigned long lastTime = 0;               │
│ const unsigned long DEBOUNCE_MS = 50;              │
│                                                    │
│ void IRAM_ATTR buttonISR() {                       │
│     unsigned long now = millis();                  │
│     if (now - lastTime > DEBOUNCE_MS) {            │
│         buttonPressed = true;                      │
│         lastTime = now;                            │
│     }                                              │
│ }                                                  │
└────────────────────────────────────────────────────┘

Timing diagram showing: Button press → Debounce window → Valid detection
```

---

## Slide 24: Perbandingan STM32 vs ESP32

**Prompt:**
```
Buat slide "Perbandingan Interrupt & Timer: STM32 vs ESP32" dengan tabel komprehensif:

┌─────────────────────┬────────────────────┬────────────────────┐
│ Aspek               │ STM32F103C8T6      │ ESP32              │
├─────────────────────┼────────────────────┼────────────────────┤
│ Core                │ ARM Cortex-M3      │ Xtensa LX6 Dual    │
│ Interrupt Ctrl      │ NVIC               │ Interrupt Matrix   │
│ Priority Levels     │ 16 (4-bit)         │ 7 levels           │
│ External Int        │ 16 EXTI lines      │ All GPIO pins      │
│ Timer Count         │ 4 (16-bit)         │ 4 (64-bit)         │
│ ISR Attribute       │ None               │ IRAM_ATTR required │
│ Critical Section    │ __disable_irq()    │ portENTER_CRITICAL │
│ Vector Table        │ Fixed location     │ Configurable       │
│ Latency             │ 12 cycles          │ ~20-50 cycles      │
└─────────────────────┴────────────────────┴────────────────────┘
```

---

## Slide 25: Latihan Perhitungan

**Prompt:**
```
Buat slide "Latihan Perhitungan Timer" dengan soal:

SOAL 1: STM32 Timer
Buatlah timer yang menghasilkan interrupt setiap 250ms
Clock APB1 = 72 MHz

Jawab:
PSC = _____, ARR = _____
(Hint: 250ms = 4 Hz)

SOAL 2: ESP32 Timer
Buatlah timer untuk LED blink dengan periode 500ms
Base clock = 80 MHz, Prescaler = 80

Jawab:
Alarm value = _____ µs
(Hint: 500ms = 500,000 µs)

SOAL 3: Debounce Time
Button bounce berlangsung selama 15ms.
Berapa minimum debounce delay yang aman?

Jawab: _____ ms
(Hint: Tambah margin safety)

Format: kotak soal dengan tempat jawaban kosong, jawaban di slide berikutnya.
```

---

## Slide 26: Jawaban Latihan

**Prompt:**
```
Buat slide "Jawaban Latihan" dengan solusi:

SOAL 1 - Jawaban:
4 Hz = 72MHz / ((PSC+1) × (ARR+1))
(PSC+1) × (ARR+1) = 18,000,000

Pilihan: PSC = 7199, ARR = 2499
Check: 72MHz / (7200 × 2500) = 4 Hz ✓

SOAL 2 - Jawaban:
Prescaler 80 → 80MHz/80 = 1MHz = 1µs per tick
500ms = 500,000 µs
Alarm value = 500000

SOAL 3 - Jawaban:
Minimum debounce = 15ms
Safety margin = 2-3x
Recommended: 30-50ms

Gunakan checkmark hijau untuk jawaban benar.
```

---

## Slide 27: Summary

**Prompt:**
```
Buat slide "Summary" dengan ringkasan dalam format infographic:

┌────────────────────────────────────────────────────────┐
│                    KEY TAKEAWAYS                        │
├────────────────────────────────────────────────────────┤
│                                                        │
│ 1️⃣ INTERRUPT vs POLLING                               │
│    Interrupt = Efisien, Real-time                     │
│    Polling = Simple, CPU-intensive                    │
│                                                        │
│ 2️⃣ STM32 INTERRUPT                                    │
│    NVIC + EXTI → 16 lines, 16 priority levels        │
│                                                        │
│ 3️⃣ ESP32 INTERRUPT                                    │
│    Interrupt Matrix + IRAM_ATTR = WAJIB              │
│                                                        │
│ 4️⃣ TIMER                                              │
│    Timer_Freq = Clock / (PSC+1)                      │
│    Period = (ARR+1) / Timer_Freq                     │
│                                                        │
│ 5️⃣ BEST PRACTICE                                      │
│    Short ISR + Flag + Process in main loop           │
│                                                        │
└────────────────────────────────────────────────────────┘
```

---

## Slide 28: Referensi

**Prompt:**
```
Buat slide "Referensi" dengan daftar:

📚 Dokumentasi Resmi:
1. STM32F103 Reference Manual (RM0008) - ST
2. ARM Cortex-M3 Technical Reference - ARM
3. ESP32 Technical Reference Manual - Espressif
4. ESP-IDF Programming Guide - Espressif

📖 Buku:
1. "Mastering STM32" - Carmine Noviello
2. "The Definitive Guide to ARM Cortex-M3" - Joseph Yiu

🔗 Online Resources:
1. DeepBlue Embedded - STM32 Tutorials
2. Random Nerd Tutorials - ESP32 Guide
3. Espressif GitHub Examples

Format dengan ikon buku, dokumen, dan link untuk setiap kategori.
```

---

## Catatan Penggunaan

1. **Setiap prompt dapat digunakan untuk:**
   - AI image generator (DALL-E, Midjourney, Stable Diffusion)
   - Manual design di PowerPoint/Canva
   - Referensi konten untuk presentasi

2. **Konsistensi Visual:**
   - Maintain color scheme di semua slide
   - Gunakan font yang sama
   - Ukuran diagram proporsional

3. **Interaktivitas:**
   - Slide latihan bisa dijadikan quiz
   - Timing diagram bisa dianimasikan
   - Code snippet highlight satu per satu
