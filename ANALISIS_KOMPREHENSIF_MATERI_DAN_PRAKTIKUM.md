# ANALISIS KOMPREHENSIF MATERI DAN PRAKTIKUM
## Praktikum Sistem Embedded STM32 & ESP32

**Tanggal Analisis:** 5 Februari 2026  
**Workspace:** /home/sirobo/Documents/Praktikum Sistem Embedded

---

## BAGIAN 1: RINGKASAN TOPIK UTAMA PER BAB

### BAB 01: GPIO DAN DIGITAL I/O
**Status Materi:** Template yang masih kosong (placeholder)

#### Topik Utama:
- Konsep dasar GPIO (General Purpose Input/Output)
- Konfigurasi GPIO sebagai input dan output
- Input debouncing
- Output driving strength
- GPIO matrix (khusus ESP32)
- State machine untuk button handling

#### Program/Praktikum:
- **ESP32:** 12 program
  - ESP32_01_LED_Blink (dasar GPIO output)
  - ESP32_02_Multi-LED_Running_Pattern
  - ESP32_03_LED_Breathing_Effect (PWM)
  - ESP32_04_Button_Debounce (state machine)
  - ESP32_05_Long_Press_vs_Short_Press_Detection
  - ESP32_06_Toggle_LED_dengan_Latch_Behavior
  - ESP32_07_GPIO_Drive_Strength_Configuration
  - ESP32_08_DIP_Switch_Reader
  - ESP32_09_LED_Brightness_Control_via_Serial
  - ESP32_10_GPIO_Matrix
  - ESP32_11_Emergency_Stop_Logic
  - ESP32_12_LED_Test_Pattern

- **STM32:** 12 program (struktur sama dengan ESP32)

---

### BAB 02: INTERRUPT DAN TIMER
**Status Materi:** Template yang masih kosong (placeholder)

#### Topik Utama:
- Konsep dasar interrupt dan timer
- Konfigurasi interrupt external
- Timer interrupt handling
- Jenis-jenis interrupt
- Priority management
- Interrupt nesting

#### Program/Praktikum:
- **ESP32:** 12 program
- **STM32:** 12 program

---

### BAB 03: SERIAL UART COMMUNICATION
**Status Materi:** Template yang masih kosong (placeholder)

#### Topik Utama:
- Protokol UART/Serial Communication
- Konfigurasi baud rate
- Transmit dan Receive
- Interrupt-driven UART
- Ring buffer implementation
- Error handling

#### Program/Praktikum:
- **ESP32:** 12 program
- **STM32:** 12 program

---

### BAB 04: ANALOG-TO-DIGITAL CONVERTER (ADC)
**Status Materi:** Template yang masih kosong (placeholder)

#### Topik Utama:
- Konsep ADC dan sampling
- Resolusi dan akurasi
- Single-channel dan multi-channel conversion
- ADC interrupt handling
- DMA dengan ADC
- Calibration dan offset

#### Program/Praktikum:
- **ESP32:** 17 program (lebih comprehensive)
- **STM32:** 7 program (gap signifikan)

**⚠️ GAP:** STM32 hanya memiliki 7 program sementara ESP32 memiliki 17. Ini menunjukkan ketidakseimbangan implementasi.

---

### BAB 05: DAC DAN PWM OUTPUT
**Status Materi:** Template yang masih kosong (placeholder)

#### Topik Utama:
- Digital-to-Analog Converter (DAC)
- Pulse Width Modulation (PWM)
- PWM frequency dan duty cycle
- Multi-channel PWM
- LED brightness control
- Motor control basics

#### Program/Praktikum:
- **ESP32:** 9 program (hanya 9, berkurang dari bab sebelumnya)
- **STM32:** 5 program (paling sedikit)

**⚠️ GAP:** Jumlah program berkurang signifikan. Hanya 14 program total untuk topik PWM/DAC yang penting.

---

### BAB 06: I2C BUS DAN SENSOR
**Status Materi:** Template yang masih kosong (placeholder)

#### Topik Utama:
- Protokol I2C (Inter-Integrated Circuit)
- Master-slave communication
- Address dan data frame format
- Clock stretching
- Multi-master I2C
- Common I2C sensors (IMU, temperature, humidity)

#### Program/Praktikum:
- **ESP32:** 12 program
- **STM32:** 0 program ⚠️ **CRITICAL GAP!**

**🔴 CRITICAL GAP:** STM32 TIDAK MEMILIKI IMPLEMENTASI I2C SAMA SEKALI!  
Ini adalah topik sangat penting untuk embedded systems.

---

### BAB 07: SPI BUS DAN STORAGE
**Status Materi:** Template yang masih kosong (placeholder)

#### Topik Utama:
- Protokol SPI (Serial Peripheral Interface)
- Clock polarity dan phase (CPOL, CPHA)
- Master-slave addressing
- Multi-slave SPI
- Flash memory management
- SD card interfacing

#### Program/Praktikum:
- **ESP32:** 12 program
- **STM32:** 0 program ⚠️ **CRITICAL GAP!**

**🔴 CRITICAL GAP:** STM32 TIDAK MEMILIKI IMPLEMENTASI SPI SAMA SEKALI!  
Padahal SPI sangat penting untuk storage dan sensor external.

---

### BAB 08: DIRECT MEMORY ACCESS (DMA)
**Status Materi:** Template yang masih kosong (placeholder)

#### Topik Utama:
- Konsep DMA dan transfer modes
- Channel configuration
- DMA interrupt
- Memory-to-memory, peripheral-to-memory, memory-to-peripheral
- Circular buffer dengan DMA
- Performance optimization

#### Program/Praktikum:
- **ESP32:** 7 program
- **STM32:** 11 program (lebih banyak dari ESP32)

**Note:** STM32 lebih optimal untuk DMA karena memiliki dedicated DMA controller.

---

### BAB 09: FreeRTOS TASK MANAGEMENT
**Status Materi:** Template yang masih kosong (placeholder)

#### Topik Utama:
- Task creation dan deletion
- Task scheduling dan context switching
- Task states (ready, running, blocked, suspended)
- Stack management
- Task priority
- vTaskDelay dan task synchronization

#### Program/Praktikum:
- **ESP32:** 6 program
- **STM32:** 13 program (lebih comprehensive)

---

### BAB 10: FreeRTOS QUEUE DAN SEMAPHORE
**Status Materi:** Template yang masih kosong (placeholder)

#### Topik Utama:
- Queue creation dan management
- Inter-task communication (ITC)
- Binary semaphore, counting semaphore, mutex
- Deadlock prevention
- Interrupt service routine (ISR) interaction
- Priority inheritance

#### Program/Praktikum:
- **ESP32:** 11 program
- **STM32:** 20 program (lebih comprehensive)

---

### BAB 11: FreeRTOS TIMER DAN NOTIFICATION
**Status Materi:** Template yang masih kosong (placeholder)

#### Topik Utama:
- Software timer implementation
- Timer callback functions
- One-shot vs auto-reload timers
- Task notification
- Direct-to-task notification
- Binary semaphore alternative

#### Program/Praktikum:
- **ESP32:** 14 program
- **STM32:** 27 program (paling banyak!)

**Note:** Modul ini memiliki implementasi paling lengkap untuk STM32.

---

### BAB 12: FreeRTOS MEMORY DAN ADVANCED
**Status Materi:** Template yang masih kosong (placeholder)

#### Topik Utama:
- Memory management schemes
- Heap fragmentation
- Heap allocation strategies
- Event groups
- Stream buffer
- Co-routines (legacy feature)
- Security dan encryption basics

#### Program/Praktikum:
- **ESP32:** 14 program
- **STM32:** 23 program

---

### BAB 13: NETWORK CONNECTIVITY
**Status Materi:** Template yang masih kosong (placeholder)

#### Topik Utama:
- Network basics (TCP/IP)
- WiFi connectivity (khusus ESP32)
- Ethernet (dapat digunakan STM32 dengan modul external)
- Socket programming
- Server dan client implementation
- HTTP/MQTT protocol basics

#### Program/Praktikum:
- **ESP32:** 14 program
- **STM32:** 14 program (memungkinkan dengan ethernet shield)

---

## BAGIAN 2: RINGKASAN PROGRAM PRAKTIKUM

### Statistik Total Program:

| Modul | Topik | ESP32 | STM32 | Total | Status |
|-----|-------|-------|-------|-------|--------|
| 01 | GPIO & Digital I/O | 12 | 12 | 24 | ✅ Balanced |
| 02 | Interrupt & Timer | 12 | 12 | 24 | ✅ Balanced |
| 03 | Serial UART | 12 | 12 | 24 | ✅ Balanced |
| 04 | ADC | 17 | 7 | 24 | ⚠️ Imbalanced |
| 05 | DAC/PWM | 9 | 5 | 14 | ⚠️ Kurang |
| 06 | I2C Sensor | 12 | 0 | 12 | 🔴 **STM32 Kosong** |
| 07 | SPI Storage | 12 | 0 | 12 | 🔴 **STM32 Kosong** |
| 08 | DMA | 7 | 11 | 18 | ✅ STM32 Lebih |
| 09 | FreeRTOS Task | 6 | 13 | 19 | ✅ STM32 Lebih |
| 10 | FreeRTOS Queue/Semaphore | 11 | 20 | 31 | ✅ STM32 Lebih |
| 11 | FreeRTOS Timer/Notification | 14 | 27 | 41 | ✅ STM32 Lebih |
| 12 | FreeRTOS Memory/Advanced | 14 | 23 | 37 | ✅ STM32 Lebih |
| 13 | Network | 14 | 14 | 28 | ✅ Balanced |
| **14** | **Industrial** | **12** | **12** | **24** | ✅ Balanced |
| **TOTAL** | | **~166** | **~168** | **~334** | - |

---

## BAGIAN 3: ANALISIS vs "Mastering STM32-2nd.pdf"

### File Referensi yang Ditemukan:
```
Referensi/STM32/
├── Mastering STM32-2nd.pdf ⭐ (File utama referensi)
├── Mastering STM32.pdf
├── Mastering-the-FreeRTOS-Real-Time-Kernel.v1.1.0.pdf
├── 161204_Mastering_the_FreeRTOS_Real_Time_Kernel-A_Hands-On_Tutorial_Guide.pdf
├── Mastering_the_FreeRTOS_Real_Time_Kernel_A_Hands_On_Tutorial_Guide.pdf
├── STM32_Lec2.pdf
└── ESP32/
    ├── ESP32 Technical Reference Manual
    ├── ESP32-C3 Datasheet
    └── [Various ESP32 documentation]
```

### Pemetaan dengan "Mastering STM32-2nd.pdf":

**Catatan:** Berdasarkan struktur program yang tersedia, kemungkinan besar konten "Mastering STM32-2nd.pdf" mencakup:

1. **Modul 1-3: Basic GPIO, Interrupt, UART** ✅ TERIMPLEMENTASI
   - Sesuai dengan Modul 01-03 praktikum

2. **Modul 4-5: ADC & DAC/PWM** ⚠️ PARTIAL
   - ADC sudah ada 7 program STM32
   - DAC/PWM hanya 5 program (sangat minimal)

3. **Modul 6-7: Communication Protocols (I2C, SPI)** 🔴 TIDAK TERIMPLEMENTASI
   - **I2C: 0 program STM32** - CRITICAL GAP
   - **SPI: 0 program STM32** - CRITICAL GAP
   - Kedua topik ini sangat fundamental dalam "Mastering STM32-2nd"

4. **Modul 8: DMA** ✅ WELL-IMPLEMENTED
   - 11 program STM32 (lebih banyak dari ESP32)
   - Sesuai ekspektasi karena STM32 memiliki DMA dedicated

5. **Modul 9-13: FreeRTOS & Advanced Topics** ✅ COMPREHENSIVE
   - Modul FreeRTOS (09-12) sangat lengkap
   - Network (13) terjangkau untuk keduanya

---

## BAGIAN 4: IDENTIFIKASI GAP - MATERI YANG BELUM TERIMPLEMENTASIKAN

### 🔴 CRITICAL GAPS (Sangat Penting, Belum Ada):

#### 1. **I2C Communication untuk STM32 (Modul 06)**
- **Status:** 0 dari 12 program
- **Materi yang Diperlukan:**
  - I2C Master initialization dan configuration
  - I2C address dan data frame generation
  - I2C clock stretching handling
  - Common sensors dengan I2C:
    - BMP280/BMP390 (barometric pressure)
    - BMI160 (IMU - accelerometer/gyroscope)
    - MPU6050 (6-axis IMU)
    - TCS3200 (color sensor)
    - LM75 (temperature sensor)
    - ADS1115 (16-bit ADC)
    - RTC modules (DS1307, DS3231)
  - Multi-slave I2C scanning
  - Error detection dan recovery

#### 2. **SPI Communication untuk STM32 (Modul 07)**
- **Status:** 0 dari 12 program
- **Materi yang Diperlukan:**
  - SPI mode configuration (CPOL, CPHA)
  - Master-slave communication
  - DMA dengan SPI
  - Multi-slave SPI dengan chip select management
  - SD card/eMMC interface
  - Flash memory programming
  - Common sensors dengan SPI:
    - MAX7219 (LED driver)
    - MCP23S17 (I/O expander)
    - SD card reader
    - NRF24L01 (wireless transceiver)
  - SPI DMA transfer optimization

#### 3. **DAC/PWM untuk STM32 (Modul 05)**
- **Status:** Hanya 5 program (sangat minimal)
- **Program Tambahan yang Diperlukan:**
  - Multi-channel PWM output
  - PWM frequency modulation
  - Motor control (DC motor speed, servo control)
  - DAC waveform generation
  - PWM triggering ADC
  - Complementary PWM output (untuk power electronics)
  - PWM duty cycle modulation techniques

---

### ⚠️ MODERATE GAPS (Penting, Implementasi Tidak Seimbang):

#### 4. **ADC untuk STM32 (Modul 04)**
- **Status:** 7 program (vs ESP32: 17)
- **Program Tambahan yang Diperlukan:**
  - ADC multi-channel sequential conversion
  - ADC continuous mode
  - ADC DMA integration (transfer multiple samples)
  - ADC interrupt dengan callback
  - Temperature sensor internal
  - Oversampling untuk akurasi
  - ADC calibration techniques
  - Real-time signal processing dari ADC

#### 5. **FreeRTOS Task untuk ESP32 (Modul 09)**
- **Status:** 6 program (vs STM32: 13)
- **Program Tambahan yang Diperlukan:**
  - Task creation dengan dynamic priority
  - Core affinity untuk dual-core ESP32
  - Task suspension dan resumption
  - Task cleanup dan deletion
  - Stack size optimization
  - Real-time task scheduling examples
  - Multi-core task synchronization

#### 6. **FreeRTOS Queue/Semaphore untuk ESP32 (Modul 10)**
- **Status:** 11 program (vs STM32: 20)
- **Program Tambahan yang Diperlukan:**
  - Mutex implementation dan deadlock prevention
  - Recursive mutex usage
  - Binary vs counting semaphore comparison
  - ISR-safe queue operations
  - Multiple producer-consumer patterns
  - Priority inversion solutions
  - Queue overflow handling

---

### 📝 DOCUMENTATION GAPS (Materi Kosong/Template):

Semua file `Materi.md` di setiap bab masih menggunakan **template placeholder**:
```markdown
## 📚 Materi Pembelajaran

### Pendahuluan
(Isi materi pembelajaran di sini)

### Teori Dasar
(Isi teori dasar di sini)

### Implementasi
(Isi implementasi di sini)
```

**Diperlukan:**
- Penulisan lengkap teori untuk setiap bab
- Diagram dan flowchart yang menjelaskan konsep
- Referensi ke datasheet STM32F103C8T6 dan ESP32
- Penjelasan praktis cara kerja setiap program
- Best practices dan common pitfalls

---

## BAGIAN 5: REKOMENDASI IMPLEMENTASI

### Prioritas Tinggi (HARUS DIKERJAKAN):

1. **✅ I2C untuk STM32 (Modul 06)**
   - Minimal: 8-10 program untuk cover common sensors
   - Estimasi: 2-3 minggu kerja
   
2. **✅ SPI untuk STM32 (Modul 07)**
   - Minimal: 8-10 program untuk cover common devices
   - Estimasi: 2-3 minggu kerja

3. **✅ Lengkapi DAC/PWM untuk STM32 (Modul 05)**
   - Tambah minimal: 5-8 program lagi
   - Estimasi: 1-2 minggu kerja

### Prioritas Sedang (LEBIH BAIK DIKERJAKAN):

4. **⚠️ Perluas ADC untuk STM32 (Modul 04)**
   - Tambah minimal: 8-10 program
   - Estimasi: 1-2 minggu kerja

5. **⚠️ Perluas FreeRTOS untuk ESP32 (Modul 09-10)**
   - Tambah minimal: 5-7 program
   - Estimasi: 1-2 minggu kerja

### Prioritas Rendah (NICE TO HAVE):

6. **📝 Lengkapi Dokumentasi Materi**
   - Setiap bab membutuhkan 5-15 halaman konten
   - Estimasi: 4-6 minggu kerja

---

## BAGIAN 6: MATRIX KELENGKAPAN IMPLEMENTASI

### Legend:
- ✅ = Selesai dan balanced
- ⚠️ = Partial/Imbalanced
- 🔴 = Critical gap (belum ada sama sekali)
- 📝 = Dokumentasi placeholder

| Aspek | Status | Catatan |
|-------|--------|---------|
| **GPIO/Digital IO** | ✅ | 24 program, seimbang |
| **Interrupt/Timer** | ✅ | 24 program, seimbang |
| **UART Serial** | ✅ | 24 program, seimbang |
| **ADC** | ⚠️ | Imbalanced (17 vs 7) |
| **DAC/PWM** | ⚠️ | Kurang lengkap (14 total, target 20) |
| **I2C** | 🔴 | STM32 0/12 - CRITICAL |
| **SPI** | 🔴 | STM32 0/12 - CRITICAL |
| **DMA** | ✅ | STM32 11, ESP32 7 |
| **FreeRTOS Task** | ⚠️ | ESP32 kurang (6 vs 13) |
| **FreeRTOS Queue/Semaphore** | ⚠️ | ESP32 kurang (11 vs 20) |
| **FreeRTOS Timer/Notification** | ✅ | STM32 27 (comprehensive) |
| **FreeRTOS Memory/Advanced** | ✅ | STM32 23, ESP32 14 |
| **Network** | ✅ | 28 program, seimbang |
| **Dokumentasi Teori** | 📝 | Semua masih template |

---

## BAGIAN 7: KESIMPULAN DAN REKOMENDASI AKSI

### Ringkasan Status:
- **Total Program:** ~334 program (166 ESP32, 168 STM32)
- **Program Seimbang:** ~60% (8 bab)
- **Program Imbalanced:** ~23% (3 bab)
- **Program Critical Gap:** ~15% (2 bab)
- **Dokumentasi:** 0% lengkap (semua template)

### Rekomendasi Keputusan:

1. **Segera Implementasikan:**
   - [ ] I2C untuk STM32 (Modul 06)
   - [ ] SPI untuk STM32 (Modul 07)
   - [ ] Tambahan DAC/PWM untuk STM32 (Modul 05)

2. **Dalam 1 Bulan:**
   - [ ] Lengkapi ADC untuk STM32
   - [ ] Tambah FreeRTOS untuk ESP32
   - [ ] Mulai dokumentasi teori fundamental

3. **Jangka Panjang:**
   - [ ] Lengkapi semua dokumentasi materi
   - [ ] Tambah diagram dan flowchart
   - [ ] Video tutorial untuk setiap program
   - [ ] Troubleshooting guide

### Estimasi Total Kerja:
- **Implementasi Program Baru:** 6-8 minggu
- **Dokumentasi Materi:** 4-6 minggu
- **Testing & Refinement:** 2-3 minggu
- **Total:** ~3-4 bulan untuk mencapai 90% kelengkapan

---

## LAMPIRAN: DETAIL PROGRAM PER BAB

### Modul 01 - GPIO Digital I/O (24 Program)

**ESP32 Programs:**
1. ESP32_01_LED_Blink_-_Dasar_GPIO_Output
2. ESP32_02_Multi-LED_Running_Pattern
3. ESP32_03_LED_Breathing_Effect_menggunakan_LEDC_PWM
4. ESP32_04_Button_Debounce_menggunakan_State_Machine
5. ESP32_05_Long_Press_vs_Short_Press_Detection
6. ESP32_06_Toggle_LED_dengan_Latch_Behavior
7. ESP32_07_GPIO_Drive_Strength_Configuration
8. ESP32_08_DIP_Switch_Reader
9. ESP32_09_LED_Brightness_Control_via_Serial
10. ESP32_10_GPIO_Matrix
11. ESP32_11_Emergency_Stop_Logic
12. ESP32_12_LED_Test_Pattern

**STM32 Programs:** (sama dengan ESP32, 12 program)

### Modul 04 - ADC (24 Program Total, 17 ESP32 + 7 STM32)

**Ketidakseimbangan:** ESP32 memiliki 17 program (lebih comprehensive untuk sensor) sementara STM32 hanya 7 program.

### Modul 06 - I2C Sensor (12 Program, HANYA ESP32)

**🔴 CRITICAL:** STM32 tidak memiliki implementasi I2C sama sekali!

### Modul 07 - SPI Storage (12 Program, HANYA ESP32)

**🔴 CRITICAL:** STM32 tidak memiliki implementasi SPI sama sekali!

---

*Generated: 5 February 2026*
*Workspace: /home/sirobo/Documents/Praktikum Sistem Embedded*
