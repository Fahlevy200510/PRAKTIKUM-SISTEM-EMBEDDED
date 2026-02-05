# Prompt untuk Pembuatan PPT - Bagian 1
## Modul 06: I2C Bus dan Sensor Integration

### Instruksi Umum untuk AI Image Generator
- Style: Modern, professional, technical illustration
- Color scheme: Blue (#0066CC), Green (#00AA55), White background
- Resolution: 1920x1080 (16:9 aspect ratio)
- Font style: Clean sans-serif (Roboto, Open Sans)

---

## SLIDE 1 - Judul Utama

**Prompt:**
```
Create a professional presentation title slide with:
- Main title "I2C Bus dan Sensor Integration" in large bold blue text
- Subtitle "Modul 06 - Praktikum Sistem Embedded" 
- Background: Modern circuit board pattern with I2C data lines highlighted
- Visual elements: BME280, OLED SSD1306, DS3231 RTC, EEPROM chip icons
- Two microcontroller boards: STM32 Blue Pill and ESP32 DevKit
- I2C bus connections shown as two parallel lines (SDA, SCL) connecting all devices
- University/institution logo placeholder in corner
- Clean, professional, educational style
```

---

## SLIDE 2 - Capaian Pembelajaran

**Prompt:**
```
Create an educational slide showing learning objectives with:
- Title "Capaian Pembelajaran" at top
- 7 numbered objectives with checkmark icons:
  1. Memahami prinsip kerja protokol I2C
  2. Mengidentifikasi sinyal SDA dan SCL
  3. Mengkonfigurasi I2C sebagai Master
  4. Membaca data dari sensor I2C (BME280, DS3231, OLED)
  5. Mengakses EEPROM eksternal
  6. Menangani multiple device pada I2C bus
  7. Troubleshooting komunikasi I2C
- Background: Soft gradient with subtle circuit patterns
- Icons representing each objective (sensor, clock, display, memory)
```

---

## SLIDE 3 - Apa itu I2C?

**Prompt:**
```
Create an infographic explaining I2C protocol:
- Title "Inter-Integrated Circuit (I2C)"
- Developed by Philips (NXP) in 1982
- Key features in visual boxes:
  • 2-wire communication (SDA + SCL)
  • Multi-master, multi-slave capable
  • 7-bit addressing (127 devices)
  • Speed: up to 3.4 Mbps
- Simple diagram showing two wires connecting multiple devices
- Historical timeline element
- Philips/NXP logo reference
```

---

## SLIDE 4 - I2C vs SPI vs UART

**Prompt:**
```
Create a comparison table slide with:
- Title "Perbandingan Protokol Serial"
- Three columns: I2C, SPI, UART
- Rows comparing:
  • Number of wires (2, 4+, 2)
  • Topology (Bus, Point-to-point, P2P)
  • Max devices (127, unlimited, 1)
  • Duplex (Half, Full, Full)
  • Max speed (3.4M, 10M+, 1M)
  • Complexity (Medium, Low, Low)
- Visual icons for each protocol
- Highlight I2C column with accent color
- Use checkmarks and X marks for pros/cons
```

---

## SLIDE 5 - Arsitektur I2C Bus

**Prompt:**
```
Create a detailed I2C bus architecture diagram:
- Title "Arsitektur I2C Bus"
- Show Vcc power rail at top (3.3V)
- Two pull-up resistors (4.7kΩ labeled) connecting to SDA and SCL lines
- Horizontal bus lines for SDA and SCL
- Connected devices:
  • Master (MCU) - highlighted
  • Slave 1 (Sensor)
  • Slave 2 (EEPROM)
  • Slave 3 (RTC)
- Ground rail at bottom
- Open-drain output symbol
- Color coding: SDA=green, SCL=blue
- Labels for all components
```

---

## SLIDE 6 - Sinyal I2C

**Prompt:**
```
Create a slide explaining I2C signals:
- Title "Sinyal SDA dan SCL"
- Two sections:
  1. SDA (Serial Data):
     - Bidirectional data line
     - Open-drain output
     - Data valid when SCL HIGH
  2. SCL (Serial Clock):
     - Clock from Master
     - Open-drain output
     - Controls timing
- Simple waveform showing both signals
- Pull-up resistor symbol
- Bidirectional arrow for SDA
- Unidirectional arrow for SCL
```

---

## SLIDE 7 - I2C Timing Diagram

**Prompt:**
```
Create a detailed I2C timing diagram showing:
- Title "I2C Timing Diagram"
- Three sections:
  1. START Condition: SDA goes LOW while SCL is HIGH
  2. Data Transfer: Multiple bits with setup/hold times labeled
  3. STOP Condition: SDA goes HIGH while SCL is HIGH
- Clear labeling of:
  • Setup time
  • Hold time
  • Data valid region
- SDA and SCL as two separate waveforms
- Annotations with arrows pointing to key transitions
- Clean, technical drawing style
```

---

## SLIDE 8 - I2C Frame Format

**Prompt:**
```
Create a slide showing I2C frame structure:
- Title "Format Frame I2C (7-bit Address)"
- Visual frame diagram with boxes:
  | START | 7-bit Address | R/W | ACK | 8-bit Data | ACK | STOP |
  |   1   |  A6...A0      |  1  |  1  |   D7...D0  |  1  |   1  |
- Color coding for each field type
- R/W bit explanation: 0=Write, 1=Read
- ACK/NACK explanation with SDA LOW/HIGH
- Bit counting for each field
- Total bits calculation
```

---

## SLIDE 9 - I2C Speed Modes

**Prompt:**
```
Create an infographic showing I2C speed modes:
- Title "Mode Kecepatan I2C"
- Horizontal speed scale/meter visualization
- Four modes with icons:
  1. Standard Mode: 100 kbps (Sensors, EEPROM)
  2. Fast Mode: 400 kbps (Display, IMU)
  3. Fast Mode Plus: 1 Mbps (High-speed sensors)
  4. High Speed: 3.4 Mbps (Special applications)
- Application examples for each mode
- Speed comparison bar chart
- Color gradient from slow (cool) to fast (warm)
```

---

## SLIDE 10 - I2C pada STM32F103

**Prompt:**
```
Create a technical slide for STM32 I2C:
- Title "I2C pada STM32F103"
- STM32F103 chip diagram/icon
- Features list with icons:
  • 2 I2C peripherals (I2C1, I2C2)
  • 7-bit and 10-bit addressing
  • Multi-master capable
  • DMA support
  • SMBus compatible
- Pin mapping table:
  | Peripheral | SCL | SDA | Remap SCL | Remap SDA |
  | I2C1       | PB6 | PB7 | PB8       | PB9       |
  | I2C2       | PB10| PB11| -         | -         |
- STM32 Blue Pill board image reference
```

---

## SLIDE 11 - I2C pada ESP32

**Prompt:**
```
Create a technical slide for ESP32 I2C:
- Title "I2C pada ESP32"
- ESP32 chip/board diagram
- Features list with icons:
  • 2 I2C controllers (I2C_NUM_0, I2C_NUM_1)
  • Flexible GPIO mapping (any GPIO)
  • Master and Slave mode
  • Internal pull-up support
  • Clock stretching
- Default pins highlight: SDA=GPIO21, SCL=GPIO22
- ESP32 DevKit pinout reference
- Comparison note with STM32
```

---

## SLIDE 12 - BME280 Sensor Overview

**Prompt:**
```
Create a product overview slide for BME280:
- Title "BME280 - Environmental Sensor"
- BME280 module photo/illustration
- Specification boxes:
  • Temperature: -40°C to +85°C, ±1.0°C
  • Humidity: 0-100% RH, ±3% RH
  • Pressure: 300-1100 hPa, ±1 hPa
- I2C Address: 0x76 or 0x77
- Applications icons: Weather station, HVAC, IoT
- Pinout diagram of BME280 module
- Bosch logo reference
```

---

## SLIDE 13 - BME280 Register Map

**Prompt:**
```
Create a technical slide showing BME280 registers:
- Title "BME280 Register Map"
- Table format:
  | Register | Address | Description |
  | ID       | 0xD0    | Chip ID (0x60) |
  | ctrl_hum | 0xF2    | Humidity control |
  | ctrl_meas| 0xF4    | Temp & Press control |
  | config   | 0xF5    | Rate, filter, interface |
  | Data     | 0xF7-0xFE | Measurement data |
- Memory map visualization
- Highlighting data registers
- Read/Write indicators
```

---

## SLIDE 14 - SSD1306 OLED Display

**Prompt:**
```
Create a product overview slide for SSD1306:
- Title "SSD1306 - OLED Display"
- OLED display module photo showing 128x64 pixels
- Specifications:
  • Resolution: 128×64 pixels
  • Type: Monochrome OLED
  • I2C Address: 0x3C or 0x3D
  • Voltage: 3.3V or 5V
- Display showing sample graphics
- Pinout diagram
- Solomon Systech logo reference
- Application examples: IoT dashboard, status display
```

---

## SLIDE 15 - DS3231 Real-Time Clock

**Prompt:**
```
Create a product overview slide for DS3231:
- Title "DS3231 - Real-Time Clock"
- DS3231 module photo with battery
- Key features:
  • Accuracy: ±2ppm (±1 min/year)
  • Battery backup (CR2032)
  • Temperature compensated crystal
  • I2C Address: 0x68 (fixed)
- Register overview for time keeping
- Module pinout
- Maxim Integrated logo
- Applications: Data logging, scheduling, timestamps
```

---

## SLIDE 16 - 24LC256 EEPROM

**Prompt:**
```
Create a product overview slide for 24LC256:
- Title "24LC256 - I2C EEPROM"
- EEPROM chip DIP package illustration
- Specifications:
  • Capacity: 256 Kbit (32KB)
  • Page size: 64 bytes
  • I2C Address: 0x50-0x57
  • Write cycle: 5ms max
- Address configuration with A0, A1, A2 pins
- Memory organization diagram
- Microchip logo
- Applications: Configuration storage, data logging
```

---

## SLIDE 17 - Konfigurasi I2C STM32

**Prompt:**
```
Create a code walkthrough slide:
- Title "Konfigurasi I2C STM32 (HAL)"
- Code snippet with syntax highlighting:
  I2C_HandleTypeDef hi2c1;
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  HAL_I2C_Init(&hi2c1);
- Key parameters highlighted with arrows
- GPIO configuration note (PB6, PB7 as AF_OD)
- Visual flow diagram showing init sequence
```

---

## SLIDE 18 - Konfigurasi I2C ESP32

**Prompt:**
```
Create a code walkthrough slide:
- Title "Konfigurasi I2C ESP32 (Arduino)"
- Two approaches:
  1. Simple (Wire library):
     Wire.begin(21, 22);
     Wire.setClock(400000);
  2. Advanced (ESP-IDF):
     i2c_config_t conf;
     i2c_param_config(I2C_NUM_0, &conf);
     i2c_driver_install();
- Flexible GPIO mapping highlight
- Internal pull-up option
- Comparison with STM32 approach
```

---

## SLIDE 19 - I2C Bus Scanning

**Prompt:**
```
Create a slide about I2C bus scanning:
- Title "I2C Bus Scanner"
- Purpose: Detect all connected devices
- Scanning process visualization:
  1. Send address (0x03 to 0x77)
  2. Check for ACK
  3. List found devices
- Sample output grid showing addresses
- Common addresses highlighted with device names
- Both STM32 and ESP32 code snippets
- Troubleshooting tips
```

---

## SLIDE 20 - Pull-up Resistor Design

**Prompt:**
```
Create a technical slide about pull-up resistors:
- Title "Perhitungan Pull-up Resistor"
- Formula diagram:
  Rp_min = (Vcc - Vol) / Iol
  Rp_max = tr / (0.8473 × Cb)
- Variables explanation with icons
- Recommended values table:
  | Mode | Bus Cap | Rp Value |
  | Standard | <100pF | 4.7kΩ |
  | Standard | <200pF | 2.2kΩ |
  | Fast | <100pF | 2.2kΩ |
- Visual showing resistor placement in circuit
- Rule of thumb: 4.7kΩ for most applications
```

---

## SLIDE 21 - Multi-Device I2C

**Prompt:**
```
Create a slide showing multi-device I2C setup:
- Title "Multi-Device pada I2C Bus"
- Bus diagram with 4 devices:
  • BME280 (0x76)
  • SSD1306 OLED (0x3C)
  • DS3231 RTC (0x68)
  • 24LC256 EEPROM (0x50)
- Single SDA/SCL lines connecting all
- Address labels for each device
- Data flow arrows
- Conflict resolution note
- Practical wiring tips
```

---

## SLIDE 22 - I2C Read Operation

**Prompt:**
```
Create a timing diagram for I2C read:
- Title "Operasi Read I2C"
- Complete read sequence:
  1. START
  2. Device Address + Write
  3. Register Address
  4. Repeated START
  5. Device Address + Read
  6. Data byte(s)
  7. NACK + STOP
- Both master and slave perspectives
- ACK/NACK indicators
- Data direction arrows
- Typical use case: Reading sensor data
```

---

## SLIDE 23 - I2C Write Operation

**Prompt:**
```
Create a timing diagram for I2C write:
- Title "Operasi Write I2C"
- Complete write sequence:
  1. START
  2. Device Address + Write
  3. Register Address
  4. Data byte(s)
  5. STOP
- Master to slave data flow
- ACK after each byte
- Typical use case: Configuration register
- Page write for EEPROM
```

---

## SLIDE 24 - Troubleshooting I2C

**Prompt:**
```
Create a troubleshooting guide slide:
- Title "Troubleshooting I2C"
- Common problems table:
  | Problem | Symptom | Solution |
  | SDA stuck | No comm | Bus recovery |
  | No ACK | NACK | Check address/power |
  | Timeout | Hangs | Increase timeout |
  | Data corruption | Wrong data | Check pull-ups |
- Visual checklist icons
- Logic analyzer screenshot example
- Debug code snippet
- Decision flowchart
```

---

## SLIDE 25 - I2C Bus Recovery

**Prompt:**
```
Create a slide about bus recovery procedure:
- Title "I2C Bus Recovery"
- Problem: SDA stuck LOW
- Recovery algorithm visualization:
  1. Send 9 clock pulses on SCL
  2. Check if SDA released
  3. Generate STOP condition
  4. Reinitialize I2C
- Code snippet for recovery
- Before/after waveforms
- When to use recovery
- Prevention tips
```

---

## SLIDE 26 - Best Practices

**Prompt:**
```
Create a best practices slide:
- Title "Best Practices I2C"
- Hardware section:
  • Use appropriate pull-up resistors
  • Keep traces short (<30cm)
  • Add decoupling capacitors
  • Consider separate buses
- Software section:
  • Always check return values
  • Implement timeout handling
  • Use bus recovery
  • Validate with CRC
- Icons for each practice
- Do's and Don'ts format
```

---

## SLIDE 27 - Praktikum Overview

**Prompt:**
```
Create a practicum overview slide:
- Title "Praktikum I2C Sensor"
- 12 programs for each platform (STM32 & ESP32)
- Program list with icons:
  1-2. I2C Bus Scanner
  3-4. BME280 Sensor Reading
  5-6. SSD1306 OLED Display
  7-8. DS3231 RTC Operations
  9-10. 24LC256 EEPROM
  11-12. Multi-Device Integration
- Hardware requirements summary
- Time allocation
- Learning progression diagram
```

---

### Catatan untuk Pembuat PPT:

1. **Konsistensi Visual**: Gunakan template yang sama untuk semua slide
2. **Animasi**: Tambahkan animasi sederhana untuk diagram timing
3. **Code Highlighting**: Gunakan syntax highlighting untuk semua code
4. **Waveforms**: Animasikan sinyal I2C untuk menunjukkan komunikasi
5. **Interactive Elements**: Tambahkan hyperlink untuk navigasi antar slide
6. **Font Size**: Minimum 24pt untuk body text, 36pt untuk judul
7. **Color Coding**: Konsisten gunakan warna untuk SDA (hijau) dan SCL (biru)
