# Prompt untuk Pembuatan PPT - Bagian 2
## Modul 06: I2C Bus dan Sensor Integration (Slide 28-50)

### Instruksi Lanjutan
- Melanjutkan dari PPT_Prompts_1.md (Slide 1-27)
- Style dan color scheme tetap konsisten
- Focus pada aplikasi praktis dan advanced topics

---

## SLIDE 28 - Demo: I2C Scanner

**Prompt:**
```
Create a demonstration slide for I2C Scanner:
- Title "Demo: I2C Bus Scanner"
- Split screen layout:
  Left: Code snippet (highlighted)
  Right: Serial monitor output
- Sample output showing:
     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
  00:          -- -- -- -- -- -- -- -- -- -- -- -- --
  30: -- -- -- -- -- -- -- -- -- -- -- -- 3C -- -- --
  50: 50 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
  60: -- -- -- -- -- -- -- -- 68 -- -- -- -- -- -- --
  70: -- -- -- -- -- -- 76 --
- Device identification legend
- Both STM32 and ESP32 versions shown
```

---

## SLIDE 29 - Demo: BME280 Reading

**Prompt:**
```
Create a demonstration slide for BME280:
- Title "Demo: Pembacaan BME280"
- Real sensor data display:
  • Temperature: 25.43 °C
  • Humidity: 65.21 %RH
  • Pressure: 1013.25 hPa
  • Altitude: 0.00 m
- Graph showing data over time
- Calibration process visualization
- Oversampling configuration
- Heat index and dew point calculations
```

---

## SLIDE 30 - Demo: OLED Display

**Prompt:**
```
Create a demonstration slide for SSD1306:
- Title "Demo: SSD1306 OLED Display"
- OLED screen mockups showing:
  1. Text display "Hello World"
  2. Graphics (lines, circles, rectangles)
  3. Animated bouncing ball
  4. Progress bar
  5. Scrolling text
- Display buffer concept
- Resolution 128x64 pixels grid
- Font rendering example
```

---

## SLIDE 31 - Demo: DS3231 RTC

**Prompt:**
```
Create a demonstration slide for DS3231:
- Title "Demo: DS3231 RTC"
- Clock display showing current time
- Features demonstrated:
  • Date/time reading
  • Temperature from RTC
  • Alarm setting
  • Battery backup indication
- BCD format explanation
- Register read/write visualization
- Timestamp format examples
```

---

## SLIDE 32 - Demo: EEPROM Operations

**Prompt:**
```
Create a demonstration slide for EEPROM:
- Title "Demo: 24LC256 EEPROM"
- Memory operations visualization:
  • Byte write/read
  • Page write (64 bytes)
  • String storage
- Memory dump output format
- Write cycle timing (5ms)
- Address calculation example
- Wear leveling concept introduction
```

---

## SLIDE 33 - Demo: Multi-Device System

**Prompt:**
```
Create a demonstration slide for integrated system:
- Title "Demo: Multi-Device Integration"
- System diagram showing all devices connected
- OLED displaying:
  • Time from RTC (top)
  • Temperature/Humidity from BME280 (middle)
  • Data logging status (bottom)
- EEPROM storing sensor data with timestamps
- Real-time update visualization
- Code architecture overview
```

---

## SLIDE 34 - Aplikasi: Weather Station

**Prompt:**
```
Create an application slide:
- Title "Aplikasi: Weather Station"
- Complete weather station design:
  • BME280 for environmental data
  • DS3231 for timestamps
  • SSD1306 for local display
  • EEPROM for data logging
- Block diagram
- Data flow visualization
- Sample user interface on OLED
- 24-hour history graph
```

---

## SLIDE 35 - Aplikasi: Data Logger

**Prompt:**
```
Create an application slide:
- Title "Aplikasi: Data Logger"
- Data logging system:
  • Sensor readings at intervals
  • RTC timestamps
  • EEPROM storage
  • Wear leveling algorithm
- Memory management visualization
- Circular buffer concept
- Data retrieval process
- Export format (CSV)
```

---

## SLIDE 36 - Aplikasi: Smart Home Display

**Prompt:**
```
Create an application slide:
- Title "Aplikasi: Smart Home Display"
- Home dashboard on OLED:
  • Time and date
  • Indoor temperature/humidity
  • Weather icons
  • Alert notifications
- Multiple rooms data concept
- Periodic screen refresh
- Power saving modes
- User interaction buttons
```

---

## SLIDE 37 - Clock Stretching

**Prompt:**
```
Create a technical slide about clock stretching:
- Title "Clock Stretching"
- Definition and purpose
- Timing diagram showing:
  • Master sends clock
  • Slave holds SCL LOW
  • Master waits
  • Slave releases SCL
  • Communication continues
- Use cases:
  • Slow slave processing
  • EEPROM write cycles
- Timeout considerations
- STM32 vs ESP32 handling
```

---

## SLIDE 38 - Bus Arbitration

**Prompt:**
```
Create a technical slide about bus arbitration:
- Title "Arbitration Multi-Master"
- Multi-master scenario diagram
- Arbitration process:
  1. Both masters start simultaneously
  2. Monitor SDA line
  3. Loser detects conflict
  4. Winner continues
- Timing diagram with conflict
- Wired-AND logic explanation
- Practical considerations
- When to use multi-master
```

---

## SLIDE 39 - 10-bit Addressing

**Prompt:**
```
Create a technical slide about 10-bit addressing:
- Title "10-bit I2C Addressing"
- Address format comparison:
  • 7-bit: 127 devices
  • 10-bit: 1024 devices
- Frame format diagram:
  | 1111 0XX | R/W | ACK | XX XXXX XX | ACK |
- Reserved addresses explanation
- When to use 10-bit
- Compatibility with 7-bit devices
- Code implementation notes
```

---

## SLIDE 40 - I2C Level Shifting

**Prompt:**
```
Create a technical slide about level shifting:
- Title "Level Shifting I2C"
- Problem: 3.3V and 5V devices on same bus
- Solutions:
  1. MOSFET-based bidirectional
  2. Dedicated level shifter IC
  3. Resistor voltage divider (limited)
- Circuit diagrams for each method
- Component recommendations
- Timing considerations
- Best practices
```

---

## SLIDE 41 - I2C DMA Transfer

**Prompt:**
```
Create a technical slide about DMA with I2C:
- Title "I2C dengan DMA"
- Benefits:
  • CPU offloading
  • Continuous data transfer
  • Efficient for large data
- DMA configuration diagram
- STM32 DMA channels for I2C
- ESP32 DMA considerations
- Performance comparison graph
- Use cases: Display updates, sensor arrays
```

---

## SLIDE 42 - I2C Interrupt Handling

**Prompt:**
```
Create a technical slide about interrupts:
- Title "I2C Interrupt Handling"
- Interrupt events:
  • Transfer complete
  • Error detected
  • Address match (slave)
  • NACK received
- Interrupt service routine flow
- Priority configuration
- State machine diagram
- Non-blocking I2C operations
- Example code structure
```

---

## SLIDE 43 - Error Handling Strategies

**Prompt:**
```
Create a slide about error handling:
- Title "Strategi Error Handling"
- Error types and responses:
  | Error | Detection | Recovery |
  | NACK | ACK bit | Retry/abort |
  | Timeout | Timer | Bus recovery |
  | Bus busy | Status | Wait/reset |
  | Arbitration lost | SDA | Restart |
- Retry mechanism flowchart
- Logging and diagnostics
- Graceful degradation
```

---

## SLIDE 44 - Performance Optimization

**Prompt:**
```
Create a slide about I2C optimization:
- Title "Optimasi Performa I2C"
- Optimization techniques:
  • Use Fast Mode (400kHz) when possible
  • Batch multiple reads
  • Reduce transactions
  • Use DMA for large transfers
  • Optimize pull-up resistors
- Benchmark comparison chart
- Power consumption considerations
- Code optimization tips
```

---

## SLIDE 45 - Project Preview

**Prompt:**
```
Create a project preview slide:
- Title "Project: Smart Environmental Monitor"
- Dual-MCU system:
  • STM32: Sensor hub (BME280, more sensors)
  • ESP32: Display & connectivity
- Communication: I2C between MCUs
- Features list:
  • Multi-point sensing
  • Data logging with timestamps
  • OLED dashboard
  • Alert system
- System architecture diagram
- Challenge levels indicated
```

---

## SLIDE 46 - Perbandingan STM32 vs ESP32 I2C

**Prompt:**
```
Create a comparison slide:
- Title "STM32 vs ESP32: I2C Implementation"
- Side-by-side comparison:
  | Feature | STM32F103 | ESP32 |
  | Peripherals | 2 | 2 |
  | Default pins | Fixed | Flexible |
  | Max speed | 400kHz | 1MHz |
  | Internal pull-up | No | Yes |
  | DMA support | Yes | Yes |
- Pros and cons for each
- When to choose which
- Code complexity comparison
```

---

## SLIDE 47 - Common Mistakes

**Prompt:**
```
Create a slide about common mistakes:
- Title "Kesalahan Umum I2C"
- Mistake cards with solutions:
  1. Missing pull-up resistors
  2. Wrong address (not shifted)
  3. Incorrect voltage levels
  4. Too long bus wires
  5. Not checking return values
  6. Blocking code with no timeout
  7. Address conflicts
- Visual icons showing X for mistake, ✓ for solution
- Code examples of correct vs incorrect
```

---

## SLIDE 48 - Resources & Tools

**Prompt:**
```
Create a resources slide:
- Title "Sumber Belajar & Tools"
- Documentation links:
  • NXP I2C Specification
  • Datasheets
  • HAL documentation
- Tools:
  • Logic analyzer for debugging
  • I2C scanner utility
  • Protocol decoder
- Libraries:
  • Wire.h (Arduino)
  • HAL I2C (STM32)
  • ESP-IDF I2C
- Online simulators
```

---

## SLIDE 49 - Rangkuman

**Prompt:**
```
Create a summary slide:
- Title "Rangkuman"
- Key points with icons:
  1. I2C: 2-wire protocol (SDA, SCL)
  2. 7-bit addressing: 127 devices
  3. STM32: 2 I2C, HAL library
  4. ESP32: 2 I2C, flexible GPIO
  5. Pull-up resistors essential
  6. Multiple sensors on one bus
  7. Error handling critical
- Visual recap of the module
- Connection to next module preview
```

---

## SLIDE 50 - Penutup & Tugas

**Prompt:**
```
Create a closing slide:
- Title "Tugas & Evaluasi"
- Praktikum tasks:
  1. Complete all 12 programs (STM32 & ESP32)
  2. Document results with screenshots
  3. Answer analysis questions
- Project requirements:
  • Smart Environmental Monitor
  • Both MCUs required
  • Video demonstration
- Evaluation criteria summary
- Deadline information
- Contact for questions
- "Terima Kasih" closing
```

---

### Catatan Tambahan untuk Bagian 2:

1. **Demo Slides**: Sertakan screenshot asli atau mockup realistis
2. **Application Slides**: Tunjukkan use case nyata yang relevan
3. **Technical Slides**: Pastikan diagram timing akurat
4. **Code Examples**: Highlight bagian penting dari kode
5. **Comparisons**: Gunakan format tabel untuk kemudahan pemahaman
6. **Summary**: Recap poin utama dengan visual yang memorable

### Integrasi dengan Praktikum:

- Setiap demo slide harus sesuai dengan program di Jobsheet
- Gunakan hasil output yang konsisten dengan kode
- Referensikan nomor program untuk kemudahan navigasi
- Sertakan QR code untuk akses cepat ke repository kode
