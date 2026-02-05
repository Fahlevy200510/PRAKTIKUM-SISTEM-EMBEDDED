# Referensi Lengkap
## Modul 05: DAC (Digital-to-Analog Converter) & PWM (Pulse Width Modulation)

---

## 📚 Dokumentasi Resmi

### STM32
| Dokumen | Deskripsi | Link |
|---------|-----------|------|
| **RM0008** | STM32F103 Reference Manual | [ST.com](https://www.st.com/resource/en/reference_manual/rm0008.pdf) |
| **AN3126** | Audio and Waveform Generation using DAC | [ST.com](https://www.st.com/resource/en/application_note/an3126.pdf) |
| **AN4013** | STM32 Timer Cookbook | [ST.com](https://www.st.com/resource/en/application_note/an4013.pdf) |
| **UM1785** | STM32Cube HAL and LL Drivers | [ST.com](https://www.st.com/resource/en/user_manual/um1785.pdf) |
| **DS5319** | STM32F103x8/B Datasheet | [ST.com](https://www.st.com/resource/en/datasheet/stm32f103c8.pdf) |

### ESP32
| Dokumen | Deskripsi | Link |
|---------|-----------|------|
| **Technical Reference** | ESP32 Technical Reference Manual | [Espressif](https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf) |
| **LEDC API Guide** | LED Control (PWM) API Reference | [ESP-IDF Docs](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html) |
| **DAC API Guide** | Digital-to-Analog Converter API | [ESP-IDF Docs](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/dac.html) |
| **Arduino-ESP32 Docs** | Arduino Core for ESP32 | [GitHub](https://docs.espressif.com/projects/arduino-esp32/) |

---

## 📖 Buku Referensi

### Embedded Systems
| Judul | Penulis | Chapter Relevan |
|-------|---------|-----------------|
| **Mastering STM32** | Carmine Noviello | Chapter 11: DAC, Chapter 10: Timers |
| **The Definitive Guide to ARM Cortex-M3** | Joseph Yiu | Chapter 8-9: Peripherals |
| **Programming with STM32** | Donald Norris | Chapter 7: PWM & DAC |
| **ESP32 Technical Tutorials** | Neil Cameron | Chapter on LEDC & DAC |
| **Make: AVR Programming** | Elliot Williams | PWM concepts (applicable) |

### Signal Processing & Electronics
| Judul | Penulis | Topik |
|-------|---------|-------|
| **The Art of Electronics** | Horowitz & Hill | DAC/ADC Fundamentals |
| **Digital Signal Processing** | Proakis & Manolakis | Sampling, Quantization |
| **Practical Electronics for Inventors** | Scherz & Monk | PWM Circuits |

---

## 🎓 Tutorial & Online Courses

### Video Tutorials
| Platform | Judul | Link |
|----------|-------|------|
| **YouTube - Controllers Tech** | STM32 DAC Tutorial | [Link](https://www.youtube.com/watch?v=DAC_STM32) |
| **YouTube - Phil's Lab** | STM32 PWM Generation | [Link](https://www.youtube.com/c/PhilsLab) |
| **YouTube - DroneBot Workshop** | ESP32 PWM Tutorial | [Link](https://www.youtube.com/c/Dronebotworkshop) |
| **YouTube - Random Nerd Tutorials** | ESP32 LEDC PWM | [Link](https://www.youtube.com/c/RandomNerdTutorials) |
| **YouTube - Andreas Spiess** | ESP32 DAC for Audio | [Link](https://www.youtube.com/c/AndreasSpiess) |

### Written Tutorials
| Website | Topik | Link |
|---------|-------|------|
| **DeepBlue Embedded** | STM32 DAC HAL Tutorial | [Link](https://deepbluembedded.com/stm32-dac/) |
| **Random Nerd Tutorials** | ESP32 PWM Guide | [Link](https://randomnerdtutorials.com/esp32-pwm-arduino-ide/) |
| **ControllersTech** | STM32 PWM Tutorial | [Link](https://controllerstech.com/pwm-in-stm32/) |
| **Last Minute Engineers** | Servo Motor Control | [Link](https://lastminuteengineers.com/servo-motor-arduino-tutorial/) |
| **Circuit Digest** | DAC Tutorial | [Link](https://circuitdigest.com/article/digital-to-analog-converter-dac) |

### Online Courses
| Platform | Course | Deskripsi |
|----------|--------|-----------|
| **Udemy** | Mastering Microcontroller with Embedded Driver Development | Comprehensive STM32 |
| **Coursera** | Introduction to Embedded Systems | ARM Basics |
| **edX** | Embedded Systems - Shape The World | TI LaunchPad (concepts applicable) |

---

## 📄 Application Notes & White Papers

### STM32 Application Notes
| Code | Title | Focus |
|------|-------|-------|
| **AN3126** | Audio and waveform generation using DAC | Audio synthesis with DAC |
| **AN4013** | STM32 cross-series timer overview | Timer configurations |
| **AN4277** | Using STM32 Discovery kit as audio recorder | Audio applications |
| **AN3265** | LED lamp control with STM32 | PWM LED control |
| **AN2592** | How to achieve the best ADC accuracy | DAC/ADC calibration |

### Application Notes Umum
| Publisher | Title | Topic |
|-----------|-------|-------|
| **Texas Instruments** | SLAA533 | LED Lighting with PWM |
| **Analog Devices** | MT-013 | Evaluating DAC Accuracy |
| **Microchip** | AN539 | Using PWM to Generate Analog Output |
| **NXP** | AN3174 | PWM Motor Control |

---

## 🔧 Libraries & Code Examples

### STM32
```
GitHub Repositories:
├── STM32CubeF1 (Official HAL/LL Drivers)
│   └── https://github.com/STMicroelectronics/STM32CubeF1
├── stm32-dac-examples
│   └── https://github.com/topics/stm32-dac
└── stm32-pwm-examples
    └── https://github.com/topics/stm32-pwm
```

### ESP32
```
GitHub Repositories:
├── arduino-esp32 (Official Arduino Core)
│   └── https://github.com/espressif/arduino-esp32
├── esp-idf (Official ESP-IDF Framework)
│   └── https://github.com/espressif/esp-idf
├── ESP32Servo (Servo Library)
│   └── https://github.com/madhephaestus/ESP32Servo
└── ESP32-audioI2S (Audio with DAC)
    └── https://github.com/schreibfaul1/ESP32-audioI2S
```

---

## 🧪 Tools & Software

### Development
| Tool | Purpose | Platform |
|------|---------|----------|
| **STM32CubeIDE** | STM32 Development | Windows/Linux/Mac |
| **STM32CubeMX** | Configuration Generator | Windows/Linux/Mac |
| **PlatformIO** | Multi-platform IDE | VS Code Extension |
| **Arduino IDE** | ESP32/Arduino Development | All platforms |
| **ESP-IDF** | ESP32 Native Framework | All platforms |

### Debugging & Analysis
| Tool | Purpose | Link |
|------|---------|------|
| **PulseView** | Logic Analyzer Software | [Link](https://sigrok.org/wiki/PulseView) |
| **Audacity** | Audio Analysis | [Link](https://www.audacityteam.org/) |
| **Oscilloscope Apps** | Mobile Oscilloscope | Various |
| **Serial Studio** | Serial Data Visualization | [Link](https://serial-studio.github.io/) |

### Simulation
| Tool | Purpose | Link |
|------|---------|------|
| **Wokwi** | ESP32 Online Simulator | [Link](https://wokwi.com/) |
| **Proteus** | STM32 Simulation | Commercial |
| **LTspice** | Circuit Simulation | [Link](https://www.analog.com/ltspice) |
| **Falstad Circuit Simulator** | Online Circuit Sim | [Link](https://www.falstad.com/circuit/) |

---

## 📊 Datasheet Komponen Pendukung

### Audio
| Komponen | Tipe | Datasheet |
|----------|------|-----------|
| **PAM8403** | Audio Amplifier | [Link](https://www.diodes.com/assets/Datasheets/PAM8403.pdf) |
| **LM386** | Audio Amplifier | [Link](https://www.ti.com/lit/ds/symlink/lm386.pdf) |
| **MAX98357A** | I2S Audio DAC | [Link](https://datasheets.maximintegrated.com/en/ds/MAX98357A.pdf) |

### Motor Control
| Komponen | Tipe | Datasheet |
|----------|------|-----------|
| **L298N** | Dual H-Bridge | [Link](https://www.st.com/resource/en/datasheet/l298.pdf) |
| **L293D** | Quad Half H-Bridge | [Link](https://www.ti.com/lit/ds/symlink/l293.pdf) |
| **TB6612FNG** | Motor Driver | [Link](https://toshiba.semicon-storage.com/info/docget.jsp?did=10660&prodName=TB6612FNG) |

### Servo
| Komponen | Spesifikasi | Datasheet |
|----------|-------------|-----------|
| **SG90** | Micro Servo 180° | [Link](http://www.ee.ic.ac.uk/pcheung/teaching/DE1_EE/stores/sg90_datasheet.pdf) |
| **MG996R** | High Torque Servo | Various sources |
| **DS3218** | High Voltage Servo | Various sources |

---

## 🌐 Forum & Komunitas

### Forums
| Platform | Community | Link |
|----------|-----------|------|
| **ST Community** | STM32 Official Forum | [Link](https://community.st.com/) |
| **ESP32 Forum** | Espressif Forum | [Link](https://esp32.com/) |
| **Arduino Forum** | Arduino Community | [Link](https://forum.arduino.cc/) |
| **EEVBlog Forum** | Electronics Forum | [Link](https://www.eevblog.com/forum/) |
| **Stack Overflow** | Programming Q&A | Tag: stm32, esp32 |

### Reddit Communities
| Subreddit | Focus |
|-----------|-------|
| r/embedded | Embedded Systems |
| r/stm32 | STM32 Specific |
| r/esp32 | ESP32 Specific |
| r/AskElectronics | Electronics Help |

### Discord Servers
| Server | Focus |
|--------|-------|
| **ESP32 Developers** | ESP32 Community |
| **Embedded Systems** | General Embedded |
| **Electronics** | Electronics Hobbyists |

---

## 📱 Mobile Apps (Tools)

| App | Platform | Purpose |
|-----|----------|---------|
| **Oscilloscope Pro** | Android | Audio oscilloscope |
| **Function Generator** | Android/iOS | Signal generation |
| **Electronics Toolkit** | Android/iOS | Calculator & references |
| **Fritzing** | Desktop | Schematic drawing |

---

## 📝 Catatan Penggunaan Referensi

### Prioritas Referensi
1. **Dokumentasi Resmi** - Selalu cek datasheet dan reference manual
2. **Application Notes** - Untuk implementasi praktis
3. **Buku Teks** - Untuk pemahaman konsep mendalam
4. **Tutorial Online** - Untuk quick start dan troubleshooting

### Tips Mencari Referensi
```
Search Keywords:
├── STM32: "STM32F103 DAC HAL", "STM32 PWM Timer tutorial"
├── ESP32: "ESP32 LEDC PWM", "ESP32 DAC waveform"
├── Konsep: "R-2R ladder DAC", "PWM duty cycle calculation"
└── Troubleshooting: "DAC output stuck", "PWM frequency calculation"
```

### Verifikasi Informasi
- Cross-check dengan datasheet resmi
- Test implementasi pada hardware
- Perhatikan versi software/library
- Cek tanggal publikasi (terutama untuk tutorial)

---

## 📅 Update Log

| Tanggal | Update |
|---------|--------|
| 2024-01 | Initial reference compilation |
| 2024-02 | Added ESP32 LEDC documentation |
| 2024-03 | Added audio-related references |

---

**Catatan:** Selalu cek versi terbaru dari dokumentasi resmi karena API dan fitur dapat berubah.
