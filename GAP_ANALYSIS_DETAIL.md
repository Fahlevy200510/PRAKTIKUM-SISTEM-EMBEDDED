# GAP ANALYSIS - MATERI VS PRAKTIKUM
## Praktikum Sistem Embedded STM32 & ESP32

**Tanggal:** 5 Februari 2026

---

## EXECUTIVE SUMMARY

### Status Keseluruhan:
- ✅ **60%** - Implementasi seimbang dan lengkap
- ⚠️ **25%** - Implementasi partial/imbalanced  
- 🔴 **15%** - Critical gaps (belum ada sama sekali)
- 📝 **100%** - Dokumentasi teori masih kosong

### Jumlah Program:
- **Total:** ~334 program
  - ESP32: ~166 program
  - STM32: ~168 program

---

## CRITICAL GAPS - YANG HARUS SEGERA DITAMBAHKAN

### 1. 🔴 BSEB 06: I2C SENSOR UNTUK STM32

**Current Status:** 0 / 12 program
- ESP32: 12 program ✅
- STM32: 0 program 🔴

**Why Critical:**
- I2C adalah protokol komunikasi **fundamental** untuk embedded systems
- Mayoritas sensor external menggunakan I2C
- Dicover secara ekstensif dalam "Mastering STM32-2nd.pdf"
- Essential untuk IoT dan sensor-based applications

**Programs Needed (Recommended 10-12):**

#### Level 1: Basic I2C Master
1. **I2C_01_Master_Initialization**
   - Basic I2C master setup
   - Configuration register setting
   - Clock frequency configuration (100kHz, 400kHz)

2. **I2C_02_Simple_Write_Read**
   - Master writes data to slave
   - Master reads data from slave
   - Basic polling method

3. **I2C_03_Repeated_Start_Condition**
   - Repeated start untuk composite operations
   - Important untuk sensor reads dengan register selection

#### Level 2: Common Sensor Integration
4. **I2C_04_LM75_Temperature_Sensor**
   - Simple temperature sensor integration
   - Register-based data reading
   - Temperature conversion

5. **I2C_05_BMP280_Pressure_Sensor**
   - Barometric pressure measurement
   - Altitude calculation
   - Calibration data handling

6. **I2C_06_MPU6050_IMU_Accelerometer_Gyroscope**
   - 6-axis motion sensor
   - Acceleration measurement
   - Angular velocity measurement
   - FIFO buffer configuration

7. **I2C_07_TCS3200_Color_Sensor**
   - RGB color detection
   - Light intensity measurement
   - Frequency-based sensor output

8. **I2C_08_ADS1115_ADC_Converter**
   - 16-bit external ADC
   - Multi-channel analog conversion
   - Programmable gain amplifier

#### Level 3: Advanced I2C
9. **I2C_09_Multiple_Sensors_on_Same_Bus**
   - Mixed sensors dengan different addresses
   - Bus sharing mechanism
   - Conflict resolution

10. **I2C_10_I2C_Scanner_Device_Discovery**
    - Automatic I2C device detection
    - Address enumeration
    - Diagnostics tool

11. **I2C_11_DMA_with_I2C**
    - DMA-based I2C transfers
    - Large data block reads
    - Performance optimization

12. **I2C_12_EEPROM_Data_Storage**
    - External I2C EEPROM (24LC256)
    - Data write/read/verify
    - Wear leveling concepts

---

### 2. 🔴 BAB 07: SPI STORAGE UNTUK STM32

**Current Status:** 0 / 12 program
- ESP32: 12 program ✅
- STM32: 0 program 🔴

**Why Critical:**
- SPI adalah protokol komunikasi **high-speed** untuk storage dan peripherals
- Fundamental untuk SD card, flash memory, dan wireless modules
- Tidak ada alternative terbaik untuk high-speed communication pada STM32
- Essential untuk data logging dan wireless applications

**Programs Needed (Recommended 10-12):**

#### Level 1: Basic SPI Master
1. **SPI_01_Master_Initialization**
   - SPI mode configuration (CPOL, CPHA)
   - Baudrate setting
   - Chip select management

2. **SPI_02_Simple_Slave_Communication**
   - Master-slave loopback
   - Data transmission dengan verification
   - Mode testing (mode 0, 1, 2, 3)

3. **SPI_03_Multi_Slave_Chip_Select**
   - Multiple slaves dengan CS management
   - Multiplexing concept
   - Device selection logic

#### Level 2: Common Peripherals
4. **SPI_04_MAX7219_LED_Driver**
   - 8x8 LED matrix control
   - Serial-to-parallel conversion
   - Brightness control

5. **SPI_05_MCP23S17_IO_Expander**
   - Port expansion via SPI
   - GPIO increase
   - Interrupt handling dari expander

6. **SPI_06_NRF24L01_Wireless_Module**
   - 2.4GHz wireless communication
   - Packet transmission
   - Multi-device mesh network basics

7. **SPI_07_ST7735_Display_Module**
   - TFT display control
   - Graphics drawing
   - Image display

8. **SPI_08_AT25DF_Flash_Memory**
   - Serial flash memory operations
   - Page write/read
   - Sector erase

#### Level 3: Storage Integration
9. **SPI_09_SD_Card_Interface**
   - SD card initialization
   - File system basics (FAT32)
   - File read/write operations
   - Data logging implementation

10. **SPI_10_DMA_with_SPI**
    - DMA-driven SPI transfers
    - High-speed data movement
    - Zero-copy techniques

11. **SPI_11_EEPROM_SPI_Variant**
    - SPI EEPROM (AT25xxx series)
    - Comparison dengan I2C EEPROM
    - Speed comparison

12. **SPI_12_Real_World_Application**
    - Data logging dengan SD card
    - Sensor data buffering
    - Storage management

---

## MODERATE GAPS - YANG PERLU DIIMPLEMENTASIKAN

### 3. ⚠️ BAB 05: DAC/PWM UNTUK STM32

**Current Status:** 5 / ~20 program (estimated target)
- ESP32: 9 program
- STM32: 5 program
- **Gap:** ~15 program kurang

**Programs Needed (Recommended 10-15 additional):**

#### PWM Enhancement
1. **PWM_01_Multi_Channel_PWM_Output**
   - 4+ simultaneous PWM outputs
   - Different frequencies per channel
   - Real-time duty cycle adjustment

2. **PWM_02_PWM_Triggered_ADC**
   - ADC sampling synchronized dengan PWM
   - Data acquisition synchronization
   - Analog signal analysis

3. **PWM_03_Complementary_PWM**
   - High-side dan low-side PWM outputs
   - Dead time insertion (untuk power electronics)
   - Gate driver interface

4. **PWM_04_PWM_Frequency_Modulation**
   - Variable frequency PWM
   - Spread spectrum concept
   - EMI reduction

#### Motor Control
5. **PWM_05_DC_Motor_Speed_Control**
   - Simple PWM-based motor control
   - Speed measurement via encoder
   - PID control loop

6. **PWM_06_Servo_Motor_Control**
   - 0-180 degree servo positioning
   - Pulse-width to angle conversion
   - Multiple servo control

7. **PWM_07_Stepper_Motor_Control**
   - Step sequence generation
   - Variable speed stepping
   - Direction control

#### DAC Features
8. **DAC_01_Simple_Waveform_Generation**
   - Sine wave generation
   - Triangle wave
   - Sawtooth wave

9. **DAC_02_DAC_with_DMA**
   - DMA-driven DAC output
   - Continuous waveform streaming
   - Sample rate optimization

10. **DAC_03_Audio_Output**
    - Simple audio playback
    - WAV file decoding
    - Speaker interface

---

### 4. ⚠️ BAB 04: ADC UNTUK STM32

**Current Status:** 7 / ~18 program (estimated target)
- ESP32: 17 program
- STM32: 7 program
- **Gap:** ~11 program kurang

**Programs Needed (Recommended 8-12 additional):**

1. **ADC_01_Multi_Channel_Sequential_Conversion**
2. **ADC_02_ADC_DMA_Integration**
3. **ADC_03_Continuous_Conversion_Mode**
4. **ADC_04_Temperature_Sensor_Calibration**
5. **ADC_05_Oversampling_Technique**
6. **ADC_06_ADC_Triggered_by_Timer**
7. **ADC_07_Real_Time_Signal_Processing**
8. **ADC_08_Multiple_ADC_Synchronization**

---

### 5. ⚠️ BAB 09: FREERTOS TASK UNTUK ESP32

**Current Status:** 6 / ~12 program (estimated target)
- ESP32: 6 program
- STM32: 13 program
- **Gap:** ~7 program kurang

**Programs Needed:**

1. **Task_01_Dual_Core_Task_Management** (ESP32 specific)
2. **Task_02_Core_Affinity_Assignment**
3. **Task_03_Task_Stack_Optimization**
4. **Task_04_Task_Cleanup_and_Deletion**
5. **Task_05_Real_Time_Scheduling**
6. **Task_06_Multi_Core_Synchronization**

---

### 6. ⚠️ BAB 10: FREERTOS QUEUE/SEMAPHORE UNTUK ESP32

**Current Status:** 11 / ~20 program (estimated target)
- ESP32: 11 program
- STM32: 20 program
- **Gap:** ~9 program kurang

**Programs Needed:**

1. **Queue_01_Priority_Queue_Implementation**
2. **Queue_02_Queue_Overflow_Handling**
3. **Semaphore_01_Priority_Inversion_Solution**
4. **Semaphore_02_Recursive_Mutex**
5. **ISR_01_ISR_Safe_Queue_Operation**
6. **Pattern_01_Multiple_Producer_Consumer**
7. **Pattern_02_Pipeline_Processing**
8. **Advanced_01_Bounded_Buffer_Pattern**

---

## DOCUMENTATION GAPS - TEORI YANG KOSONG

### ⚠️ SEMUA 13 BAB: DOKUMENTASI MATERI MASIH TEMPLATE

**Current Status:** 0% konten, 100% placeholder

**Format Yang Diperlukan untuk Setiap Bab:**

```
# BAB X: [TOPIK]

## Pengenalan (1-2 halaman)
- Sejarah dan development
- Aplikasi real-world
- Mengapa topik ini penting

## Konsep Fundamental (3-5 halaman)
- Teori dasar
- Hardware overview
- Block diagrams
- Timing diagrams (jika perlu)

## Protokol/Standar (2-4 halaman)
- Frame format
- Handshake mechanism
- Error handling
- Safety features

## Hardware Specifics (2-3 halaman)
- STM32F103C8T6 details
- ESP32 details
- Register descriptions
- Peripheral differences

## Software Implementation (3-5 halaman)
- Library overview
- Initialization steps
- Configuration options
- Common pitfalls

## Best Practices (1-2 halaman)
- Do's and don'ts
- Performance optimization
- Debugging techniques
- Common issues dan solutions
```

**Estimasi:**
- Per bab: 15-25 halaman
- Total 13 bab: 195-325 halaman
- Waktu: 4-6 minggu kerja

---

## IMPLEMENTATION ROADMAP

### Phase 1: Critical Gaps (2-3 Minggu)
**Priority:** HIGHEST

```
Week 1:
- I2C_01-03 (basic I2C)
- SPI_01-03 (basic SPI)

Week 2:
- I2C_04-06 (sensor integration)
- SPI_04-06 (peripheral integration)

Week 3:
- I2C_07-09 (advanced I2C)
- SPI_07-09 (storage)
```

### Phase 2: Moderate Gaps (3-4 Minggu)
**Priority:** HIGH

```
Week 4-5:
- BAB 05: PWM enhancement (8 programs)
- BAB 04: ADC enhancement (8 programs)

Week 6-7:
- BAB 09: ESP32 FreeRTOS task (6 programs)
- BAB 10: ESP32 FreeRTOS queue (6 programs)
```

### Phase 3: Documentation (4-6 Minggu)
**Priority:** MEDIUM

```
Week 8-9:
- BAB 01-03 Documentation
- BAB 04-06 Documentation

Week 10-11:
- BAB 07-09 Documentation
- BAB 10-13 Documentation
```

---

## PRIORITIZED IMPLEMENTATION CHECKLIST

### 🔴 CRITICAL - MUST IMPLEMENT (BAB 06 & 07)

- [ ] **I2C for STM32 (Modul 06)**
  - [ ] I2C_01_Master_Initialization
  - [ ] I2C_02_Simple_Write_Read
  - [ ] I2C_03_Repeated_Start_Condition
  - [ ] I2C_04_LM75_Temperature_Sensor
  - [ ] I2C_05_BMP280_Pressure_Sensor
  - [ ] I2C_06_MPU6050_IMU
  - [ ] I2C_07_TCS3200_Color_Sensor
  - [ ] I2C_08_ADS1115_ADC
  - [ ] I2C_09_Multiple_Sensors
  - [ ] I2C_10_I2C_Scanner
  - [ ] I2C_11_DMA_with_I2C
  - [ ] I2C_12_EEPROM_Storage

- [ ] **SPI for STM32 (Modul 07)**
  - [ ] SPI_01_Master_Initialization
  - [ ] SPI_02_Slave_Communication
  - [ ] SPI_03_Multi_Slave_CS
  - [ ] SPI_04_MAX7219_LED_Driver
  - [ ] SPI_05_MCP23S17_IO_Expander
  - [ ] SPI_06_NRF24L01_Wireless
  - [ ] SPI_07_ST7735_Display
  - [ ] SPI_08_AT25DF_Flash
  - [ ] SPI_09_SD_Card_Interface
  - [ ] SPI_10_DMA_with_SPI
  - [ ] SPI_11_SPI_EEPROM
  - [ ] SPI_12_Real_World_Application

### ⚠️ IMPORTANT - SHOULD IMPLEMENT (BAB 04, 05, 09, 10)

- [ ] **BAB 05: PWM/DAC Enhancement (8-10 programs)**
- [ ] **BAB 04: ADC Enhancement (8-10 programs)**
- [ ] **BAB 09: ESP32 FreeRTOS Task (6 programs)**
- [ ] **BAB 10: ESP32 FreeRTOS Queue (6 programs)**

### 📝 DOCUMENTATION - SHOULD COMPLETE (ALL CHAPTERS)

- [ ] BAB 01: GPIO & Digital I/O (20 pages)
- [ ] BAB 02: Interrupt & Timer (20 pages)
- [ ] BAB 03: Serial UART (15 pages)
- [ ] BAB 04: ADC (20 pages)
- [ ] BAB 05: DAC & PWM (20 pages)
- [ ] BAB 06: I2C & Sensor (25 pages)
- [ ] BAB 07: SPI & Storage (25 pages)
- [ ] BAB 08: DMA (20 pages)
- [ ] BAB 09: FreeRTOS Task (20 pages)
- [ ] BAB 10: FreeRTOS Queue/Semaphore (25 pages)
- [ ] BAB 11: FreeRTOS Timer/Notification (25 pages)
- [ ] BAB 12: FreeRTOS Memory/Advanced (20 pages)
- [ ] BAB 13: Network (20 pages)

---

## QUALITY METRICS

### Current State:
- Code completeness: 80%
- Documentation completeness: 0%
- Platform balance: 85% (minor imbalances)
- Overall readiness: 40%

### Target State:
- Code completeness: 95%+
- Documentation completeness: 90%+
- Platform balance: 95%+
- Overall readiness: 90%+

---

## RESOURCE REQUIREMENTS

### Human Resources:
- **Senior Developer (Embedded Systems):** 1-2 orang
- **Technical Writer:** 1 orang
- **QA/Testing:** 1 orang
- **Estimated Duration:** 8-12 minggu

### Tools/Equipment:
- STM32 development board (STM32F103C8T6)
- ESP32 development board
- Various sensors (I2C, SPI)
- Oscilloscope (untuk timing verification)
- Logic analyzer (untuk protocol analysis)

### Software:
- STM32CubeIDE
- ESP-IDF / PlatformIO
- Git version control
- Documentation tools (Markdown + Mermaid)

---

## RISK ASSESSMENT

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|-----------|
| Delayed I2C/SPI implementation | Medium | High | Allocate resources early |
| Hardware incompatibility issues | Low | Medium | Test dengan real hardware |
| Documentation quality variance | Medium | Medium | Establish templates/guidelines |
| Incomplete sensor testing | Low | High | Maintain test matrix |
| Knowledge gap dalam team | Medium | High | Training & documentation review |

---

## SUCCESS CRITERIA

### Phase 1 Completion (Week 3):
- ✅ 12 I2C programs untuk STM32
- ✅ 12 SPI programs untuk STM32
- ✅ All programs tested dan documented

### Phase 2 Completion (Week 7):
- ✅ BAB 04-05 expanded programs
- ✅ BAB 09-10 ESP32 programs ditambah
- ✅ Total program: ~380+

### Phase 3 Completion (Week 13):
- ✅ Semua 13 BAB memiliki dokumentasi lengkap
- ✅ 90%+ content completeness
- ✅ All programs tested dengan hardware real
- ✅ Video tutorials untuk key topics

---

*End of Gap Analysis Document*
*Generated: 5 February 2026*
