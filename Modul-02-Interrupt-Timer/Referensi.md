# Referensi
## Modul 02: Interrupt dan Timer

---

## 📚 Dokumentasi Resmi

### STM32
1. **STM32F103C8 Reference Manual (RM0008)**
   - URL: https://www.st.com/resource/en/reference_manual/rm0008.pdf
   - Chapter 10: Interrupts and events
   - Chapter 13-15: General-purpose timers
   - Wajib baca untuk pemahaman mendalam register-level

2. **STM32F103C8 Datasheet**
   - URL: https://www.st.com/resource/en/datasheet/stm32f103c8.pdf
   - Pinout dan electrical characteristics
   - Timer specifications dan limitations

3. **ARM Cortex-M3 Technical Reference Manual**
   - URL: https://developer.arm.com/documentation/ddi0337/
   - NVIC (Nested Vectored Interrupt Controller)
   - Exception handling dan priority

4. **STM32 Application Notes**
   - AN4228: STM32 EXTI Application Note
     - URL: https://www.st.com/resource/en/application_note/an4228.pdf
   - AN4776: Timer cookbook for STM32 microcontrollers
     - URL: https://www.st.com/resource/en/application_note/an4776.pdf
   - AN4013: Timer overview and timer firmware
     - URL: https://www.st.com/resource/en/application_note/an4013.pdf

### ESP32
1. **ESP32 Technical Reference Manual**
   - URL: https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf
   - Chapter 5: Interrupts
   - Chapter 17: Timer Group
   - Wajib baca untuk understanding hardware timer

2. **ESP-IDF Programming Guide**
   - Interrupt Allocation: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/intr_alloc.html
   - Timer Group: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gptimer.html
   - GPIO Interrupt: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html

3. **ESP32 Datasheet**
   - URL: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
   - GPIO specifications
   - Timer specifications

---

## 📖 Buku Referensi

### Wajib
1. **Mastering STM32** - Carmine Noviello
   - Publisher: Leanpub
   - URL: https://leanpub.com/mastering-stm32
   - Chapter 11: Timers
   - Chapter 12: Interrupts
   - Penjelasan praktis dengan banyak contoh code

2. **The Definitive Guide to ARM Cortex-M3 and Cortex-M4 Processors** - Joseph Yiu
   - Publisher: Newnes (3rd Edition)
   - ISBN: 978-0124080829
   - Chapter 7: Exceptions
   - Chapter 8: NVIC
   - Referensi standar untuk ARM architecture

### Pendukung
3. **Programming with STM32: Getting Started with the Nucleo Board and C/C++** - Donald Norris
   - Publisher: McGraw-Hill
   - ISBN: 978-1260031317
   - Beginner-friendly dengan project examples

4. **Beginning STM32: Developing with FreeRTOS, libopencm3, and GCC** - Warren Gay
   - Publisher: Apress
   - ISBN: 978-1484236239
   - Open-source approach dengan libopencm3

5. **Internet of Things with ESP8266 and ESP32** - Marco Schwartz
   - Publisher: Packt
   - Practical projects dengan ESP32

---

## 🔗 Tutorial Online

### STM32 Tutorials
1. **DeepBlue Embedded - STM32 Tutorials**
   - URL: https://deepbluembedded.com/stm32-tutorials/
   - STM32 External Interrupt Example
   - STM32 Timers Tutorial
   - Excellent practical examples

2. **STM32 HAL Tutorial Series**
   - URL: https://www.youtube.com/playlist?list=PLnMKNibPkDnGtuIl5v0CvC81Am7SKpj02
   - Video tutorial lengkap dari Controller Tech
   - Hands-on dengan STM32CubeIDE

3. **Embedded Systems Programming on ARM Cortex-M3/M4**
   - URL: https://www.udemy.com/course/embedded-system-programming-on-arm-cortex-m3m4/
   - Course lengkap di Udemy
   - Mencakup interrupt dan timer detail

4. **STM32 Guide - Phil's Lab**
   - URL: https://www.youtube.com/c/PhilsLab
   - Practical PCB design dan firmware
   - High quality production

### ESP32 Tutorials
1. **Random Nerd Tutorials - ESP32**
   - URL: https://randomnerdtutorials.com/esp32-tutorials/
   - ESP32 Interrupt Tutorial
   - ESP32 Timer Tutorial
   - Beginner-friendly dengan gambar

2. **ESP32 Official Examples**
   - URL: https://github.com/espressif/arduino-esp32/tree/master/libraries
   - Official Arduino-ESP32 examples
   - Timer examples dalam library

3. **Last Minute Engineers**
   - URL: https://lastminuteengineers.com/esp32-arduino-ide-tutorial/
   - Step-by-step ESP32 tutorials
   - Diagram wiring yang jelas

---

## 📄 Paper & Artikel Ilmiah

1. **"A Survey on Interrupt Handling in Real-Time Operating Systems"**
   - Journal of Systems Architecture
   - DOI: 10.1016/j.sysarc.2020.101813
   - Comprehensive survey tentang interrupt handling

2. **"Real-Time Response in Embedded Systems"**
   - ACM Computing Surveys
   - DOI: 10.1145/3340555
   - Best practices untuk real-time response

3. **"Timer Design Patterns for Embedded Systems"**
   - Embedded Systems Conference Paper
   - Patterns untuk timer-based applications

4. **"Comparison of STM32 and ESP32 for IoT Applications"**
   - IEEE IoT Conference 2021
   - Performance comparison kedua platform

---

## 🎥 Video Tutorial

### YouTube Channels
1. **Controllers Tech**
   - URL: https://www.youtube.com/c/ControllersTech
   - STM32 Timer series lengkap
   - Interrupt handling tutorial

2. **Phil's Lab**
   - URL: https://www.youtube.com/c/PhilsLab
   - Professional-grade tutorials
   - Hardware and firmware

3. **Andreas Spiess**
   - URL: https://www.youtube.com/c/AndreasSpiess
   - ESP32 tutorials excellent
   - Real-world projects

4. **EE Enthusiast**
   - URL: https://www.youtube.com/c/EEEnthusiast
   - ARM programming tutorials
   - Beginner-friendly

### Specific Videos
1. **"STM32 External Interrupt - Complete Tutorial"**
   - Controllers Tech
   - Duration: 25 minutes
   - Comprehensive EXTI coverage

2. **"ESP32 Timer Tutorial - From Basics to Advanced"**
   - Random Nerd Tutorials
   - Duration: 30 minutes
   - Practical timer examples

3. **"Understanding NVIC - ARM Cortex-M3"**
   - EE Enthusiast
   - Duration: 20 minutes
   - Architecture deep-dive

---

## 🛠️ Tools & Software

### Development Environment
1. **PlatformIO IDE**
   - URL: https://platformio.org/
   - Cross-platform development
   - Mendukung STM32 dan ESP32

2. **STM32CubeIDE**
   - URL: https://www.st.com/en/development-tools/stm32cubeide.html
   - Official ST IDE
   - Code generation dan debugging

3. **VS Code + Extensions**
   - PlatformIO Extension
   - Cortex-Debug Extension
   - Serial Monitor Extension

### Debugging Tools
1. **ST-Link Utility**
   - Programming dan debugging STM32
   - Memory viewer

2. **Segger J-Link**
   - Professional debugging
   - Real-time trace

3. **Logic Analyzer / Oscilloscope**
   - Saleae Logic Pro
   - DSO Nano
   - Untuk timing analysis

### Simulation
1. **Wokwi ESP32 Simulator**
   - URL: https://wokwi.com/
   - Free online simulator
   - Mendukung interrupt dan timer

2. **Proteus Design Suite**
   - STM32 simulation
   - Circuit design dan testing

---

## 📱 Mobile Apps

1. **Serial Bluetooth Terminal**
   - Android/iOS
   - Debugging via Bluetooth

2. **nRF Connect**
   - BLE debugging
   - Useful untuk ESP32 BLE

---

## 🔧 Hardware References

### Development Boards
1. **STM32F103C8T6 "Blue Pill"**
   - Schematic: https://github.com/rogerclarkmelbourne/Arduino_STM32/blob/master/docs/bluepill_schematics.pdf
   - Pinout diagram
   - Known issues (3.3V regulator, USB resistor)

2. **ESP32 DevKitC**
   - Schematic: https://dl.espressif.com/dl/schematics/esp32_devkitc_v4-sch.pdf
   - Pinout diagram
   - Boot mode selection

### Components
1. **Push Button Datasheet**
   - Tactile switch specifications
   - Debounce characteristics (~5-20ms)

2. **LED Specifications**
   - Forward voltage drop
   - Current requirements

---

## 📝 Code Repositories

1. **Arduino-STM32 Official**
   - URL: https://github.com/stm32duino/Arduino_Core_STM32
   - Official STM32 Arduino core
   - Timer examples included

2. **Arduino-ESP32 Official**
   - URL: https://github.com/espressif/arduino-esp32
   - Official ESP32 Arduino core
   - Extensive examples

3. **PlatformIO Examples**
   - URL: https://github.com/platformio/platform-ststm32/tree/master/examples
   - STM32 examples untuk PlatformIO

4. **Praktikum Repository** (Course-specific)
   - Semua kode praktikum tersedia
   - Template projects
   - Solution examples

---

## 📋 Cheat Sheets & Quick References

1. **ARM Cortex-M3 Quick Reference**
   - NVIC registers
   - Exception priority
   - URL: ARM Developer website

2. **ESP32 Pinout Reference**
   - GPIO capabilities
   - Interrupt-capable pins
   - URL: Random Nerd Tutorials

3. **Timer Calculation Formulas**
   - STM32: Timer_Freq = APB_Clock / (PSC + 1)
   - ESP32: Timer_Freq = 80MHz / Prescaler
   - Period = Counter_Max / Timer_Freq

---

## 📞 Community & Support

1. **STM32 Community Forum**
   - URL: https://community.st.com/
   - Official ST support
   - Active community

2. **ESP32 Forum**
   - URL: https://www.esp32.com/
   - Official Espressif forum
   - Technical discussions

3. **Stack Overflow**
   - Tags: [stm32], [esp32], [interrupt], [timer]
   - Quick problem solving

4. **Reddit Communities**
   - r/stm32
   - r/esp32
   - r/embedded

---

## ⚠️ Catatan Penting

1. **Dokumentasi resmi SELALU menjadi sumber utama** - datasheet dan reference manual adalah authoritative source

2. **Versi library dan SDK berubah** - selalu cek compatibility dengan versi yang digunakan

3. **Code examples online mungkin outdated** - verifikasi dengan dokumentasi resmi

4. **Community forums bagus untuk troubleshooting** - tapi jawaban perlu diverifikasi

5. **Video tutorials bagus untuk visual learning** - tapi ikuti dengan hands-on practice
