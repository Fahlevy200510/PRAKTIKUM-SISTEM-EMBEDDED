# Prompt untuk Pembuatan PPT - Bagian 2 (Praktikum & Aplikasi)
## Modul 02: Interrupt dan Timer

> **Instruksi Umum untuk AI Image Generator:**
> - Style: Technical illustration, clean, professional
> - Color scheme: Biru tua (#1a365d), Hijau (#38a169), Abu-abu (#718096)
> - Resolusi: 1920x1080 (16:9)
> - Font style: Modern sans-serif

---

## Slide 1: Cover Bagian 2

**Prompt:**
```
Buat slide cover untuk PPT Bagian 2 dengan judul "BAB 02: INTERRUPT DAN TIMER",
subtitle "Bagian 2: Implementasi Praktis & Hands-on Lab". Background menampilkan
foto workbench dengan breadboard, STM32 Blue Pill, ESP32, dan oscilloscope
menampilkan timing waveform. Style: modern tech aesthetic dengan overlay gradient.
```

---

## Slide 2: Agenda Praktikum

**Prompt:**
```
Buat slide "Agenda Praktikum" dengan timeline horizontal:

SESI 1 (50 menit):
├─ Setup Hardware (15 min)
├─ External Interrupt Demo (20 min)
└─ Hands-on Task 1 (15 min)

SESI 2 (50 menit):
├─ Timer Configuration (15 min)
├─ Timer Interrupt Demo (20 min)
└─ Hands-on Task 2 (15 min)

SESI 3 (50 menit):
├─ Integration (20 min)
├─ Advanced Applications (15 min)
└─ Q&A + Assessment (15 min)

Gunakan timeline dengan progress bar dan ikon untuk setiap kegiatan.
```

---

## Slide 3: Hardware Setup Overview

**Prompt:**
```
Buat slide "Hardware Setup" dengan foto-foto komponen:

Komponen Utama:
┌─────────────────┬─────────────────┬─────────────────┐
│  STM32 Blue Pill│    ESP32        │   Breadboard    │
│  [foto board]   │   DevKitC       │   + Jumpers     │
│                 │  [foto board]   │   [foto]        │
└─────────────────┴─────────────────┴─────────────────┘

Komponen Pendukung:
• 4x Push Button (Tactile Switch)
• 4x LED 5mm (R, G, Y, B)
• 4x Resistor 330Ω
• 4x Resistor 10kΩ
• ST-Link V2 Programmer
• USB Cables (2x)

Checklist box di samping setiap item.
```

---

## Slide 4: Skema Rangkaian STM32

**Prompt:**
```
Buat slide "Skema Rangkaian STM32" dengan diagram wiring:

                 STM32F103C8T6
              ┌────────────────────┐
    BTN1 ─────┤PA0 (EXTI0)    PC13├───[LED_BUILTIN]
    BTN2 ─────┤PA1 (EXTI1)    PB3 ├───[R 330Ω]──LED1
    BTN3 ─────┤PB0 (EXTI0)    PB4 ├───[R 330Ω]──LED2
    BTN4 ─────┤PB1 (EXTI1)    PB5 ├───[R 330Ω]──LED3
              │                    │
      GND ────┤GND           3.3V ├────
      3.3V ───┤3.3V          GND  ├────
              └────────────────────┘

Button Wiring Detail:
3.3V ──[10kΩ]──┬── GPIO (INPUT)
               │
              ─┴─ Button ── GND

Catatan: PC13 = Active LOW (internal LED)
```

---

## Slide 5: Skema Rangkaian ESP32

**Prompt:**
```
Buat slide "Skema Rangkaian ESP32" dengan diagram wiring:

                   ESP32 DevKitC
              ┌────────────────────┐
    BTN1 ─────┤GPIO0 (BOOT)  GPIO2├───[LED_BUILTIN]
    BTN2 ─────┤GPIO13        GPIO4├───[R 330Ω]──LED1
    BTN3 ─────┤GPIO15        GPIO5├───[R 330Ω]──LED2
    BTN4 ─────┤GPIO14        GPIO18├──[R 330Ω]──LED3
              │                    │
      GND ────┤GND            3.3V├────
      5V  ────┤5V (USB)       GND ├────
              └────────────────────┘

⚠️ CATATAN:
• GPIO0 = Boot button (sudah ada di board)
• GPIO2 = Built-in LED (beberapa board)
• Input-only: GPIO34-39 (tidak ada pull-up)

Gunakan warna kabel: Merah=VCC, Hitam=GND, Lainnya=Signal
```

---

## Slide 6: Demo 1 - External Interrupt STM32

**Prompt:**
```
Buat slide "Demo 1: External Interrupt STM32" dengan:

Tujuan: Menyalakan LED saat tombol ditekan menggunakan interrupt

Konfigurasi:
• Button pada PA0 (EXTI0)
• LED pada PB3
• Trigger: Falling Edge

Expected Behavior:
[Timeline diagram]
Button: ────┐___________┌────────
            ↓ EXTI0_IRQ
LED:    ────┐XXXXXXXXX┌──────────
            Toggle     Toggle

Serial Output:
> Button pressed! Count: 1
> Button pressed! Count: 2
> Button pressed! Count: 3

Status box: "✓ No polling required - CPU efficient!"
```

---

## Slide 7: Code Demo 1 - STM32 Interrupt

**Prompt:**
```
Buat slide code untuk STM32 External Interrupt dengan syntax highlighting:

#include <Arduino.h>

#define BTN_PIN PA0
#define LED_PIN PB3

volatile uint32_t pressCount = 0;
volatile bool buttonPressed = false;

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BTN_PIN, INPUT_PULLUP);
    
    // Attach interrupt
    attachInterrupt(digitalPinToInterrupt(BTN_PIN),
                    buttonISR, FALLING);
    
    Serial.println("STM32 Interrupt Demo Ready!");
}

void buttonISR() {
    buttonPressed = true;
    pressCount++;
}

void loop() {
    if (buttonPressed) {
        buttonPressed = false;
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        Serial.printf("Button pressed! Count: %d\n", pressCount);
    }
}

Highlight: volatile keyword, attachInterrupt(), ISR function
```

---

## Slide 8: Demo 2 - External Interrupt ESP32

**Prompt:**
```
Buat slide "Demo 2: External Interrupt ESP32" dengan perbandingan:

Perbedaan Utama dari STM32:
┌────────────────────────────────────────────────────┐
│ 1. IRAM_ATTR wajib untuk ISR                       │
│ 2. millis() bisa dipanggil dalam ISR               │
│ 3. Semua GPIO bisa jadi interrupt source           │
└────────────────────────────────────────────────────┘

Code ESP32:
void IRAM_ATTR buttonISR() {  // ◄── IRAM_ATTR!
    unsigned long now = millis();
    if (now - lastPress > 50) {  // Debounce
        buttonPressed = true;
        lastPress = now;
    }
}

void setup() {
    pinMode(GPIO_NUM_4, INPUT_PULLUP);
    attachInterrupt(GPIO_NUM_4, buttonISR, FALLING);
}

⚠️ Tanpa IRAM_ATTR = Guru Panic (Crash)!
```

---

## Slide 9: Hands-on Task 1

**Prompt:**
```
Buat slide "Hands-on Task 1: Multiple Button Handler" dengan:

TUGAS:
Buat program yang mendeteksi 3 tombol dengan interrupt berbeda
dan menyalakan LED yang sesuai.

Requirements:
┌────────────────────────────────────────────────────┐
│ Button 1 → Toggle LED 1 (Hijau)                    │
│ Button 2 → Toggle LED 2 (Kuning)                   │
│ Button 3 → Toggle LED 3 (Merah)                    │
│ All buttons → Print status ke Serial              │
└────────────────────────────────────────────────────┘

Checklist:
□ Semua button menggunakan interrupt (bukan polling)
□ Setiap ISR memiliki counter sendiri
□ Implementasi debouncing
□ Serial output menampilkan status

Waktu: 15 menit
Tingkat: ⭐⭐ (Menengah)
```

---

## Slide 10: Demo 3 - Hardware Timer STM32

**Prompt:**
```
Buat slide "Demo 3: Hardware Timer STM32" dengan:

Tujuan: LED blink 1 Hz menggunakan timer interrupt (tanpa delay)

Timer Configuration:
┌────────────────────────────────────────────────────┐
│ Clock APB1 = 72 MHz                                │
│ Target: 1 Hz (1 second period)                     │
│                                                    │
│ Perhitungan:                                       │
│ 72,000,000 / 1 Hz = 72,000,000 counts             │
│                                                    │
│ Pilih: PSC = 7199, ARR = 9999                      │
│ Timer_Freq = 72MHz / 7200 = 10 kHz                │
│ Period = 10000 / 10 kHz = 1 second ✓              │
└────────────────────────────────────────────────────┘

Timing Diagram:
TIM2_CNT: 0...1000...5000...9999 | 0...1000...
                               ↓ Update IRQ
LED:      ─────────┐___________┌───────────
                   Toggle      Toggle
```

---

## Slide 11: Code Demo 3 - STM32 Timer

**Prompt:**
```
Buat slide code Timer STM32 dengan penjelasan:

HardwareTimer *Timer2;

void setup() {
    Serial.begin(115200);
    pinMode(PC13, OUTPUT);
    
    // Initialize Timer 2
    Timer2 = new HardwareTimer(TIM2);
    
    // Set overflow frequency to 1 Hz
    Timer2->setOverflow(1, HERTZ_FORMAT);
    
    // Attach callback
    Timer2->attachInterrupt(timerCallback);
    
    // Start timer
    Timer2->resume();
    
    Serial.println("Timer Demo Started!");
}

void timerCallback() {
    digitalWrite(PC13, !digitalRead(PC13));
    Serial.println("Timer tick!");
}

void loop() {
    // Main loop is FREE to do other tasks!
    // No delay() needed for LED blink
}

Callout boxes:
• "HERTZ_FORMAT = timer calculates PSC/ARR automatically"
• "Main loop tidak terblokir!"
```

---

## Slide 12: Demo 4 - Hardware Timer ESP32

**Prompt:**
```
Buat slide "Demo 4: Hardware Timer ESP32" dengan code:

hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
volatile uint32_t isrCount = 0;

void IRAM_ATTR onTimer() {
    portENTER_CRITICAL_ISR(&timerMux);
    isrCount++;
    portEXIT_CRITICAL_ISR(&timerMux);
    
    // Toggle LED langsung di ISR (cepat)
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
    
    // Timer 0, prescaler 80 (1MHz), count up
    timer = timerBegin(0, 80, true);
    
    // Attach interrupt
    timerAttachInterrupt(timer, &onTimer, true);
    
    // Alarm 500ms (500,000 µs), auto-reload
    timerAlarmWrite(timer, 500000, true);
    
    timerAlarmEnable(timer);
}

void loop() {
    // Monitor ISR count
    Serial.printf("ISR count: %d\n", isrCount);
    delay(2000);
}

Highlight: portMUX, IRAM_ATTR, critical section
```

---

## Slide 13: Hands-on Task 2

**Prompt:**
```
Buat slide "Hands-on Task 2: Stopwatch" dengan spesifikasi:

TUGAS:
Buat stopwatch sederhana menggunakan timer dan button interrupt

Fitur:
┌────────────────────────────────────────────────────┐
│ • Button 1: Start/Stop stopwatch                   │
│ • Button 2: Reset stopwatch                        │
│ • Timer: Update counter setiap 100ms               │
│ • Serial: Display waktu dalam format MM:SS.S       │
└────────────────────────────────────────────────────┘

Expected Output:
> Stopwatch Ready!
> [START] 00:00.0
> [RUNNING] 00:01.5
> [RUNNING] 00:03.2
> [STOP] 00:04.8
> [RESET] 00:00.0

Bonus Points:
★ Tambah fitur lap time
★ Tambah buzzer beep setiap menit

Waktu: 20 menit
Tingkat: ⭐⭐⭐ (Menengah-Lanjut)
```

---

## Slide 14: Advanced Application - Frequency Counter

**Prompt:**
```
Buat slide "Advanced: Frequency Counter" dengan diagram sistem:

Konsep:
┌─────────────────────────────────────────────────────┐
│ Hitung jumlah pulse input dalam periode waktu tetap│
│ (Gate Time) menggunakan 2 interrupt sources        │
└─────────────────────────────────────────────────────┘

System Diagram:
                    ┌─────────────┐
  Signal Input ────►│ Input Capture│────► Pulse Count
  (GPIO + INT)      │  Interrupt   │
                    └─────────────┘
                          │
                    ┌─────────────┐
  Gate Timer ──────►│ Timer       │────► Gate = 1 second
  (1 Hz)            │ Interrupt   │
                    └─────────────┘
                          │
                          ▼
               Frequency = Pulse Count / Gate Time

Timing:
Gate:   ├────── 1 second ──────┤
Input:  ▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌ (count = 1000)
Result: Frequency = 1000 Hz
```

---

## Slide 15: Code - Frequency Counter

**Prompt:**
```
Buat slide code Frequency Counter dengan penjelasan:

volatile uint32_t pulseCount = 0;
volatile bool measureReady = false;
hw_timer_t *gateTimer = NULL;

// Pulse input interrupt
void IRAM_ATTR pulseISR() {
    pulseCount++;
}

// Gate timer interrupt (1 second)
void IRAM_ATTR gateISR() {
    measureReady = true;
}

void setup() {
    Serial.begin(115200);
    
    // Input untuk signal (contoh: GPIO4)
    pinMode(4, INPUT);
    attachInterrupt(4, pulseISR, RISING);
    
    // Gate timer 1 detik
    gateTimer = timerBegin(0, 80, true);
    timerAttachInterrupt(gateTimer, &gateISR, true);
    timerAlarmWrite(gateTimer, 1000000, true);  // 1s
    timerAlarmEnable(gateTimer);
}

void loop() {
    if (measureReady) {
        measureReady = false;
        uint32_t frequency = pulseCount;
        pulseCount = 0;
        
        Serial.printf("Frequency: %d Hz\n", frequency);
    }
}

Use case: Mengukur RPM motor, frekuensi sinyal, pulse sensor
```

---

## Slide 16: Advanced Application - Reaction Time Tester

**Prompt:**
```
Buat slide "Advanced: Reaction Time Tester" dengan:

Game Flow:
┌─────────────────────────────────────────────────────┐
│ 1. LED mati, tunggu random (1-5 detik)              │
│ 2. LED menyala → Timer mulai                        │
│ 3. User tekan button → Timer stop                   │
│ 4. Tampilkan reaction time dalam milidetik          │
│ 5. Ulangi game                                      │
└─────────────────────────────────────────────────────┘

Timing Diagram:
State:  [WAIT]        [REACT]        [RESULT]
LED:    ──────────────┬XXXXXXXXXXXXXX┬────────
                      │              │
Button:               │        ▼     │
Timer:                │ Start  Stop  │
                      │◄─── Δt ────►│
                      │ "Reaction: 250ms"

Challenge:
⚠️ Jika user tekan SEBELUM LED menyala = "Too early!"
⚠️ Timeout setelah 2 detik = "Too slow!"
```

---

## Slide 17: Code - Reaction Time Tester

**Prompt:**
```
Buat slide code Reaction Time dengan state machine:

enum GameState { IDLE, WAITING, REACT, RESULT };
volatile GameState state = IDLE;
volatile unsigned long startTime = 0;
volatile unsigned long reactionTime = 0;

void IRAM_ATTR buttonISR() {
    if (state == WAITING) {
        // Pressed too early!
        state = IDLE;
    } else if (state == REACT) {
        reactionTime = micros() - startTime;
        state = RESULT;
    }
}

void loop() {
    switch (state) {
        case IDLE:
            Serial.println("Press to start...");
            if (buttonPressed) startGame();
            break;
            
        case WAITING:
            // Random delay 1-5 seconds
            delay(random(1000, 5000));
            digitalWrite(LED, HIGH);
            startTime = micros();
            state = REACT;
            break;
            
        case RESULT:
            Serial.printf("Reaction: %d µs\n", 
                          reactionTime);
            state = IDLE;
            break;
    }
}

Highlight state machine pattern dan micros() untuk precision.
```

---

## Slide 18: Interrupt Priority Demonstration

**Prompt:**
```
Buat slide "Demo: Interrupt Priority" dengan:

Skenario Test:
┌─────────────────────────────────────────────────────┐
│ BTN1 (Priority HIGH): Print "HIGH" setiap 10ms      │
│ BTN2 (Priority LOW): Print "LOW" setiap 10ms        │
│                                                     │
│ Test: Tekan BTN2, lalu tekan BTN1 saat BTN2 handler │
│       masih running                                 │
└─────────────────────────────────────────────────────┘

Expected Output (Nested Interrupt):
LOW handler start
    HIGH handler start    ◄─── HIGH interrupt LOW
    HIGH handler end
LOW handler end           ◄─── LOW continues

Timeline:
BTN2:    ┌──────────────────┐
         │   LOW Handler    │
BTN1:    │  ┌──────────┐    │
         │  │HIGH Hndlr│    │
         ▼  ▼          ▼    ▼
Time:    t0 t1         t2   t3

⚠️ Ini adalah "Nested Interrupt" - fitur NVIC!
```

---

## Slide 19: Common Mistakes & Debugging

**Prompt:**
```
Buat slide "Common Mistakes & Debugging" dengan:

❌ MISTAKE 1: Tidak clear flag
void EXTI0_IRQHandler(void) {
    // Handler code...
    // Lupa: EXTI->PR = EXTI_PR_PR0;
}
Result: ISR dipanggil terus-menerus!

❌ MISTAKE 2: Lupa IRAM_ATTR (ESP32)
void buttonISR() {  // Missing IRAM_ATTR!
    buttonPressed = true;
}
Result: Guru panic crash!

❌ MISTAKE 3: Tidak pakai volatile
bool buttonPressed = false;  // Missing volatile!
void IRAM_ATTR buttonISR() {
    buttonPressed = true;
}
Result: Compiler optimize → flag tidak pernah update!

❌ MISTAKE 4: Blocking dalam ISR
void IRAM_ATTR buttonISR() {
    Serial.println("Test");  // BLOCKING!
    delay(100);              // BLOCKING!
}
Result: System hang atau timing issues!

Gunakan ❌ merah untuk kesalahan dan ✓ hijau untuk koreksi.
```

---

## Slide 20: Debugging Techniques

**Prompt:**
```
Buat slide "Debugging Techniques" dengan tips:

1. GPIO Toggle Method:
┌────────────────────────────────────────────────────┐
│ void IRAM_ATTR myISR() {                           │
│     digitalWrite(DEBUG_PIN, HIGH);                 │
│     // ISR code                                    │
│     digitalWrite(DEBUG_PIN, LOW);                  │
│ }                                                  │
│                                                    │
│ → Ukur pulse width dengan oscilloscope            │
│ → Mengetahui berapa lama ISR berjalan             │
└────────────────────────────────────────────────────┘

2. Counter Verification:
┌────────────────────────────────────────────────────┐
│ volatile uint32_t isrCounter = 0;                  │
│ void IRAM_ATTR myISR() { isrCounter++; }           │
│                                                    │
│ void loop() {                                      │
│     static uint32_t lastCount = 0;                 │
│     if (isrCounter != lastCount) {                 │
│         Serial.printf("ISR: %d\n", isrCounter);    │
│         lastCount = isrCounter;                    │
│     }                                              │
│ }                                                  │
└────────────────────────────────────────────────────┘

3. LED Status Indicator:
• ISR entry: LED ON
• ISR exit: LED OFF
• Blink rate = ISR frequency
```

---

## Slide 21: Performance Comparison

**Prompt:**
```
Buat slide "Performance Comparison" dengan tabel benchmark:

INTERRUPT LATENCY TEST (Button → LED Toggle):

┌─────────────────┬────────────────┬────────────────┐
│ Method          │ STM32F103      │ ESP32          │
├─────────────────┼────────────────┼────────────────┤
│ Polling (1ms)   │ ~1000 µs       │ ~1000 µs       │
│ Polling (100µs) │ ~100 µs        │ ~100 µs        │
│ Interrupt       │ ~2 µs          │ ~5 µs          │
├─────────────────┼────────────────┼────────────────┤
│ CPU Usage Poll  │ ~90%           │ ~90%           │
│ CPU Usage INT   │ ~0.1%          │ ~0.1%          │
└─────────────────┴────────────────┴────────────────┘

TIMER ACCURACY TEST (1 second interval):

┌─────────────────┬────────────────┬────────────────┐
│ Method          │ STM32 Error    │ ESP32 Error    │
├─────────────────┼────────────────┼────────────────┤
│ delay(1000)     │ ±5ms           │ ±10ms          │
│ Hardware Timer  │ ±0.1ms         │ ±0.1ms         │
└─────────────────┴────────────────┴────────────────┘

Highlight: "Hardware Timer = 100x more accurate!"
```

---

## Slide 22: Hands-on Task 3 - Integration

**Prompt:**
```
Buat slide "Hands-on Task 3: Integration Challenge" dengan:

FINAL CHALLENGE:
Buat sistem "Event Logger" yang menggabungkan semua konsep

Requirements:
┌────────────────────────────────────────────────────┐
│ 1. 3 Button dengan External Interrupt              │
│ 2. Timer untuk timestamp (microsecond precision)   │
│ 3. Heartbeat LED (blink 1 Hz via timer)            │
│ 4. Serial output dengan format:                    │
│    [TIMESTAMP] EVENT: Button_X pressed             │
└────────────────────────────────────────────────────┘

Expected Output:
[0.000000] System Started
[0.000000] Heartbeat LED: ON
[1.000000] Heartbeat LED: OFF
[1.523456] EVENT: Button_1 pressed
[2.000000] Heartbeat LED: ON
[2.891234] EVENT: Button_2 pressed
[3.000000] Heartbeat LED: OFF
...

Bonus: Hitung dan tampilkan rata-rata response time

Waktu: 25 menit
Tingkat: ⭐⭐⭐⭐ (Lanjut)
```

---

## Slide 23: Project Preview

**Prompt:**
```
Buat slide "Project Preview: Security Monitoring System" dengan:

System Overview:
┌─────────────────────────────────────────────────────┐
│                                                     │
│  [PIR Sensor] ──┐                     ┌─► [Alert]   │
│  [Door Switch]──┼──► STM32 ◄──UART──► ESP32 ──► [Log]│
│  [Window Sens]──┤    (Edge)           (Hub)  └─► [Serial]│
│  [Panic Button]─┘                                   │
│                                                     │
└─────────────────────────────────────────────────────┘

Key Features:
• NVIC Priority untuk different sensors
• Timer-based timestamp dengan precision
• Interrupt-driven response (no polling)
• UART communication dengan checksum

Deadline: 2 minggu
Deliverables: Code + Documentation + Video Demo

"This project combines ALL concepts from this module!"
```

---

## Slide 24: Assessment Criteria

**Prompt:**
```
Buat slide "Assessment Criteria" dengan rubrik:

PRAKTIKUM (40%):
┌────────────────────────────────────────────────────┐
│ ✓ Task 1: Multiple Button Handler    [10 points]   │
│ ✓ Task 2: Stopwatch                  [15 points]   │
│ ✓ Task 3: Event Logger               [15 points]   │
└────────────────────────────────────────────────────┘

LAPORAN (30%):
┌────────────────────────────────────────────────────┐
│ ✓ Dokumentasi lengkap                [10 points]   │
│ ✓ Analisis hasil                     [10 points]   │
│ ✓ Jawaban pertanyaan                 [10 points]   │
└────────────────────────────────────────────────────┘

PEMAHAMAN (20%):
┌────────────────────────────────────────────────────┐
│ ✓ Bisa menjelaskan konsep interrupt  [10 points]   │
│ ✓ Bisa menghitung timer              [10 points]   │
└────────────────────────────────────────────────────┘

KEAKTIFAN (10%):
┌────────────────────────────────────────────────────┐
│ ✓ Partisipasi dan inisiatif          [10 points]   │
└────────────────────────────────────────────────────┘
```

---

## Slide 25: Quick Reference Card

**Prompt:**
```
Buat slide "Quick Reference Card" untuk di-print:

STM32 INTERRUPT:
attachInterrupt(pin, ISR, mode);
detachInterrupt(pin);
// modes: RISING, FALLING, CHANGE

STM32 TIMER:
HardwareTimer *tim = new HardwareTimer(TIMx);
tim->setOverflow(freq, HERTZ_FORMAT);
tim->attachInterrupt(callback);
tim->resume();

ESP32 INTERRUPT:
void IRAM_ATTR myISR() { ... }
attachInterrupt(pin, myISR, mode);

ESP32 TIMER:
hw_timer_t *t = timerBegin(num, prescaler, countUp);
timerAttachInterrupt(t, &ISR, edge);
timerAlarmWrite(t, value, autoReload);
timerAlarmEnable(t);

TIMER CALCULATION:
Timer_Freq = Clock / (PSC + 1)
Period = (ARR + 1) / Timer_Freq

Format: business card style, bisa di-print 4 per halaman.
```

---

## Slide 26: FAQ

**Prompt:**
```
Buat slide "Frequently Asked Questions" dengan:

Q1: Mengapa ISR harus singkat?
A: ISR memblokir interrupt lain. Jika terlalu lama,
   sistem menjadi tidak responsif dan bisa miss event.

Q2: Kapan pakai polling vs interrupt?
A: Polling: event sangat sering (>10kHz), sederhana
   Interrupt: event sporadis, perlu response cepat

Q3: Berapa max interrupt yang bisa aktif?
A: STM32F103: 16 EXTI + timer + peripheral
   ESP32: 32 per core × 2 cores = 64 slots

Q4: Bagaimana debug ISR yang tidak bisa Serial.print?
A: Gunakan GPIO toggle + oscilloscope, atau
   set flag dan print di main loop.

Q5: Timer tidak akurat, kenapa?
A: Cek: clock source, prescaler calculation,
   interrupt priority, ISR execution time.

Format Q&A dengan warna berbeda untuk question dan answer.
```

---

## Slide 27: Resources & Next Steps

**Prompt:**
```
Buat slide "Resources & Next Steps" dengan:

📚 RESOURCES:
• GitHub Repository: [QR Code]
• Video Tutorial: [QR Code]
• Dokumentasi Online: [QR Code]

🔗 USEFUL LINKS:
• STM32 Timer Cookbook (AN4776)
• ESP-IDF Timer Documentation
• ARM NVIC Tutorial

📅 NEXT MODULE:
BAB 03: Serial Communication (UART)
• Mengirim data antar MCU
• Protokol komunikasi
• Integration dengan interrupt

💡 PREPARATION:
• Review konsep serial communication
• Pastikan hardware ready
• Install serial terminal (HTerm/Realterm)

Gunakan QR code placeholder untuk setiap link.
```

---

## Slide 28: Closing

**Prompt:**
```
Buat slide "Terima Kasih" dengan:

🎯 KEY TAKEAWAYS:
1. Interrupt = Responsif & Efisien
2. Timer = Presisi & Non-blocking
3. Best Practice = Short ISR + Flag Pattern
4. Debug = GPIO Toggle + Counter

📧 CONTACT:
[Email instruktur]
[Office hours]

🙋 QUESTIONS?

Background: gradient dengan circuit pattern subtle
Footer: "BAB 02: Interrupt dan Timer | Praktikum Sistem Embedded"

Tambahkan ikon questions/discussion untuk Q&A session.
```

---

## Catatan untuk Pembuatan PPT

1. **Konsistensi dengan Bagian 1:**
   - Gunakan color scheme yang sama
   - Font dan style harus seragam
   - Transition antar slide konsisten

2. **Interaktivitas:**
   - Slide demo bisa ditambah video recording
   - Hands-on task dengan timer countdown
   - Quick reference bisa di-print sebagai handout

3. **Timing Presentasi:**
   - Bagian 2 total: ~90 menit
   - Sisakan waktu untuk hands-on (45 menit)
   - Demo live coding jika memungkinkan
