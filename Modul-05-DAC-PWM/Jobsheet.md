# JOBSHEET BAB 05: DAC dan PWM Output

## 📋 Informasi Praktikum

| Item | Keterangan |
|------|------------|
| **Topik** | DAC (Digital-to-Analog Converter) dan PWM (Pulse Width Modulation) |
| **Platform** | STM32F103C8T6 (Blue Pill), ESP32 DevKit V1 |
| **Jumlah Program STM32** | 5 Program |
| **Jumlah Program ESP32** | 7 Program |
| **Durasi** | 3 x 50 menit |
| **Tools** | PlatformIO, STM32CubeIDE, Serial Monitor, Oscilloscope (optional) |

---

## 🎯 Tujuan Praktikum

Setelah menyelesaikan praktikum ini, mahasiswa mampu:

1. Memahami prinsip kerja DAC dan PWM pada mikrokontroler
2. Mengkonfigurasi dan memprogram DAC pada STM32 (12-bit) dan ESP32 (8-bit)
3. Mengimplementasikan PWM untuk berbagai aplikasi (LED dimming, motor, servo)
4. Membandingkan karakteristik DAC vs PWM sebagai output analog
5. Menggunakan DMA untuk waveform generation
6. Menerapkan hardware fade pada ESP32 LEDC
7. Melakukan debugging dan analisis sinyal output

---

## 🔧 Alat dan Komponen

### Hardware
| No | Komponen | Jumlah | Keterangan |
|----|----------|--------|------------|
| 1 | STM32F103C8T6 (Blue Pill) | 1 | Mikrokontroler utama |
| 2 | ESP32 DevKit V1 | 1 | Mikrokontroler utama |
| 3 | ST-Link V2 | 1 | Programmer STM32 |
| 4 | USB Cable Micro/Type-C | 2 | Koneksi dan programming |
| 5 | LED 5mm (Merah, Hijau, Biru) | 3 | Indikator output |
| 6 | Resistor 330Ω | 3 | Current limiting LED |
| 7 | Resistor 10kΩ | 2 | RC filter |
| 8 | Kapasitor 100nF | 2 | RC filter |
| 9 | Motor DC 3-6V | 1 | Aktuator PWM |
| 10 | Module L298N / L293D | 1 | Motor driver |
| 11 | Servo Motor SG90 | 1 | Servo control |
| 12 | Potentiometer 10kΩ | 1 | Input analog |
| 13 | Speaker/Buzzer 8Ω | 1 | Audio output (optional) |
| 14 | Breadboard | 1 | Prototyping |
| 15 | Kabel Jumper | 20+ | Koneksi |

### Software
- PlatformIO IDE / VS Code
- STM32CubeIDE (optional)
- Serial Monitor / PuTTY
- Oscilloscope software (optional)

---

## 📐 Skema Rangkaian

### Rangkaian DAC Output (STM32)
```
STM32F103C8T6
      │
      ├── PA4 (DAC_OUT1) ──┬── Oscilloscope
      │                    │
      │                    ├── 10kΩ ──┬── Analog Out (filtered)
      │                    │          │
      │                    │         100nF
      │                    │          │
      │                    │         GND
      │
      ├── PA5 (DAC_OUT2) ────── LED + 330Ω ── GND
      │
      └── GND ─────────────────── GND
```

### Rangkaian DAC Output (ESP32)
```
ESP32 DevKit
      │
      ├── GPIO25 (DAC1) ──┬── Oscilloscope
      │                   │
      │                   ├── 10kΩ ──┬── Analog Out (filtered)
      │                   │          │
      │                   │         100nF
      │                   │          │
      │                   │         GND
      │
      ├── GPIO26 (DAC2) ────── LED + 330Ω ── GND
      │
      └── GND ──────────────────── GND
```

### Rangkaian PWM LED Dimming
```
STM32/ESP32
      │
      ├── PA6/GPIO25 (PWM) ────── LED + 330Ω ── GND
      │
      ├── PA7/GPIO26 (PWM) ────── LED + 330Ω ── GND
      │
      ├── PB0/GPIO27 (PWM) ────── LED + 330Ω ── GND
      │
      └── GND ─────────────────────────────── GND
```

### Rangkaian Motor Control
```
STM32/ESP32                    L298N Module
      │                              │
      ├── PA6/GPIO25 (PWM) ────────► ENA
      │                              │
      ├── PA0/GPIO26 (DIR_A) ──────► IN1
      │                              │
      ├── PA1/GPIO27 (DIR_B) ──────► IN2
      │                              │
      ├── 5V ────────────────────── 5V Logic
      │                              │
      ├── GND ────────────────────── GND
      │                              │
      │                              ├── OUT1 ──┬── Motor DC
      │                              └── OUT2 ──┘      │
      │                                               12V
      │                                              Power
```

### Rangkaian Servo Control
```
STM32/ESP32                    Servo SG90
      │                              │
      ├── PA6/GPIO25 (PWM) ────────► Signal (Orange)
      │                              │
      ├── 5V ─────────────────────── VCC (Red)
      │                              │
      └── GND ────────────────────── GND (Brown)
```

---

## 📝 Percobaan

### Bagian A: DAC Output

---

#### Program 1: DAC Basic Output (STM32)
**File:** `praktikum/STM32/STM32_01_DAC_Output/src/main.c`

**Tujuan:** Menghasilkan tegangan analog menggunakan DAC 12-bit

**Langkah:**
1. Buat project baru dengan PlatformIO untuk STM32F103C8
2. Ketik kode program berikut:

```c
/**
 * Program 1: DAC Basic Output - STM32
 * Menghasilkan tegangan analog ramp 0-3.3V
 * Pin: PA4 (DAC_OUT1)
 */

#include "stm32f1xx_hal.h"

DAC_HandleTypeDef hdac;

void SystemClock_Config(void);
void DAC_Init(void);
void Error_Handler(void);

int main(void) {
    HAL_Init();
    SystemClock_Config();
    DAC_Init();
    
    uint16_t dac_value = 0;
    
    while (1) {
        // Ramp up 0V to 3.3V
        for (dac_value = 0; dac_value < 4096; dac_value += 16) {
            HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, dac_value);
            HAL_Delay(10);
        }
        
        // Ramp down 3.3V to 0V
        for (dac_value = 4095; dac_value > 0; dac_value -= 16) {
            HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, dac_value);
            HAL_Delay(10);
        }
    }
}

void DAC_Init(void) {
    __HAL_RCC_DAC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    // Configure PA4 as analog
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // Configure DAC
    hdac.Instance = DAC;
    HAL_DAC_Init(&hdac);
    
    DAC_ChannelConfTypeDef sConfig = {0};
    sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
    sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
    HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1);
    
    // Start DAC
    HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
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

void Error_Handler(void) {
    while(1);
}
```

**Analisis:**
- DAC 12-bit menghasilkan 4096 level tegangan (0-4095)
- Tegangan output: Vout = (3.3V × DAC_Value) / 4095
- Output buffer mencegah loading effect

---

#### Program 2: DAC Sine Wave Generator (STM32)
**File:** `praktikum/STM32/STM32_02_DAC_Sine/src/main.c`

**Tujuan:** Menghasilkan gelombang sinus menggunakan DAC dengan lookup table

```c
/**
 * Program 2: DAC Sine Wave Generator - STM32
 * Menghasilkan gelombang sinus dengan lookup table
 * Pin: PA4 (DAC_OUT1)
 */

#include "stm32f1xx_hal.h"
#include <math.h>

#define SINE_SAMPLES 100
#define PI 3.14159265359

DAC_HandleTypeDef hdac;
TIM_HandleTypeDef htim6;
uint16_t sine_table[SINE_SAMPLES];

void SystemClock_Config(void);
void DAC_Init(void);
void TIM6_Init(void);
void Generate_SineTable(void);

int main(void) {
    HAL_Init();
    SystemClock_Config();
    
    Generate_SineTable();
    DAC_Init();
    TIM6_Init();
    
    // Start DAC with DMA
    HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)sine_table, 
                      SINE_SAMPLES, DAC_ALIGN_12B_R);
    
    // Start Timer
    HAL_TIM_Base_Start(&htim6);
    
    while (1) {
        // Sine wave generated automatically by DMA
        HAL_Delay(1000);
    }
}

void Generate_SineTable(void) {
    for (int i = 0; i < SINE_SAMPLES; i++) {
        // Generate sine: amplitude 2048, offset 2048 (center at 1.65V)
        float angle = (2.0 * PI * i) / SINE_SAMPLES;
        sine_table[i] = (uint16_t)(2048 + 2000 * sin(angle));
    }
}

void DAC_Init(void) {
    __HAL_RCC_DAC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();
    
    // Configure PA4
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // Configure DAC
    hdac.Instance = DAC;
    HAL_DAC_Init(&hdac);
    
    DAC_ChannelConfTypeDef sConfig = {0};
    sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;
    sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
    HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1);
}

void TIM6_Init(void) {
    __HAL_RCC_TIM6_CLK_ENABLE();
    
    // Timer untuk sample rate 10kHz (100 samples × 100Hz sine)
    htim6.Instance = TIM6;
    htim6.Init.Prescaler = 72 - 1;        // 72MHz / 72 = 1MHz
    htim6.Init.Period = 100 - 1;          // 1MHz / 100 = 10kHz
    htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
    HAL_TIM_Base_Init(&htim6);
    
    // Configure TRGO
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig);
}
```

---

#### Program 3: DAC Basic Output (ESP32)
**File:** `praktikum/ESP32/ESP32_01_DAC_Output/src/main.cpp`

**Tujuan:** Menghasilkan tegangan analog menggunakan DAC 8-bit ESP32

```cpp
/**
 * Program 3: DAC Basic Output - ESP32
 * Menghasilkan tegangan analog ramp 0-3.3V
 * Pin: GPIO25 (DAC1)
 */

#include <Arduino.h>

#define DAC_PIN 25  // DAC1 = GPIO25, DAC2 = GPIO26

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=================================");
    Serial.println("Program: DAC Basic Output - ESP32");
    Serial.println("=================================");
    Serial.println("DAC on GPIO25 (0-255 = 0-3.3V)");
    Serial.println();
}

void loop() {
    Serial.println("Ramp UP: 0V -> 3.3V");
    
    // Ramp up
    for (int i = 0; i < 256; i++) {
        dacWrite(DAC_PIN, i);
        
        if (i % 32 == 0) {
            float voltage = (i / 255.0) * 3.3;
            Serial.printf("DAC Value: %3d, Voltage: %.2f V\n", i, voltage);
        }
        delay(10);
    }
    
    Serial.println("\nRamp DOWN: 3.3V -> 0V");
    
    // Ramp down
    for (int i = 255; i >= 0; i--) {
        dacWrite(DAC_PIN, i);
        
        if (i % 32 == 0) {
            float voltage = (i / 255.0) * 3.3;
            Serial.printf("DAC Value: %3d, Voltage: %.2f V\n", i, voltage);
        }
        delay(10);
    }
    
    Serial.println("\n--- Cycle Complete ---\n");
    delay(1000);
}
```

---

#### Program 4: DAC Sine Wave dengan Cosine Generator (ESP32)
**File:** `praktikum/ESP32/ESP32_02_DAC_Sine_Wave/src/main.cpp`

**Tujuan:** Menggunakan hardware cosine wave generator ESP32

```cpp
/**
 * Program 4: DAC Sine Wave dengan Cosine Generator - ESP32
 * Menggunakan hardware cosine wave generator
 * Pin: GPIO25 (DAC1)
 */

#include <Arduino.h>
#include <driver/dac.h>

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=====================================");
    Serial.println("Program: DAC Cosine Wave Generator");
    Serial.println("=====================================");
    
    // Configure cosine wave generator
    dac_cw_config_t cw_config = {
        .en_ch = DAC_CHANNEL_1,          // Use DAC1 (GPIO25)
        .scale = DAC_CW_SCALE_1,         // Full amplitude
        .phase = DAC_CW_PHASE_0,         // 0 degree phase
        .freq = 1000,                     // 1 kHz frequency
        .offset = 0                       // No DC offset
    };
    
    // Apply configuration
    ESP_ERROR_CHECK(dac_cw_generator_config(&cw_config));
    
    // Enable cosine wave generator
    ESP_ERROR_CHECK(dac_cw_generator_enable());
    
    // Enable DAC output
    ESP_ERROR_CHECK(dac_output_enable(DAC_CHANNEL_1));
    
    Serial.println("Cosine wave generator started!");
    Serial.println("Frequency: 1000 Hz");
    Serial.println("Output: GPIO25");
    Serial.println("Use oscilloscope to view waveform");
}

void loop() {
    // Demonstrate frequency change
    static uint32_t frequencies[] = {100, 500, 1000, 2000, 5000};
    static int freq_index = 0;
    
    delay(3000);
    
    freq_index = (freq_index + 1) % 5;
    
    dac_cw_config_t cw_config = {
        .en_ch = DAC_CHANNEL_1,
        .scale = DAC_CW_SCALE_1,
        .phase = DAC_CW_PHASE_0,
        .freq = frequencies[freq_index],
        .offset = 0
    };
    
    dac_cw_generator_config(&cw_config);
    
    Serial.printf("Frequency changed to: %d Hz\n", frequencies[freq_index]);
}
```

---

### Bagian B: PWM Output

---

#### Program 5: PWM LED Dimming (STM32)
**File:** `praktikum/STM32/STM32_03_PWM_LED/src/main.c`

**Tujuan:** Mengontrol kecerahan LED menggunakan PWM

```c
/**
 * Program 5: PWM LED Dimming - STM32
 * Mengontrol kecerahan LED dengan PWM
 * Pin: PA6 (TIM3_CH1)
 */

#include "stm32f1xx_hal.h"

TIM_HandleTypeDef htim3;

void SystemClock_Config(void);
void PWM_Init(void);

int main(void) {
    HAL_Init();
    SystemClock_Config();
    PWM_Init();
    
    uint16_t duty = 0;
    int8_t direction = 1;
    
    while (1) {
        // Update duty cycle
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, duty);
        
        // Fade effect
        duty += direction * 10;
        
        if (duty >= 1000) {
            direction = -1;
            duty = 1000;
        } else if (duty <= 0) {
            direction = 1;
            duty = 0;
        }
        
        HAL_Delay(20);
    }
}

void PWM_Init(void) {
    __HAL_RCC_TIM3_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    // Configure PA6 as alternate function
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // Timer configuration for 1kHz PWM
    // PWM Freq = 72MHz / (72 * 1000) = 1kHz
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 72 - 1;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 1000 - 1;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_PWM_Init(&htim3);
    
    // PWM Channel configuration
    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1);
    
    // Start PWM
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}
```

---

#### Program 6: PWM Motor Control (STM32)
**File:** `praktikum/STM32/STM32_04_PWM_Motor/src/main.c`

**Tujuan:** Mengontrol kecepatan dan arah motor DC

```c
/**
 * Program 6: PWM Motor Control - STM32
 * Mengontrol kecepatan dan arah motor DC dengan H-Bridge
 * Pin: PA6 (PWM), PA0 (DIR_A), PA1 (DIR_B)
 */

#include "stm32f1xx_hal.h"

TIM_HandleTypeDef htim3;

#define DIR_A_PIN GPIO_PIN_0
#define DIR_B_PIN GPIO_PIN_1
#define DIR_PORT  GPIOA

void SystemClock_Config(void);
void PWM_Init(void);
void GPIO_Init(void);
void Motor_SetSpeed(int16_t speed);

int main(void) {
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    PWM_Init();
    
    while (1) {
        // Forward acceleration
        for (int speed = 0; speed <= 100; speed += 5) {
            Motor_SetSpeed(speed);
            HAL_Delay(100);
        }
        
        HAL_Delay(2000);  // Run at full speed
        
        // Deceleration
        for (int speed = 100; speed >= 0; speed -= 5) {
            Motor_SetSpeed(speed);
            HAL_Delay(100);
        }
        
        HAL_Delay(1000);
        
        // Reverse acceleration
        for (int speed = 0; speed >= -100; speed -= 5) {
            Motor_SetSpeed(speed);
            HAL_Delay(100);
        }
        
        HAL_Delay(2000);
        
        // Deceleration
        for (int speed = -100; speed <= 0; speed += 5) {
            Motor_SetSpeed(speed);
            HAL_Delay(100);
        }
        
        HAL_Delay(1000);
    }
}

void Motor_SetSpeed(int16_t speed) {
    // speed: -100 to +100 (percentage)
    
    if (speed >= 0) {
        // Forward
        HAL_GPIO_WritePin(DIR_PORT, DIR_A_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(DIR_PORT, DIR_B_PIN, GPIO_PIN_RESET);
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, speed * 10);
    } else {
        // Reverse
        HAL_GPIO_WritePin(DIR_PORT, DIR_A_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(DIR_PORT, DIR_B_PIN, GPIO_PIN_SET);
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, (-speed) * 10);
    }
}

void GPIO_Init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DIR_A_PIN | DIR_B_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DIR_PORT, &GPIO_InitStruct);
}

void PWM_Init(void) {
    __HAL_RCC_TIM3_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // 20kHz PWM for motor (above audible range)
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 36 - 1;       // 72MHz / 36 = 2MHz
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 1000 - 1;        // 2MHz / 1000 = 20kHz
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_PWM_Init(&htim3);
    
    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1);
    
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}
```

---

#### Program 7: Servo Control (STM32)
**File:** `praktikum/STM32/STM32_05_Servo/src/main.c`

**Tujuan:** Mengontrol posisi servo motor

```c
/**
 * Program 7: Servo Control - STM32
 * Mengontrol posisi servo motor SG90
 * Pin: PA6 (TIM3_CH1)
 * Servo: 50Hz, 0.5ms-2.5ms pulse
 */

#include "stm32f1xx_hal.h"

TIM_HandleTypeDef htim3;

void SystemClock_Config(void);
void Servo_Init(void);
void Servo_SetAngle(uint8_t angle);

int main(void) {
    HAL_Init();
    SystemClock_Config();
    Servo_Init();
    
    while (1) {
        // Sweep 0 to 180 degrees
        for (int angle = 0; angle <= 180; angle += 5) {
            Servo_SetAngle(angle);
            HAL_Delay(50);
        }
        
        HAL_Delay(1000);
        
        // Sweep 180 to 0 degrees
        for (int angle = 180; angle >= 0; angle -= 5) {
            Servo_SetAngle(angle);
            HAL_Delay(50);
        }
        
        HAL_Delay(1000);
        
        // Test specific positions
        Servo_SetAngle(0);    HAL_Delay(1000);
        Servo_SetAngle(45);   HAL_Delay(1000);
        Servo_SetAngle(90);   HAL_Delay(1000);
        Servo_SetAngle(135);  HAL_Delay(1000);
        Servo_SetAngle(180);  HAL_Delay(1000);
    }
}

void Servo_SetAngle(uint8_t angle) {
    // Servo pulse: 0.5ms (0°) to 2.5ms (180°)
    // Period: 20ms (50Hz)
    // Timer period: 20000 (1µs resolution)
    // Pulse range: 500 to 2500
    
    if (angle > 180) angle = 180;
    
    uint16_t pulse = 500 + ((uint32_t)angle * 2000) / 180;
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulse);
}

void Servo_Init(void) {
    __HAL_RCC_TIM3_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // 50Hz PWM for servo
    // 72MHz / 72 = 1MHz, 1MHz / 20000 = 50Hz
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 72 - 1;       // 1µs resolution
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 20000 - 1;       // 20ms period
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_PWM_Init(&htim3);
    
    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 1500;              // 90° (center)
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1);
    
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}
```

---

#### Program 8: PWM LED Dimming (ESP32)
**File:** `praktikum/ESP32/ESP32_03_PWM_LED_Control/src/main.cpp`

**Tujuan:** LED dimming dengan LEDC peripheral ESP32

```cpp
/**
 * Program 8: PWM LED Dimming - ESP32
 * Menggunakan LEDC peripheral untuk LED dimming
 * Pin: GPIO25
 */

#include <Arduino.h>

#define LED_PIN       25
#define PWM_CHANNEL   0
#define PWM_FREQ      5000
#define PWM_RESOLUTION 8    // 8-bit (0-255)

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("================================");
    Serial.println("Program: PWM LED Dimming - ESP32");
    Serial.println("================================");
    
    // Configure LEDC PWM
    ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(LED_PIN, PWM_CHANNEL);
    
    Serial.printf("PWM Frequency: %d Hz\n", PWM_FREQ);
    Serial.printf("PWM Resolution: %d bit (0-%d)\n", PWM_RESOLUTION, (1 << PWM_RESOLUTION) - 1);
    Serial.println();
}

void loop() {
    Serial.println("Fade IN...");
    
    // Fade in
    for (int duty = 0; duty <= 255; duty++) {
        ledcWrite(PWM_CHANNEL, duty);
        
        if (duty % 32 == 0) {
            int percentage = (duty * 100) / 255;
            Serial.printf("Duty: %3d/255 (%3d%%)\n", duty, percentage);
        }
        delay(10);
    }
    
    delay(500);
    Serial.println("\nFade OUT...");
    
    // Fade out
    for (int duty = 255; duty >= 0; duty--) {
        ledcWrite(PWM_CHANNEL, duty);
        
        if (duty % 32 == 0) {
            int percentage = (duty * 100) / 255;
            Serial.printf("Duty: %3d/255 (%3d%%)\n", duty, percentage);
        }
        delay(10);
    }
    
    delay(500);
    Serial.println("\n--- Cycle Complete ---\n");
}
```

---

#### Program 9: Hardware Fade (ESP32)
**File:** `praktikum/ESP32/ESP32_04_PWM_Motor_Control/src/main.cpp`

**Tujuan:** Menggunakan hardware fade LEDC untuk efek smooth

```cpp
/**
 * Program 9: PWM Motor Control dengan Hardware Fade - ESP32
 * Menggunakan LEDC hardware fade untuk motor control smooth
 * Pin: GPIO25 (PWM), GPIO26 (DIR_A), GPIO27 (DIR_B)
 */

#include <Arduino.h>
#include <driver/ledc.h>

#define PWM_PIN     25
#define DIR_A_PIN   26
#define DIR_B_PIN   27

#define PWM_CHANNEL LEDC_CHANNEL_0
#define PWM_TIMER   LEDC_TIMER_0
#define PWM_MODE    LEDC_HIGH_SPEED_MODE
#define PWM_FREQ    20000   // 20kHz
#define PWM_RESOLUTION LEDC_TIMER_10_BIT

void Motor_Init(void);
void Motor_SetSpeed(int speed, int fade_time_ms);
void Motor_Stop(void);

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("====================================");
    Serial.println("Program: PWM Motor Control - ESP32");
    Serial.println("====================================");
    
    Motor_Init();
    
    Serial.println("Motor initialized!");
    Serial.println();
}

void loop() {
    Serial.println("Forward: Accelerate 0 -> 100%");
    digitalWrite(DIR_A_PIN, HIGH);
    digitalWrite(DIR_B_PIN, LOW);
    Motor_SetSpeed(100, 2000);  // Fade to 100% in 2 seconds
    delay(3000);
    
    Serial.println("Forward: Decelerate 100 -> 0%");
    Motor_SetSpeed(0, 2000);    // Fade to 0% in 2 seconds
    delay(1000);
    
    Serial.println("Reverse: Accelerate 0 -> 100%");
    digitalWrite(DIR_A_PIN, LOW);
    digitalWrite(DIR_B_PIN, HIGH);
    Motor_SetSpeed(100, 2000);
    delay(3000);
    
    Serial.println("Reverse: Decelerate 100 -> 0%");
    Motor_SetSpeed(0, 2000);
    delay(1000);
    
    Serial.println("\n--- Cycle Complete ---\n");
}

void Motor_Init(void) {
    // Configure direction pins
    pinMode(DIR_A_PIN, OUTPUT);
    pinMode(DIR_B_PIN, OUTPUT);
    digitalWrite(DIR_A_PIN, LOW);
    digitalWrite(DIR_B_PIN, LOW);
    
    // Configure LEDC timer
    ledc_timer_config_t timer_config = {
        .speed_mode = PWM_MODE,
        .duty_resolution = PWM_RESOLUTION,
        .timer_num = PWM_TIMER,
        .freq_hz = PWM_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_config);
    
    // Configure LEDC channel
    ledc_channel_config_t channel_config = {
        .gpio_num = PWM_PIN,
        .speed_mode = PWM_MODE,
        .channel = PWM_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = PWM_TIMER,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&channel_config);
    
    // Install fade function
    ledc_fade_func_install(0);
}

void Motor_SetSpeed(int speed, int fade_time_ms) {
    // speed: 0-100 (percentage)
    if (speed < 0) speed = 0;
    if (speed > 100) speed = 100;
    
    uint32_t duty = (speed * 1023) / 100;  // 10-bit resolution
    
    ledc_set_fade_time_and_start(
        PWM_MODE,
        PWM_CHANNEL,
        duty,
        fade_time_ms,
        LEDC_FADE_WAIT_DONE
    );
    
    Serial.printf("Speed set to %d%% (duty: %d)\n", speed, duty);
}

void Motor_Stop(void) {
    digitalWrite(DIR_A_PIN, LOW);
    digitalWrite(DIR_B_PIN, LOW);
    ledc_set_duty(PWM_MODE, PWM_CHANNEL, 0);
    ledc_update_duty(PWM_MODE, PWM_CHANNEL);
}
```

---

#### Program 10: Servo Control (ESP32)
**File:** `praktikum/ESP32/ESP32_05_Servo_Control/src/main.cpp`

**Tujuan:** Mengontrol servo dengan library ESP32Servo

```cpp
/**
 * Program 10: Servo Control - ESP32
 * Mengontrol posisi servo motor SG90
 * Pin: GPIO25
 */

#include <Arduino.h>
#include <ESP32Servo.h>

#define SERVO_PIN 25

Servo myServo;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("============================");
    Serial.println("Program: Servo Control - ESP32");
    Serial.println("============================");
    
    // Allow allocation of all timers
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    
    // Attach servo (standard 500-2500µs range)
    myServo.setPeriodHertz(50);           // Standard 50Hz servo
    myServo.attach(SERVO_PIN, 500, 2500); // Min/max pulse width
    
    Serial.println("Servo attached to GPIO25");
    Serial.println();
}

void loop() {
    Serial.println("Sweep: 0° -> 180°");
    
    // Sweep from 0 to 180
    for (int angle = 0; angle <= 180; angle += 5) {
        myServo.write(angle);
        Serial.printf("Angle: %3d°\n", angle);
        delay(50);
    }
    
    delay(1000);
    
    Serial.println("\nSweep: 180° -> 0°");
    
    // Sweep from 180 to 0
    for (int angle = 180; angle >= 0; angle -= 5) {
        myServo.write(angle);
        Serial.printf("Angle: %3d°\n", angle);
        delay(50);
    }
    
    delay(1000);
    
    // Test specific positions
    Serial.println("\nTest specific positions:");
    
    int positions[] = {0, 45, 90, 135, 180};
    for (int i = 0; i < 5; i++) {
        Serial.printf("Moving to %d°\n", positions[i]);
        myServo.write(positions[i]);
        delay(1000);
    }
    
    Serial.println("\n--- Cycle Complete ---\n");
    delay(2000);
}
```

---

#### Program 11: PWM Pseudo-DAC dengan Filter (ESP32)
**File:** `praktikum/ESP32/ESP32_06_PWM_Pseudo_DAC/src/main.cpp`

**Tujuan:** Menggunakan PWM + RC filter sebagai pseudo-DAC

```cpp
/**
 * Program 11: PWM Pseudo-DAC - ESP32
 * Menggunakan PWM high-frequency + RC filter untuk pseudo-DAC
 * Pin: GPIO25 (PWM Output -> RC Filter -> Analog Out)
 * Filter: R=10kΩ, C=100nF (fc ≈ 159Hz)
 */

#include <Arduino.h>

#define PWM_PIN       25
#define PWM_CHANNEL   0
#define PWM_FREQ      100000   // 100kHz for smooth filtering
#define PWM_RESOLUTION 10      // 10-bit (0-1023)

#define ADC_PIN       34       // To measure filtered output

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("===================================");
    Serial.println("Program: PWM Pseudo-DAC - ESP32");
    Serial.println("===================================");
    Serial.println("Connect RC filter: GPIO25 -> 10k -> [ADC34] -> 100nF -> GND");
    Serial.println();
    
    // Configure high-frequency PWM
    ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(PWM_PIN, PWM_CHANNEL);
    
    // Configure ADC for measuring output
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
    
    Serial.printf("PWM Frequency: %d Hz\n", PWM_FREQ);
    Serial.printf("PWM Resolution: %d bit\n", PWM_RESOLUTION);
    Serial.println();
}

void loop() {
    Serial.println("Generating voltage steps...\n");
    Serial.println("Target V | PWM Duty | Measured V");
    Serial.println("---------|----------|----------");
    
    // Generate voltage steps
    for (float target = 0.0; target <= 3.3; target += 0.33) {
        // Calculate duty cycle for target voltage
        int duty = (int)((target / 3.3) * 1023);
        
        // Set PWM
        ledcWrite(PWM_CHANNEL, duty);
        
        // Wait for RC filter to settle
        delay(100);
        
        // Read filtered voltage
        int adc_raw = analogRead(ADC_PIN);
        float measured = (adc_raw / 4095.0) * 3.3;
        
        Serial.printf(" %.2f V   |  %4d    |  %.2f V\n", target, duty, measured);
    }
    
    Serial.println("\nGenerating sine wave approximation...");
    
    // Generate sine wave using PWM
    for (int t = 0; t < 360; t += 5) {
        float angle = t * 3.14159 / 180.0;
        float value = (sin(angle) + 1.0) / 2.0;  // 0 to 1
        int duty = (int)(value * 1023);
        
        ledcWrite(PWM_CHANNEL, duty);
        delay(10);
    }
    
    Serial.println("\n--- Cycle Complete ---\n");
    delay(2000);
}
```

---

#### Program 12: RGB LED Color Mixing (ESP32)
**File:** `praktikum/ESP32/ESP32_07_RGB_LED_PWM/src/main.cpp`

**Tujuan:** Mengontrol LED RGB dengan 3 channel PWM

```cpp
/**
 * Program 12: RGB LED Color Mixing - ESP32
 * Mengontrol LED RGB dengan 3 channel PWM
 * Pin: GPIO25 (Red), GPIO26 (Green), GPIO27 (Blue)
 */

#include <Arduino.h>

#define RED_PIN     25
#define GREEN_PIN   26
#define BLUE_PIN    27

#define RED_CHANNEL   0
#define GREEN_CHANNEL 1
#define BLUE_CHANNEL  2

#define PWM_FREQ      5000
#define PWM_RESOLUTION 8

// Predefined colors (R, G, B)
struct Color {
    const char* name;
    uint8_t r, g, b;
};

Color colors[] = {
    {"Red",     255, 0,   0  },
    {"Green",   0,   255, 0  },
    {"Blue",    0,   0,   255},
    {"Yellow",  255, 255, 0  },
    {"Cyan",    0,   255, 255},
    {"Magenta", 255, 0,   255},
    {"White",   255, 255, 255},
    {"Orange",  255, 128, 0  },
    {"Purple",  128, 0,   255},
    {"Pink",    255, 192, 203}
};

void setRGB(uint8_t r, uint8_t g, uint8_t b);
void fadeToColor(uint8_t r, uint8_t g, uint8_t b, int duration_ms);

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("================================");
    Serial.println("Program: RGB LED PWM - ESP32");
    Serial.println("================================");
    
    // Configure PWM channels
    ledcSetup(RED_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
    ledcSetup(GREEN_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
    ledcSetup(BLUE_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
    
    // Attach pins
    ledcAttachPin(RED_PIN, RED_CHANNEL);
    ledcAttachPin(GREEN_PIN, GREEN_CHANNEL);
    ledcAttachPin(BLUE_PIN, BLUE_CHANNEL);
    
    Serial.println("RGB LED initialized!");
    Serial.println();
}

void loop() {
    // Display predefined colors
    Serial.println("Showing predefined colors...\n");
    
    int numColors = sizeof(colors) / sizeof(colors[0]);
    
    for (int i = 0; i < numColors; i++) {
        Serial.printf("Color: %-8s (R:%3d, G:%3d, B:%3d)\n",
                      colors[i].name,
                      colors[i].r, colors[i].g, colors[i].b);
        
        fadeToColor(colors[i].r, colors[i].g, colors[i].b, 500);
        delay(1500);
    }
    
    // Rainbow effect
    Serial.println("\nRainbow effect...");
    
    for (int hue = 0; hue < 360; hue += 2) {
        // HSV to RGB conversion (simplified)
        float h = hue / 60.0;
        int i = (int)h;
        float f = h - i;
        
        uint8_t r, g, b;
        
        switch (i % 6) {
            case 0: r = 255; g = 255 * f;     b = 0;           break;
            case 1: r = 255 * (1-f); g = 255; b = 0;           break;
            case 2: r = 0;   g = 255; b = 255 * f;             break;
            case 3: r = 0;   g = 255 * (1-f); b = 255;         break;
            case 4: r = 255 * f;     g = 0;   b = 255;         break;
            case 5: r = 255; g = 0;   b = 255 * (1-f);         break;
        }
        
        setRGB(r, g, b);
        delay(20);
    }
    
    Serial.println("\n--- Cycle Complete ---\n");
    delay(2000);
}

void setRGB(uint8_t r, uint8_t g, uint8_t b) {
    ledcWrite(RED_CHANNEL, r);
    ledcWrite(GREEN_CHANNEL, g);
    ledcWrite(BLUE_CHANNEL, b);
}

void fadeToColor(uint8_t r, uint8_t g, uint8_t b, int duration_ms) {
    static uint8_t current_r = 0, current_g = 0, current_b = 0;
    
    int steps = duration_ms / 10;
    
    for (int i = 0; i <= steps; i++) {
        float t = (float)i / steps;
        
        uint8_t new_r = current_r + (r - current_r) * t;
        uint8_t new_g = current_g + (g - current_g) * t;
        uint8_t new_b = current_b + (b - current_b) * t;
        
        setRGB(new_r, new_g, new_b);
        delay(10);
    }
    
    current_r = r;
    current_g = g;
    current_b = b;
}
```

---

## 📊 Tabel Perbandingan DAC vs PWM

| Karakteristik | DAC | PWM + Filter |
|--------------|-----|--------------|
| **Resolusi STM32** | 12-bit (4096 level) | Timer dependent |
| **Resolusi ESP32** | 8-bit (256 level) | 1-20 bit |
| **Output Ripple** | Sangat rendah | Tergantung filter |
| **Settling Time** | ~3µs | Tergantung RC |
| **Pin Requirement** | Dedicated (PA4/PA5, GPIO25/26) | Any GPIO |
| **CPU Load** | DMA available | Timer based |
| **Best For** | Audio, precision | LED, motor, power |

---

## 📝 Tugas Praktikum

### Tugas 1: Analisis DAC
1. Ukur tegangan output DAC pada setiap level (0, 64, 128, 192, 255 untuk ESP32)
2. Hitung error antara nilai teoritis dan terukur
3. Plot grafik linearity DAC
4. **Deliverable:** Tabel pengukuran dan analisis error

### Tugas 2: Karakterisasi PWM
1. Ukur frekuensi dan duty cycle PWM dengan oscilloscope/logic analyzer
2. Bandingkan PWM 1kHz, 5kHz, dan 20kHz untuk LED dimming
3. Amati dan dokumentasikan flicker pada setiap frekuensi
4. **Deliverable:** Screenshot waveform dan analisis

### Tugas 3: Implementasi Servo
1. Modifikasi program servo untuk mengikuti input potentiometer
2. Implementasikan smooth movement dengan interpolasi
3. Buat fungsi untuk mencatat posisi dan replay movement
4. **Deliverable:** Video demonstrasi dan kode program

### Tugas 4: Audio Generation (Tantangan)
1. Gunakan DAC untuk menghasilkan tone audio sederhana
2. Implementasikan fungsi untuk memainkan nada C, D, E, F, G, A, B
3. Buat melody sederhana
4. **Deliverable:** Video demonstrasi audio

---

## ❓ Pertanyaan Analisis

1. Mengapa ESP32 DAC hanya 8-bit sedangkan STM32 12-bit? Apa implikasinya?

2. Jelaskan mengapa frekuensi PWM 20kHz lebih baik untuk motor DC dibanding 1kHz!

3. Hitung nilai R dan C untuk filter low-pass dengan cutoff 100Hz. Mengapa cutoff ini cocok untuk audio?

4. Apa yang terjadi jika servo menerima sinyal PWM dengan periode bukan 20ms?

5. Bagaimana cara meningkatkan resolusi efektif DAC 8-bit ESP32?

---

## 🔍 Troubleshooting

| Problem | Kemungkinan Penyebab | Solusi |
|---------|---------------------|--------|
| DAC output 0V | Pin tidak dikonfigurasi analog | Cek konfigurasi GPIO |
| PWM tidak keluar | Timer tidak start | Panggil HAL_TIM_PWM_Start() |
| Servo jitter | Interrupt mengganggu | Gunakan hardware timer |
| Motor noise | PWM freq < 20kHz | Tingkatkan frekuensi |
| LED flicker | PWM freq terlalu rendah | Minimal 100Hz untuk mata |
| DAC stepping terlihat | Resolusi kurang | Gunakan dithering/interpolasi |

---

## 📊 Rubrik Penilaian Praktikum

| Komponen | Bobot | Kriteria |
|----------|-------|----------|
| Implementasi Program | 40% | Semua 12 program berjalan dengan benar |
| Laporan & Dokumentasi | 25% | Kelengkapan, analisis, screenshot |
| Pemahaman Konsep | 20% | Jawaban pertanyaan analisis |
| Tugas Tambahan | 15% | Kreativitas, modifikasi program |

---

## 📚 Referensi Tambahan

1. [STM32 DAC Application Note AN3126](https://www.st.com/resource/en/application_note/an3126.pdf)
2. [ESP32 LEDC PWM Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html)
3. [ESP32 DAC Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/dac.html)
4. "Mastering STM32" - Chapter 11: DAC
5. PWM Application in Motor Control - Texas Instruments

