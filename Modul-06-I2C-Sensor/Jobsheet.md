# JOBSHEET BAB 06: I2C Bus dan Sensor Integration

## 📋 Informasi Umum

| Item | Keterangan |
|------|------------|
| **Mata Kuliah** | Praktikum Sistem Embedded |
| **Topik** | I2C Bus dan Integrasi Sensor |
| **Waktu** | 3 × 170 menit |
| **Tools** | PlatformIO, STM32CubeIDE, Logic Analyzer |

### Persiapan Hardware

**Komponen yang Dibutuhkan:**

| No | Komponen | Jumlah | Keterangan |
|----|----------|--------|------------|
| 1 | STM32F103C8T6 (Blue Pill) | 1 | Microcontroller utama |
| 2 | ESP32 DevKit V1 | 1 | Microcontroller utama |
| 3 | BME280 Module | 1 | Temp/Humidity/Pressure sensor |
| 4 | SSD1306 OLED 128×64 | 1 | Display I2C |
| 5 | DS3231 RTC Module | 1 | Real-Time Clock |
| 6 | 24LC256 EEPROM | 1 | External storage |
| 7 | Resistor 4.7kΩ | 4 | Pull-up resistors |
| 8 | Breadboard | 1 | Prototyping |
| 9 | Jumper Wires | ~30 | Koneksi |
| 10 | ST-Link V2 | 1 | Programmer STM32 |
| 11 | USB Cable | 2 | Power dan programming |

### Skema Koneksi

**STM32F103 I2C Connections:**
```
STM32F103        I2C Devices (BME280, OLED, RTC, EEPROM)
┌─────────┐      ┌──────────────────┐
│      PB6├──────┤SCL (All Devices) │
│      PB7├──────┤SDA (All Devices) │
│     3.3V├──────┤VCC               │
│      GND├──────┤GND               │
└─────────┘      └──────────────────┘
                 Note: 4.7kΩ pull-ups on SDA and SCL
```

**ESP32 I2C Connections:**
```
ESP32            I2C Devices
┌─────────┐      ┌──────────────────┐
│    GPIO22├─────┤SCL (All Devices) │
│    GPIO21├─────┤SDA (All Devices) │
│      3.3V├─────┤VCC               │
│       GND├─────┤GND               │
└──────────┘     └──────────────────┘
```

---

## 🔬 Daftar Praktikum

### Jumlah Program STM32: 12
### Jumlah Program ESP32: 12

---

## PROGRAM 1: I2C Bus Scanner (STM32)

### 📝 Deskripsi
Membuat scanner untuk mendeteksi semua perangkat I2C yang terhubung pada bus.

### 🎯 Tujuan
- Memahami cara kerja addressing I2C
- Mendeteksi perangkat yang terhubung pada bus
- Troubleshooting koneksi I2C

### 📊 Diagram Blok
```
┌─────────────┐     ┌─────────┐     ┌─────────┐
│   STM32     │────►│  I2C    │────►│ Device  │
│  (Master)   │◄────│   Bus   │◄────│ (Slave) │
└─────────────┘     └─────────┘     └─────────┘
                         │
                    ┌────┴────┐
                   Rp        Rp
                    │         │
                   Vcc       Vcc
```

### 💻 Kode Program

**File: `STM32_01_I2C_Bus_Scan/src/main.c`**

```c
/**
 * @file main.c
 * @brief I2C Bus Scanner for STM32F103
 * @details Scans I2C bus and reports all connected devices
 */

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

/* Private variables */
I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart1;

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
void I2C_Scan(void);

/* UART redirect for printf */
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

/* Known I2C device database */
typedef struct {
    uint8_t address;
    const char* name;
} I2C_Device_t;

const I2C_Device_t known_devices[] = {
    {0x3C, "SSD1306 OLED (Addr 0)"},
    {0x3D, "SSD1306 OLED (Addr 1)"},
    {0x50, "24LC EEPROM (A0=A1=A2=0)"},
    {0x51, "24LC EEPROM (A0=1)"},
    {0x52, "24LC EEPROM (A1=1)"},
    {0x57, "24LC EEPROM (All=1)"},
    {0x68, "DS3231 RTC / MPU6050"},
    {0x69, "MPU6050 (AD0=1)"},
    {0x76, "BME280/BMP280 (SDO=0)"},
    {0x77, "BME280/BMP280 (SDO=1)"},
    {0x27, "PCF8574 LCD I2C"},
    {0x20, "PCF8574A"},
    {0x48, "ADS1115 ADC"},
    {0x00, NULL}  // End marker
};

const char* get_device_name(uint8_t addr) {
    for (int i = 0; known_devices[i].name != NULL; i++) {
        if (known_devices[i].address == addr) {
            return known_devices[i].name;
        }
    }
    return "Unknown Device";
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_I2C1_Init();
    
    printf("\r\n========================================\r\n");
    printf("   STM32F103 I2C Bus Scanner\r\n");
    printf("========================================\r\n\n");
    
    while (1) {
        printf("Press Enter to scan I2C bus...\r\n");
        
        // Wait for user input
        uint8_t ch;
        HAL_UART_Receive(&huart1, &ch, 1, HAL_MAX_DELAY);
        
        I2C_Scan();
        
        HAL_Delay(1000);
    }
}

void I2C_Scan(void) {
    uint8_t devices_found = 0;
    HAL_StatusTypeDef result;
    
    printf("\r\nScanning I2C bus (addresses 0x01 - 0x7F)...\r\n\n");
    printf("     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\r\n");
    
    for (uint8_t row = 0; row < 8; row++) {
        printf("%02X: ", row * 16);
        
        for (uint8_t col = 0; col < 16; col++) {
            uint8_t addr = row * 16 + col;
            
            if (addr < 0x03 || addr > 0x77) {
                printf("   ");  // Reserved addresses
                continue;
            }
            
            result = HAL_I2C_IsDeviceReady(&hi2c1, addr << 1, 2, 10);
            
            if (result == HAL_OK) {
                printf("%02X ", addr);
                devices_found++;
            } else {
                printf("-- ");
            }
        }
        printf("\r\n");
    }
    
    printf("\r\n----------------------------------------\r\n");
    printf("Total devices found: %d\r\n\n", devices_found);
    
    // List found devices with names
    if (devices_found > 0) {
        printf("Device List:\r\n");
        for (uint8_t addr = 0x03; addr <= 0x77; addr++) {
            result = HAL_I2C_IsDeviceReady(&hi2c1, addr << 1, 1, 10);
            if (result == HAL_OK) {
                printf("  0x%02X - %s\r\n", addr, get_device_name(addr));
            }
        }
    }
    printf("----------------------------------------\r\n\n");
}

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

static void MX_I2C1_Init(void) {
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;  // 100 kHz Standard Mode
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    HAL_I2C_Init(&hi2c1);
}

static void MX_USART1_UART_Init(void) {
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart1);
}

static void MX_GPIO_Init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    // Configure LED on PC13
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

/* I2C MSP Initialization */
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    if (hi2c->Instance == I2C1) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_I2C1_CLK_ENABLE();
        
        // PB6 -> SCL, PB7 -> SDA
        GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
}

void HAL_UART_MspInit(UART_HandleTypeDef* huart) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    if (huart->Instance == USART1) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_USART1_CLK_ENABLE();
        
        // PA9 -> TX, PA10 -> RX
        GPIO_InitStruct.Pin = GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        
        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}
```

### 📈 Hasil yang Diharapkan
```
========================================
   STM32F103 I2C Bus Scanner
========================================

Scanning I2C bus (addresses 0x01 - 0x7F)...

     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
00:          -- -- -- -- -- -- -- -- -- -- -- -- --
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- -- 3C -- -- --
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
50: 50 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- -- 68 -- -- -- -- -- -- --
70: -- -- -- -- -- -- 76 --

----------------------------------------
Total devices found: 4

Device List:
  0x3C - SSD1306 OLED (Addr 0)
  0x50 - 24LC EEPROM (A0=A1=A2=0)
  0x68 - DS3231 RTC / MPU6050
  0x76 - BME280/BMP280 (SDO=0)
----------------------------------------
```

### ❓ Pertanyaan Analisis
1. Mengapa scanning dimulai dari address 0x03 dan bukan 0x00?
2. Apa yang terjadi jika dua device memiliki address yang sama?
3. Bagaimana pengaruh pull-up resistor terhadap hasil scanning?

---

## PROGRAM 2: I2C Bus Scanner (ESP32)

### 📝 Deskripsi
Implementasi I2C scanner pada ESP32 dengan Wire library.

### 💻 Kode Program

**File: `ESP32_01_I2C_Bus_Scan/src/main.cpp`**

```cpp
/**
 * @file main.cpp
 * @brief I2C Bus Scanner for ESP32
 * @details Scans I2C bus using Wire library
 */

#include <Arduino.h>
#include <Wire.h>

// I2C Pin definitions
#define I2C_SDA 21
#define I2C_SCL 22

// Known device database
struct I2CDevice {
    uint8_t address;
    const char* name;
};

const I2CDevice knownDevices[] = {
    {0x3C, "SSD1306 OLED"},
    {0x3D, "SSD1306 OLED (Alt)"},
    {0x50, "24LC EEPROM"},
    {0x68, "DS3231 RTC / MPU6050"},
    {0x76, "BME280 (SDO=GND)"},
    {0x77, "BME280 (SDO=VCC)"},
    {0x27, "LCD I2C (PCF8574)"},
    {0x20, "PCF8574A"},
    {0x48, "ADS1115 ADC"},
    {0x29, "VL53L0X ToF"},
    {0x1E, "HMC5883L Compass"},
    {0x00, nullptr}  // End marker
};

const char* getDeviceName(uint8_t addr) {
    for (int i = 0; knownDevices[i].name != nullptr; i++) {
        if (knownDevices[i].address == addr) {
            return knownDevices[i].name;
        }
    }
    return "Unknown Device";
}

void scanI2C() {
    Serial.println("\n========================================");
    Serial.println("     ESP32 I2C Bus Scanner");
    Serial.println("========================================\n");
    
    Serial.printf("SDA Pin: GPIO%d\n", I2C_SDA);
    Serial.printf("SCL Pin: GPIO%d\n", I2C_SCL);
    Serial.println();
    
    uint8_t devicesFound = 0;
    
    Serial.println("Scanning...\n");
    Serial.println("     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");
    
    for (uint8_t row = 0; row < 8; row++) {
        Serial.printf("%02X: ", row * 16);
        
        for (uint8_t col = 0; col < 16; col++) {
            uint8_t addr = row * 16 + col;
            
            if (addr < 0x03 || addr > 0x77) {
                Serial.print("   ");
                continue;
            }
            
            Wire.beginTransmission(addr);
            uint8_t error = Wire.endTransmission();
            
            if (error == 0) {
                Serial.printf("%02X ", addr);
                devicesFound++;
            } else if (error == 4) {
                Serial.print("?? ");  // Unknown error
            } else {
                Serial.print("-- ");
            }
        }
        Serial.println();
    }
    
    Serial.println();
    Serial.println("----------------------------------------");
    Serial.printf("Total devices found: %d\n\n", devicesFound);
    
    if (devicesFound > 0) {
        Serial.println("Detected Devices:");
        Serial.println("----------------------------------------");
        
        for (uint8_t addr = 0x03; addr <= 0x77; addr++) {
            Wire.beginTransmission(addr);
            if (Wire.endTransmission() == 0) {
                Serial.printf("  0x%02X - %s\n", addr, getDeviceName(addr));
            }
        }
        Serial.println("----------------------------------------");
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n");
    Serial.println("================================");
    Serial.println("  I2C Scanner Initialization");
    Serial.println("================================");
    
    // Initialize I2C
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(100000);  // 100 kHz
    
    Serial.println("I2C initialized successfully!");
    Serial.println("Type 's' to scan I2C bus\n");
}

void loop() {
    if (Serial.available()) {
        char cmd = Serial.read();
        
        if (cmd == 's' || cmd == 'S') {
            scanI2C();
        } else if (cmd == 'f') {
            // Fast mode scan
            Wire.setClock(400000);
            Serial.println("Switched to Fast Mode (400 kHz)");
            scanI2C();
            Wire.setClock(100000);
        }
    }
    
    // Auto scan every 10 seconds
    static unsigned long lastScan = 0;
    if (millis() - lastScan > 10000) {
        lastScan = millis();
        scanI2C();
    }
}
```

### 📈 Hasil yang Diharapkan
Output serupa dengan STM32 scanner, menampilkan semua device yang terdeteksi.

---

## PROGRAM 3: BME280 Sensor Reading (STM32)

### 📝 Deskripsi
Membaca data temperatur, kelembaban, dan tekanan dari sensor BME280.

### 💻 Kode Program

**File: `STM32_02_BME280_Sensor/src/main.c`**

```c
/**
 * @file main.c
 * @brief BME280 Sensor Reading for STM32F103
 * @details Reads temperature, humidity, and pressure
 */

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <math.h>

/* BME280 I2C Address */
#define BME280_ADDR     0x76

/* BME280 Registers */
#define BME280_REG_ID           0xD0
#define BME280_REG_CTRL_HUM     0xF2
#define BME280_REG_STATUS       0xF3
#define BME280_REG_CTRL_MEAS    0xF4
#define BME280_REG_CONFIG       0xF5
#define BME280_REG_PRESS_MSB    0xF7
#define BME280_REG_CALIB00      0x88
#define BME280_REG_CALIB26      0xE1

/* BME280 Chip ID */
#define BME280_CHIP_ID          0x60

/* Handles */
I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart1;

/* BME280 Calibration Data */
typedef struct {
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;
    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;
    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4;
    int16_t  dig_H5;
    int8_t   dig_H6;
} BME280_CalibData_t;

BME280_CalibData_t calib_data;
int32_t t_fine;

/* Function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);

int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

/* BME280 Functions */
HAL_StatusTypeDef BME280_ReadReg(uint8_t reg, uint8_t* data, uint16_t len) {
    return HAL_I2C_Mem_Read(&hi2c1, BME280_ADDR << 1, reg, 
                            I2C_MEMADD_SIZE_8BIT, data, len, 100);
}

HAL_StatusTypeDef BME280_WriteReg(uint8_t reg, uint8_t data) {
    return HAL_I2C_Mem_Write(&hi2c1, BME280_ADDR << 1, reg,
                             I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
}

uint8_t BME280_Init(void) {
    uint8_t chip_id;
    uint8_t calib[26];
    uint8_t calib_h[7];
    
    // Read chip ID
    if (BME280_ReadReg(BME280_REG_ID, &chip_id, 1) != HAL_OK) {
        printf("Failed to read BME280 ID\r\n");
        return 0;
    }
    
    if (chip_id != BME280_CHIP_ID) {
        printf("Invalid chip ID: 0x%02X (expected 0x%02X)\r\n", chip_id, BME280_CHIP_ID);
        return 0;
    }
    
    printf("BME280 detected! Chip ID: 0x%02X\r\n", chip_id);
    
    // Read calibration data
    BME280_ReadReg(BME280_REG_CALIB00, calib, 26);
    BME280_ReadReg(BME280_REG_CALIB26, calib_h, 7);
    
    // Parse calibration data
    calib_data.dig_T1 = (uint16_t)(calib[1] << 8) | calib[0];
    calib_data.dig_T2 = (int16_t)(calib[3] << 8) | calib[2];
    calib_data.dig_T3 = (int16_t)(calib[5] << 8) | calib[4];
    
    calib_data.dig_P1 = (uint16_t)(calib[7] << 8) | calib[6];
    calib_data.dig_P2 = (int16_t)(calib[9] << 8) | calib[8];
    calib_data.dig_P3 = (int16_t)(calib[11] << 8) | calib[10];
    calib_data.dig_P4 = (int16_t)(calib[13] << 8) | calib[12];
    calib_data.dig_P5 = (int16_t)(calib[15] << 8) | calib[14];
    calib_data.dig_P6 = (int16_t)(calib[17] << 8) | calib[16];
    calib_data.dig_P7 = (int16_t)(calib[19] << 8) | calib[18];
    calib_data.dig_P8 = (int16_t)(calib[21] << 8) | calib[20];
    calib_data.dig_P9 = (int16_t)(calib[23] << 8) | calib[22];
    
    calib_data.dig_H1 = calib[25];
    calib_data.dig_H2 = (int16_t)(calib_h[1] << 8) | calib_h[0];
    calib_data.dig_H3 = calib_h[2];
    calib_data.dig_H4 = (int16_t)(calib_h[3] << 4) | (calib_h[4] & 0x0F);
    calib_data.dig_H5 = (int16_t)(calib_h[5] << 4) | ((calib_h[4] >> 4) & 0x0F);
    calib_data.dig_H6 = (int8_t)calib_h[6];
    
    // Configure sensor
    BME280_WriteReg(BME280_REG_CTRL_HUM, 0x01);
    BME280_WriteReg(BME280_REG_CONFIG, 0xA0);
    BME280_WriteReg(BME280_REG_CTRL_MEAS, 0x27);
    
    return 1;
}

int32_t BME280_CompensateTemp(int32_t adc_T) {
    int32_t var1, var2;
    var1 = ((((adc_T >> 3) - ((int32_t)calib_data.dig_T1 << 1))) * 
            ((int32_t)calib_data.dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)calib_data.dig_T1)) * 
             ((adc_T >> 4) - ((int32_t)calib_data.dig_T1))) >> 12) * 
            ((int32_t)calib_data.dig_T3)) >> 14;
    t_fine = var1 + var2;
    return (t_fine * 5 + 128) >> 8;
}

uint32_t BME280_CompensatePress(int32_t adc_P) {
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)calib_data.dig_P6;
    var2 = var2 + ((var1 * (int64_t)calib_data.dig_P5) << 17);
    var2 = var2 + (((int64_t)calib_data.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)calib_data.dig_P3) >> 8) + 
           ((var1 * (int64_t)calib_data.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calib_data.dig_P1) >> 33;
    if (var1 == 0) return 0;
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calib_data.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)calib_data.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)calib_data.dig_P7) << 4);
    return (uint32_t)p;
}

uint32_t BME280_CompensateHum(int32_t adc_H) {
    int32_t v_x1_u32r;
    v_x1_u32r = (t_fine - ((int32_t)76800));
    v_x1_u32r = (((((adc_H << 14) - (((int32_t)calib_data.dig_H4) << 20) - 
                   (((int32_t)calib_data.dig_H5) * v_x1_u32r)) + 
                  ((int32_t)16384)) >> 15) * 
                (((((((v_x1_u32r * ((int32_t)calib_data.dig_H6)) >> 10) * 
                    (((v_x1_u32r * ((int32_t)calib_data.dig_H3)) >> 11) + 
                     ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * 
                  ((int32_t)calib_data.dig_H2) + 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * 
                               ((int32_t)calib_data.dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
    return (uint32_t)(v_x1_u32r >> 12);
}

void BME280_ReadData(float* temperature, float* pressure, float* humidity) {
    uint8_t data[8];
    int32_t adc_T, adc_P, adc_H;
    BME280_ReadReg(BME280_REG_PRESS_MSB, data, 8);
    adc_P = ((int32_t)data[0] << 12) | ((int32_t)data[1] << 4) | ((int32_t)data[2] >> 4);
    adc_T = ((int32_t)data[3] << 12) | ((int32_t)data[4] << 4) | ((int32_t)data[5] >> 4);
    adc_H = ((int32_t)data[6] << 8) | (int32_t)data[7];
    *temperature = BME280_CompensateTemp(adc_T) / 100.0f;
    *pressure = BME280_CompensatePress(adc_P) / 25600.0f;
    *humidity = BME280_CompensateHum(adc_H) / 1024.0f;
}

float BME280_CalculateAltitude(float pressure, float seaLevelPressure) {
    return 44330.0f * (1.0f - powf(pressure / seaLevelPressure, 0.1903f));
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_I2C1_Init();
    
    printf("\r\n========================================\r\n");
    printf("   STM32F103 BME280 Sensor Demo\r\n");
    printf("========================================\r\n\n");
    
    if (!BME280_Init()) {
        printf("BME280 initialization failed!\r\n");
        while(1) {
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
            HAL_Delay(100);
        }
    }
    
    printf("BME280 initialized successfully!\r\n\n");
    
    while (1) {
        float temperature, pressure, humidity;
        BME280_ReadData(&temperature, &pressure, &humidity);
        float altitude = BME280_CalculateAltitude(pressure, 1013.25f);
        
        printf("----------------------------------------\r\n");
        printf("Temperature: %.2f C\r\n", temperature);
        printf("Humidity:    %.2f %%RH\r\n", humidity);
        printf("Pressure:    %.2f hPa\r\n", pressure);
        printf("Altitude:    %.2f m (approx)\r\n", altitude);
        printf("----------------------------------------\r\n\n");
        
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_Delay(2000);
    }
}

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

static void MX_I2C1_Init(void) {
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 400000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    HAL_I2C_Init(&hi2c1);
}

static void MX_USART1_UART_Init(void) {
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart1);
}

static void MX_GPIO_Init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

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
}

void HAL_UART_MspInit(UART_HandleTypeDef* huart) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (huart->Instance == USART1) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_USART1_CLK_ENABLE();
        GPIO_InitStruct.Pin = GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}
```

### 📈 Hasil yang Diharapkan
```
========================================
   STM32F103 BME280 Sensor Demo
========================================

BME280 detected! Chip ID: 0x60
BME280 initialized successfully!

----------------------------------------
Temperature: 25.43 C
Humidity:    65.21 %RH
Pressure:    1013.25 hPa
Altitude:    0.00 m (approx)
----------------------------------------
```

---

## PROGRAM 4: BME280 Sensor Reading (ESP32)

### 📝 Deskripsi
Membaca sensor BME280 menggunakan library Adafruit pada ESP32.

### 💻 Kode Program

**File: `ESP32_02_BME280_Sensor/src/main.cpp`**

```cpp
/**
 * @file main.cpp
 * @brief BME280 Environmental Sensor on ESP32
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define I2C_SDA 21
#define I2C_SCL 22
#define BME280_ADDR 0x76
#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n================================");
    Serial.println("  ESP32 BME280 Sensor Demo");
    Serial.println("================================\n");
    
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(400000);
    
    if (!bme.begin(BME280_ADDR, &Wire)) {
        Serial.println("Could not find BME280!");
        while (1);
    }
    
    bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                    Adafruit_BME280::SAMPLING_X16,
                    Adafruit_BME280::SAMPLING_X16,
                    Adafruit_BME280::SAMPLING_X16,
                    Adafruit_BME280::FILTER_X16,
                    Adafruit_BME280::STANDBY_MS_0_5);
    
    Serial.println("BME280 initialized!\n");
}

void loop() {
    float temp = bme.readTemperature();
    float hum = bme.readHumidity();
    float press = bme.readPressure() / 100.0F;
    float alt = bme.readAltitude(SEALEVELPRESSURE_HPA);
    
    Serial.println("----------------------------------------");
    Serial.printf("Temperature: %.2f C\n", temp);
    Serial.printf("Humidity:    %.2f %%RH\n", hum);
    Serial.printf("Pressure:    %.2f hPa\n", press);
    Serial.printf("Altitude:    %.2f m\n", alt);
    Serial.println("----------------------------------------\n");
    
    delay(2000);
}
```

**platformio.ini:**
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps = 
    adafruit/Adafruit BME280 Library@^2.2.2
    adafruit/Adafruit Unified Sensor@^1.1.9
```

---

## PROGRAM 5-6: SSD1306 OLED Display (STM32 & ESP32)

### 📝 Deskripsi
Menampilkan teks dan grafik pada OLED display SSD1306 128×64.

### 💻 Kode ESP32

**File: `ESP32_03_OLED_Display/src/main.cpp`**

```cpp
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
    Serial.begin(115200);
    Wire.begin(21, 22);
    
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("SSD1306 allocation failed!");
        while (1);
    }
    
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(10, 10);
    display.println("ESP32");
    display.setCursor(10, 30);
    display.println("OLED");
    display.display();
}

void loop() {
    // Demo animations
    display.clearDisplay();
    display.drawRect(0, 0, 128, 64, WHITE);
    display.drawCircle(64, 32, 20, WHITE);
    display.display();
    delay(2000);
    
    display.clearDisplay();
    for (int i = 0; i < 128; i += 8) {
        display.drawLine(0, 32, i, 0, WHITE);
        display.drawLine(0, 32, i, 63, WHITE);
    }
    display.display();
    delay(2000);
}
```

---

## PROGRAM 7-8: DS3231 RTC (STM32 & ESP32)

### 💻 Kode ESP32

**File: `ESP32_04_DS3231_RTC/src/main.cpp`**

```cpp
#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

void setup() {
    Serial.begin(115200);
    Wire.begin(21, 22);
    
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC!");
        while (1);
    }
    
    if (rtc.lostPower()) {
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
}

void loop() {
    DateTime now = rtc.now();
    
    Serial.printf("%04d/%02d/%02d %02d:%02d:%02d\n",
                  now.year(), now.month(), now.day(),
                  now.hour(), now.minute(), now.second());
    Serial.printf("Temperature: %.2f C\n\n", rtc.getTemperature());
    
    delay(1000);
}
```

---

## PROGRAM 9-10: 24LC256 EEPROM (STM32 & ESP32)

### 💻 Kode ESP32

**File: `ESP32_05_EEPROM_24LC256/src/main.cpp`**

```cpp
#include <Arduino.h>
#include <Wire.h>

#define EEPROM_ADDR 0x50

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

void setup() {
    Serial.begin(115200);
    Wire.begin(21, 22);
    
    // Write test
    EEPROM_WriteByte(0x0000, 0xAB);
    Serial.printf("Written: 0xAB\n");
    
    // Read back
    uint8_t data = EEPROM_ReadByte(0x0000);
    Serial.printf("Read: 0x%02X\n", data);
}

void loop() {
    delay(1000);
}
```

---

## PROGRAM 11-12: Multi-Device & Bus Recovery

### 💻 Kode ESP32

**File: `ESP32_11_Multi_Device/src/main.cpp`**

```cpp
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>

Adafruit_BME280 bme;
Adafruit_SSD1306 display(128, 64, &Wire, -1);
RTC_DS3231 rtc;

void i2cBusRecovery() {
    Wire.end();
    pinMode(21, INPUT_PULLUP);
    pinMode(22, OUTPUT);
    for (int i = 0; i < 9; i++) {
        digitalWrite(22, LOW);
        delayMicroseconds(5);
        digitalWrite(22, HIGH);
        delayMicroseconds(5);
    }
    Wire.begin(21, 22);
}

void setup() {
    Serial.begin(115200);
    Wire.begin(21, 22);
    
    bme.begin(0x76);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    rtc.begin();
}

void loop() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    
    DateTime now = rtc.now();
    display.setCursor(0, 0);
    display.printf("%02d:%02d:%02d", now.hour(), now.minute(), now.second());
    
    display.setCursor(0, 16);
    display.printf("T:%.1fC H:%.0f%%", bme.readTemperature(), bme.readHumidity());
    
    display.setCursor(0, 32);
    display.printf("P:%.0f hPa", bme.readPressure()/100.0);
    
    display.display();
    delay(1000);
}
```

---

## 📝 Tugas dan Latihan

### Tugas 1: I2C Device Integration
Buat sistem monitoring lingkungan yang membaca BME280, menampilkan pada OLED, dan menyimpan timestamp ke EEPROM menggunakan DS3231.

### Tugas 2: Multi-Sensor Dashboard
Implementasikan dashboard dengan grafik suhu 1 menit terakhir pada OLED dengan alarm threshold.

### Tugas 3: Bus Recovery Implementation
Implementasikan mekanisme recovery otomatis dengan deteksi timeout dan re-initialization.

---

## ✅ Checklist Evaluasi

| No | Kriteria | Bobot |
|----|----------|-------|
| 1 | I2C Scanner berfungsi | 10% |
| 2 | BME280 data akurat | 15% |
| 3 | OLED display grafik | 15% |
| 4 | RTC waktu akurat | 15% |
| 5 | EEPROM read/write | 15% |
| 6 | Multi-device integration | 15% |
| 7 | Bus recovery | 10% |
| 8 | Dokumentasi | 5% |

---

## 📚 Referensi

1. NXP I2C-bus Specification (UM10204)
2. BME280 Datasheet - Bosch Sensortec
3. SSD1306 Datasheet - Solomon Systech
4. DS3231 Datasheet - Maxim Integrated
5. 24LC256 Datasheet - Microchip

