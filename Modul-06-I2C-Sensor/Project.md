# Project Modul 06: Smart Environmental Monitor dengan Dual-MCU I2C

## 📋 Informasi Project

| Item | Keterangan |
|------|------------|
| **Mata Kuliah** | Praktikum Sistem Embedded |
| **Modul** | 06 - I2C Bus dan Sensor Integration |
| **Tingkat Kesulitan** | ⭐⭐⭐⭐ (Advanced) |
| **Waktu Pengerjaan** | 2-3 minggu |
| **Platform** | STM32F103C8T6 + ESP32 DevKit V1 |

---

## 🎯 Deskripsi Project

Membangun **Smart Environmental Monitor** yang menggunakan arsitektur **Dual-MCU** dimana:

- **STM32F103** berperan sebagai **Sensor Hub** yang mengumpulkan data dari multiple sensor I2C
- **ESP32** berperan sebagai **Display Controller** yang menampilkan data pada OLED dan menyediakan user interface

Kedua MCU berkomunikasi melalui **I2C Bus** dimana ESP32 sebagai Master dan STM32 sebagai Slave, membentuk sistem multi-master capable dengan sensor I2C lainnya.

---

## 🎓 Tujuan Pembelajaran

Setelah menyelesaikan project ini, mahasiswa mampu:

1. Mengimplementasikan komunikasi I2C multi-device kompleks
2. Mengkonfigurasi MCU sebagai I2C Slave (STM32)
3. Menangani multiple I2C bus pada satu sistem
4. Mengintegrasikan berbagai sensor I2C (BME280, DS3231, dsb)
5. Membuat real-time data logging dengan timestamps
6. Mendesain user interface pada OLED display
7. Menerapkan error handling dan recovery pada I2C

---

## 📐 Arsitektur Sistem

### Diagram Blok

```
                    ┌─────────────────────────────────────────────────────────┐
                    │                  SMART ENVIRONMENTAL MONITOR             │
                    └─────────────────────────────────────────────────────────┘
                    
    ┌──────────────────────────────┐         ┌──────────────────────────────┐
    │       STM32F103C8T6          │         │        ESP32 DevKit          │
    │        (Sensor Hub)          │         │    (Display Controller)      │
    │                              │   I2C   │                              │
    │  ┌─────────────────────┐    │◄───────►│    ┌─────────────────────┐   │
    │  │   I2C1 (Master)     │    │  Slave  │    │   I2C0 (Master)     │   │
    │  │   - BME280 (0x76)   │    │  0x08   │    │   - SSD1306 (0x3C)  │   │
    │  │   - DS3231 (0x68)   │    │         │    │   - STM32 (0x08)    │   │
    │  │   - AHT20 (0x38)    │    │         │    └─────────────────────┘   │
    │  └─────────────────────┘    │         │                              │
    │                              │         │    ┌─────────────────────┐   │
    │  ┌─────────────────────┐    │         │    │   I2C1 (Master)     │   │
    │  │   I2C2 (Slave)      │    │         │    │   - 24LC256 (0x50)  │   │
    │  │   Address: 0x08     │────┼─────────┼───►│                     │   │
    │  └─────────────────────┘    │         │    └─────────────────────┘   │
    │                              │         │                              │
    │  ┌─────────────────────┐    │         │    ┌─────────────────────┐   │
    │  │   GPIO              │    │         │    │   GPIO              │   │
    │  │   - Status LED (PC13)│    │         │    │   - Button (GPIO0)  │   │
    │  │   - Alert LED (PA0)  │    │         │    │   - Buzzer (GPIO25) │   │
    │  └─────────────────────┘    │         │    └─────────────────────┘   │
    └──────────────────────────────┘         └──────────────────────────────┘
```

### Koneksi I2C

```
┌──────────────────────────────────────────────────────────────────────────────┐
│                              I2C BUS TOPOLOGY                                 │
├──────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│    STM32 I2C1 Bus (PB6=SCL, PB7=SDA) - Sensor Bus                           │
│    ─────────────────────────────────────────────                            │
│         │         │         │         │                                      │
│     ┌───┴───┐ ┌───┴───┐ ┌───┴───┐ ┌───┴───┐                                │
│     │BME280 │ │DS3231 │ │ AHT20 │ │ BH1750│                                │
│     │ 0x76  │ │ 0x68  │ │ 0x38  │ │ 0x23  │                                │
│     └───────┘ └───────┘ └───────┘ └───────┘                                │
│                                                                              │
│    ESP32 I2C0 Bus (GPIO22=SCL, GPIO21=SDA) - Main Bus                       │
│    ─────────────────────────────────────────────                            │
│         │         │         │                                                │
│     ┌───┴───┐ ┌───┴───┐ ┌───┴───┐                                          │
│     │SSD1306│ │STM32  │ │24LC256│                                          │
│     │ 0x3C  │ │ 0x08  │ │ 0x50  │                                          │
│     └───────┘ └─Slave─┘ └───────┘                                          │
│                                                                              │
└──────────────────────────────────────────────────────────────────────────────┘
```

---

## 🔧 Hardware Requirements

### Komponen Utama

| No | Komponen | Jumlah | Keterangan |
|----|----------|--------|------------|
| 1 | STM32F103C8T6 (Blue Pill) | 1 | Sensor Hub |
| 2 | ESP32 DevKit V1 | 1 | Display Controller |
| 3 | BME280 Module | 1 | Temp/Hum/Press sensor |
| 4 | DS3231 RTC Module | 1 | Real-time clock |
| 5 | SSD1306 OLED 128×64 | 1 | Display |
| 6 | 24LC256 EEPROM | 1 | Data storage |
| 7 | Resistor 4.7kΩ | 4 | I2C pull-ups |
| 8 | Resistor 10kΩ | 2 | Button pull-ups |
| 9 | LED 5mm (2 warna) | 2 | Status indicators |
| 10 | Push Button | 2 | User input |
| 11 | Buzzer 5V | 1 | Audio alert |
| 12 | Breadboard | 2 | Prototyping |
| 13 | Jumper Wires | ~50 | Connections |

### Komponen Opsional (Bonus)

| No | Komponen | Keterangan |
|----|----------|------------|
| 1 | AHT20/AHT21 | Additional temp/hum sensor |
| 2 | BH1750 | Light intensity sensor |
| 3 | MQ-135 | Air quality (via ADC) |
| 4 | PCF8574 | I2C GPIO expander |

---

## 📝 Spesifikasi Fungsional

### 1. Sensor Hub (STM32)

**Fungsi Utama:**
- Membaca data dari semua sensor I2C setiap 1 detik
- Menyimpan data dalam buffer internal
- Merespon request dari ESP32 Master
- Mengirim data dalam format terstruktur

**Data yang Dikumpulkan:**
```c
typedef struct {
    uint32_t timestamp;      // Unix timestamp dari DS3231
    float temperature;       // °C dari BME280
    float humidity;          // %RH dari BME280
    float pressure;          // hPa dari BME280
    float altitude;          // m (calculated)
    uint8_t status;          // Sensor status flags
    uint16_t checksum;       // Data integrity
} SensorData_t;
```

**I2C Slave Protocol:**
| Register | Address | Description | Size |
|----------|---------|-------------|------|
| STATUS | 0x00 | Sensor hub status | 1 byte |
| TIMESTAMP | 0x01 | Current timestamp | 4 bytes |
| TEMPERATURE | 0x05 | Temperature value | 4 bytes |
| HUMIDITY | 0x09 | Humidity value | 4 bytes |
| PRESSURE | 0x0D | Pressure value | 4 bytes |
| ALTITUDE | 0x11 | Altitude value | 4 bytes |
| ALL_DATA | 0x20 | Complete struct | 18 bytes |
| CONFIG | 0x30 | Configuration | 4 bytes |

### 2. Display Controller (ESP32)

**Fungsi Utama:**
- Request data dari STM32 secara periodik
- Menampilkan data pada OLED dengan UI yang informatif
- Menyimpan data ke EEPROM dengan timestamps
- Menangani user input (button)
- Alert system (buzzer, LED)

**Display Screens:**
1. **Main Dashboard** - Current readings
2. **History Graph** - Temperature trend
3. **Statistics** - Min/Max/Average
4. **Settings** - Threshold configuration
5. **Data Log** - Stored records

**User Interface State Machine:**
```
                    ┌─────────────┐
                    │   STARTUP   │
                    └──────┬──────┘
                           │
                    ┌──────▼──────┐
            ┌──────►│  DASHBOARD  │◄──────┐
            │       └──────┬──────┘       │
            │              │              │
    ┌───────┴──────┐   Button    ┌───────┴──────┐
    │   HISTORY    │◄──────────►│   SETTINGS   │
    └───────┬──────┘             └───────┬──────┘
            │                            │
    ┌───────▼──────┐             ┌───────▼──────┐
    │  STATISTICS  │             │   DATA LOG   │
    └──────────────┘             └──────────────┘
```

### 3. Data Logging

**Storage Format (EEPROM):**
```c
// Header at address 0x0000
typedef struct {
    uint32_t magic;          // 0xDEADBEEF
    uint16_t version;        // Format version
    uint16_t record_count;   // Number of records
    uint16_t head_ptr;       // Circular buffer head
    uint16_t tail_ptr;       // Circular buffer tail
} EEPROM_Header_t;

// Records start at address 0x0020
typedef struct {
    uint32_t timestamp;
    int16_t temperature;     // °C × 100
    uint16_t humidity;       // %RH × 100
    uint16_t pressure;       // (hPa - 900) × 10
} EEPROM_Record_t;          // 10 bytes per record
```

**Capacity:**
- EEPROM: 32KB (24LC256)
- Header: 32 bytes
- Per record: 10 bytes
- Max records: ~3,200 records
- At 1 minute interval: ~53 hours of data

### 4. Alert System

**Configurable Thresholds:**
| Parameter | Default Min | Default Max | Alert Type |
|-----------|-------------|-------------|------------|
| Temperature | 15°C | 35°C | Buzzer + LED |
| Humidity | 30% | 80% | LED only |
| Pressure | 980 hPa | 1030 hPa | Log only |

---

## 💻 Implementasi Kode

### STM32 - Sensor Hub (I2C Slave)

**File: `stm32_sensor_hub/src/main.c`**

```c
/**
 * @file main.c
 * @brief STM32 Sensor Hub - I2C Slave with Multiple Sensors
 */

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

/* I2C Addresses */
#define I2C_SLAVE_ADDR      0x08
#define BME280_ADDR         0x76
#define DS3231_ADDR         0x68

/* Register Map */
#define REG_STATUS          0x00
#define REG_TIMESTAMP       0x01
#define REG_TEMPERATURE     0x05
#define REG_HUMIDITY        0x09
#define REG_PRESSURE        0x0D
#define REG_ALTITUDE        0x11
#define REG_ALL_DATA        0x20
#define REG_CONFIG          0x30

/* Handles */
I2C_HandleTypeDef hi2c1;  // Master - Sensors
I2C_HandleTypeDef hi2c2;  // Slave - ESP32
UART_HandleTypeDef huart1;

/* Sensor Data Structure */
typedef struct __attribute__((packed)) {
    uint32_t timestamp;
    float temperature;
    float humidity;
    float pressure;
    float altitude;
    uint8_t status;
    uint16_t checksum;
} SensorData_t;

volatile SensorData_t sensor_data;
volatile uint8_t i2c_register = 0;
volatile uint8_t data_ready = 0;

/* BME280 Functions (simplified) */
extern uint8_t BME280_Init(void);
extern void BME280_ReadData(float* temp, float* hum, float* press);

/* DS3231 Functions */
extern uint8_t DS3231_Init(void);
extern uint32_t DS3231_GetTimestamp(void);

/* Checksum calculation */
uint16_t calculate_checksum(SensorData_t* data) {
    uint8_t* ptr = (uint8_t*)data;
    uint16_t sum = 0;
    for (int i = 0; i < sizeof(SensorData_t) - 2; i++) {
        sum += ptr[i];
    }
    return sum;
}

/* I2C Slave callbacks */
void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, 
                          uint16_t AddrMatchCode) {
    if (hi2c->Instance == I2C2) {
        if (TransferDirection == I2C_DIRECTION_TRANSMIT) {
            // Master is writing - receive register address
            HAL_I2C_Slave_Seq_Receive_IT(hi2c, (uint8_t*)&i2c_register, 1, 
                                         I2C_FIRST_FRAME);
        } else {
            // Master is reading - send data based on register
            uint8_t* data_ptr = NULL;
            uint16_t data_len = 0;
            
            switch (i2c_register) {
                case REG_STATUS:
                    data_ptr = (uint8_t*)&sensor_data.status;
                    data_len = 1;
                    break;
                case REG_TIMESTAMP:
                    data_ptr = (uint8_t*)&sensor_data.timestamp;
                    data_len = 4;
                    break;
                case REG_TEMPERATURE:
                    data_ptr = (uint8_t*)&sensor_data.temperature;
                    data_len = 4;
                    break;
                case REG_HUMIDITY:
                    data_ptr = (uint8_t*)&sensor_data.humidity;
                    data_len = 4;
                    break;
                case REG_PRESSURE:
                    data_ptr = (uint8_t*)&sensor_data.pressure;
                    data_len = 4;
                    break;
                case REG_ALL_DATA:
                    data_ptr = (uint8_t*)&sensor_data;
                    data_len = sizeof(SensorData_t);
                    break;
                default:
                    data_len = 0;
                    break;
            }
            
            if (data_len > 0) {
                HAL_I2C_Slave_Seq_Transmit_IT(hi2c, data_ptr, data_len, 
                                              I2C_LAST_FRAME);
            }
        }
    }
}

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c) {
    HAL_I2C_EnableListen_IT(hi2c);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {
    HAL_I2C_EnableListen_IT(hi2c);
}

/* Update sensor readings */
void update_sensors(void) {
    float temp, hum, press;
    
    // Read BME280
    BME280_ReadData(&temp, &hum, &press);
    
    sensor_data.temperature = temp;
    sensor_data.humidity = hum;
    sensor_data.pressure = press;
    sensor_data.altitude = 44330.0f * (1.0f - powf(press / 1013.25f, 0.1903f));
    
    // Read DS3231 timestamp
    sensor_data.timestamp = DS3231_GetTimestamp();
    
    // Update status
    sensor_data.status = 0x01;  // Data valid
    
    // Calculate checksum
    sensor_data.checksum = calculate_checksum((SensorData_t*)&sensor_data);
    
    data_ready = 1;
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();      // Master for sensors
    MX_I2C2_Init_Slave(); // Slave for ESP32
    MX_USART1_UART_Init();
    
    printf("\r\n=== STM32 Sensor Hub Started ===\r\n");
    
    // Initialize sensors
    if (!BME280_Init()) {
        printf("BME280 init failed!\r\n");
        sensor_data.status |= 0x80;  // Error flag
    }
    
    if (!DS3231_Init()) {
        printf("DS3231 init failed!\r\n");
        sensor_data.status |= 0x40;  // Error flag
    }
    
    // Enable I2C slave listening
    HAL_I2C_EnableListen_IT(&hi2c2);
    
    uint32_t last_update = 0;
    
    while (1) {
        // Update sensors every second
        if (HAL_GetTick() - last_update >= 1000) {
            last_update = HAL_GetTick();
            
            update_sensors();
            
            // Debug output
            printf("T:%.1f H:%.1f P:%.1f A:%.1f\r\n",
                   sensor_data.temperature,
                   sensor_data.humidity,
                   sensor_data.pressure,
                   sensor_data.altitude);
            
            // Toggle LED
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        }
    }
}

/* I2C2 Slave Initialization */
static void MX_I2C2_Init_Slave(void) {
    hi2c2.Instance = I2C2;
    hi2c2.Init.ClockSpeed = 100000;
    hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c2.Init.OwnAddress1 = I2C_SLAVE_ADDR << 1;
    hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    HAL_I2C_Init(&hi2c2);
}

/* GPIO for I2C2 (PB10=SCL, PB11=SDA) */
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    if (hi2c->Instance == I2C1) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_I2C1_CLK_ENABLE();
        GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
    else if (hi2c->Instance == I2C2) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_I2C2_CLK_ENABLE();
        GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        
        HAL_NVIC_SetPriority(I2C2_EV_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);
        HAL_NVIC_SetPriority(I2C2_ER_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(I2C2_ER_IRQn);
    }
}
```

### ESP32 - Display Controller

**File: `esp32_display_controller/src/main.cpp`**

```cpp
/**
 * @file main.cpp
 * @brief ESP32 Display Controller - I2C Master with OLED and EEPROM
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// I2C Addresses
#define STM32_SLAVE_ADDR    0x08
#define OLED_ADDR           0x3C
#define EEPROM_ADDR         0x50

// Pins
#define I2C_SDA             21
#define I2C_SCL             22
#define BUTTON_PIN          0
#define BUZZER_PIN          25
#define LED_PIN             2

// Display
#define SCREEN_WIDTH        128
#define SCREEN_HEIGHT       64

// STM32 Registers
#define REG_STATUS          0x00
#define REG_ALL_DATA        0x20

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Sensor Data Structure (must match STM32)
struct __attribute__((packed)) SensorData {
    uint32_t timestamp;
    float temperature;
    float humidity;
    float pressure;
    float altitude;
    uint8_t status;
    uint16_t checksum;
};

SensorData currentData;
SensorData historyData[60];  // 1 hour of data at 1 min intervals
int historyIndex = 0;

// Thresholds
float tempMin = 15.0, tempMax = 35.0;
float humMin = 30.0, humMax = 80.0;

// UI State
enum Screen { DASHBOARD, HISTORY, STATISTICS, SETTINGS, DATALOG };
Screen currentScreen = DASHBOARD;

// Button handling
volatile bool buttonPressed = false;
unsigned long lastDebounce = 0;

void IRAM_ATTR buttonISR() {
    if (millis() - lastDebounce > 200) {
        buttonPressed = true;
        lastDebounce = millis();
    }
}

// Read data from STM32 Slave
bool readSTM32Data() {
    Wire.beginTransmission(STM32_SLAVE_ADDR);
    Wire.write(REG_ALL_DATA);
    if (Wire.endTransmission() != 0) {
        Serial.println("STM32 write error");
        return false;
    }
    
    delay(5);
    
    uint8_t bytesRead = Wire.requestFrom(STM32_SLAVE_ADDR, sizeof(SensorData));
    if (bytesRead != sizeof(SensorData)) {
        Serial.printf("Read error: got %d bytes\n", bytesRead);
        return false;
    }
    
    uint8_t* ptr = (uint8_t*)&currentData;
    for (int i = 0; i < sizeof(SensorData); i++) {
        ptr[i] = Wire.read();
    }
    
    // Verify checksum
    uint16_t calcSum = 0;
    for (int i = 0; i < sizeof(SensorData) - 2; i++) {
        calcSum += ptr[i];
    }
    
    if (calcSum != currentData.checksum) {
        Serial.println("Checksum mismatch!");
        return false;
    }
    
    return true;
}

// EEPROM functions
void EEPROM_WriteByte(uint16_t addr, uint8_t data) {
    Wire.beginTransmission(EEPROM_ADDR);
    Wire.write((uint8_t)(addr >> 8));
    Wire.write((uint8_t)(addr & 0xFF));
    Wire.write(data);
    Wire.endTransmission();
    delay(5);
}

uint8_t EEPROM_ReadByte(uint16_t addr) {
    Wire.beginTransmission(EEPROM_ADDR);
    Wire.write((uint8_t)(addr >> 8));
    Wire.write((uint8_t)(addr & 0xFF));
    Wire.endTransmission();
    Wire.requestFrom(EEPROM_ADDR, 1);
    return Wire.read();
}

void saveToEEPROM(SensorData* data) {
    // Simplified - implement circular buffer in production
    static uint16_t writeAddr = 0x0020;
    
    uint8_t* ptr = (uint8_t*)data;
    for (int i = 0; i < 10; i++) {  // Save compact record
        EEPROM_WriteByte(writeAddr++, ptr[i]);
    }
    
    if (writeAddr >= 32768 - 32) writeAddr = 0x0020;  // Wrap
}

// Alert checking
void checkAlerts() {
    bool alert = false;
    
    if (currentData.temperature < tempMin || currentData.temperature > tempMax) {
        alert = true;
        digitalWrite(LED_PIN, HIGH);
    }
    
    if (currentData.humidity < humMin || currentData.humidity > humMax) {
        alert = true;
    }
    
    if (alert) {
        tone(BUZZER_PIN, 2000, 100);
    } else {
        digitalWrite(LED_PIN, LOW);
    }
}

// Display functions
void drawDashboard() {
    display.clearDisplay();
    display.setTextColor(WHITE);
    
    // Header
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("ENVIRONMENTAL MONITOR");
    display.drawLine(0, 10, 127, 10, WHITE);
    
    // Temperature (large)
    display.setTextSize(2);
    display.setCursor(0, 14);
    display.printf("%.1fC", currentData.temperature);
    
    // Humidity
    display.setCursor(75, 14);
    display.printf("%.0f%%", currentData.humidity);
    
    // Pressure
    display.setTextSize(1);
    display.setCursor(0, 35);
    display.printf("Press: %.1f hPa", currentData.pressure);
    
    // Altitude
    display.setCursor(0, 45);
    display.printf("Alt: %.0f m", currentData.altitude);
    
    // Status bar
    display.drawLine(0, 54, 127, 54, WHITE);
    display.setCursor(0, 56);
    
    // Time from timestamp
    uint32_t ts = currentData.timestamp;
    int h = (ts / 3600) % 24;
    int m = (ts / 60) % 60;
    int s = ts % 60;
    display.printf("%02d:%02d:%02d", h, m, s);
    
    // Status indicator
    display.setCursor(100, 56);
    display.print(currentData.status == 0x01 ? "OK" : "ERR");
    
    display.display();
}

void drawHistory() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("TEMPERATURE HISTORY");
    display.drawLine(0, 10, 127, 10, WHITE);
    
    // Draw graph axes
    display.drawLine(10, 15, 10, 55, WHITE);   // Y axis
    display.drawLine(10, 55, 120, 55, WHITE);  // X axis
    
    // Find min/max for scaling
    float minT = 100, maxT = -40;
    for (int i = 0; i < 60; i++) {
        if (historyData[i].temperature < minT) minT = historyData[i].temperature;
        if (historyData[i].temperature > maxT) maxT = historyData[i].temperature;
    }
    
    // Draw temperature line
    for (int i = 1; i < 60; i++) {
        int x1 = 10 + ((i-1) * 110 / 60);
        int x2 = 10 + (i * 110 / 60);
        int y1 = 55 - (int)((historyData[i-1].temperature - minT) / (maxT - minT) * 38);
        int y2 = 55 - (int)((historyData[i].temperature - minT) / (maxT - minT) * 38);
        display.drawLine(x1, y1, x2, y2, WHITE);
    }
    
    // Labels
    display.setCursor(0, 56);
    display.printf("%.0f-%.0fC", minT, maxT);
    
    display.display();
}

void drawStatistics() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("STATISTICS");
    display.drawLine(0, 10, 127, 10, WHITE);
    
    // Calculate stats
    float sumT = 0, sumH = 0, sumP = 0;
    float minT = 100, maxT = -40;
    float minH = 100, maxH = 0;
    int count = 0;
    
    for (int i = 0; i < 60; i++) {
        if (historyData[i].status == 0x01) {
            sumT += historyData[i].temperature;
            sumH += historyData[i].humidity;
            sumP += historyData[i].pressure;
            if (historyData[i].temperature < minT) minT = historyData[i].temperature;
            if (historyData[i].temperature > maxT) maxT = historyData[i].temperature;
            if (historyData[i].humidity < minH) minH = historyData[i].humidity;
            if (historyData[i].humidity > maxH) maxH = historyData[i].humidity;
            count++;
        }
    }
    
    if (count > 0) {
        display.setCursor(0, 14);
        display.printf("Temp Avg: %.1fC", sumT / count);
        display.setCursor(0, 24);
        display.printf("Temp Min/Max: %.1f/%.1fC", minT, maxT);
        display.setCursor(0, 34);
        display.printf("Hum Avg: %.1f%%", sumH / count);
        display.setCursor(0, 44);
        display.printf("Hum Min/Max: %.0f/%.0f%%", minH, maxH);
        display.setCursor(0, 54);
        display.printf("Samples: %d", count);
    } else {
        display.setCursor(0, 30);
        display.print("No data yet");
    }
    
    display.display();
}

void handleButton() {
    if (buttonPressed) {
        buttonPressed = false;
        currentScreen = (Screen)((currentScreen + 1) % 5);
        Serial.printf("Screen: %d\n", currentScreen);
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=== ESP32 Display Controller ===\n");
    
    // GPIO setup
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);
    
    // I2C setup
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(100000);
    
    // OLED setup
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println("SSD1306 init failed!");
        while (1);
    }
    
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(10, 20);
    display.println("ENV MON");
    display.setTextSize(1);
    display.setCursor(20, 45);
    display.println("Initializing...");
    display.display();
    delay(2000);
    
    // Initialize history
    memset(historyData, 0, sizeof(historyData));
    
    Serial.println("System ready!");
}

void loop() {
    static unsigned long lastRead = 0;
    static unsigned long lastHistory = 0;
    
    handleButton();
    
    // Read from STM32 every second
    if (millis() - lastRead >= 1000) {
        lastRead = millis();
        
        if (readSTM32Data()) {
            Serial.printf("T:%.1f H:%.1f P:%.1f\n",
                         currentData.temperature,
                         currentData.humidity,
                         currentData.pressure);
            
            checkAlerts();
        }
    }
    
    // Store history every minute
    if (millis() - lastHistory >= 60000) {
        lastHistory = millis();
        
        historyData[historyIndex] = currentData;
        historyIndex = (historyIndex + 1) % 60;
        
        saveToEEPROM(&currentData);
    }
    
    // Update display
    switch (currentScreen) {
        case DASHBOARD:
            drawDashboard();
            break;
        case HISTORY:
            drawHistory();
            break;
        case STATISTICS:
            drawStatistics();
            break;
        case SETTINGS:
            // Draw settings screen
            break;
        case DATALOG:
            // Draw data log screen
            break;
    }
    
    delay(100);
}
```

---

## 📊 Kriteria Penilaian

### Komponen Nilai

| Komponen | Bobot | Kriteria |
|----------|-------|----------|
| **Hardware Assembly** | 15% | Rangkaian benar, rapi, dan berfungsi |
| **STM32 Sensor Hub** | 20% | I2C Slave berfungsi, semua sensor terbaca |
| **ESP32 Display** | 20% | UI responsif, semua screen berfungsi |
| **Data Communication** | 15% | I2C antar MCU stabil, checksum valid |
| **Data Logging** | 10% | EEPROM read/write benar, persistent |
| **Alert System** | 5% | Threshold detection dan notification |
| **Error Handling** | 5% | Recovery dari error, graceful degradation |
| **Dokumentasi** | 5% | Laporan lengkap dan jelas |
| **Video Demo** | 5% | Demonstrasi fitur lengkap |

### Level Pencapaian

| Grade | Skor | Kriteria |
|-------|------|----------|
| **A** | 85-100 | Semua fitur berfungsi + bonus features |
| **B** | 70-84 | Semua fitur utama berfungsi |
| **C** | 55-69 | Sebagian besar fitur berfungsi |
| **D** | 40-54 | Minimal sensor dan display bekerja |
| **E** | <40 | Tidak memenuhi minimum requirements |

### Bonus Points (+5% each)

- [ ] Sensor tambahan (AHT20, BH1750, dll)
- [ ] WiFi connectivity dengan web dashboard
- [ ] SD Card logging sebagai backup
- [ ] Power management (sleep modes)
- [ ] Custom PCB design

---

## 📦 Deliverables

### 1. Source Code
- [ ] STM32 project (PlatformIO/CubeIDE)
- [ ] ESP32 project (PlatformIO)
- [ ] Well-commented code
- [ ] README dengan instruksi build

### 2. Dokumentasi
- [ ] Schematic diagram
- [ ] Wiring photos
- [ ] Block diagram
- [ ] Flow charts

### 3. Laporan
- [ ] Pendahuluan dan tujuan
- [ ] Teori singkat I2C Multi-device
- [ ] Desain sistem
- [ ] Implementasi
- [ ] Hasil pengujian
- [ ] Analisis dan kesimpulan

### 4. Video Demonstrasi
- [ ] Durasi: 5-10 menit
- [ ] Hardware overview
- [ ] All features demonstration
- [ ] I2C communication proof
- [ ] Error handling demo

---

## 📅 Timeline

| Minggu | Aktivitas |
|--------|-----------|
| 1 | Hardware assembly, sensor testing individual |
| 2 | STM32 Slave implementation, ESP32 Master implementation |
| 3 | Integration, debugging, documentation |

---

## 💡 Tips Pengerjaan

1. **Start Simple**: Test setiap sensor secara individual dulu
2. **I2C Debugging**: Gunakan logic analyzer atau serial monitor
3. **Checksum**: Selalu validasi data yang diterima
4. **Error Handling**: Implementasikan timeout dan retry
5. **Modular Code**: Pisahkan driver sensor dari logic utama
6. **Version Control**: Gunakan Git untuk tracking perubahan

---

## 📚 Referensi Tambahan

1. STM32 I2C Slave HAL Tutorial
2. ESP32 Wire Library Documentation
3. Adafruit SSD1306 Library Examples
4. BME280 Datasheet - Calibration Section
5. "Mastering STM32" - I2C Chapter
