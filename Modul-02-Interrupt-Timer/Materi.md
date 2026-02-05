# BAB 02: Interrupt dan Timer

## 🎯 Capaian Pembelajaran

Setelah menyelesaikan bab ini, mahasiswa diharapkan mampu:

1. Memahami konsep dasar Interrupt pada arsitektur ARM Cortex-M dan Xtensa
2. Menguasai mekanisme NVIC pada STM32 dan sistem interrupt ESP32
3. Memahami dan mengimplementasikan Timer/Counter pada kedua platform
4. Mengembangkan aplikasi berbasis interrupt-driven untuk sistem real-time
5. Menerapkan teknik debouncing berbasis interrupt dan timer
6. Mengoptimalkan penggunaan sumber daya dengan interrupt dan timer

---

## 📚 Materi Pembelajaran

### 1. Pendahuluan Interrupt

#### 1.1 Definisi dan Konsep Dasar

**Interrupt** adalah mekanisme hardware yang memungkinkan processor menghentikan sementara eksekusi program utama untuk menangani event penting yang memerlukan perhatian segera. Setelah interrupt ditangani, processor melanjutkan eksekusi program utama dari titik terakhir.

**Mengapa Interrupt Penting?**
- **Responsivitas**: Sistem dapat merespons event eksternal dengan segera
- **Efisiensi**: Tidak perlu polling terus-menerus (CPU-intensive)
- **Determinisme**: Waktu respons yang dapat diprediksi
- **Real-time**: Mendukung aplikasi yang time-critical

#### 1.2 Perbandingan: Polling vs Interrupt

| Aspek | Polling | Interrupt |
|-------|---------|-----------|
| **Mekanisme** | CPU terus memeriksa status | CPU diberitahu saat event terjadi |
| **Efisiensi CPU** | Rendah (CPU sibuk memeriksa) | Tinggi (CPU bebas melakukan hal lain) |
| **Waktu Respons** | Tergantung frekuensi polling | Sangat cepat (mikrodetik) |
| **Kompleksitas** | Sederhana | Lebih kompleks |
| **Power Consumption** | Tinggi | Rendah |
| **Cocok Untuk** | Sistem sederhana | Sistem real-time |

```
Polling:
┌─────────────────────────────────────────────────────┐
│ while(true) {                                       │
│   if (button_pressed()) {  // Cek terus-menerus     │
│     handle_button();                                │
│   }                                                 │
│   // CPU tidak bisa mengerjakan hal lain efektif    │
│ }                                                   │
└─────────────────────────────────────────────────────┘

Interrupt:
┌─────────────────────────────────────────────────────┐
│ // ISR dipanggil otomatis saat tombol ditekan       │
│ void EXTI0_IRQHandler(void) {                       │
│   handle_button();                                  │
│   clear_interrupt_flag();                           │
│ }                                                   │
│                                                     │
│ // Main program bebas melakukan hal lain            │
│ while(true) {                                       │
│   do_other_tasks();  // Efisien!                    │
│ }                                                   │
└─────────────────────────────────────────────────────┘
```

---

### 2. Arsitektur Interrupt STM32F103C8T6

#### 2.1 Nested Vectored Interrupt Controller (NVIC)

STM32F103 menggunakan **NVIC** yang merupakan bagian integral dari core ARM Cortex-M3. NVIC mengelola semua interrupt dengan fitur:

- **Nested Interrupts**: Interrupt prioritas tinggi dapat meng-interrupt handler prioritas rendah
- **Vectored**: Setiap interrupt memiliki alamat handler sendiri
- **Programmable Priority**: 16 level prioritas (4 bit)
- **Low Latency**: 12 clock cycles untuk masuk ke handler

```
┌─────────────────────────────────────────────────────────────┐
│                    NVIC Architecture                         │
├─────────────────────────────────────────────────────────────┤
│  ┌──────────┐    ┌──────────────┐    ┌──────────────────┐   │
│  │ External │───▶│   NVIC       │───▶│ Cortex-M3 Core   │   │
│  │ Events   │    │ • Priority   │    │ • Save Context   │   │
│  └──────────┘    │ • Pending    │    │ • Jump to ISR    │   │
│                  │ • Enable/    │    │ • Restore        │   │
│  ┌──────────┐    │   Disable    │    └──────────────────┘   │
│  │ Peripheral│───▶│              │                          │
│  │ Interrupts│    └──────────────┘                          │
│  └──────────┘                                               │
└─────────────────────────────────────────────────────────────┘
```

#### 2.2 External Interrupt (EXTI) pada STM32

STM32F103 memiliki 16 jalur EXTI (EXTI0-EXTI15) yang dapat dikonfigurasi untuk mendeteksi:
- **Rising Edge**: Transisi LOW → HIGH
- **Falling Edge**: Transisi HIGH → LOW
- **Both Edges**: Kedua transisi

**Mapping GPIO ke EXTI:**
```
GPIO Pin      EXTI Line
────────────────────────
PA0, PB0, PC0  → EXTI0
PA1, PB1, PC1  → EXTI1
PA2, PB2, PC2  → EXTI2
...
PA15, PB15, PC15 → EXTI15
```

**Catatan Penting**: Hanya SATU pin dengan nomor yang sama dapat aktif di EXTI. Contoh: PA0 dan PB0 tidak bisa keduanya menggunakan EXTI0 bersamaan.

#### 2.3 Konfigurasi EXTI STM32 (Register Level)

```c
// 1. Enable clock untuk AFIO dan GPIO
RCC->APB2ENR |= RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPAEN;

// 2. Konfigurasi pin sebagai input dengan pull-up
GPIOA->CRL &= ~(GPIO_CRL_MODE0 | GPIO_CRL_CNF0);
GPIOA->CRL |= GPIO_CRL_CNF0_1;  // Input with pull-up/pull-down
GPIOA->ODR |= GPIO_ODR_ODR0;    // Enable pull-up

// 3. Map PA0 ke EXTI0
AFIO->EXTICR[0] &= ~AFIO_EXTICR1_EXTI0;  // PA0

// 4. Konfigurasi trigger (falling edge)
EXTI->FTSR |= EXTI_FTSR_TR0;   // Falling trigger
EXTI->RTSR &= ~EXTI_RTSR_TR0;  // Disable rising

// 5. Enable interrupt mask
EXTI->IMR |= EXTI_IMR_MR0;

// 6. Enable interrupt di NVIC
NVIC_EnableIRQ(EXTI0_IRQn);
NVIC_SetPriority(EXTI0_IRQn, 2);
```

#### 2.4 Interrupt Handler STM32

```c
// ISR harus memiliki nama yang tepat sesuai vector table
void EXTI0_IRQHandler(void) {
    // WAJIB: Clear pending bit agar interrupt tidak terus dipanggil
    if (EXTI->PR & EXTI_PR_PR0) {
        EXTI->PR = EXTI_PR_PR0;  // Clear by writing 1
        
        // Handler code
        button_pressed_flag = true;
    }
}
```

---

### 3. Arsitektur Interrupt ESP32

#### 3.1 Dual-Core Interrupt System

ESP32 memiliki arsitektur dual-core (PRO_CPU dan APP_CPU) dengan sistem interrupt yang lebih fleksibel:

```
┌─────────────────────────────────────────────────────────────┐
│                 ESP32 Interrupt Matrix                       │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌──────────────┐         ┌──────────────┐                  │
│  │  PRO_CPU     │         │  APP_CPU     │                  │
│  │  (Core 0)    │         │  (Core 1)    │                  │
│  │  32 Slots    │         │  32 Slots    │                  │
│  └──────┬───────┘         └──────┬───────┘                  │
│         │                        │                          │
│         └────────┬───────────────┘                          │
│                  │                                          │
│         ┌────────▼────────┐                                 │
│         │ Interrupt Matrix│                                 │
│         │  (Crossbar)     │                                 │
│         └────────┬────────┘                                 │
│                  │                                          │
│    ┌─────────────┼─────────────┐                            │
│    │             │             │                            │
│  ┌─▼──┐       ┌──▼─┐        ┌──▼─┐                          │
│  │GPIO│       │Timer│       │UART│  ... (71 sources)        │
│  └────┘       └────┘        └────┘                          │
└─────────────────────────────────────────────────────────────┘
```

#### 3.2 GPIO Interrupt ESP32

ESP32 mendukung interrupt pada SEMUA GPIO pin (kecuali input-only pins: 34-39):

**Interrupt Modes:**
```c
// Mode interrupt yang tersedia
#define DISABLED         0x00
#define RISING           0x01  // LOW → HIGH
#define FALLING          0x02  // HIGH → LOW
#define CHANGE           0x03  // Both edges
#define ONLOW            0x04  // Level LOW
#define ONHIGH           0x05  // Level HIGH
#define ONLOW_WE         0x0C  // LOW with wakeup
#define ONHIGH_WE        0x0D  // HIGH with wakeup
```

#### 3.3 Implementasi GPIO Interrupt ESP32 (Arduino)

```cpp
// Variabel volatile untuk komunikasi ISR-main
volatile bool buttonPressed = false;
volatile unsigned long lastInterruptTime = 0;

// ISR dengan IRAM_ATTR agar disimpan di RAM (akses cepat)
void IRAM_ATTR buttonISR() {
    unsigned long currentTime = millis();
    
    // Software debounce dalam ISR
    if (currentTime - lastInterruptTime > 200) {
        buttonPressed = true;
        lastInterruptTime = currentTime;
    }
}

void setup() {
    pinMode(GPIO_NUM_4, INPUT_PULLUP);
    
    // Attach interrupt dengan mode FALLING
    attachInterrupt(digitalPinToInterrupt(GPIO_NUM_4), 
                    buttonISR, 
                    FALLING);
}

void loop() {
    if (buttonPressed) {
        buttonPressed = false;
        Serial.println("Button pressed!");
    }
    // CPU bebas melakukan hal lain
}
```

#### 3.4 IRAM_ATTR Explained

```cpp
// IRAM_ATTR sangat PENTING untuk ESP32 ISR!
void IRAM_ATTR myISR() {
    // Kode ISR di sini
}
```

**Mengapa IRAM_ATTR diperlukan?**
1. ESP32 menyimpan kode program di Flash eksternal (SPI Flash)
2. Flash memerlukan waktu akses yang tidak konsisten
3. Saat interrupt terjadi, kode ISR HARUS tersedia segera
4. IRAM_ATTR memaksa kode disimpan di Internal RAM (akses < 1 cycle)
5. Tanpa IRAM_ATTR, sistem bisa crash (guru panic)

---

### 4. Timer pada STM32F103C8T6

#### 4.1 Jenis Timer STM32F103

| Timer | Tipe | Resolusi | Channel | Fitur Khusus |
|-------|------|----------|---------|--------------|
| TIM1 | Advanced | 16-bit | 4 | PWM, Dead-time, Break |
| TIM2 | General | 16-bit | 4 | Input Capture, PWM |
| TIM3 | General | 16-bit | 4 | Input Capture, PWM |
| TIM4 | General | 16-bit | 4 | Input Capture, PWM |
| SysTick | System | 24-bit | - | Delay, RTOS tick |

#### 4.2 Arsitektur Timer STM32

```
┌─────────────────────────────────────────────────────────────┐
│                    Timer Block Diagram                       │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  APB Clock (72MHz)                                          │
│       │                                                     │
│       ▼                                                     │
│  ┌─────────┐    ┌─────────┐    ┌─────────┐    ┌─────────┐  │
│  │Prescaler│───▶│ Counter │───▶│ Compare │───▶│ Output  │  │
│  │ (PSC)   │    │  (CNT)  │    │ (CCR)   │    │ Channel │  │
│  │ 16-bit  │    │ 16-bit  │    │ 16-bit  │    │         │  │
│  └─────────┘    └────┬────┘    └─────────┘    └─────────┘  │
│                      │                                      │
│                      ▼                                      │
│               ┌─────────────┐                               │
│               │ Auto-Reload │                               │
│               │    (ARR)    │                               │
│               │   16-bit    │                               │
│               └─────────────┘                               │
│                                                             │
│  Timer_Freq = APB_Clock / (PSC + 1)                         │
│  Period = Timer_Freq / (ARR + 1)                            │
│  Overflow_Time = (PSC + 1) × (ARR + 1) / APB_Clock          │
└─────────────────────────────────────────────────────────────┘
```

#### 4.3 Perhitungan Timer STM32

**Rumus Dasar:**
```
Timer Clock = APB Clock / (Prescaler + 1)
Update Period = (ARR + 1) / Timer Clock
Update Frequency = Timer Clock / (ARR + 1)
```

**Contoh: Timer 1 Hz (1 detik) dengan clock 72 MHz:**
```
1 Hz = 72,000,000 / ((PSC+1) × (ARR+1))
72,000,000 = (PSC+1) × (ARR+1)

Pilihan:
• PSC = 7199, ARR = 9999
  → Timer = 72MHz / 7200 = 10kHz
  → Period = 10000 / 10kHz = 1 detik ✓

• PSC = 35999, ARR = 1999
  → Timer = 72MHz / 36000 = 2kHz
  → Period = 2000 / 2kHz = 1 detik ✓
```

#### 4.4 Konfigurasi Timer STM32 (Register Level)

```c
// Timer 2 dengan interrupt setiap 100ms

// 1. Enable clock
RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

// 2. Set prescaler (72MHz / 7200 = 10kHz)
TIM2->PSC = 7200 - 1;

// 3. Set auto-reload (10kHz / 1000 = 10Hz = 100ms)
TIM2->ARR = 1000 - 1;

// 4. Enable update interrupt
TIM2->DIER |= TIM_DIER_UIE;

// 5. Enable interrupt di NVIC
NVIC_EnableIRQ(TIM2_IRQn);
NVIC_SetPriority(TIM2_IRQn, 3);

// 6. Start timer
TIM2->CR1 |= TIM_CR1_CEN;
```

#### 4.5 Timer Interrupt Handler STM32

```c
volatile uint32_t timerCounter = 0;

void TIM2_IRQHandler(void) {
    if (TIM2->SR & TIM_SR_UIF) {
        TIM2->SR &= ~TIM_SR_UIF;  // Clear flag
        
        timerCounter++;
        // Toggle LED setiap 10 interrupt (1 detik)
        if (timerCounter >= 10) {
            GPIOC->ODR ^= GPIO_ODR_ODR13;
            timerCounter = 0;
        }
    }
}
```

---

### 5. Timer pada ESP32

#### 5.1 Hardware Timer ESP32

ESP32 memiliki 4 hardware timer 64-bit yang sangat presisi:

| Timer Group | Timer | Resolusi | Counter |
|-------------|-------|----------|---------|
| Group 0 | Timer 0 | 64-bit | Up/Down |
| Group 0 | Timer 1 | 64-bit | Up/Down |
| Group 1 | Timer 0 | 64-bit | Up/Down |
| Group 1 | Timer 1 | 64-bit | Up/Down |

#### 5.2 Konfigurasi Timer ESP32 (Arduino)

```cpp
hw_timer_t *timer = NULL;
volatile int interruptCounter = 0;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer() {
    portENTER_CRITICAL_ISR(&timerMux);
    interruptCounter++;
    portEXIT_CRITICAL_ISR(&timerMux);
}

void setup() {
    Serial.begin(115200);
    
    // Timer 0, prescaler 80 (80MHz/80 = 1MHz = 1µs per tick)
    timer = timerBegin(0, 80, true);
    
    // Attach interrupt handler
    timerAttachInterrupt(timer, &onTimer, true);
    
    // Set alarm setiap 1,000,000 µs = 1 detik
    timerAlarmWrite(timer, 1000000, true);  // true = auto-reload
    
    // Enable alarm
    timerAlarmEnable(timer);
}

void loop() {
    if (interruptCounter > 0) {
        portENTER_CRITICAL(&timerMux);
        interruptCounter--;
        portEXIT_CRITICAL(&timerMux);
        
        Serial.println("Timer interrupt occurred!");
    }
}
```

#### 5.3 ESP32 Timer API Reference

```cpp
// Inisialisasi timer
// timer_num: 0-3
// prescaler: 2-65536 (80 = 1µs @ 80MHz)
// countUp: true = count up, false = count down
hw_timer_t* timerBegin(uint8_t timer_num, uint16_t prescaler, bool countUp);

// Attach interrupt
timerAttachInterrupt(hw_timer_t *timer, void (*fn)(void), bool edge);

// Set alarm value
// alarm_value: count value untuk trigger
// autoreload: true = repeat, false = one-shot
timerAlarmWrite(hw_timer_t *timer, uint64_t alarm_value, bool autoreload);

// Enable/disable alarm
timerAlarmEnable(hw_timer_t *timer);
timerAlarmDisable(hw_timer_t *timer);

// Baca counter
uint64_t timerRead(hw_timer_t *timer);

// Reset counter
timerRestart(hw_timer_t *timer);
```

---

### 6. Perbandingan Interrupt dan Timer: STM32 vs ESP32

#### 6.1 Tabel Perbandingan

| Aspek | STM32F103C8T6 | ESP32 |
|-------|---------------|-------|
| **Core** | ARM Cortex-M3 | Xtensa LX6 Dual-Core |
| **Interrupt Controller** | NVIC | Interrupt Matrix |
| **Priority Levels** | 16 (4-bit) | 7 levels |
| **External Interrupt** | 16 EXTI lines | All GPIOs |
| **Timer Count** | 4 (16-bit) | 4 (64-bit) |
| **Timer Resolution** | 16-bit | 64-bit |
| **Clock Speed** | 72 MHz | 80 MHz (typical) |
| **ISR Attribute** | None needed | IRAM_ATTR required |
| **Critical Section** | `__disable_irq()` | `portENTER_CRITICAL()` |

#### 6.2 Contoh Perbandingan Kode

**STM32 Timer Interrupt:**
```c
// Setup
TIM2->PSC = 7199;
TIM2->ARR = 9999;
TIM2->DIER |= TIM_DIER_UIE;
NVIC_EnableIRQ(TIM2_IRQn);
TIM2->CR1 |= TIM_CR1_CEN;

// ISR
void TIM2_IRQHandler(void) {
    if (TIM2->SR & TIM_SR_UIF) {
        TIM2->SR &= ~TIM_SR_UIF;
        // Handle timer
    }
}
```

**ESP32 Timer Interrupt:**
```cpp
// Setup
hw_timer_t *timer = timerBegin(0, 80, true);
timerAttachInterrupt(timer, &onTimer, true);
timerAlarmWrite(timer, 1000000, true);
timerAlarmEnable(timer);

// ISR (MUST have IRAM_ATTR!)
void IRAM_ATTR onTimer() {
    portENTER_CRITICAL_ISR(&timerMux);
    // Handle timer
    portEXIT_CRITICAL_ISR(&timerMux);
}
```

---

### 7. Best Practices untuk Interrupt dan Timer

#### 7.1 Aturan Umum ISR

1. **Singkat dan Cepat**
   - ISR harus selesai secepat mungkin
   - Maksimal beberapa mikrodetik
   - Hindari operasi blocking

2. **Tidak Ada Blocking**
   ```cpp
   // SALAH - Jangan lakukan ini di ISR!
   void IRAM_ATTR badISR() {
       delay(100);              // ❌ Blocking
       Serial.println("Hi");    // ❌ Bisa blocking
       float x = sqrt(123.45);  // ❌ Operasi berat
   }
   
   // BENAR - ISR yang baik
   void IRAM_ATTR goodISR() {
       flag = true;  // ✓ Set flag saja
       counter++;    // ✓ Operasi sederhana
   }
   ```

3. **Gunakan `volatile`**
   ```cpp
   volatile bool flag = false;       // ✓ Compiler tidak akan optimize
   volatile uint32_t counter = 0;    // ✓ Nilai selalu dibaca dari RAM
   ```

4. **Critical Section untuk Shared Data**
   ```cpp
   // ESP32
   portENTER_CRITICAL(&mux);
   sharedData++;
   portEXIT_CRITICAL(&mux);
   
   // STM32
   __disable_irq();
   sharedData++;
   __enable_irq();
   ```

#### 7.2 Debouncing dengan Timer

**Hardware vs Software Debounce:**
```cpp
// Timer-based debounce (lebih reliable)
volatile unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;  // 50ms

void IRAM_ATTR buttonISR() {
    unsigned long now = millis();
    if (now - lastDebounceTime > debounceDelay) {
        buttonPressed = true;
        lastDebounceTime = now;
    }
}
```

#### 7.3 Pattern: Flag-based Interrupt Handling

```cpp
// ISR hanya set flag
volatile bool eventOccurred = false;

void IRAM_ATTR eventISR() {
    eventOccurred = true;
}

// Main loop handle event
void loop() {
    if (eventOccurred) {
        eventOccurred = false;
        
        // Handle event di sini (bisa panjang)
        processEvent();
        sendToSerial();
        updateDisplay();
    }
    
    // Tugas lain bisa berjalan
    doOtherTasks();
}
```

---

### 8. Aplikasi Praktis

#### 8.1 Frequency Counter

```cpp
// Menghitung frekuensi sinyal input menggunakan interrupt

volatile uint32_t pulseCount = 0;
volatile bool measureReady = false;
hw_timer_t *gateTimer = NULL;

// Interrupt untuk setiap pulse input
void IRAM_ATTR pulseISR() {
    pulseCount++;
}

// Timer interrupt setiap 1 detik (gate time)
void IRAM_ATTR gateISR() {
    measureReady = true;
}

void setup() {
    Serial.begin(115200);
    
    // Input frequency pada GPIO 4
    pinMode(4, INPUT);
    attachInterrupt(digitalPinToInterrupt(4), pulseISR, RISING);
    
    // Gate timer 1 detik
    gateTimer = timerBegin(0, 80, true);
    timerAttachInterrupt(gateTimer, &gateISR, true);
    timerAlarmWrite(gateTimer, 1000000, true);
    timerAlarmEnable(gateTimer);
}

void loop() {
    if (measureReady) {
        measureReady = false;
        uint32_t freq = pulseCount;
        pulseCount = 0;
        
        Serial.printf("Frequency: %d Hz\n", freq);
    }
}
```

#### 8.2 Multi-Button Handler dengan Single Interrupt

```cpp
// Efficient multi-button handling

#define BTN_A  4
#define BTN_B  5
#define BTN_C  18

volatile uint8_t buttonState = 0;

void IRAM_ATTR buttonAISR() { buttonState |= 0x01; }
void IRAM_ATTR buttonBISR() { buttonState |= 0x02; }
void IRAM_ATTR buttonCISR() { buttonState |= 0x04; }

void setup() {
    pinMode(BTN_A, INPUT_PULLUP);
    pinMode(BTN_B, INPUT_PULLUP);
    pinMode(BTN_C, INPUT_PULLUP);
    
    attachInterrupt(digitalPinToInterrupt(BTN_A), buttonAISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(BTN_B), buttonBISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(BTN_C), buttonCISR, FALLING);
}

void loop() {
    if (buttonState) {
        uint8_t state = buttonState;
        buttonState = 0;
        
        if (state & 0x01) Serial.println("Button A pressed");
        if (state & 0x02) Serial.println("Button B pressed");
        if (state & 0x04) Serial.println("Button C pressed");
    }
}
```

---

### 9. Troubleshooting Interrupt dan Timer

#### 9.1 Masalah Umum dan Solusi

| Masalah | Kemungkinan Penyebab | Solusi |
|---------|---------------------|--------|
| ISR tidak dipanggil | Interrupt tidak di-enable | Cek NVIC/attachInterrupt |
| ISR dipanggil terus-menerus | Flag tidak di-clear | Tambahkan clear flag |
| Sistem hang | ISR terlalu lama | Pindahkan kode ke loop |
| Nilai variabel tidak update | Tidak pakai `volatile` | Tambahkan volatile |
| ESP32 crash (guru panic) | ISR tanpa IRAM_ATTR | Tambahkan IRAM_ATTR |
| Timer tidak akurat | Prescaler/ARR salah | Hitung ulang nilai |
| Button bounce | Tidak ada debounce | Implementasi debounce |

#### 9.2 Debugging Tips

1. **Gunakan GPIO Toggle untuk Timing**
   ```cpp
   void IRAM_ATTR myISR() {
       digitalWrite(DEBUG_PIN, HIGH);  // Measure with oscilloscope
       // ISR code
       digitalWrite(DEBUG_PIN, LOW);
   }
   ```

2. **Counter untuk Verifikasi**
   ```cpp
   volatile uint32_t isrCounter = 0;
   
   void IRAM_ATTR myISR() {
       isrCounter++;
   }
   
   // Print di loop untuk verifikasi
   Serial.printf("ISR called %d times\n", isrCounter);
   ```

---

## 📖 Referensi

### Dokumentasi Resmi
1. **STM32F103C8 Reference Manual** (RM0008) - ST Microelectronics
2. **STM32F103C8 Datasheet** - ST Microelectronics
3. **ARM Cortex-M3 Technical Reference Manual** - ARM Limited
4. **ESP32 Technical Reference Manual** - Espressif Systems
5. **ESP-IDF Programming Guide** - Espressif Systems

### Buku Referensi
1. *Mastering STM32* - Carmine Noviello (Leanpub)
2. *The Definitive Guide to ARM Cortex-M3* - Joseph Yiu
3. *ESP32 Programming for IoT* - Sever Spanulescu

### Tutorial Online
1. [Interrupt Handling in STM32](https://www.st.com/resource/en/application_note/an4228.pdf)
2. [ESP32 Interrupt Tutorial](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/intr_alloc.html)
3. [STM32 Timer Cookbook](https://www.st.com/resource/en/application_note/an4776.pdf)

