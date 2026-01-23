# Bill of Materials (BOM) Extended - Programs 51-100
## Complete Hardware Requirements for Advanced Programs

**Last Updated**: 2026-01-22  
**Programs Covered**: 51-100  
**Total Estimated Cost**: Rp 2,500,000 - Rp 4,000,000  

---

## Program 51-60: IoT & Connectivity

### Program 51: WiFi ESP8266
| Component | Qty | Unit Price | Total | Notes |
|-----------|-----|------------|-------|-------|
| ESP8266 ESP-01S | 1 | Rp 30.000 | Rp 30.000 | WiFi 802.11 b/g/n |
| 3.3V LDO Regulator | 1 | Rp 5.000 | Rp 5.000 | AMS1117 atau similar |
| Capacitor 10µF | 2 | Rp 2.000 | Rp 4.000 | Decoupling |
| Resistor 10k | 2 | Rp 500 | Rp 1.000 | Pull-ups |
| USB-to-UART adapter | 1 | Rp 15.000 | Rp 15.000 | CP2102 or CH340 |
| **Subtotal** | | | **Rp 55.000** | |

### Program 52: Bluetooth HC-05
| Component | Qty | Unit Price | Total | Notes |
|-----------|-----|------------|-------|-------|
| HC-05 Bluetooth Module | 1 | Rp 45.000 | Rp 45.000 | Serial Bluetooth |
| 3.3V LDO Regulator | 1 | Rp 5.000 | Rp 5.000 | |
| Capacitor 10µF | 2 | Rp 2.000 | Rp 4.000 | Decoupling |
| Resistor 10k | 2 | Rp 500 | Rp 1.000 | Pull-up |
| Resistor divider 2k+1k | 1 set | Rp 1.000 | Rp 1.000 | Level shifting |
| **Subtotal** | | | **Rp 56.000** | |

### Program 53: LoRa SX1278
| Component | Qty | Unit Price | Total | Notes |
|-----------|-----|------------|-------|-------|
| SX1278 LoRa Module | 1 | Rp 150.000 | Rp 150.000 | 433MHz atau 868MHz |
| 433MHz Antenna | 1 | Rp 20.000 | Rp 20.000 | Quarter-wave dipole |
| Resistor 10k | 4 | Rp 500 | Rp 2.000 | Pull-ups & bias |
| Capacitor 100nF | 5 | Rp 1.000 | Rp 5.000 | Decoupling |
| **Subtotal** | | | **Rp 177.000** | |

### Program 54: 4G SIM7000 LTE
| Component | Qty | Unit Price | Total | Notes |
|-----------|-----|------------|-------|-------|
| SIM7000 Module | 1 | Rp 400.000 | Rp 400.000 | 4G/LTE/2G fallback |
| Active SIM Card | 1 | Rp 25.000 | Rp 25.000 | Any mobile operator |
| 3.8V Power Supply | 1 | Rp 50.000 | Rp 50.000 | 2A minimum |
| Antenna GSM/LTE | 1 | Rp 40.000 | Rp 40.000 | High-gain antenna |
| **Subtotal** | | | **Rp 515.000** | |

### Program 55: MQTT Broker
*Same WiFi hardware as Program 51*
- Broker software: Mosquitto (free, open-source)
- Server: PC atau Raspberry Pi

### Program 56-60: IoT Protocols
**CoAP, NB-IoT, 5G, Thread, Zigbee**: Similar to above programs

**IoT Subtotal (51-60)**: Rp 800.000 - Rp 1,200.000

---

## Program 61-70: Advanced Control & Processing

### Program 61: PID Motor Control
| Component | Qty | Unit Price | Total | Notes |
|-----------|-----|------------|-------|-------|
| DC Motor 12V | 1 | Rp 80.000 | Rp 80.000 | With encoder |
| Motor Driver L298N | 1 | Rp 25.000 | Rp 25.000 | Or BTS7960 |
| Rotary Encoder | 1 | Rp 20.000 | Rp 20.000 | 400 PPR |
| Power Supply 12V 2A | 1 | Rp 50.000 | Rp 50.000 | |
| Potentiometer 10k | 1 | Rp 3.000 | Rp 3.000 | Setpoint |
| **Subtotal** | | | **Rp 178.000** | |

### Program 62: Fuzzy Logic
*Uses sensors from Programs 37, 50*
- No additional hardware needed

### Program 63: Kalman Filter Sensor Fusion
| Component | Qty | Unit Price | Total | Notes |
|-----------|-----|------------|-------|-------|
| MPU6050 IMU | 1 | Rp 30.000 | Rp 30.000 | 6-axis IMU |
| HMC5883L Magnetometer | 1 | Rp 25.000 | Rp 25.000 | 3-axis compass |
| BMP280 Barometer | 1 | Rp 25.000 | Rp 25.000 | Altitude sensor |
| Resistor 4.7k | 4 | Rp 500 | Rp 2.000 | I2C pull-ups |
| **Subtotal** | | | **Rp 82.000** | |

### Program 64: FFT Audio Processing
| Component | Qty | Unit Price | Total | Notes |
|-----------|-----|------------|-------|-------|
| Microphone module MAX4466 | 1 | Rp 35.000 | Rp 35.000 | Amplified mic |
| Audio connector 3.5mm | 1 | Rp 5.000 | Rp 5.000 | Line-in jack |
| Capacitor 100µF | 1 | Rp 2.000 | Rp 2.000 | Coupling |
| OLED 0.96" display | 1 | Rp 30.000 | Rp 30.000 | Visualization |
| **Subtotal** | | | **Rp 72.000** | |

### Program 65-70: Gesture, BLDC, Robotics
| Component | Qty | Unit Price | Total | Notes |
|-----------|-----|------------|-------|-------|
| HC-SR04 Ultrasonic (3x) | 3 | Rp 15.000 | Rp 45.000 | Program 65 |
| BLDC Motor 12V | 1 | Rp 150.000 | Rp 150.000 | Program 66 |
| BLDC Controller ESC | 1 | Rp 40.000 | Rp 40.000 | |
| Servo motor SG90 (5x) | 5 | Rp 15.000 | Rp 75.000 | Program 70 |
| Robotic arm kit | 1 | Rp 500.000 | Rp 500.000 | Program 70 |
| Power supply 12V 5A | 1 | Rp 100.000 | Rp 100.000 | For motors |
| **Subtotal** | | | **Rp 910.000** | |

**Control & Processing Subtotal (61-70)**: Rp 1,270.000

---

## Program 71-80: Machine Learning & DSP

### Program 71: Neural Network
*Computation only, no additional hardware*

### Program 72: TensorFlow Lite
*PC/Development machine for training, embedded inference on MCU*

### Program 73-80: Audio & Signal Processing
| Component | Qty | Unit Price | Total | Notes |
|-----------|-----|------------|-------|-------|
| Microphone module (73-80) | 1 | Rp 35.000 | Rp 35.000 | MFCC, filter, ECG |
| Precision ADC ADS1115 | 1 | Rp 20.000 | Rp 20.000 | 16-bit (Program 79) |
| ECG Electrode pads | 10 | Rp 5.000 | Rp 50.000 | Program 80 |
| ECG front-end module | 1 | Rp 150.000 | Rp 150.000 | Amplifier & filter |
| **Subtotal** | | | **Rp 255.000** | |

**ML & DSP Subtotal (71-80)**: Rp 255.000

---

## Program 81-90: Security & Optimization

### Program 81-90: Cryptography
*No additional hardware needed - all software implementation*
- Uses internal flash & RAM
- Optional: USB interface for testing

**Security Subtotal (81-90)**: Rp 0 (software only)

---

## Program 91-100: Production Ready Systems

### Program 91-100: Smart Device System
| Component | Qty | Unit Price | Total | Notes |
|-----------|-----|------------|-------|-------|
| DS3231 RTC module | 1 | Rp 25.000 | Rp 25.000 | Program 92, 94 |
| MicroSD card 32GB | 1 | Rp 80.000 | Rp 80.000 | Program 92, 94 |
| MicroSD module SPI | 1 | Rp 15.000 | Rp 15.000 | |
| WiFi ESP8266 | 1 | Rp 30.000 | Rp 30.000 | Program 93 |
| USB cable Micro | 2 | Rp 5.000 | Rp 10.000 | |
| Relay module 5V | 1 | Rp 15.000 | Rp 15.000 | Actuator |
| Power supply 5V 2A | 1 | Rp 25.000 | Rp 25.000 | |
| **Subtotal** | | | **Rp 200.000** | |

**Production Ready Subtotal (91-100)**: Rp 200.000

---

## TOTAL HARDWARE SUMMARY

### Core Requirements (Base)
- STM32F103C8T6 Blue Pill: Rp 35.000
- ST-Link V2: Rp 25.000
- Basic components: Rp 160.000
- **Subtotal**: Rp 220.000

### Program 1-50 (Already documented)
- **Subtotal**: Rp 535.000

### Program 51-100 (Extended)
| Category | Cost |
|----------|------|
| IoT & Connectivity (51-60) | Rp 800.000 - 1.200.000 |
| Control & Processing (61-70) | Rp 1.270.000 |
| ML & DSP (71-80) | Rp 255.000 |
| Security (81-90) | Rp 0 (software) |
| Production Systems (91-100) | Rp 200.000 |
| **Subtotal Extended** | **Rp 2.525.000** |

### GRAND TOTAL
| Tier | Cost | Programs |
|------|------|----------|
| **Minimum** (1-10) | Rp 160.000 | 10 programs |
| **Intermediate** (1-30) | Rp 350.000 | 30 programs |
| **Advanced** (1-50) | Rp 760.000 | 50 programs |
| **Expert** (1-70) | Rp 2.030.000 | 70 programs |
| **Complete** (1-100) | **Rp 2.790.000** | **100 programs** |

---

## RECOMMENDED PROCUREMENT STRATEGY

### Phase 1 (Week 1-2): Basics
- Budget: Rp 220.000
- Covers: Programs 1-10

### Phase 2 (Week 3-4): Intermediate
- Budget: +Rp 130.000
- Covers: Programs 11-30

### Phase 3 (Week 5-6): Advanced
- Budget: +Rp 410.000
- Covers: Programs 31-50

### Phase 4 (Week 7-10): IoT & Control
- Budget: +Rp 2.070.000
- Covers: Programs 51-70

### Phase 5 (Week 11-18): ML & Production
- Budget: +Rp 455.000
- Covers: Programs 71-100

---

## PREFERRED SUPPLIERS (Indonesia)

### Electronics Retailers
1. **Tokopedia** - Largest marketplace
   - Filters by region for faster shipping
   - Many 3rd party sellers

2. **Bukalapak** - Good for bulk
   - Price negotiation possible
   - Local sellers

3. **Lazada** - Fast shipping via logistics
   - Flash sales & vouchers
   - Warranty guaranteed

4. **Shopee** - Competitive pricing
   - Free shipping offers
   - Seller ratings

### Physical Stores (Jakarta)
1. **Glodok Plaza** - Wholesale & retail
   - Best for STM32 boards
   - Competitive prices

2. **Sadhar Plaza** - Sensor specialists
   - Wide variety of modules
   - Expert consultation

3. **Harco Mas** - Large inventory
   - One-stop shopping
   - Reasonable prices

4. **Mall Elektronik Bekasi** - Growing selection
   - Local support

### Recommended Approach
- Buy basic components locally (lower shipping cost)
- Order modules from online (better price & selection)
- Bulk buying saves ~10-20% per item

---

## ALTERNATIVE COST REDUCTION STRATEGIES

### 1. Use Simulation First
- Use STM32 CubeMX simulator for initial testing
- Reduces need for hardware in early phases
- Save: Rp 400.000+

### 2. Module Sharing
- Share expensive modules (oscilloscope, logic analyzer)
- Borrow from university/institution
- Save: Rp 450.000

### 3. DIY Alternatives
- Build simple modules instead of buying
- Example: Power supply, breadboard circuits
- Save: Rp 200.000

### 4. Open Source Libraries
- Use existing libraries instead of buying specialized modules
- Example: TinyML libraries, FreeRTOS
- Save: Development time

### 5. Start with Minimum, Upgrade Later
- Begin with Rp 160.000
- Add modules progressively
- Spread cost over time

---

## QUALITY CHECKLIST

✅ **When buying components:**
- [ ] Check seller rating (minimum 4.5 stars)
- [ ] Verify product authenticity
- [ ] Check warranty/return policy
- [ ] Ask for datasheet if available
- [ ] Confirm stock availability
- [ ] Check delivery time

✅ **When receiving:**
- [ ] Inspect packaging for damage
- [ ] Verify all items received
- [ ] Check expiry dates (for electrolytic caps)
- [ ] Test connectivity (if possible)
- [ ] Keep invoice & receipt

---

## MAINTENANCE & REPLACEMENT

### Expected Lifespan
| Component | Lifespan | Replacement Cost |
|-----------|----------|------------------|
| STM32 Board | 5+ years | Rp 35.000 |
| ST-Link V2 | 5+ years | Rp 25.000 |
| USB Cable | 1-2 years | Rp 5.000 |
| Electrolytic Cap | 2-5 years | Rp 2.000 each |
| Motors | 1-3 years | Rp 15-150.000 |
| Sensors | 3-5 years | Rp 15-40.000 |

### Maintenance Tips
- Store in dry, temperature-controlled place
- Avoid electrostatic discharge
- Replace capacitors if bulging
- Keep all components in labeled containers
- Maintain spare components inventory

---

**Total Investment: Rp 2,790,000 for Complete 100-Program System**

*This is a comprehensive educational investment for mastering STM32 embedded systems development.*

Last Updated: 2026-01-22
