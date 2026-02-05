# Praktikum Sistem Embedded
## STM32F103C8T6 & ESP32 Development Projects

[![PlatformIO](https://img.shields.io/badge/PlatformIO-Ready-orange.svg)](https://platformio.org)
[![STM32](https://img.shields.io/badge/STM32-F103C8T6-blue.svg)](https://www.st.com/en/microcontrollers-microprocessors/stm32f103c8.html)
[![ESP32](https://img.shields.io/badge/ESP32-DevKitC-red.svg)](https://www.espressif.com/en/products/socs/esp32)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

---

## 📋 Daftar Isi

- [Tentang Praktikum](#-tentang-praktikum)
- [Struktur Folder](#-struktur-folder)
- [Hardware Requirements](#-hardware-requirements)
- [Software Requirements](#-software-requirements)
- [Instalasi dan Setup](#-instalasi-dan-setup)
- [Cara Menggunakan](#-cara-menggunakan)
- [Daftar Pertemuan](#-daftar-pertemuan)
- [Konvensi Penamaan](#-konvensi-penamaan)
- [Kompilasi Program](#-kompilasi-program)
- [Troubleshooting](#-troubleshooting)
- [Kontribusi](#-kontribusi)
- [Lisensi](#-lisensi)

---

## 🎯 Tentang Praktikum

Repository ini berisi **334 program praktikum** untuk pembelajaran sistem embedded menggunakan **STM32F103C8T6 (Blue Pill)** dan **ESP32 DevKit C**. Materi disusun secara sistematis dalam **12 pertemuan** yang mencakup topik dari dasar hingga advanced.

### Topik yang Dibahas:
- 🔌 Digital I/O & GPIO Control
- ⚡ Interrupt & Timer Programming
- 📡 Serial Communication (UART/USART)
- 📊 ADC, DAC, dan PWM
- 🔗 I2C Communication & Sensor Integration
- 💾 SPI Communication & Storage
- 🚀 DMA High-Throughput Transfer
- 🔄 FreeRTOS Fundamentals
- 🧵 FreeRTOS Queues & Synchronization
- 🎛️ FreeRTOS Advanced Features
- 🌐 Network Connectivity
- 🏭 Industrial Design Patterns

### Statistik Program:
- **174 Program STM32** (Blue Pill)
- **160 Program ESP32** (DevKit C)
- **334 Program Total**

---

## 📁 Struktur Folder

```
Praktikum Sistem Embedded/
│
├── Pertemuan_01_Fondasi_Digital_IO/          # 12 STM32 + 12 ESP32
│   ├── STM32/
│   │   ├── STM32_01_LED_Blink/
│   │   ├── STM32_02_Multi-LED_Running_Pattern/
│   │   └── ...
│   └── ESP32/
│       ├── ESP32_01_LED_Blink/
│       ├── ESP32_02_Multi-LED_Running_Pattern/
│       └── ...
│
├── Pertemuan_02_Interrupt_Timer/             # 12 STM32 + 12 ESP32
│   ├── STM32/
│   └── ESP32/
│
├── Pertemuan_03_Serial_CLI/                  # 12 STM32 + 12 ESP32
│   ├── STM32/
│   └── ESP32/
│
├── Pertemuan_04_ADC_DAC_PWM/                 # 12 STM32 + 25 ESP32
│   ├── STM32/
│   └── ESP32/
│
├── Pertemuan_05_I2C_Sensor/                  # 0 STM32 + 12 ESP32
│   └── ESP32/
│
├── Pertemuan_06_SPI_Storage/                 # 0 STM32 + 12 ESP32
│   └── ESP32/
│
├── Pertemuan_07_DMA_HighThroughput/          # 11 STM32 + 6 ESP32
│   ├── STM32/
│   └── ESP32/
│
├── Pertemuan_08_FreeRTOS_Fundamental/        # 13 STM32 + 6 ESP32
│   ├── STM32/
│   └── ESP32/
│
├── Pertemuan_09_FreeRTOS_Intermediate/       # 20 STM32 + 11 ESP32
│   ├── STM32/
│   └── ESP32/
│
├── Pertemuan_10_FreeRTOS_Advanced/           # 56 STM32 + 26 ESP32
│   ├── STM32/
│   └── ESP32/
│
├── Pertemuan_11_Network_Connectivity/        # 14 STM32 + 14 ESP32
│   ├── STM32/
│   └── ESP32/
│
└── Pertemuan_12_Industrial_Patterns/         # 12 STM32 + 12 ESP32
    ├── STM32/
    └── ESP32/
```

---

## 🔧 Hardware Requirements

### STM32 Development:
- **STM32F103C8T6 Blue Pill Board**
- ST-Link V2 Programmer
- USB to Serial Converter (CP2102/FTDI)
- Breadboard & Jumper Wires
- LED, Resistor, Push Button
- Sensor: DHT11/DHT22, BMP180/BMP280, OLED Display
- SD Card Module (SPI)
- W5500 Ethernet Module (untuk Pertemuan 11)

### ESP32 Development:
- **ESP32 DevKit C / WROOM-32**
- USB Cable (Micro USB)
- Breadboard & Jumper Wires
- LED, Resistor, Push Button
- Sensor: DHT11/DHT22, BMP180/BMP280, OLED Display
- SD Card Module (SPI)
- WiFi Network Access

### Common Components:
- Power Supply 3.3V/5V
- Logic Level Converter (if needed)
- Multimeter
- Oscilloscope (optional, for debugging)

---

## 💻 Software Requirements

### Required:
1. **[Visual Studio Code](https://code.visualstudio.com/)** (Latest version)
2. **[PlatformIO IDE Extension](https://platformio.org/install/ide?install=vscode)**
3. **Git** (for version control)

### Driver Installation:
- **STM32**: ST-Link V2 Driver
- **ESP32**: CP210x USB to UART Bridge Driver

### Supported Operating Systems:
- ✅ Windows 10/11
- ✅ Linux (Ubuntu 20.04+)
- ✅ macOS (Catalina+)

---

## 🚀 Instalasi dan Setup

### 1. Clone Repository

```bash
git clone https://github.com/username/praktikum-sistem-embedded.git
cd praktikum-sistem-embedded
```

### 2. Install PlatformIO Core

PlatformIO akan otomatis menginstall dependencies yang diperlukan saat pertama kali build project.

### 3. Setup Board

#### STM32F103C8T6:
- Hubungkan ST-Link V2 ke STM32 (SWDIO, SWCLK, GND, 3.3V)
- Verifikasi koneksi: `pio device list`

#### ESP32:
- Hubungkan ESP32 via USB
- Tekan tombol BOOT saat upload (jika diperlukan)
- Verifikasi port: `pio device list`

---

## 📖 Cara Menggunakan

### Open Project in VS Code

1. Buka VS Code
2. File → Open Folder → Pilih folder program (contoh: `Pertemuan_01_Fondasi_Digital_IO/STM32/STM32_01_LED_Blink`)
3. PlatformIO akan otomatis mendeteksi `platformio.ini`

### Build & Upload

#### Via VS Code PlatformIO Toolbar:
- ✅ **Build**: Klik ikon ✓ (Build)
- ⬆️ **Upload**: Klik ikon → (Upload)
- 🔌 **Monitor**: Klik ikon 🔌 (Serial Monitor)

#### Via Terminal:

```bash
# Build project
pio run

# Upload to board
pio run --target upload

# Open serial monitor
pio device monitor

# Clean build
pio run --target clean
```

### Configuration

Setiap program memiliki file `platformio.ini`:

```ini
[env:bluepill_f103c8]
platform = ststm32
board = bluepill_f103c8
framework = arduino
upload_protocol = stlink
monitor_speed = 115200
```

---

## 📚 Daftar Pertemuan

### Pertemuan 1: Fondasi Digital I/O (24 Programs)
**Topik**: GPIO basics, LED control, button input, debouncing
- ✅ 12 Program STM32
- ✅ 12 Program ESP32

**Program Utama**:
- LED Blink
- Multi-LED Running Pattern
- LED Breathing Effect
- Button Interrupt
- Multi-Button State Machine

---

### Pertemuan 2: Interrupt & Timer (24 Programs)
**Topik**: External interrupts, timer configuration, PWM generation
- ✅ 12 Program STM32
- ✅ 12 Program ESP32

**Program Utama**:
- EXTI Interrupt
- Timer Periodic
- PWM Generation
- Hardware Timer ISR
- Watchdog Timer

---

### Pertemuan 3: Serial Communication (24 Programs)
**Topik**: UART/USART, CLI interface, data parsing
- ✅ 12 Program STM32
- ✅ 12 Program ESP32

**Program Utama**:
- UART Echo
- Ring Buffer
- Command Parser
- Printf Redirect
- AT Command Parser

---

### Pertemuan 4: ADC, DAC & PWM (37 Programs)
**Topik**: Analog input, PWM motor control, sensor reading
- ✅ 12 Program STM32
- ✅ 25 Program ESP32

**Program Utama**:
- ADC Single/Multi Channel
- ADC DMA Continuous
- PWM Motor Control
- LED Dimming PWM
- Temperature Sensor Reading

---

### Pertemuan 5: I2C Sensor (12 Programs)
**Topik**: I2C protocol, sensor interfacing
- ❌ 0 Program STM32 (ESP32 only)
- ✅ 12 Program ESP32

**Program Utama**:
- I2C Scanner
- OLED Display (SSD1306)
- BMP280 Barometer
- DHT Sensor via I2C
- Multi-Sensor Polling

---

### Pertemuan 6: SPI Storage (12 Programs)
**Topik**: SPI communication, SD card, external memory
- ❌ 0 Program STM32 (ESP32 only)
- ✅ 12 Program ESP32

**Program Utama**:
- SPI Loopback Test
- SD Card Read/Write
- File System Operations
- SPI Flash Memory
- Data Logger

---

### Pertemuan 7: DMA High Throughput (17 Programs)
**Topik**: Direct Memory Access, high-speed data transfer
- ✅ 11 Program STM32
- ✅ 6 Program ESP32

**Program Utama**:
- DMA Memory to Memory
- UART RX/TX DMA
- ADC DMA Continuous
- SPI DMA Transfer
- Multi-DMA Concurrent

---

### Pertemuan 8: FreeRTOS Fundamental (19 Programs)
**Topik**: Task creation, scheduling, basic RTOS concepts
- ✅ 13 Program STM32
- ✅ 6 Program ESP32

**Program Utama**:
- Dynamic Task Injection
- Rate Monotonic Scheduling
- Absolute Timing Control
- Idle Task Hook
- Task Suspend/Resume

---

### Pertemuan 9: FreeRTOS Intermediate (31 Programs)
**Topik**: Queues, mutex, semaphores, synchronization
- ✅ 20 Program STM32
- ✅ 11 Program ESP32

**Program Utama**:
- Queue Operations
- Mailbox Pattern
- Recursive Mutex
- Priority Inversion Fix
- Counting Semaphore
- Barrier Synchronization

---

### Pertemuan 10: FreeRTOS Advanced (82 Programs)
**Topik**: Task notifications, timers, ISR, memory management
- ✅ 56 Program STM32
- ✅ 26 Program ESP32

**Program Utama**:
- Direct Task Notification
- Software Timer
- Deferred ISR Processing
- Stream Buffer
- Heap Usage Monitoring
- Static Memory Allocation
- Task State Machine
- System Health Monitor

---

### Pertemuan 11: Network Connectivity (28 Programs)
**Topik**: TCP/IP, HTTP, MQTT, WebSocket
- ✅ 14 Program STM32
- ✅ 14 Program ESP32

**Program Utama**:
- TCP Client/Server
- HTTP GET/POST
- MQTT Publisher/Subscriber
- WebSocket Communication
- UDP Broadcast
- DNS Lookup

---

### Pertemuan 12: Industrial Patterns (24 Programs)
**Topik**: Production-ready patterns, design patterns
- ✅ 12 Program STM32
- ✅ 12 Program ESP32

**Program Utama**:
- Modbus RTU Slave
- PubSub Pattern
- Active Object Pattern
- Watchdog Manager
- Graceful Shutdown
- Data Logger
- Simple CLI

---

## 📝 Konvensi Penamaan

### Folder Structure:
```
Board_Number_Program_Name/
```

### Contoh:
```
STM32_01_LED_Blink/
ESP32_12_WiFi_WebServer/
```

### Rules:
- Prefix: `STM32_` atau `ESP32_`
- Number: 2 digit (`01`, `02`, ..., `99`)
- Name: Snake_Case (menggunakan underscore)
- No spaces, special characters

---

## 🔨 Kompilasi Program

### Compile All Programs (Bash Script)

Buat file `compile_all.sh`:

```bash
#!/bin/bash

echo "=== Compiling All STM32 & ESP32 Programs ==="

# Compile all programs
for pertemuan in Pertemuan_*/; do
    echo ""
    echo "Processing: $pertemuan"
    
    # STM32
    if [ -d "$pertemuan/STM32" ]; then
        for prog in "$pertemuan/STM32"/STM32_*/; do
            if [ -f "$prog/platformio.ini" ]; then
                echo "  Building: $prog"
                cd "$prog"
                pio run > /dev/null 2>&1
                if [ $? -eq 0 ]; then
                    echo "    ✓ SUCCESS"
                else
                    echo "    ✗ FAILED"
                fi
                cd - > /dev/null
            fi
        done
    fi
    
    # ESP32
    if [ -d "$pertemuan/ESP32" ]; then
        for prog in "$pertemuan/ESP32"/ESP32_*/; do
            if [ -f "$prog/platformio.ini" ]; then
                echo "  Building: $prog"
                cd "$prog"
                pio run > /dev/null 2>&1
                if [ $? -eq 0 ]; then
                    echo "    ✓ SUCCESS"
                else
                    echo "    ✗ FAILED"
                fi
                cd - > /dev/null
            fi
        done
    fi
done

echo ""
echo "=== Compilation Complete ==="
```

Run:
```bash
chmod +x compile_all.sh
./compile_all.sh
```

---

## 🐛 Troubleshooting

### STM32 Issues

#### Upload Failed:
```
Error: libusb_open() failed with LIBUSB_ERROR_ACCESS
```
**Solution**: Run with sudo or setup udev rules
```bash
sudo usermod -a -G dialout $USER
```

#### Board Not Detected:
```
Error: Could not find device
```
**Solution**: Check ST-Link connection, install drivers

---

### ESP32 Issues

#### Upload Failed:
```
Serial port busy or not available
```
**Solution**: Close serial monitor, press BOOT button during upload

#### Port Permission Denied:
```bash
sudo chmod 666 /dev/ttyUSB0
```

---

### PlatformIO Issues

#### Library Not Found:
```
pio lib install [library_name]
```

#### Platform Update:
```bash
pio platform update
pio upgrade
```

#### Clean Build:
```bash
pio run --target clean
rm -rf .pio
```

---

## 🤝 Kontribusi

Kontribusi sangat diterima! Ikuti langkah berikut:

1. Fork repository ini
2. Buat branch baru (`git checkout -b feature/AmazingFeature`)
3. Commit perubahan (`git commit -m 'Add some AmazingFeature'`)
4. Push ke branch (`git push origin feature/AmazingFeature`)
5. Buat Pull Request

### Code Style:
- Gunakan indentasi 4 spasi
- Tambahkan comment untuk kode kompleks
- Follow Arduino/PlatformIO conventions
- Test sebelum commit

---

## 📄 Lisensi

Project ini dilisensikan dibawah **MIT License** - lihat file [LICENSE](LICENSE) untuk detail.

---

## 👥 Authors

- **Praktikum Sistem Embedded Team**
- Contributors: [List contributors]

---

## 📞 Kontak

- 📧 Email: praktikum@example.com
- 🌐 Website: https://example.com
- 💬 Discord: [Join Server]

---

## 🙏 Acknowledgments

- STMicroelectronics untuk STM32 HAL Library
- Espressif untuk ESP-IDF & Arduino-ESP32
- PlatformIO Team
- FreeRTOS Kernel Contributors
- Arduino Community

---

## 📊 Status Project

![GitHub last commit](https://img.shields.io/github/last-commit/username/repo)
![GitHub issues](https://img.shields.io/github/issues/username/repo)
![GitHub pull requests](https://img.shields.io/github/issues-pr/username/repo)
![GitHub](https://img.shields.io/github/license/username/repo)

---

**Happy Coding! 🚀**

> *"Learn by doing. Master embedded systems one program at a time."*

---

## 📈 Progress Tracking

| Pertemuan | STM32 | ESP32 | Total | Status |
|-----------|-------|-------|-------|--------|
| 01 - Fondasi Digital I/O | 12 | 12 | 24 | ✅ Complete |
| 02 - Interrupt Timer | 12 | 12 | 24 | ✅ Complete |
| 03 - Serial CLI | 12 | 12 | 24 | ✅ Complete |
| 04 - ADC DAC PWM | 12 | 25 | 37 | ✅ Complete |
| 05 - I2C Sensor | 0 | 12 | 12 | ✅ Complete |
| 06 - SPI Storage | 0 | 12 | 12 | ✅ Complete |
| 07 - DMA HighThroughput | 11 | 6 | 17 | ✅ Complete |
| 08 - FreeRTOS Fundamental | 13 | 6 | 19 | ✅ Complete |
| 09 - FreeRTOS Intermediate | 20 | 11 | 31 | ✅ Complete |
| 10 - FreeRTOS Advanced | 56 | 26 | 82 | ✅ Complete |
| 11 - Network Connectivity | 14 | 14 | 28 | ✅ Complete |
| 12 - Industrial Patterns | 12 | 12 | 24 | ✅ Complete |
| **TOTAL** | **174** | **160** | **334** | ✅ **Complete** |

---

*Last Updated: February 5, 2026*
