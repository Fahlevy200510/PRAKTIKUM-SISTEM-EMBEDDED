# BAB 01: GPIO dan Digital I/O

## 🎯 Capaian Pembelajaran

Setelah menyelesaikan bab ini, mahasiswa diharapkan mampu:

1. **Memahami** arsitektur dan fungsi GPIO pada mikrokontroler STM32 dan ESP32
2. **Mengkonfigurasi** pin GPIO sebagai input maupun output dengan berbagai mode
3. **Mengimplementasikan** teknik debouncing untuk input dari push button
4. **Menerapkan** kontrol LED dengan berbagai pola dan teknik PWM sederhana
5. **Menganalisis** perbedaan karakteristik GPIO antara STM32 dan ESP32
6. **Merancang** sistem digital I/O untuk aplikasi embedded sederhana
7. **Menerapkan** best practices dalam pemrograman GPIO

---

## 📚 1. Pendahuluan

### 1.1 Apa itu GPIO?

**GPIO (General Purpose Input/Output)** adalah pin pada mikrokontroler yang dapat dikonfigurasi secara fleksibel sebagai input atau output digital. GPIO merupakan interface paling dasar dan fundamental dalam sistem embedded untuk berinteraksi dengan dunia luar.

```
┌─────────────────────────────────────────────────────────────┐
│                    MIKROKONTROLER                           │
│  ┌─────────────────────────────────────────────────────┐   │
│  │                      CPU                             │   │
│  │   ┌─────────┐    ┌─────────┐    ┌─────────┐        │   │
│  │   │ Program │    │  Data   │    │  ALU    │        │   │
│  │   │ Memory  │    │ Memory  │    │         │        │   │
│  │   └─────────┘    └─────────┘    └─────────┘        │   │
│  └─────────────────────────────────────────────────────┘   │
│                           │                                 │
│                    ┌──────┴──────┐                         │
│                    │ GPIO Block  │                         │
│                    │  Registers  │                         │
│                    └──────┬──────┘                         │
│                           │                                 │
└───────────────────────────┼─────────────────────────────────┘
                            │
            ┌───────────────┼───────────────┐
            │               │               │
         ┌──┴──┐         ┌──┴──┐         ┌──┴──┐
         │PIN 0│         │PIN 1│         │PIN n│
         └──┬──┘         └──┬──┘         └──┬──┘
            │               │               │
         ┌──┴──┐         ┌──┴──┐         ┌──┴──┐
         │ LED │         │Button│        │Sensor│
         └─────┘         └─────┘         └─────┘
```

### 1.2 Pentingnya GPIO dalam Embedded Systems

GPIO adalah **building block** fundamental untuk:

| Aplikasi | Contoh Penggunaan |
|----------|-------------------|
| **Aktuator** | Mengendalikan LED, relay, motor driver |
| **Sensor Digital** | Membaca push button, limit switch, PIR sensor |
| **Komunikasi** | Bit-banging SPI, I2C, atau protokol custom |
| **Debugging** | LED status, logic analyzer interface |
| **User Interface** | Keypad matrix, DIP switch, seven segment |

### 1.3 Perbandingan GPIO: STM32 vs ESP32

| Fitur | STM32F103C8T6 | ESP32 |
|-------|---------------|-------|
| **Arsitektur** | ARM Cortex-M3 | Xtensa LX6 Dual Core |
| **Tegangan I/O** | 3.3V (5V tolerant pada beberapa pin) | 3.3V (TIDAK 5V tolerant) |
| **Jumlah GPIO** | 37 pin | 34 pin |
| **Drive Current** | Max 25mA per pin | Max 40mA (20mA recommended) |
| **Internal Pull-up/down** | Ya (40kΩ typical) | Ya (45kΩ typical) |
| **Alternate Functions** | Timer, USART, SPI, I2C, ADC | Touch, ADC, DAC, PWM, etc |
| **Built-in LED** | PC13 (Active LOW) | GPIO2 (Active HIGH) |
| **Konfigurasi Mode** | 8 mode per pin | Input/Output/Disable |

---

## 📚 2. Teori Dasar GPIO

### 2.1 Struktur Internal GPIO

#### 2.1.1 STM32F103 GPIO Structure

```
                          VDD (3.3V)
                             │
                         ┌───┴───┐
                         │       │ Pull-up
                         │  Rpu  │ Resistor
                         │       │ (~40kΩ)
                         └───┬───┘
                             │
        ┌────────────────────┼────────────────────┐
        │                    │                    │
        │    ┌───────────────┼───────────────┐   │
        │    │               │               │   │
    ┌───┴────┤          ┌────┴────┐          │   │
    │        │          │         │          │   │
    │  Input │          │  I/O    │          │   │  Output
    │ Driver │◀─────────┤  Pad    ├──────────┤   │  Driver
    │        │          │         │          │   │
    └───┬────┘          └────┬────┘          │   │
        │                    │               │   │
        │    └───────────────┼───────────────┘   │
        │                    │                    │
        │                ┌───┴───┐               │
        │                │       │ Pull-down     │
        │                │  Rpd  │ Resistor      │
        │                │       │ (~40kΩ)       │
        │                └───┬───┘               │
        │                    │                    │
        │                   GND                   │
        │                                         │
        └─────────────────────────────────────────┘
                     ↓
              To Input Register
```

#### 2.1.2 Mode GPIO pada STM32F103

```c
// 8 Mode GPIO pada STM32F103
typedef enum {
    GPIO_MODE_INPUT          = 0x00,  // Input floating
    GPIO_MODE_INPUT_PULLUP   = 0x01,  // Input dengan pull-up
    GPIO_MODE_INPUT_PULLDOWN = 0x02,  // Input dengan pull-down
    GPIO_MODE_OUTPUT_PP      = 0x03,  // Output Push-Pull
    GPIO_MODE_OUTPUT_OD      = 0x04,  // Output Open-Drain
    GPIO_MODE_AF_PP          = 0x05,  // Alternate Function Push-Pull
    GPIO_MODE_AF_OD          = 0x06,  // Alternate Function Open-Drain
    GPIO_MODE_ANALOG         = 0x07   // Mode Analog
} GPIO_Mode;
```

#### 2.1.3 Mode GPIO pada ESP32

```c
// Mode GPIO pada ESP32
#define INPUT             0x01  // Input floating
#define OUTPUT            0x02  // Output
#define INPUT_PULLUP      0x05  // Input dengan pull-up internal
#define INPUT_PULLDOWN    0x09  // Input dengan pull-down internal
#define OUTPUT_OPEN_DRAIN 0x12  // Output open-drain
```

### 2.2 Output Push-Pull vs Open-Drain

```
PUSH-PULL OUTPUT:                    OPEN-DRAIN OUTPUT:
                                     
    VDD                                  VDD
     │                                    │
  ┌──┴──┐                              ┌──┴──┐
  │ P-FET│ ◀ ON saat output HIGH       │ Ext │ External
  └──┬──┘                              │Pull-│ Pull-up
     │                                  │ up  │ (Opsional)
     ├───── OUTPUT PIN                 └──┬──┘
     │                                    │
  ┌──┴──┐                                 ├───── OUTPUT PIN
  │ N-FET│ ◀ ON saat output LOW          │
  └──┬──┘                              ┌──┴──┐
     │                                 │ N-FET│ ◀ ON saat LOW
    GND                                └──┬──┘
                                          │
                                         GND

Karakteristik:                     Karakteristik:
- Dapat drive HIGH dan LOW         - Hanya dapat pull LOW
- Sumber arus lebih tinggi         - Butuh external pull-up
- Standar untuk LED drive          - Untuk I2C, level shifting
```

### 2.3 Konsep Debouncing

Ketika tombol ditekan, terjadi **bouncing** - kontak mekanis yang menghasilkan pulsa tidak stabil:

```
TANPA DEBOUNCING:
                 Bouncing period (~5-50ms)
                 ├────────────┤
    HIGH ────────┐   ┌┐  ┌┐  ┌┐
                 │   ││  ││  ││
                 │   ││  ││  ││
    LOW          └───┘└──┘└──┘└────────────────
                 │               │
              Button           Button
              Pressed          Stable
              
DENGAN DEBOUNCING:
    HIGH ────────┐
                 │   
                 │   
    LOW          └─────────────────────────────
                 │             │
              Button        Debounced
              Pressed       Signal
```

#### Teknik Debouncing:

**1. Hardware Debouncing (RC Filter):**
```
            ┌─────┐
  Button    │     │    MCU
    │       │  R  │     │
    ├───────┤     ├─────┼───── GPIO Input
    │       │10kΩ │     │
  ┌─┴─┐     └─────┘   ┌─┴─┐
  │   │               │ C │ 100nF
  │GND│               │   │
  └───┘               └─┬─┘
                        │
                       GND
```

**2. Software Debouncing:**
```c
// Metode 1: Simple Delay
if (digitalRead(BUTTON) == LOW) {
    delay(50);  // Wait for bounce to settle
    if (digitalRead(BUTTON) == LOW) {
        // Button benar-benar ditekan
    }
}

// Metode 2: State Machine (lebih baik)
#define DEBOUNCE_TIME 50
unsigned long lastDebounceTime = 0;
int lastButtonState = HIGH;
int buttonState = HIGH;

void checkButton() {
    int reading = digitalRead(BUTTON);
    
    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }
    
    if ((millis() - lastDebounceTime) > DEBOUNCE_TIME) {
        if (reading != buttonState) {
            buttonState = reading;
            if (buttonState == LOW) {
                // Button pressed action
            }
        }
    }
    lastButtonState = reading;
}
```

### 2.4 Current Sourcing dan Sinking

```
SOURCING (LED ke GND):              SINKING (LED ke VDD):
                                    
    GPIO Pin (HIGH)                     VDD
        │                                │
        │ →→→ Arus mengalir             │
        │                              ┌─┴─┐
      ┌─┴─┐                            │LED│ Anode
      │   │ Resistor                   │ ▼ │
      │ R │ 220Ω                       └─┬─┘
      │   │                              │
      └─┬─┘                            ┌─┴─┐
        │                              │   │ Resistor
      ┌─┴─┐                            │ R │ 220Ω
      │LED│ Anode                      │   │
      │ ▼ │                            └─┬─┘
      └─┬─┘ Cathode                      │ ◀◀◀ Arus mengalir
        │                                │
       GND                          GPIO Pin (LOW)

Catatan STM32F103:
- PC13 hanya dapat SINK ~3mA (gunakan external LED jika perlu lebih)
- Pin lain dapat source/sink hingga 25mA

Catatan ESP32:
- Semua GPIO dapat source/sink hingga 40mA
- Recommended: 20mA untuk lifetime lebih baik
```

### 2.5 Perhitungan Resistor LED

```
Formula: R = (Vcc - Vf) / If

Dimana:
- Vcc = Tegangan supply (3.3V)
- Vf  = Forward voltage LED (merah~2V, biru/putih~3V)
- If  = Forward current yang diinginkan (10-20mA typical)

Contoh untuk LED merah @ 10mA:
R = (3.3V - 2.0V) / 0.010A = 130Ω
→ Gunakan 150Ω atau 220Ω (nilai standar terdekat)

Contoh untuk LED biru @ 10mA:
R = (3.3V - 3.0V) / 0.010A = 30Ω
→ Gunakan 33Ω atau 47Ω
```

---

## 📚 3. GPIO pada STM32F103C8T6

### 3.1 Pinout STM32F103C8T6 (Blue Pill)

```
                    USB
                   ┌───┐
            ┌──────┤   ├──────┐
            │      └───┘      │
      PB12 ─┤1              40├─ VBat
      PB13 ─┤2              39├─ PC13 ◀── LED (Active LOW)
      PB14 ─┤3              38├─ PC14
      PB15 ─┤4              37├─ PC15
       PA8 ─┤5              36├─ PA0  ◀── Button/ADC
       PA9 ─┤6  TX1         35├─ PA1
      PA10 ─┤7  RX1         34├─ PA2
      PA11 ─┤8  USB-        33├─ PA3
      PA12 ─┤9  USB+        32├─ PA4  ◀── SPI1_NSS/DAC
      PA15 ─┤10             31├─ PA5  ◀── SPI1_SCK
       PB3 ─┤11             30├─ PA6  ◀── SPI1_MISO
       PB4 ─┤12             29├─ PA7  ◀── SPI1_MOSI
       PB5 ─┤13             28├─ PB0
       PB6 ─┤14 I2C1_SCL    27├─ PB1
       PB7 ─┤15 I2C1_SDA    26├─ PB10 ◀── I2C2_SCL
       PB8 ─┤16             25├─ PB11 ◀── I2C2_SDA
       PB9 ─┤17             24├─ Reset
      5V   ─┤18             23├─ 3.3V
       GND ─┤19             22├─ GND
       3V3 ─┤20             21├─ GND
            │                  │
            └──────────────────┘
                  ↑    ↑
               SWDIO SWCLK
               (PA13)(PA14)
```

### 3.2 Register GPIO STM32F103

```c
// Alamat Base GPIO
#define GPIOA_BASE  0x40010800
#define GPIOB_BASE  0x40010C00
#define GPIOC_BASE  0x40011000

// Struktur Register GPIO
typedef struct {
    volatile uint32_t CRL;   // Configuration Register Low (pin 0-7)
    volatile uint32_t CRH;   // Configuration Register High (pin 8-15)
    volatile uint32_t IDR;   // Input Data Register
    volatile uint32_t ODR;   // Output Data Register
    volatile uint32_t BSRR;  // Bit Set/Reset Register
    volatile uint32_t BRR;   // Bit Reset Register
    volatile uint32_t LCKR;  // Lock Register
} GPIO_TypeDef;
```

### 3.3 Contoh Akses Register Langsung

```c
// Contoh: Toggle PC13 menggunakan BSRR (lebih efisien dari ODR)
// BSRR bit 0-15: Set, bit 16-31: Reset

// Set PC13 HIGH (LED OFF karena active low)
GPIOC->BSRR = (1 << 13);

// Reset PC13 LOW (LED ON)
GPIOC->BSRR = (1 << (13 + 16));  // atau
GPIOC->BRR = (1 << 13);

// Toggle menggunakan ODR
GPIOC->ODR ^= (1 << 13);
```

---

## 📚 4. GPIO pada ESP32

### 4.1 Pinout ESP32 DevKitC

```
                    ┌─────────────────┐
                    │     USB-C       │
                    │    ┌─────┐      │
              EN ───┤    │     │    ├─── GPIO23 (MOSI)
         GPIO36(VP)─┤    │     │    ├─── GPIO22 (SCL)
         GPIO39(VN)─┤    └─────┘    ├─── GPIO1 (TX0)
          GPIO34 ───┤               ├─── GPIO3 (RX0)
          GPIO35 ───┤               ├─── GPIO21 (SDA)
          GPIO32 ───┤               ├─── GND
          GPIO33 ───┤               ├─── GPIO19 (MISO)
          GPIO25 ───┤     ESP32     ├─── GPIO18 (SCK)
          GPIO26 ───┤    DevKitC    ├─── GPIO5  (SS)
          GPIO27 ───┤               ├─── GPIO17 (TX2)
          GPIO14 ───┤               ├─── GPIO16 (RX2)
          GPIO12 ───┤               ├─── GPIO4
             GND ───┤               ├─── GPIO0 (BOOT)
          GPIO13 ───┤               ├─── GPIO2 ◀── LED Built-in
            3V3 ───┤               ├─── GPIO15
          GPIO15 ───┤               ├─── GND
            3V3 ───┤               ├─── 3V3
                    │               │
                    └───────────────┘

Catatan Penting:
- GPIO34-39: INPUT ONLY (tidak bisa output)
- GPIO6-11: Terhubung ke flash (JANGAN GUNAKAN)
- GPIO0: Boot mode (hati-hati saat desain)
- GPIO2: Built-in LED (bisa digunakan)
```

### 4.2 Fitur Khusus GPIO ESP32

```c
// 1. Touch Sensor (GPIO 0, 2, 4, 12-15, 27, 32, 33)
#include <Arduino.h>
int touchValue = touchRead(GPIO_NUM_4);

// 2. RTC GPIO (dapat aktif saat deep sleep)
// GPIO 0, 2, 4, 12-15, 25-27, 32-39
esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 0);

// 3. Capacitive Touch Wakeup
touchAttachInterrupt(T0, callback, threshold);
esp_sleep_enable_touchpad_wakeup();

// 4. GPIO Matrix - Semua peripheral dapat di-route ke GPIO manapun
// (dengan beberapa pengecualian)
```

### 4.3 Drive Strength ESP32

```c
// ESP32 memiliki 4 level drive strength
typedef enum {
    GPIO_DRIVE_CAP_0 = 0,  // ~5mA
    GPIO_DRIVE_CAP_1 = 1,  // ~10mA
    GPIO_DRIVE_CAP_2 = 2,  // ~20mA (default)
    GPIO_DRIVE_CAP_3 = 3,  // ~40mA
} gpio_drive_cap_t;

// Contoh penggunaan
gpio_set_drive_capability(GPIO_NUM_2, GPIO_DRIVE_CAP_3);
```

---

## 📚 5. Best Practices GPIO

### 5.1 Checklist Keamanan

```
✅ LAKUKAN:
□ Gunakan resistor pembatas arus untuk LED (150-330Ω)
□ Gunakan pull-up/pull-down untuk input floating
□ Implementasikan debouncing untuk mechanical switches
□ Periksa voltage level compatibility sebelum koneksi
□ Gunakan level shifter untuk interfacing 5V devices dengan ESP32

❌ HINDARI:
□ Melebihi maximum current per pin
□ Menghubungkan 5V langsung ke GPIO ESP32
□ Menggunakan GPIO6-11 pada ESP32 (flash SPI)
□ Membiarkan input pin floating tanpa pull resistor
□ Short circuit pada output pin
```

### 5.2 Pola Pemrograman yang Baik

```c
// 1. Definisikan pin di satu tempat (config.h)
#define LED_PIN     GPIO_NUM_2
#define BUTTON_PIN  GPIO_NUM_0

// 2. Gunakan fungsi wrapper
void ledOn() {
    digitalWrite(LED_PIN, HIGH);
}

void ledOff() {
    digitalWrite(LED_PIN, LOW);
}

void ledToggle() {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
}

// 3. Gunakan enum untuk state
typedef enum {
    STATE_IDLE,
    STATE_RUNNING,
    STATE_ERROR
} SystemState;

// 4. Non-blocking code dengan millis()
void loop() {
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate >= 1000) {
        lastUpdate = millis();
        // Aksi periodik
    }
}
```

### 5.3 Debugging GPIO

```c
// 1. Serial debugging
Serial.printf("GPIO%d state: %d\n", pin, digitalRead(pin));

// 2. LED indicator untuk state
void indicateState(SystemState state) {
    switch(state) {
        case STATE_IDLE:    ledOff(); break;
        case STATE_RUNNING: ledBlink(500); break;
        case STATE_ERROR:   ledBlink(100); break;
    }
}

// 3. Logic analyzer friendly - tambahkan test points
#define DEBUG_PIN GPIO_NUM_25
void debugPulse() {
    digitalWrite(DEBUG_PIN, HIGH);
    digitalWrite(DEBUG_PIN, LOW);
}
```

---

## 📚 6. Implementasi Praktikum

### 6.1 Daftar Program Praktikum

| No | Program | STM32 | ESP32 | Deskripsi |
|----|---------|-------|-------|-----------|
| 01 | LED Blink | ✅ | ✅ | Dasar GPIO Output |
| 02 | Multi-LED Running | ✅ | ✅ | Pattern sequencing |
| 03 | LED Breathing | ✅ | ✅ | Software PWM / LEDC |
| 04 | Button Debounce | ✅ | ✅ | State machine debouncing |
| 05 | Long/Short Press | ✅ | ✅ | Press duration detection |
| 06 | Toggle Latch | ✅ | ✅ | Latch behavior |
| 07 | Drive Strength | ✅ | ✅ | Current configuration |
| 08 | DIP Switch | ✅ | ✅ | Multiple input reading |
| 09 | Serial Control | ✅ | ✅ | LED control via Serial |
| 10 | GPIO Matrix | ✅ | ✅ | Bit manipulation |
| 11 | Emergency Stop | ✅ | ✅ | Safety interlock |
| 12 | Test Pattern | ✅ | ✅ | Diagnostic pattern |

### 6.2 Skema Koneksi Standar

```
KONEKSI PRAKTIKUM GPIO:

STM32F103C8T6:                    ESP32 DevKitC:
┌─────────────────┐               ┌─────────────────┐
│                 │               │                 │
│  PC13 ──[LED]── GND (Built-in)  │  GPIO2 ──[LED]── GND (Built-in)
│                 │               │                 │
│  PA0 ──[R]──[LED]── GND         │  GPIO4 ──[R]──[LED]── GND
│  PA1 ──[R]──[LED]── GND         │  GPIO5 ──[R]──[LED]── GND
│  PA2 ──[R]──[LED]── GND         │  GPIO18──[R]──[LED]── GND
│  PA3 ──[R]──[LED]── GND         │  GPIO19──[R]──[LED]── GND
│                 │               │                 │
│  PB0 ──[BTN]── GND              │  GPIO21──[BTN]── GND
│  PB1 ──[BTN]── GND              │  GPIO22──[BTN]── GND
│                 │               │                 │
│  PA9  (TX) ────────             │  GPIO1  (TX) ────────
│  PA10 (RX) ────────             │  GPIO3  (RX) ────────
│                 │               │                 │
└─────────────────┘               └─────────────────┘

R = Resistor 220Ω
BTN = Push Button dengan internal pull-up
```

---

## 📚 7. Troubleshooting

### 7.1 Masalah Umum dan Solusi

| Masalah | Kemungkinan Penyebab | Solusi |
|---------|---------------------|--------|
| LED tidak menyala | Polaritas terbalik, resistor terlalu besar | Periksa anode/cathode, ukur dengan multimeter |
| LED redup | Resistor terlalu besar, drive current rendah | Kurangi nilai resistor (min 100Ω) |
| Button bouncing | Tidak ada debouncing | Implementasi software debounce |
| Input floating | Tidak ada pull-up/down | Aktifkan internal pull-up atau tambah external |
| GPIO tidak responsif | Pin salah, mode salah | Verifikasi pin dan konfigurasi |
| ESP32 boot loop | GPIO0 tertarik LOW | Lepaskan koneksi ke GPIO0 saat boot |

### 7.2 Kode Diagnostik

```c
// Program diagnostik GPIO
void testAllGPIO() {
    Serial.println("GPIO Diagnostic Test");
    
    // Test output
    for (int i = 0; i < NUM_PINS; i++) {
        if (isValidOutputPin(i)) {
            pinMode(pins[i], OUTPUT);
            digitalWrite(pins[i], HIGH);
            delay(100);
            digitalWrite(pins[i], LOW);
            Serial.printf("Pin %d: OUTPUT OK\n", pins[i]);
        }
    }
    
    // Test input
    for (int i = 0; i < NUM_PINS; i++) {
        if (isValidInputPin(i)) {
            pinMode(pins[i], INPUT_PULLUP);
            Serial.printf("Pin %d: %s\n", pins[i], 
                digitalRead(pins[i]) ? "HIGH" : "LOW");
        }
    }
}
```

---

## 📖 Referensi

### Dokumentasi Resmi
1. **STM32F103C8T6 Reference Manual** (RM0008) - STMicroelectronics
2. **STM32F103C8T6 Datasheet** - STMicroelectronics
3. **ESP32 Technical Reference Manual** - Espressif Systems
4. **ESP32 Datasheet** - Espressif Systems

### Buku Referensi
1. Carmine Noviello, "Mastering STM32" 2nd Edition
2. Neil Kolban, "Kolban's Book on ESP32"

### Online Resources
1. [STM32duino Wiki](https://github.com/stm32duino/wiki/wiki)
2. [ESP32 Arduino Core Documentation](https://docs.espressif.com/projects/arduino-esp32/)
3. [Random Nerd Tutorials - ESP32](https://randomnerdtutorials.com/esp32/)

---

## 📝 Latihan Soal

### Soal Teori

1. Jelaskan perbedaan antara mode Push-Pull dan Open-Drain!
2. Mengapa PC13 pada STM32 Blue Pill disebut "active LOW"?
3. Hitunglah resistor yang diperlukan untuk LED hijau (Vf=2.2V) dengan arus 15mA pada sistem 3.3V!
4. Apa yang terjadi jika GPIO input dibiarkan floating? Bagaimana solusinya?
5. Sebutkan 3 metode debouncing dan jelaskan kelebihan masing-masing!

### Soal Praktik

1. Buatlah program LED blink dengan frekuensi yang dapat diubah melalui Serial Monitor!
2. Implementasikan counter yang naik saat button short press dan reset saat long press!
3. Buatlah program traffic light sederhana dengan 3 LED!

---

*Modul ini adalah bagian dari Praktikum Sistem Embedded*
*Versi 1.0 - Februari 2026*

