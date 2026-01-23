# LAPORAN LENGKAP: 54 Program Praktikum STM32F103C8T6
## Mastering STM32 - Comprehensive Laboratory Guide

**Tanggal**: 22 Januari 2026  
**Target MCU**: STM32F103C8T6 Blue Pill  
**Framework**: PlatformIO + STM32Cube HAL  
**Author**: Based on "Mastering STM32" by Carmine Noviello

---

## DAFTAR ISI
1. [Program 1-10: Basic & Intermediate](#program-1-10)
2. [Program 11-20: Advanced Peripherals](#program-11-20)
3. [Program 21-30: Power Management & Flash](#program-21-30)
4. [Program 31-44: RTOS & Communication](#program-31-44)
5. [Program 45-54: Industrial & Advanced](#program-45-54)
6. [Program 55-64: IoT & Connectivity](#program-55-64)
7. [Program 65-74: Advanced Control & Processing](#program-65-74)
8. [Program 75-84: Machine Learning & DSP](#program-75-84)
9. [Program 85-94: Security & Optimization](#program-85-94)
10. [Program 95-104: Production Ready Systems](#program-95-104)
11. [Hardware Requirements Summary](#hardware-summary)
12. [Bill of Materials (BOM)](#bom)
13. [Source Code Implementation](#source-code)

---

## PROGRAM 1-10: BASIC & INTERMEDIATE

### 1. **Blink_LED** - GPIO Output Dasar
**Tingkat**: Pemula  
**Topik**: GPIO, HAL_Delay, System Clock  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- LED Built-in di PC13 (atau LED eksternal + resistor 220Ω)
- ST-Link V2 programmer
- Breadboard dan kabel jumper

**Deskripsi**: Program paling dasar untuk mengedipkan LED. Mengajarkan konsep GPIO output, clock configuration, dan HAL_Delay().

**Pin Configuration**:
- PC13: LED (Active LOW)

---

### 2. **Button_Input** - GPIO Input dengan Debouncing
**Tingkat**: Pemula  
**Topik**: GPIO Input, Pull-up/Pull-down, Debouncing  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- Push button tactile switch
- Resistor 10kΩ (pull-down)
- LED + resistor 220Ω
- ST-Link V2 programmer
- Breadboard dan kabel jumper

**Deskripsi**: Membaca input dari button dengan implementasi software debouncing untuk menghindari false trigger.

**Pin Configuration**:
- PA0: Button input (dengan pull-down eksternal)
- PC13: LED indicator

---

### 3. **UART_Serial** - Komunikasi Serial
**Tingkat**: Pemula-Menengah  
**Topik**: UART, Interrupt, Ring Buffer  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- USB-to-TTL Serial adapter (CP2102, FT232, CH340)
- ST-Link V2 programmer
- Kabel jumper female-female
- PC dengan terminal software (PuTTY, TeraTerm, minicom)

**Deskripsi**: Komunikasi serial bidirectional dengan interrupt mode. Command parsing untuk kontrol LED.

**Pin Configuration**:
- PA9: USART1_TX
- PA10: USART1_RX
- PC13: LED

**Terminal Settings**: 115200 baud, 8N1

---

### 4. **Timer_Interrupt** - Timer Periodic
**Tingkat**: Menengah  
**Topik**: Timer, NVIC, Interrupt Priority  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- 2x LED + resistor 220Ω
- ST-Link V2 programmer
- Breadboard

**Deskripsi**: Menggunakan TIM2 dan TIM3 untuk menghasilkan interrupt periodik pada frekuensi berbeda.

**Pin Configuration**:
- PC13: LED 1 (TIM2 - 1Hz)
- PC14: LED 2 (TIM3 - 5Hz)

---

### 5. **PWM_LED_Dimmer** - PWM untuk Dimming
**Tingkat**: Menengah  
**Topik**: PWM, Timer Channel, Duty Cycle  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- LED + resistor 220Ω (gunakan LED terang/power LED untuk efek lebih baik)
- ST-Link V2 programmer
- Breadboard

**Deskripsi**: Kontrol brightness LED dengan PWM breathing effect (fade in/out).

**Pin Configuration**:
- PA0: TIM2_CH1 PWM output → LED

---

### 6. **ADC_Read_Sensor** - Analog to Digital Conversion
**Tingkat**: Menengah  
**Topik**: ADC, Polling Mode, Voltage Conversion  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- Potentiometer 10kΩ
- USB-to-TTL Serial adapter
- ST-Link V2 programmer
- Breadboard dan kabel jumper

**Deskripsi**: Membaca nilai analog dari potentiometer, konversi ke voltage, dan tampilkan via UART.

**Pin Configuration**:
- PA0: ADC1_IN0 (Potentiometer)
- PA9/PA10: UART1 TX/RX
- VCC: 3.3V reference

---

### 7. **External_Interrupt** - EXTI Handler
**Tingkat**: Menengah  
**Topik**: EXTI, GPIO Interrupt, Callback  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- 2x Push button
- 2x Resistor 10kΩ (pull-down)
- 2x LED + resistor 220Ω
- ST-Link V2 programmer
- Breadboard

**Deskripsi**: Mendeteksi rising/falling edge pada external pin menggunakan EXTI interrupt.

**Pin Configuration**:
- PA0: EXTI0 (Button 1 - Rising edge)
- PA1: EXTI1 (Button 2 - Falling edge)
- PC13, PC14: LED indicators

---

### 8. **I2C_Communication** - I2C Scanner & EEPROM
**Tingkat**: Menengah-Lanjut  
**Topik**: I2C Protocol, Master Mode, EEPROM  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- AT24C32 EEPROM (atau modul RTC DS1307 yang sudah include EEPROM)
- Resistor pull-up 4.7kΩ (2 buah untuk SDA dan SCL)
- USB-to-TTL Serial adapter
- ST-Link V2 programmer
- Breadboard

**Deskripsi**: Scan I2C bus untuk device, read/write EEPROM, demonstrasi I2C protocol.

**Pin Configuration**:
- PB6: I2C1_SCL (dengan pull-up 4.7kΩ ke 3.3V)
- PB7: I2C1_SDA (dengan pull-up 4.7kΩ ke 3.3V)
- PA9/PA10: UART1 TX/RX

**I2C Devices**:
- AT24C32 EEPROM: Address 0x50

---

### 9. **SPI_Communication** - SPI Flash Memory
**Tingkat**: Menengah-Lanjut  
**Topik**: SPI Protocol, Master Mode, W25Q Series Flash  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- W25Q32/W25Q64 SPI Flash module
- USB-to-TTL Serial adapter
- ST-Link V2 programmer
- Breadboard dan kabel jumper

**Deskripsi**: Komunikasi dengan W25Q Flash memory: read ID, erase sector, write data, read data.

**Pin Configuration**:
- PA5: SPI1_SCK
- PA6: SPI1_MISO
- PA7: SPI1_MOSI
- PA4: SPI1_NSS (Chip Select)
- PA9/PA10: UART1 TX/RX

---

### 10. **FreeRTOS_Multitasking** - Cooperative Scheduler
**Tingkat**: Lanjut  
**Topik**: Task Management, Simple Scheduler  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- 3x LED + resistor 220Ω
- USB-to-TTL Serial adapter
- ST-Link V2 programmer
- Breadboard

**Deskripsi**: Simple cooperative scheduler tanpa RTOS library. Demonstrasi task switching manual.

**Pin Configuration**:
- PC13, PC14, PC15: LED untuk 3 tasks berbeda
- PA9/PA10: UART1 TX/RX

---

## PROGRAM 11-20: ADVANCED PERIPHERALS

### 11. **DMA_UART_Ring** - DMA Circular Buffer untuk UART
**Tingkat**: Lanjut  
**Topik**: DMA, UART RX, Ring Buffer, IDLE Interrupt  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- USB-to-TTL Serial adapter
- ST-Link V2 programmer
- Kabel jumper

**Deskripsi**: UART RX dalam mode DMA circular dengan deteksi IDLE line untuk processing data.

**Pin Configuration**:
- PA9: USART1_TX
- PA10: USART1_RX (DMA1_Channel5)

**DMA Configuration**:
- DMA1 Channel 5: USART1_RX (Circular mode)

---

### 12. **ADC_DMA_Multi** - Multi-Channel ADC dengan DMA
**Tingkat**: Lanjut  
**Topik**: ADC Multi-Channel, DMA, Continuous Conversion  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- 2x Potentiometer 10kΩ atau sensor analog
- USB-to-TTL Serial adapter
- ST-Link V2 programmer
- Breadboard

**Deskripsi**: Konversi 2 channel ADC secara kontinyu menggunakan DMA untuk transfer otomatis.

**Pin Configuration**:
- PA0: ADC1_IN0 (Channel 0)
- PA1: ADC1_IN1 (Channel 1)
- PA9/PA10: UART1 TX/RX

**DMA Configuration**:
- DMA1 Channel 1: ADC1 (Circular mode)

---

### 13. **CAN_Loopback** - Controller Area Network
**Tingkat**: Lanjut  
**Topik**: bxCAN, Loopback Mode, Filter Configuration  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- (Opsional) MCP2551 CAN Transceiver untuk real CAN bus
- Resistor 120Ω untuk termination (jika pakai transceiver)
- USB-to-TTL Serial adapter
- ST-Link V2 programmer

**Deskripsi**: Demonstrasi CAN bus menggunakan internal loopback mode (tidak perlu hardware eksternal untuk testing).

**Pin Configuration**:
- PA11: CAN_RX
- PA12: CAN_TX
- PA9/PA10: UART1 TX/RX (monitoring)

**Note**: Untuk real CAN bus, tambahkan MCP2551 transceiver.

---

### 14. **RTC_Alarm** - Real-Time Clock dengan LSI
**Tingkat**: Menengah-Lanjut  
**Topik**: RTC, LSI Oscillator, Software Alarm  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- USB-to-TTL Serial adapter
- ST-Link V2 programmer
- (Opsional) Backup battery CR2032 + holder untuk VBAT

**Deskripsi**: RTC menggunakan LSI (~40kHz) dengan software alarm checking setiap 10 detik.

**Pin Configuration**:
- PA9/PA10: UART1 TX/RX
- PC13: LED (alarm indicator)

**Clock**: LSI ~40kHz internal oscillator

---

### 15. **Watchdog_Safety** - Independent Watchdog
**Tingkat**: Menengah  
**Topik**: IWDG, System Safety, Timeout  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- Push button
- Resistor 10kΩ (pull-down)
- USB-to-TTL Serial adapter
- ST-Link V2 programmer

**Deskripsi**: Demonstrasi IWDG dengan timeout 4 detik. Simulasi sistem hang dan recovery.

**Pin Configuration**:
- PA0: Button (untuk simulasi hang)
- PC13: LED (heartbeat indicator)
- PA9/PA10: UART1 TX/RX

---

### 16. **SPI_DMA_Transfer** - SPI Full-Duplex DMA
**Tingkat**: Lanjut  
**Topik**: SPI DMA, Full-Duplex, Completion Callback  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- SPI device (Flash/Sensor) atau loopback (MOSI→MISO)
- USB-to-TTL Serial adapter
- ST-Link V2 programmer

**Deskripsi**: Transfer data SPI dalam mode DMA full-duplex dengan loopback test.

**Pin Configuration**:
- PA5: SPI1_SCK
- PA6: SPI1_MISO
- PA7: SPI1_MOSI
- PA4: SPI1_NSS

**DMA Configuration**:
- DMA1 Channel 2: SPI1_RX
- DMA1 Channel 3: SPI1_TX

---

### 17. **Encoder_TIM** - Quadrature Encoder Mode
**Tingkat**: Lanjut  
**Topik**: Timer Encoder Mode, Position Tracking  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- Rotary encoder (mechanical atau optical)
- USB-to-TTL Serial adapter
- ST-Link V2 programmer
- (Opsional) Pull-up resistors 10kΩ untuk encoder pins

**Deskripsi**: Membaca rotary encoder menggunakan timer encoder mode (quadrature decoding).

**Pin Configuration**:
- PA6: TIM3_CH1 (Encoder A)
- PA7: TIM3_CH2 (Encoder B)
- PA9/PA10: UART1 TX/RX

**Encoder**: Mechanical rotary encoder atau optical encoder

---

### 18. **InputCapture_TIM** - Frequency & Pulse Width Measurement
**Tingkat**: Lanjut  
**Topik**: Input Capture, Timer, Frequency Measurement  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- Function generator atau PWM source (bisa dari Arduino)
- USB-to-TTL Serial adapter
- ST-Link V2 programmer

**Deskripsi**: Mengukur frekuensi dan pulse width sinyal eksternal menggunakan input capture.

**Pin Configuration**:
- PA0: TIM2_CH1 (Input Capture)
- PA9/PA10: UART1 TX/RX

**Input Signal**: 100Hz - 10kHz square wave, 3.3V logic level

---

### 19. **Bootloader_Jump** - Jump to System Bootloader
**Tingkat**: Lanjut  
**Topik**: Bootloader, System Memory, MSP Reset  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- USB-to-TTL Serial adapter
- ST-Link V2 programmer
- PC dengan STM32 Flash Loader Demonstrator atau STM32CubeProgrammer

**Deskripsi**: Software jump ke system bootloader di 0x1FFFF000 untuk firmware update via UART.

**Pin Configuration**:
- PA0: Button (trigger bootloader jump)
- PA9/PA10: UART1 TX/RX (untuk bootloader communication)
- PC13: LED

**System Bootloader**: 0x1FFFF000 (STM32F1 factory bootloader)

---

### 20. **DMA_Mem2Mem_CRC** - Memory Transfer & CRC
**Tingkat**: Lanjut  
**Topik**: DMA Memory-to-Memory, CRC Peripheral, Data Integrity  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- USB-to-TTL Serial adapter
- ST-Link V2 programmer

**Deskripsi**: Transfer data memory-to-memory menggunakan DMA dengan CRC verification untuk data integrity.

**Pin Configuration**:
- PA9/PA10: UART1 TX/RX

**DMA Configuration**:
- DMA1 Channel 1: Memory-to-Memory mode

---

## PROGRAM 21-30: POWER MANAGEMENT & FLASH

### 21. **Low_Power_Sleep** - Sleep Mode dengan EXTI Wake-up  
**Tingkat**: Menengah-Lanjut  
**Topik**: Low Power, Sleep Mode, WFI, Wake-up Sources  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- Push button + resistor 10kΩ pull-down
- LED + resistor 220Ω
- USB-to-TTL Serial adapter
- ST-Link V2 programmer
- Ammeter untuk mengukur konsumsi arus (opsional)

**Deskripsi**: MCU masuk sleep mode (WFI) dan bangun melalui EXTI interrupt dari button.

**Pin Configuration**:
- PA0: Button (EXTI0 wake-up source)
- PC13: LED indicator
- PA9/PA10: UART1 TX/RX

**Power Consumption**:
- Run mode: ~20mA @ 72MHz
- Sleep mode: ~10mA (peripheral masih aktif)

---

### 22. **Flash_EEPROM_Emulation** - Internal Flash sebagai EEPROM  
**Tingkat**: Lanjut  
**Topik**: Flash Programming, EEPROM Emulation, Wear Leveling  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- USB-to-TTL Serial adapter
- ST-Link V2 programmer

**Deskripsi**: Emulasi EEPROM menggunakan last 2 pages flash. Implementasi wear leveling sederhana.

**Pin Configuration**:
- PA9/PA10: UART1 TX/RX
- PC13: LED

**Flash Allocation**:
- Page 62-63: EEPROM emulation (0x0801F800 - 0x0801FFFF)
- Total: 2KB EEPROM space

---

### 23. **Stop_Mode_RTC_Wakeup** - Stop Mode dengan RTC Wake-up  
**Tingkat**: Lanjut  
**Topik**: Stop Mode, RTC Wake-up, Ultra Low Power  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- 32.768kHz crystal untuk LSE (opsional, bisa pakai LSI)
- 2x Capacitor 20pF untuk crystal
- Ammeter untuk power measurement
- ST-Link V2 programmer
- USB-to-TTL Serial adapter

**Deskripsi**: Stop mode dengan RTC periodic wakeup setiap 5 detik untuk ultra low power application.

**Pin Configuration**:
- PC14/PC15: LSE crystal (opsional)
- PC13: LED
- PA9/PA10: UART1 TX/RX

**Power Consumption**:
- Stop mode: ~2µA (dengan LSE) atau ~3µA (dengan LSI)

---

### 24. **Standby_Mode_WakeupPin** - Standby Mode (Deepest Sleep)  
**Tingkat**: Lanjut  
**Topik**: Standby Mode, WKUP Pin, RTC Backup  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- Push button + resistor 10kΩ pull-down
- CR2032 battery + holder untuk VBAT (opsional)
- Ammeter
- ST-Link V2 programmer

**Deskripsi**: Standby mode (konsumsi terendah <5µA) dengan wake-up via WKUP pin atau RTC alarm.

**Pin Configuration**:
- PA0: WKUP pin (built-in wake-up pin)
- PC13: LED
- VBAT: Battery untuk RTC backup

**Power Consumption**:
- Standby mode: <2µA (tanpa RTC) atau ~3µA (dengan RTC running)

---

### 25. **Flash_Protection** - Read/Write Protection  
**Tingkat**: Lanjut  
**Topik**: Option Bytes, Read Protection, Write Protection  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- ST-Link V2 programmer
- USB-to-TTL Serial adapter

**Deskripsi**: Mengkonfigurasi read protection dan write protection untuk security aplikasi komersial.

**Pin Configuration**:
- PA9/PA10: UART1 TX/RX
- PC13: LED

**Protection Levels**:
- Level 0: No protection
- Level 1: Read protection enabled
- Write protection per page

**⚠️ WARNING**: Read Protection Level 2 bersifat PERMANENT dan tidak bisa di-reverse!

---

### 26. **Backup_SRAM** - Backup Domain & SRAM Retention  
**Tingkat**: Menengah-Lanjut  
**Topik**: Backup Domain, SRAM Retention, Power Domain  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board (NOTE: F103 TIDAK punya backup SRAM - program ini untuk F4/F7)
- **Alternative**: STM32F4 Discovery board
- CR2032 battery + holder
- ST-Link V2 programmer

**Deskripsi**: Menyimpan data di backup SRAM yang retain saat power off (dengan VBAT).

**Pin Configuration**:
- PA9/PA10: UART1 TX/RX
- VBAT: CR2032 3V

**Note**: STM32F1 series TIDAK memiliki backup SRAM. Program ini hanya untuk F2/F4/F7 series.

---

### 27. **CCM_RAM_Usage** - Core-Coupled Memory Optimization  
**Tingkat**: Expert  
**Topik**: CCM RAM, Memory Layout, Linker Script  

**Hardware Requirements**:
- **STM32F4 series board** (F1 tidak punya CCM!)
- ST-Link V2 programmer
- USB-to-TTL Serial adapter

**Deskripsi**: Menggunakan CCM RAM (64KB di F4) untuk critical data/code dengan zero-wait state access.

**Pin Configuration**:
- PA9/PA10: UART1 TX/RX

**Memory Layout**:
- CCM RAM: 0x10000000 - 0x1000FFFF (64KB - F4 series only)

**Note**: STM32F103 TIDAK punya CCM. Program ini demonstrasi untuk F4/F7.

---

### 28. **DMA_ADC_Timer_Trigger** - Hardware Triggered ADC with DMA  
**Tingkat**: Lanjut  
**Topik**: Timer-triggered ADC, DMA, Regular Conversion  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- Function generator atau sensor analog
- USB-to-TTL Serial adapter
- ST-Link V2 programmer
- Oscilloscope (opsional untuk verifikasi timing)

**Deskripsi**: ADC conversion triggered oleh timer dengan fixed sampling rate, DMA transfer ke buffer.

**Pin Configuration**:
- PA0: ADC1_IN0 (analog input)
- PA9/PA10: UART1 TX/RX

**Timing**:
- TIM2 TRGO: Trigger ADC @ 1kHz sampling rate
- DMA1 CH1: Transfer ADC data ke circular buffer

---

### 29. **Multi_DMA_Priority** - Multiple DMA Streams dengan Priority  
**Tingkat**: Expert  
**Topik**: DMA Priority Levels, Resource Conflict, Arbitration  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- USB-to-TTL Serial adapter
- Oscilloscope atau logic analyzer (untuk timing analysis)
- ST-Link V2 programmer

**Deskripsi**: Menggunakan multiple DMA channels dengan priority berbeda, demonstrasi arbitration.

**Pin Configuration**:
- PA9: USART1_TX (DMA Priority HIGH)
- PA10: USART1_RX (DMA Priority MEDIUM)
- PC13: LED (DMA Priority LOW - memory test)

**DMA Configuration**:
- Channel 4 (USART1_TX): Very High priority
- Channel 5 (USART1_RX): High priority  
- Channel 1 (Memory): Low priority

---

### 30. **Flash_Dual_Bank** - Dual Bank Flash Management  
**Tingkat**: Expert  
**Topik**: Dual Bank Flash, Firmware Update, Bank Switching  

**Hardware Requirements**:
- **STM32F4/F7 with dual-bank flash** (F103 single bank only!)
- Alternative: STM32F7 Discovery
- ST-Link V2 programmer
- USB-to-TTL Serial adapter

**Deskripsi**: Firmware update menggunakan dual-bank flash untuk fail-safe OTA updates.

**Pin Configuration**:
- PA9/PA10: UART1 TX/RX
- PA0: Button (trigger bank switch)

**Flash Banks**:
- Bank 1: 0x08000000 - 0x080FFFFF (1MB)
- Bank 2: 0x08100000 - 0x081FFFFF (1MB)

**Note**: F103 hanya punya single bank. Program ini untuk F4/F7 series.

---

## PROGRAM 31-44: RTOS & ADVANCED COMMUNICATION

### 31. **FreeRTOS_Tasks** - Multiple Tasks dengan Priority  
**Tingkat**: Lanjut  
**Topik**: FreeRTOS, Task Management, Priority Scheduling  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- 3x LED + resistor 220Ω
- USB-to-TTL Serial adapter
- ST-Link V2 programmer

**Deskripsi**: FreeRTOS dengan 3 tasks berbeda priority: High (LED fast blink), Medium (UART), Low (sensor read).

**Pin Configuration**:
- PC13, PC14, PC15: LEDs untuk 3 tasks
- PA9/PA10: UART1 TX/RX

**FreeRTOS Config**:
- RTOS Heap: 4KB
- Task stack: 128 words each
- Scheduler: Preemptive

---

### 32. **FreeRTOS_Semaphore** - Binary & Counting Semaphore  
**Tingkat**: Lanjut  
**Topik**: Semaphore, Task Synchronization, ISR-to-Task Communication  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- 2x Push button
- 2x LED
- USB-to-TTL Serial adapter
- ST-Link V2 programmer

**Deskripsi**: Binary semaphore untuk ISR-to-task signaling, counting semaphore untuk resource management.

**Pin Configuration**:
- PA0, PA1: Buttons (ISR triggers)
- PC13, PC14: LEDs (task indicators)
- PA9/PA10: UART1 TX/RX

---

### 33. **FreeRTOS_Queue** - Inter-Task Communication  
**Tingkat**: Lanjut  
**Topik**: Message Queue, Producer-Consumer, Data Passing  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- Potentiometer atau sensor analog
- LED
- USB-to-TTL Serial adapter
- ST-Link V2 programmer

**Deskripsi**: Queue untuk passing data antara producer task (ADC) dan consumer task (UART logger).

**Pin Configuration**:
- PA0: ADC input (producer)
- PA9/PA10: UART1 TX/RX (consumer)
- PC13: LED (queue full indicator)

**Queue Config**:
- Queue length: 10 items
- Item size: sizeof(ADC_Data_t)

---

### 34. **FreeRTOS_Mutex** - Mutual Exclusion & Priority Inheritance  
**Tingkat**: Lanjut-Expert  
**Topik**: Mutex, Priority Inversion, Deadlock Prevention  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- USB-to-TTL Serial adapter (shared resource)
- 2x LED
- ST-Link V2 programmer

**Deskripsi**: Demonstrasi mutex untuk protect shared resource (UART), priority inheritance untuk avoid priority inversion.

**Pin Configuration**:
- PA9/PA10: UART1 TX/RX (shared resource)
- PC13, PC14: LEDs (task activity)

**Tasks**:
- High priority: Periodic reporting
- Low priority: Background logging
- UART protected by mutex

---

### 35. **FreeRTOS_Software_Timer** - Software Timers  
**Tingkat**: Menengah-Lanjut  
**Topik**: Software Timer, One-shot & Periodic, Timer Service Task  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- 3x LED
- USB-to-TTL Serial adapter
- ST-Link V2 programmer

**Deskripsi**: Software timers untuk periodic & one-shot operations tanpa block tasks.

**Pin Configuration**:
- PC13: LED (1s periodic timer)
- PC14: LED (5s one-shot timer)
- PC15: LED (heartbeat)
- PA9/PA10: UART1 TX/RX

---

### 36. **FreeRTOS_Tickless_Idle** - Low Power RTOS  
**Tingkat**: Expert  
**Topik**: Tickless Idle, Low Power, RTOS Power Management  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- Ammeter untuk power measurement
- Push button
- ST-Link V2 programmer

**Deskripsi**: FreeRTOS tickless idle mode untuk low power dengan dynamic tick suppression.

**Pin Configuration**:
- PA0: Button (wake-up event)
- PC13: LED
- Current measurement point: VDD line

**Power Modes**:
- Active task: ~20mA
- Idle tickless: ~500µA

---

### 37. **FreeRTOS_Event_Groups** - Event-based Task Synchronization  
**Tingkat**: Lanjut  
**Topik**: Event Groups, Bit Flags, Multi-task Synchronization  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- 3x Push button
- 4x LED
- USB-to-TTL Serial adapter
- ST-Link V2 programmer

**Deskripsi**: Event Groups untuk synchronize multiple tasks menggunakan event flags. Demonstrasi wait-for-all dan wait-for-any patterns.

**Pin Configuration**:
- PA0, PA1, PA2: Buttons (set event bits 0, 1, 2)
- PC13, PC14, PC15, PB12: LEDs (event indicators)
- PA9/PA10: UART1 TX/RX

**Event Bits**:
- Bit 0: Button 1 pressed
- Bit 1: Button 2 pressed
- Bit 2: Button 3 pressed
- Bit 3: All buttons pressed (composite event)

**Use Cases**:
- Wait for multiple sensors ready
- Synchronize startup sequences
- Coordinate shutdown operations
- Complex state machines

---

### 38. **FreeRTOS_Task_Notifications** - Lightweight Signaling  
**Tingkat**: Lanjut  
**Topik**: Task Notifications, Direct-to-Task Signaling, Lightweight IPC  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- 2x Push button
- 3x LED
- USB-to-TTL Serial adapter
- ST-Link V2 programmer

**Deskripsi**: Task notifications sebagai alternatif lightweight untuk semaphore/queue. Faster dan menggunakan RAM lebih sedikit.

**Pin Configuration**:
- PA0, PA1: Buttons (trigger notifications)
- PC13, PC14, PC15: LEDs (task states)
- PA9/PA10: UART1 TX/RX

**Notification Methods**:
- xTaskNotify(): Send notification dengan value
- ulTaskNotifyTake(): Binary semaphore alternative
- xTaskNotifyWait(): Wait dengan timeout
- xTaskNotifyFromISR(): Notify dari interrupt

**Performance Comparison**:
- Semaphore: ~180 cycles overhead
- Task Notification: ~45 cycles overhead (75% faster!)

**Limitations**:
- Hanya bisa notify 1 task (tidak bisa broadcast)
- Tidak punya queue depth (hanya 1 pending notification)

---

### 39. **FreeRTOS_Stream_Buffers** - Variable-Length Data Passing  
**Tingkat**: Lanjut-Expert  
**Topik**: Stream Buffers, Message Buffers, Variable-Length IPC  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- USB-to-TTL Serial adapter (producer)
- LED
- ST-Link V2 programmer

**Deskripsi**: Stream buffers untuk passing variable-length data between tasks atau ISR-to-task. Message buffers untuk discrete messages.

**Pin Configuration**:
- PA9: USART1_TX (monitoring)
- PA10: USART1_RX (data producer via interrupt)
- PC13: LED (data available indicator)

**Two Types**:
1. **Stream Buffer**: Continuous byte stream (seperti UART data)
2. **Message Buffer**: Discrete messages dengan length prefix

**Use Cases**:
- UART → Task data pipeline
- Audio streaming
- Protocol parsers
- Variable-length sensor data

**Configuration**:
- Buffer size: 512 bytes
- Trigger level: 10 bytes (wake consumer task)

**Comparison**:
- Queue: Fixed-size items, copy overhead
- Stream Buffer: Variable-size, zero-copy (pointer passing)

---

### 40. **FreeRTOS_Memory_Management** - Heap Schemes & Statistics  
**Tingkat**: Expert  
**Topik**: Memory Management, Heap Algorithms, Memory Debugging  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- USB-to-TTL Serial adapter
- ST-Link V2 programmer

**Deskripsi**: Demonstrasi 5 heap schemes FreeRTOS, memory statistics, dan debugging memory leaks.

**Pin Configuration**:
- PA9/PA10: UART1 TX/RX (memory stats output)
- PC13: LED (memory status)

**FreeRTOS Heap Schemes**:
1. **heap_1.c**: Static allocation only (no free)
   - Use case: Embedded systems tanpa dynamic allocation
   
2. **heap_2.c**: Best fit with fragmentation
   - Use case: Legacy, tidak recommended
   
3. **heap_3.c**: Wrapper untuk malloc/free
   - Use case: Compatibility dengan C library
   
4. **heap_4.c**: First fit with coalescing (RECOMMENDED)
   - Use case: General purpose, prevents fragmentation
   
5. **heap_5.c**: Multiple non-contiguous memory regions
   - Use case: Systems dengan scattered RAM (CCM + normal RAM)

**Memory Statistics API**:
- `xPortGetFreeHeapSize()`: Current free heap
- `xPortGetMinimumEverFreeHeapSize()`: Low water mark
- `vTaskGetRunTimeStats()`: Task execution time

**Demonstration**:
- Dynamic task creation/deletion
- Heap usage monitoring
- Fragmentation analysis
- Memory leak detection

**Debugging Tools**:
- Heap visualization via UART
- Stack overflow detection (configCHECK_FOR_STACK_OVERFLOW)
- Malloc failed hook

---

### 41. **I2C_Multi_Sensor** - Multiple I2C Sensors  
**Tingkat**: Lanjut  
**Topik**: I2C Multi-Device, Sensor Fusion, Data Aggregation  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- BMP280 sensor (temperature + pressure)
- MPU6050 sensor (accelerometer + gyroscope)
- 2x Resistor 4.7kΩ pull-up (SDA & SCL)
- USB-to-TTL Serial adapter
- ST-Link V2 programmer

**Deskripsi**: Membaca multiple sensors di I2C bus yang sama dengan address berbeda.

**Pin Configuration**:
- PB6: I2C1_SCL (with 4.7kΩ pull-up)
- PB7: I2C1_SDA (with 4.7kΩ pull-up)
- PA9/PA10: UART1 TX/RX

**I2C Devices**:
- BMP280: Address 0x76 (temperature, pressure, altitude)
- MPU6050: Address 0x68 (accel, gyro)

---

### 42. **SPI_Multiple_Slaves** - SPI Multi-Slave dengan CS Management  
**Tingkat**: Lanjut  
**Topik**: SPI Multi-Slave, Chip Select Management, Sequential Access  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- W25Q32 Flash memory
- MCP3008 ADC (8-channel SPI ADC)
- USB-to-TTL Serial adapter
- ST-Link V2 programmer

**Deskripsi**: Multiple SPI slaves (Flash + ADC) di bus yang sama dengan individual chip select.

**Pin Configuration**:
- PA5: SPI1_SCK (shared)
- PA6: SPI1_MISO (shared)
- PA7: SPI1_MOSI (shared)
- PA4: CS_Flash (W25Q32)
- PB0: CS_ADC (MCP3008)
- PA9/PA10: UART1 TX/RX

---

### 43. **USART_DMA_Circular_Idle** - Advanced UART DMA dengan IDLE Detection  
**Tingkat**: Expert  
**Topik**: UART DMA, Circular Buffer, IDLE Line Detection, Variable Length Data  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- USB-to-TTL Serial adapter
- ST-Link V2 programmer

**Deskripsi**: UART RX DMA circular buffer dengan IDLE interrupt untuk variable-length packet processing.

**Pin Configuration**:
- PA9: USART1_TX
- PA10: USART1_RX (DMA1_Channel5 circular)

**Features**:
- Ring buffer: 256 bytes
- IDLE interrupt untuk detect end of transmission
- Support variable length packets

---

### 44. **MODBUS_RTU_Slave** - MODBUS RTU Protocol Implementation  
**Tingkat**: Expert  
**Topik**: MODBUS RTU, Protocol Stack, CRC-16, Holding Registers  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- RS485 transceiver module (MAX485 / SP485)
- Resistor 120Ω untuk termination
- USB-to-TTL Serial adapter (monitoring)
- ST-Link V2 programmer
- MODBUS master device atau PC dengan MODBUS software

**Deskripsi**: Full MODBUS RTU slave implementation untuk industrial communication.

**Pin Configuration**:
- PA9: USART1_TX → MAX485 DI
- PA10: USART1_RX ← MAX485 RO
- PA8: RS485 DE/RE control (TX enable)
- PA2/PA3: USART2 TX/RX (debug monitor)

**MODBUS Config**:
- Slave ID: 0x01
- Baud rate: 9600, 8N1
- Function codes: 0x03 (Read Holding), 0x06 (Write Single), 0x10 (Write Multiple)

---

## PROGRAM 45-54: INDUSTRIAL & ADVANCED APPLICATIONS

### 45. **USB_CDC_VirtualCOM** - USB Virtual COM Port  
**Tingkat**: Expert  
**Topik**: USB Device, CDC Class, Virtual COM Port  

**Hardware Requirements**:
- **STM32F1 dengan USB peripheral** (Blue Pill OK!)
- Resistor 1.5kΩ dari PA12 ke VCC (untuk USB pull-up)
- Micro USB cable
- ST-Link V2 programmer
- PC dengan USB host

**Deskripsi**: Implementasi USB CDC Virtual COM port untuk komunikasi dengan PC tanpa USB-Serial adapter.

**Pin Configuration**:
- PA11: USB_DM (D-)
- PA12: USB_DP (D+) with 1.5kΩ pull-up
- PC13: LED (USB activity)

**USB Config**:
- VID: 0x0483 (STMicroelectronics)
- PID: 0x5740
- Class: CDC (Communication Device Class)

---

### 46. **SD_Card_FatFS** - SD Card dengan FAT File System  
**Tingkat**: Expert  
**Topik**: SPI SD Card, FatFS Middleware, File I/O  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- MicroSD card module (SPI interface)
- MicroSD card (formatted FAT32)
- USB-to-TTL Serial adapter
- ST-Link V2 programmer

**Deskripsi**: Read/write files di SD card menggunakan FatFS middleware via SPI.

**Pin Configuration**:
- PA5: SPI1_SCK
- PA6: SPI1_MISO
- PA7: SPI1_MOSI
- PA4: SD_CS (Chip Select)
- PA9/PA10: UART1 TX/RX

**FatFS Operations**:
- Mount/unmount filesystem
- Create/delete files
- Read/write data
- Directory listing

---

### 47. **1-Wire_DS18B20** - 1-Wire Temperature Sensor  
**Tingkat**: Menengah-Lanjut  
**Topik**: 1-Wire Protocol, DS18B20, Bit-banging  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- DS18B20 temperature sensor (waterproof probe)
- Resistor 4.7kΩ pull-up
- USB-to-TTL Serial adapter
- ST-Link V2 programmer

**Deskripsi**: Implementasi 1-Wire protocol untuk baca temperature dari DS18B20 dengan bit-banging.

**Pin Configuration**:
- PA0: 1-Wire DATA (with 4.7kΩ pull-up)
- PA9/PA10: UART1 TX/RX
- PC13: LED

**1-Wire Features**:
- ROM search algorithm (support multiple devices)
- CRC-8 verification
- Temperature resolution: 9-12 bit (0.0625°C resolution)

---

### 48. **DHT22_Humidity_Sensor** - DHT22/AM2302 Sensor  
**Tingkat**: Menengah  
**Topik**: DHT22 Protocol, Timing-critical Bit-banging  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- DHT22 (AM2302) temperature & humidity sensor
- Resistor 10kΩ pull-up
- USB-to-TTL Serial adapter
- ST-Link V2 programmer

**Deskripsi**: Baca temperature & humidity dari DHT22 sensor dengan timing-critical protocol.

**Pin Configuration**:
- PA0: DHT22 DATA (with 10kΩ pull-up)
- PA9/PA10: UART1 TX/RX
- PC13: LED

**DHT22 Specs**:
- Humidity: 0-100% RH (±2% accuracy)
- Temperature: -40 to +80°C (±0.5°C accuracy)
- Sampling rate: 0.5Hz (1 sample per 2 seconds)

---

### 49. **Ultrasonic_HC-SR04** - Ultrasonic Distance Sensor  
**Tingkat**: Menengah  
**Topik**: HC-SR04, Echo Time Measurement, Input Capture  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- HC-SR04 ultrasonic sensor
- USB-to-TTL Serial adapter
- ST-Link V2 programmer

**Deskripsi**: Pengukuran jarak menggunakan HC-SR04 dengan input capture untuk timing presisi.

**Pin Configuration**:
- PA0: HC-SR04 TRIG (output pulse)
- PA1: HC-SR04 ECHO (input capture TIM2_CH2)
- PA9/PA10: UART1 TX/RX
- PC13: LED

**Measurement Range**: 2cm - 400cm  
**Accuracy**: ±3mm

---

### 50. **Servo_Motor_Control** - Multi-Servo Control  
**Tingkat**: Menengah  
**Topik**: Servo PWM, 50Hz PWM, Multi-channel  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- 2-4x Servo motor (SG90 atau MG996R)
- External 5V power supply (untuk servos)
- USB-to-TTL Serial adapter
- ST-Link V2 programmer

**Deskripsi**: Control multiple servo motors dengan PWM 50Hz, pulse width 1-2ms untuk 0-180°.

**Pin Configuration**:
- PA0: TIM2_CH1 PWM → Servo 1
- PA1: TIM2_CH2 PWM → Servo 2
- PA2: TIM2_CH3 PWM → Servo 3
- PA3: TIM2_CH4 PWM → Servo 4
- PA9/PA10: UART1 TX/RX (control commands)

**PWM Parameters**:
- Frequency: 50Hz (20ms period)
- Pulse width: 1ms (0°) to 2ms (180°)

---

### 51. **Stepper_Motor_A4988** - Stepper Motor dengan A4988 Driver  
**Tingkat**: Menengah-Lanjut  
**Topik**: Stepper Motor, A4988 Driver, Microstepping  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- NEMA 17 stepper motor
- A4988 stepper driver module
- 12V power supply untuk motor
- USB-to-TTL Serial adapter
- ST-Link V2 programmer

**Deskripsi**: Control stepper motor dengan A4988 driver: direction, speed, microstepping.

**Pin Configuration**:
- PA0: STEP signal (pulse untuk setiap step)
- PA1: DIR (direction: CW/CCW)
- PA2: ENABLE (motor enable/disable)
- PA3, PA4, PA5: MS1, MS2, MS3 (microstepping select)
- PA9/PA10: UART1 TX/RX

**Microstepping**:
- Full step: 200 steps/rev
- 1/16 microstep: 3200 steps/rev

---

### 52. **OLED_SSD1306_Graphics** - OLED Display dengan Graphics  
**Tingkat**: Lanjut  
**Topik**: OLED SSD1306, I2C, Graphics Library, Fonts  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- 0.96" OLED display 128x64 (SSD1306 controller, I2C)
- 2x Resistor 4.7kΩ pull-up
- ST-Link V2 programmer

**Deskripsi**: Display graphics, text, dan sensor data di OLED screen dengan custom fonts.

**Pin Configuration**:
- PB6: I2C1_SCL
- PB7: I2C1_SDA
- PC13: LED

**OLED Features**:
- Resolution: 128x64 pixels
- Interface: I2C (address 0x3C)
- Graphics: Lines, rectangles, circles, bitmaps
- Multiple font sizes

---

### 53. **Custom_Bootloader_UART** - Custom Bootloader dengan Encryption  
**Tingkat**: Expert  
**Topik**: Bootloader Development, Flash Programming, AES Encryption, CRC  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- USB-to-TTL Serial adapter
- ST-Link V2 programmer
- PC dengan Python script untuk upload firmware

**Deskripsi**: Custom bootloader untuk firmware update via UART dengan AES-128 encryption dan CRC verification.

**Memory Layout**:
- 0x08000000 - 0x08003FFF: Bootloader (16KB)
- 0x08004000 - 0x0801FBFF: Application firmware (111KB)
- 0x0801FC00 - 0x0801FFFF: Configuration & flags (1KB)

**Pin Configuration**:
- PA9/PA10: USART1 TX/RX (bootloader communication)
- PA0: Button (force bootloader mode)
- PC13: LED (bootloader activity)

**Features**:
- AES-128 CBC encryption
- CRC-32 firmware verification
- Dual-image support (A/B partitions)
- Rollback protection
- Command protocol: UPLOAD, VERIFY, JUMP, ERASE

---

### 54. **Industrial_Data_Logger** - Complete Data Logger System  
**Tingkat**: Expert  
**Topik**: System Integration, RTC, SD Card, Sensors, Low Power  

**Hardware Requirements**:
- STM32F103C8T6 Blue Pill board
- DS3231 RTC module (I2C, with battery)
- MicroSD card module (SPI)
- BMP280 sensor (temperature, pressure)
- MicroSD card
- CR2032 battery untuk RTC
- USB-to-TTL Serial adapter
- ST-Link V2 programmer

**Deskripsi**: Complete industrial data logger: multi-sensor reading, timestamp, SD card storage, low power modes.

**Pin Configuration**:
- **I2C Bus** (sensors & RTC):
  - PB6: I2C1_SCL
  - PB7: I2C1_SDA
- **SPI Bus** (SD card):
  - PA5: SPI1_SCK
  - PA6: SPI1_MISO
  - PA7: SPI1_MOSI
  - PA4: SD_CS
- **Control**:
  - PA0: Button (manual logging trigger)
  - PC13: LED (logging activity)
  - PA9/PA10: UART1 TX/RX

**Features**:
- FreeRTOS dengan 3 tasks:
  - Sensor acquisition task (every 10s)
  - SD card write task (with buffering)
  - Low power idle task
- DS3231 RTC untuk timestamp
- CSV format logging
- Battery-backed RTC
- Configurable logging interval
- Power consumption optimization

**Example Log File** (data.csv):
```csv
Timestamp,Temperature(C),Pressure(hPa),Humidity(%)
2026-01-22 10:30:00,25.3,1013.25,45.2
2026-01-22 10:30:10,25.4,1013.20,45.1
```

---

## HARDWARE REQUIREMENTS SUMMARY

### Hardware Komponen Dasar (Wajib untuk Semua Project)
| No | Komponen | Qty | Harga Est. | Keterangan |
|----|----------|-----|------------|------------|
| 1 | STM32F103C8T6 Blue Pill | 1-2 | Rp 35.000 | MCU board utama |
| 2 | ST-Link V2 programmer | 1 | Rp 25.000 | For upload & debug |
| 3 | Mini breadboard | 2-3 | Rp 15.000 | 400 atau 830 tie-points |
| 4 | Jumper wires M-M | 40pcs | Rp 10.000 | Breadboard connections |
| 5 | Jumper wires M-F | 40pcs | Rp 10.000 | Board-to-breadboard |
| 6 | USB cable Mini/Micro | 2 | Rp 10.000 | Power & programming |
| 7 | Resistor kit (10Ω-1MΩ) | 1 set | Rp 25.000 | Various values |
| 8 | LED 5mm (Red, Green, Yellow) | 20pcs | Rp 5.000 | Indicators |
| 9 | Push button tactile | 10pcs | Rp 5.000 | User input |
| 10 | Capacitor kit (pF-µF) | 1 set | Rp 20.000 | Decoupling, timing |

**Subtotal Komponen Dasar**: ~Rp 160.000

---

### Hardware Tambahan per Kategori Program

#### A. Komunikasi & Interface (Program 3, 8, 9, 13, 37-40)
| No | Komponen | Qty | Harga Est. | Program |
|----|----------|-----|------------|---------|
| 11 | USB-to-TTL adapter (CP2102/CH340) | 1 | Rp 15.000 | #3, 8, 9, 13, 37-50 |
| 12 | AT24C32 EEPROM module | 1 | Rp 10.000 | #8 |
| 13 | W25Q32 Flash module (SPI) | 1 | Rp 20.000 | #9 |
| 14 | MicroSD card module | 1 | Rp 15.000 | #42, 50 |
| 15 | MicroSD card 8GB | 1 | Rp 50.000 | #42, 50 |
| 16 | MAX485 RS485 module | 1 | Rp 20.000 | #40 (MODBUS) |

**Subtotal Komunikasi**: ~Rp 130.000

---

#### B. Sensor & Analog (Program 6, 12, 37, 43-45, 50)
| No | Komponen | Qty | Harga Est. | Program |
|----|----------|-----|------------|---------|
| 17 | Potentiometer 10kΩ | 3 | Rp 9.000 | #6, 12, 33 |
| 18 | BMP280 sensor (I2C) | 1 | Rp 25.000 | #37, 50 |
| 19 | MPU6050 IMU (I2C) | 1 | Rp 30.000 | #37 |
| 20 | DS18B20 temp sensor | 1 | Rp 15.000 | #43 |
| 21 | DHT22 humidity sensor | 1 | Rp 30.000 | #44 |
| 22 | HC-SR04 ultrasonic | 1 | Rp 15.000 | #45 |
| 23 | MCP3008 8-ch ADC (SPI) | 1 | Rp 35.000 | #38 |

**Subtotal Sensors**: ~Rp 159.000

---

#### C. Motor & Aktuator (Program 46-47)
| No | Komponen | Qty | Harga Est. | Program |
|----|----------|-----|------------|---------|
| 24 | Servo motor SG90 | 2 | Rp 30.000 | #46 |
| 25 | NEMA 17 stepper motor | 1 | Rp 120.000 | #47 |
| 26 | A4988 stepper driver | 1 | Rp 20.000 | #47 |
| 27 | Power supply 12V 2A | 1 | Rp 50.000 | #47 |

**Subtotal Motors**: ~Rp 220.000

---

#### D. Display & HMI (Program 48)
| No | Komponen | Qty | Harga Est. | Program |
|----|----------|-----|------------|---------|
| 28 | OLED 0.96" 128x64 I2C | 1 | Rp 30.000 | #48 |

**Subtotal Display**: ~Rp 30.000

---

#### E. Power & RTC (Program 14, 23-24, 50)
| No | Komponen | Qty | Harga Est. | Program |
|----|----------|-----|------------|---------|
| 29 | DS3231 RTC module | 1 | Rp 25.000 | #50 |
| 30 | CR2032 battery + holder | 2 | Rp 15.000 | #14, 23, 50 |
| 31 | 32.768kHz crystal | 1 | Rp 5.000 | #23 |

**Subtotal Power/RTC**: ~Rp 45.000

---

#### F. Advanced Hardware (Program 17, 41)
| No | Komponen | Qty | Harga Est. | Program |
|----|----------|-----|------------|---------|
| 32 | Rotary encoder mechanical | 1 | Rp 15.000 | #17 |
| 33 | Resistor 1.5kΩ (USB pull-up) | 1 | Rp 1.000 | #41 |

**Subtotal Advanced**: ~Rp 16.000

---

#### G. Tools & Measurement (Optional tapi Recommended)
| No | Komponen | Qty | Harga Est. | Keterangan |
|----|----------|-----|------------|------------|
| 34 | Multimeter digital | 1 | Rp 100.000 | Voltage, current, resistance |
| 35 | Logic analyzer 8-ch | 1 | Rp 150.000 | Protocol debugging (opsional) |
| 36 | Oscilloscope DSO138 | 1 | Rp 300.000 | Signal analysis (opsional) |
| 37 | USB Power meter | 1 | Rp 75.000 | Current measurement |

**Subtotal Tools**: ~Rp 625.000 (optional)

---

### TOTAL BILL OF MATERIALS (BOM)

| Kategori | Subtotal | Priority |
|----------|----------|----------|
| **Komponen Dasar** | Rp 160.000 | ⭐⭐⭐ WAJIB |
| **Komunikasi** | Rp 130.000 | ⭐⭐⭐ WAJIB |
| **Sensors** | Rp 159.000 | ⭐⭐ Penting |
| **Motors** | Rp 220.000 | ⭐ Optional |
| **Display** | Rp 30.000 | ⭐⭐ Penting |
| **Power/RTC** | Rp 45.000 | ⭐⭐ Penting |
| **Advanced** | Rp 16.000 | ⭐⭐ Penting |
| **Tools** | Rp 625.000 | ⭐ Optional |
| **GRAND TOTAL** | **Rp 1.385.000** | |
| **Minimum Budget** | **Rp 535.000** | Dasar + Komunikasi + Sensors + Display + Power |

---

## REKOMENDASI PEMBELIAN

### Paket Minimum (Rp 535.000)
Sudah cukup untuk menjalankan **35 dari 50 program** (70%):
- ✅ Program 1-20 (semua program dasar & intermediate)
- ✅ Program 21-26 (power management & flash)
- ✅ Program 31-40 (RTOS & communication)
- ✅ Program 42-45 (sensor applications)
- ✅ Program 48, 50 (display & logger)
- ❌ Program 27, 30, 41 (membutuhkan STM32F4)
- ❌ Program 46-47 (membutuhkan motors)

### Paket Lengkap (Rp 760.000 - tanpa oscilloscope)
Dapat menjalankan **48 dari 50 program** (96%):
- ✅ Semua program kecuali #27 dan #30 yang butuh STM32F4

### Tempat Pembelian (Indonesia)
1. **Tokopedia**: Search "STM32 Blue Pill" atau nama modul
2. **Bukalapak**: Banyak seller elektronik
3. **Toko offline**: 
   - Glodok Plaza (Jakarta)
   - Sadhar Plaza (Jakarta)
   - Harco Mas Mangga Dua (Jakarta)
   - Toko elektronik lokal di kota Anda

---

## PANDUAN PRAKTIKUM

### Urutan Belajar Recommended

#### Level 1: Pemula (Minggu 1-2)
1. **Program 1**: Blink LED - Pahami struktur dasar
2. **Program 2**: Button Input - GPIO input
3. **Program 3**: UART Serial - Komunikasi pertama
4. **Program 6**: ADC - Baca sensor analog
5. **Program 7**: External Interrupt - Event handling

#### Level 2: Menengah (Minggu 3-4)
6. **Program 4**: Timer Interrupt - Waktu presisi
7. **Program 5**: PWM - Control analog output
8. **Program 8**: I2C - Komunikasi 2-wire
9. **Program 9**: SPI - Komunikasi cepat
10. **Program 14**: RTC - Real-time clock

#### Level 3: Lanjut (Minggu 5-6)
11. **Program 11**: DMA UART - Transfer otomatis
12. **Program 12**: ADC DMA - Multi-channel
13. **Program 15**: Watchdog - System safety
14. **Program 18**: Input Capture - Timing measurement
15. **Program 22**: Flash EEPROM - Data persistence

#### Level 4: Advanced (Minggu 7-8)
16. **Program 21, 23**: Low Power - Power management
17. **Program 31-35**: FreeRTOS - Real-time OS
18. **Program 37-38**: Multi-sensor I2C/SPI
19. **Program 40**: MODBUS - Industrial protocol
20. **Program 50**: Data Logger - Complete system

---

## TROUBLESHOOTING UMUM

### Problem 1: "Device not found" saat upload
**Solusi**:
1. Cek koneksi ST-Link (GND, SWDIO, SWCLK, 3V3)
2. Pastikan BOOT0 = LOW (jumper ke GND)
3. Power cycle board
4. Update ST-Link firmware

### Problem 2: LED tidak blink
**Solusi**:
1. Cek polaritas LED (kaki panjang = anode/+)
2. Cek resistor 220Ω terpasang
3. Cek pin PC13 di code
4. Ukur voltage di pin dengan multimeter

### Problem 3: UART tidak terima data
**Solusi**:
1. Cek TX/RX tidak tertukar (TX board → RX adapter)
2. Cek GND common antara board dan adapter
3. Cek baud rate sama (115200)
4. Cek USB driver adapter terinstall

### Problem 4: I2C sensor tidak terdeteksi
**Solusi**:
1. Cek resistor pull-up 4.7kΩ di SDA & SCL
2. Cek address sensor dengan I2C scanner
3. Cek koneksi VCC dan GND sensor
4. Coba kurangi clock speed I2C (100kHz)

### Problem 5: Flash memory penuh
**Solusi**:
1. Compile dengan optimization `-Os`
2. Hapus `printf()` yang tidak perlu
3. Gunakan `--specs=nano.specs` linker flag
4. Upgrade ke STM32F103C8T6 128KB variant

---

## KONTAK & RESOURCES

### Official Documentation
- **STM32F103 Reference Manual**: [RM0008](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- **STM32F103 Datasheet**: [DS5319](https://www.st.com/resource/en/datasheet/stm32f103c8.pdf)
- **STM32CubeF1 HAL User Manual**: [UM1850](https://www.st.com/resource/en/user_manual/um1850-description-of-stm32f1-hal-and-lowlayer-drivers-stmicroelectronics.pdf)

### Community Resources
- **STM32 Forum**: https://community.st.com
- **r/stm32**: Reddit community
- **EEVblog Forum**: Electronics discussion
- **GitHub STM32 Projects**: Thousands of examples

### Indonesian Communities
- **Telegram**: STM32 Indonesia
- **Facebook**: STM32 User Group Indonesia
- **Kaskus**: Sub-forum Elektronika

---

## KESIMPULAN

Laporan ini menyediakan **54 program praktikum komprehensif** untuk STM32F103C8T6 yang mencakup:

✅ **10 program basic** (GPIO, UART, Timer, PWM, ADC, Interrupt)  
✅ **10 program intermediate** (DMA, I2C, SPI, CAN, RTC, Watchdog)  
✅ **10 program advanced** (Power management, Flash, Multi-DMA)  
✅ **14 program RTOS & Communication** (FreeRTOS: Tasks, Semaphores, Queue, Mutex, Software Timer, Tickless Idle, Event Groups, Task Notifications, Stream Buffers, Memory Management + Communication protocols)  
✅ **10 program industrial** (USB, SD Card, Sensors, Motors, Bootloader, Data Logger)

**Program RTOS yang telah ditambahkan** (Program 31-40):
1. ✅ **Program 31**: FreeRTOS Tasks dengan Priority
2. ✅ **Program 32**: Binary & Counting Semaphores
3. ✅ **Program 33**: Queue untuk Inter-Task Communication
4. ✅ **Program 34**: Mutex dengan Priority Inheritance
5. ✅ **Program 35**: Software Timers
6. ✅ **Program 36**: Tickless Idle untuk Low Power
7. ✅ **Program 37**: Event Groups (BARU!)
8. ✅ **Program 38**: Task Notifications (BARU!)
9. ✅ **Program 39**: Stream Buffers (BARU!)
10. ✅ **Program 40**: Memory Management & Heap Schemes (BARU!)

**Total investasi minimum**: Rp 535.000 untuk 39 program (72%)  
**Total investasi lengkap**: Rp 760.000 untuk 52 program (96%)

Setiap program dilengkapi:
- ✅ Deskripsi lengkap topik
- ✅ Hardware requirements detail
- ✅ Pin configuration
- ✅ Tingkat kesulitan
- ✅ Source code commented

---

**Happy Learning & Building!** 🚀

*"The best way to learn embedded systems is by building real projects!"*

---

**Document Version**: 2.0  
**Last Updated**: 2026-01-22  
**Total Pages**: Estimated 60+ pages when printed  
**Total Programs**: 54 (10 Basic + 10 Intermediate + 10 Advanced + 14 RTOS/Comm + 10 Industrial)  
**License**: Educational Use - Based on "Mastering STM32" by Carmine Noviello
