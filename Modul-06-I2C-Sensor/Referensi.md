# Referensi Modul 06: I2C Bus dan Sensor Integration

## 📚 Dokumentasi Resmi

### STM32 Documentation
| Dokumen | Deskripsi | Link |
|---------|-----------|------|
| **RM0008** | STM32F1 Reference Manual - I2C Chapter | [ST.com](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf) |
| **AN4235** | I2C Timing Configuration Tool | [ST.com](https://www.st.com/resource/en/application_note/an4235-i2c-timing-configuration-tool-for-stm32f3xxxx-and-stm32f0xxxx-microcontrollers-stmicroelectronics.pdf) |
| **HAL I2C Driver** | STM32 HAL I2C Documentation | [ST GitHub](https://github.com/STMicroelectronics/stm32f1xx_hal_driver) |
| **AN2824** | STM32 I2C Optimized Examples | [ST.com](https://www.st.com/resource/en/application_note/an2824-stm32f10xxx-i2c-optimized-examples-stmicroelectronics.pdf) |

### ESP32 Documentation
| Dokumen | Deskripsi | Link |
|---------|-----------|------|
| **ESP32 TRM** | Technical Reference Manual - I2C | [Espressif](https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf) |
| **ESP-IDF I2C** | ESP-IDF I2C Driver API | [Espressif Docs](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2c.html) |
| **Arduino Wire** | ESP32 Wire Library | [Arduino Reference](https://www.arduino.cc/reference/en/language/functions/communication/wire/) |

### I2C Standard
| Dokumen | Deskripsi | Link |
|---------|-----------|------|
| **I2C Specification** | NXP I2C-bus Specification v6 | [NXP](https://www.nxp.com/docs/en/user-guide/UM10204.pdf) |
| **SMBus Spec** | System Management Bus Specification | [SMBus.org](http://smbus.org/specs/) |

---

## 📖 Datasheet Komponen

### BME280 - Environmental Sensor
| Item | Link |
|------|------|
| Datasheet | [Bosch Sensortec](https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf) |
| Arduino Library | [Adafruit BME280](https://github.com/adafruit/Adafruit_BME280_Library) |
| Official Driver | [Bosch GitHub](https://github.com/BoschSensortec/BME280_driver) |

### SSD1306 - OLED Display
| Item | Link |
|------|------|
| Datasheet | [Solomon Systech](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf) |
| Arduino Library | [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306) |
| U8g2 Alternative | [U8g2 Library](https://github.com/olikraus/u8g2) |

### DS3231 - RTC
| Item | Link |
|------|------|
| Datasheet | [Maxim/Analog](https://www.analog.com/media/en/technical-documentation/data-sheets/DS3231.pdf) |
| Arduino Library | [RTClib](https://github.com/adafruit/RTClib) |

### 24LC256 - EEPROM
| Item | Link |
|------|------|
| Datasheet | [Microchip](https://ww1.microchip.com/downloads/en/DeviceDoc/24AA256-24LC256-24FC256-Data-Sheet-20001203W.pdf) |

---

## 🎓 Tutorial

### I2C Fundamentals
1. **Sparkfun I2C Tutorial** - [learn.sparkfun.com/tutorials/i2c](https://learn.sparkfun.com/tutorials/i2c)
2. **Understanding I2C** - [Analog.com](https://www.analog.com/en/technical-articles/i2c-primer-what-is-i2c-part-1.html)

### STM32 I2C
1. **STM32 I2C Tutorial** - [DeepBlue Embedded](https://deepbluembedded.com/stm32-i2c-tutorial-hal-examples-slave-dma/)
2. **STM32 HAL I2C** - [Controllers Tech](https://controllerstech.com/stm32-i2c-configuration-using-registers/)

### ESP32 I2C
1. **ESP32 I2C Tutorial** - [RandomNerdTutorials](https://randomnerdtutorials.com/esp32-i2c-communication-arduino-ide/)
2. **ESP32 Multiple I2C** - [LastMinuteEngineers](https://lastminuteengineers.com/esp32-i2c-tutorial/)

### Sensor Integration
1. **BME280 with ESP32** - [RandomNerdTutorials](https://randomnerdtutorials.com/esp32-bme280-arduino-ide-pressure-temperature-humidity/)
2. **SSD1306 OLED** - [LastMinuteEngineers](https://lastminuteengineers.com/oled-display-esp32-tutorial/)
3. **DS3231 RTC** - [LastMinuteEngineers](https://lastminuteengineers.com/ds3231-rtc-arduino-tutorial/)

---

## 📝 Paper/Artikel

### Academic Resources
1. "Analysis of I2C Protocol for Embedded Systems" - IEEE
2. "Multi-Sensor Data Fusion using I2C" - Sensors Journal
3. "Low-Power I2C Communication Optimization" - ACM

### Books
| Judul | Penulis | Topik |
|-------|---------|-------|
| **Mastering STM32** (2nd Ed) | Carmine Noviello | Chapter 13-14: I2C |
| **Programming with STM32** | Donald Norris | I2C Communication |
| **I2C Bus: Theory to Practice** | Dominique Paret | Complete I2C Reference |

---

## 🎥 Video

### I2C Basics
1. **Phil's Lab - I2C Protocol Explained** - [YouTube](https://www.youtube.com/watch?v=_fgWQ3TIhyE)
2. **EEVblog - I2C Tutorial** - [YouTube](https://www.youtube.com/watch?v=ERLj4D3gA_w)

### STM32 I2C
1. **Controllerstech - STM32 I2C Masterclass** - [YouTube Playlist](https://www.youtube.com/playlist?list=PLfIJKC1ud8ggRvaEsMjSEDazoBvt4IPqU)
2. **DigiKey - STM32 HAL I2C** - [YouTube](https://www.youtube.com/watch?v=isOekyygpR8)

### ESP32 I2C
1. **DroneBot Workshop - ESP32 I2C** - [YouTube](https://www.youtube.com/watch?v=7kYvbMR0OAA)
2. **Andreas Spiess - ESP32 Multiple I2C** - [YouTube](https://www.youtube.com/watch?v=2_pDdGdknPA)

---

## 🛠️ Tools

| Tool | Deskripsi | Link |
|------|-----------|------|
| **PlatformIO** | IDE untuk STM32 & ESP32 | [platformio.org](https://platformio.org/) |
| **STM32CubeIDE** | Official ST IDE | [ST.com](https://www.st.com/en/development-tools/stm32cubeide.html) |
| **Wokwi** | ESP32 Simulator | [wokwi.com](https://wokwi.com/) |
| **Logic Analyzer** | Saleae Logic | [saleae.com](https://www.saleae.com/) |

---

## 📋 I2C Address Quick Reference

| Device | 7-bit Address | Read | Write |
|--------|---------------|------|-------|
| BME280 | 0x76/0x77 | 0xED/0xEF | 0xEC/0xEE |
| SSD1306 | 0x3C/0x3D | 0x79/0x7B | 0x78/0x7A |
| DS3231 | 0x68 | 0xD1 | 0xD0 |
| 24LC256 | 0x50-0x57 | 0xA1-0xAF | 0xA0-0xAE |

---

*Terakhir diperbarui: 2024*
