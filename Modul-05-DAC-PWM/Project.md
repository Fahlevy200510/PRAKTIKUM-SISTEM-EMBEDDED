# Project Modul 05: DAC & PWM
## 🎵 Smart Audio-Visual Controller dengan Komunikasi Dual-Platform

### 📋 Informasi Project

| Item | Detail |
|------|--------|
| **Nama Project** | Smart Audio-Visual Controller |
| **Platform** | STM32F103C8T6 + ESP32 DevKit (Dual MCU) |
| **Tingkat Kesulitan** | ⭐⭐⭐⭐ (Advanced) |
| **Durasi Pengerjaan** | 2-3 minggu |
| **Kelompok** | 2-3 orang |

---

## 🎯 Deskripsi Project

Membangun sistem Smart Audio-Visual Controller yang menggunakan **dua mikrokontroler** (STM32 dan ESP32) yang berkomunikasi melalui UART. Sistem ini menggabungkan:

1. **STM32** sebagai **Audio Generator** menggunakan DAC 12-bit untuk menghasilkan berbagai waveform audio
2. **ESP32** sebagai **Visual Controller** menggunakan PWM untuk mengontrol LED RGB dan motor dengan remote control WiFi

Kedua MCU saling berkomunikasi untuk sinkronisasi audio-visual, dimana perubahan audio akan mempengaruhi efek visual secara real-time.

---

## 🎯 Tujuan Project

### Tujuan Utama
1. Mengimplementasikan DAC 12-bit STM32 untuk audio waveform generation
2. Mengimplementasikan PWM multi-channel ESP32 untuk visual effects
3. Membuat komunikasi antar MCU melalui UART
4. Mengintegrasikan kontrol WiFi pada ESP32

### Learning Outcomes
- Menguasai penggunaan DAC untuk aplikasi audio
- Menguasai PWM untuk kontrol LED dan motor
- Memahami komunikasi serial antar mikrokontroler
- Mengembangkan kemampuan sistem terintegrasi

---

## 📐 Arsitektur Sistem

```
┌─────────────────────────────────────────────────────────────────────┐
│                    SMART AUDIO-VISUAL CONTROLLER                     │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  ┌─────────────────────┐         ┌─────────────────────────────┐    │
│  │     STM32F103       │  UART   │         ESP32               │    │
│  │   (Audio Master)    │◄───────►│    (Visual Controller)      │    │
│  │                     │         │                             │    │
│  │  ┌───────────────┐  │         │  ┌─────────────────────┐    │    │
│  │  │  Waveform     │  │         │  │    WiFi Module      │    │    │
│  │  │  Generator    │  │         │  │   (Web Interface)   │    │    │
│  │  │  - Sine       │  │         │  └─────────────────────┘    │    │
│  │  │  - Square     │  │         │            │                │    │
│  │  │  - Triangle   │  │         │  ┌─────────▼─────────┐      │    │
│  │  │  - Sawtooth   │  │         │  │   PWM Controller  │      │    │
│  │  └───────┬───────┘  │         │  │   - LED RGB       │      │    │
│  │          │          │         │  │   - Motor Speed   │      │    │
│  │  ┌───────▼───────┐  │         │  │   - Servo Angle   │      │    │
│  │  │   DAC 12-bit  │  │         │  └───────────────────┘      │    │
│  │  │   (PA4/PA5)   │  │         │                             │    │
│  │  └───────┬───────┘  │         │  ┌─────────────────────┐    │    │
│  │          │          │         │  │   Visual Output     │    │    │
│  │  ┌───────▼───────┐  │         │  │   - RGB LED Strip   │    │    │
│  │  │  Amplifier    │  │         │  │   - DC Motor        │    │    │
│  │  │  + Speaker    │  │         │  │   - Servo           │    │    │
│  │  └───────────────┘  │         │  └─────────────────────┘    │    │
│  └─────────────────────┘         └─────────────────────────────┘    │
│                                                                      │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │                    Synchronization Protocol                  │    │
│  │  STM32 → ESP32: Audio Level, Frequency, Waveform Type       │    │
│  │  ESP32 → STM32: Control Commands, Mode Selection            │    │
│  └─────────────────────────────────────────────────────────────┘    │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 🔧 Spesifikasi Hardware

### Komponen Utama

| No | Komponen | Qty | Fungsi |
|----|----------|-----|--------|
| 1 | STM32F103C8T6 | 1 | Audio waveform generator |
| 2 | ESP32 DevKit V1 | 1 | Visual controller + WiFi |
| 3 | ST-Link V2 | 1 | STM32 programmer |
| 4 | Speaker 8Ω 0.5W | 1 | Audio output |
| 5 | PAM8403 Amplifier | 1 | Audio amplifier |
| 6 | LED RGB WS2812B (8 pixel) | 1 | Visual effect |
| 7 | LED RGB Common Cathode | 3 | Direct PWM control |
| 8 | DC Motor 3-6V | 1 | Speed visual |
| 9 | Servo SG90 | 1 | Position visual |
| 10 | L298N Module | 1 | Motor driver |
| 11 | Rotary Encoder | 2 | Input control |
| 12 | Push Button | 4 | Mode selection |
| 13 | OLED 0.96" I2C | 1 | Display status |
| 14 | Potentiometer 10kΩ | 2 | Volume/parameter |

### Skema Koneksi

#### STM32 Connections
```
STM32F103C8T6
├── PA4 (DAC_OUT1) ──────► Amplifier Input
├── PA5 (DAC_OUT2) ──────► Optional second channel
├── PA9 (UART1_TX) ──────► ESP32 RX2 (GPIO16)
├── PA10 (UART1_RX) ◄───── ESP32 TX2 (GPIO17)
├── PB6 ◄─────────────────  Rotary Encoder A
├── PB7 ◄─────────────────  Rotary Encoder B
├── PB8 ◄─────────────────  Rotary Encoder Button
├── PA0 ◄─────────────────  Potentiometer (Frequency)
├── 3.3V ─────────────────► VCC components
└── GND ──────────────────► GND components
```

#### ESP32 Connections
```
ESP32 DevKit V1
├── GPIO25 ───────────────► LED Red (PWM Ch0)
├── GPIO26 ───────────────► LED Green (PWM Ch1)
├── GPIO27 ───────────────► LED Blue (PWM Ch2)
├── GPIO32 ───────────────► Motor PWM (via L298N ENA)
├── GPIO33 ───────────────► Motor DIR_A (L298N IN1)
├── GPIO14 ───────────────► Motor DIR_B (L298N IN2)
├── GPIO13 ───────────────► Servo Signal
├── GPIO16 (RX2) ◄─────── STM32 TX
├── GPIO17 (TX2) ────────► STM32 RX
├── GPIO21 (SDA) ─────────► OLED SDA
├── GPIO22 (SCL) ─────────► OLED SCL
├── GPIO4 ◄───────────────  Button Mode
├── GPIO5 ◄───────────────  Rotary Encoder A
├── GPIO18 ◄──────────────  Rotary Encoder B
├── 3.3V ─────────────────► VCC components
└── GND ──────────────────► GND components
```

---

## 📝 Spesifikasi Fungsional

### Mode Operasi

#### Mode 1: Manual Audio-Visual
- Kontrol langsung menggunakan rotary encoder dan button
- STM32: Adjust frequency (20Hz - 2kHz) dan waveform
- ESP32: LED mengikuti audio level dan frequency
- Motor kecepatan proporsional dengan volume
- Servo position mengikuti phase waveform

#### Mode 2: Preset Patterns
- 5 preset pattern audio-visual tersimpan
- LED pattern synchronized dengan audio
- Sequence pattern dengan timing
- Smooth transition antar preset

#### Mode 3: WiFi Remote Control
- Web interface untuk kontrol parameter
- Real-time update tanpa reload
- Responsive design (mobile-friendly)
- Status display di web

#### Mode 4: Music Reactive
- Audio input analysis (beat detection)
- Visual effect mengikuti beat
- Frequency spectrum pada LED strip
- Auto-sensitivity adjustment

### Protokol Komunikasi UART

```
Communication Protocol (115200 baud):

STM32 → ESP32 (Audio Data Packet):
┌────────┬──────┬──────────┬─────────┬──────────┬────────┐
│ START  │ TYPE │ LEVEL    │ FREQ    │ WAVEFORM │ CHKSUM │
│ 0xAA   │ 0x01 │ 0-255    │ 2 bytes │ 0-3      │ XOR    │
└────────┴──────┴──────────┴─────────┴──────────┴────────┘

ESP32 → STM32 (Command Packet):
┌────────┬──────┬──────────┬──────────┬────────┐
│ START  │ TYPE │ COMMAND  │ PARAM    │ CHKSUM │
│ 0x55   │ 0x02 │ 0-255    │ 2 bytes  │ XOR    │
└────────┴──────┴──────────┴──────────┴────────┘

Commands:
0x01 - Set Frequency
0x02 - Set Waveform
0x03 - Set Volume
0x04 - Play/Stop
0x05 - Load Preset
```

---

## 💻 Implementasi Software

### STM32 - Audio Generator (main.c)

```c
/**
 * STM32 Audio Generator
 * DAC-based waveform synthesis with DMA
 */

#include "stm32f1xx_hal.h"
#include <math.h>
#include <string.h>

// Waveform Configuration
#define SAMPLE_RATE     44100
#define SAMPLES_PER_CYCLE 256
#define PI              3.14159265359

// Waveform types
typedef enum {
    WAVE_SINE = 0,
    WAVE_SQUARE,
    WAVE_TRIANGLE,
    WAVE_SAWTOOTH
} WaveformType;

// Global variables
DAC_HandleTypeDef hdac;
TIM_HandleTypeDef htim6;
UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_dac1;

uint16_t waveform_buffer[SAMPLES_PER_CYCLE];
volatile uint16_t current_frequency = 440;  // Hz
volatile WaveformType current_waveform = WAVE_SINE;
volatile uint8_t volume = 128;
volatile uint8_t playing = 1;

// Function prototypes
void SystemClock_Config(void);
void DAC_Init(void);
void TIM6_Init(uint16_t frequency);
void UART_Init(void);
void GenerateWaveform(WaveformType type);
void SendAudioData(void);
void ProcessCommand(uint8_t* data);

int main(void) {
    HAL_Init();
    SystemClock_Config();
    
    DAC_Init();
    UART_Init();
    
    GenerateWaveform(WAVE_SINE);
    TIM6_Init(current_frequency);
    
    HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, 
                      (uint32_t*)waveform_buffer, 
                      SAMPLES_PER_CYCLE, DAC_ALIGN_12B_R);
    HAL_TIM_Base_Start(&htim6);
    
    uint8_t rx_buffer[8];
    uint32_t last_send = 0;
    
    while (1) {
        // Receive commands from ESP32
        if (HAL_UART_Receive(&huart1, rx_buffer, 8, 10) == HAL_OK) {
            if (rx_buffer[0] == 0x55 && rx_buffer[1] == 0x02) {
                ProcessCommand(rx_buffer);
            }
        }
        
        // Send audio data every 50ms
        if (HAL_GetTick() - last_send >= 50) {
            SendAudioData();
            last_send = HAL_GetTick();
        }
    }
}

void GenerateWaveform(WaveformType type) {
    for (int i = 0; i < SAMPLES_PER_CYCLE; i++) {
        float t = (float)i / SAMPLES_PER_CYCLE;
        float value = 0;
        
        switch (type) {
            case WAVE_SINE:
                value = sin(2 * PI * t);
                break;
                
            case WAVE_SQUARE:
                value = (t < 0.5) ? 1.0 : -1.0;
                break;
                
            case WAVE_TRIANGLE:
                value = (t < 0.5) ? (4 * t - 1) : (3 - 4 * t);
                break;
                
            case WAVE_SAWTOOTH:
                value = 2 * t - 1;
                break;
        }
        
        // Scale by volume and convert to 12-bit (0-4095)
        uint16_t dac_value = (uint16_t)((value + 1.0) * 
                             (volume / 255.0) * 2000 + 48);
        waveform_buffer[i] = dac_value;
    }
}

void TIM6_Init(uint16_t frequency) {
    __HAL_RCC_TIM6_CLK_ENABLE();
    
    // Calculate prescaler for desired frequency
    // Timer trigger rate = SAMPLES_PER_CYCLE * frequency
    uint32_t timer_freq = SAMPLES_PER_CYCLE * frequency;
    uint16_t prescaler = (72000000 / timer_freq / 100) - 1;
    
    htim6.Instance = TIM6;
    htim6.Init.Prescaler = prescaler;
    htim6.Init.Period = 100 - 1;
    htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
    HAL_TIM_Base_Init(&htim6);
    
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig);
}

void SendAudioData(void) {
    // Calculate current audio level (RMS approximation)
    uint32_t sum = 0;
    for (int i = 0; i < SAMPLES_PER_CYCLE; i++) {
        int16_t val = waveform_buffer[i] - 2048;
        sum += val * val;
    }
    uint8_t level = (uint8_t)(sqrt(sum / SAMPLES_PER_CYCLE) / 16);
    
    // Send packet to ESP32
    uint8_t packet[8];
    packet[0] = 0xAA;  // Start byte
    packet[1] = 0x01;  // Type: audio data
    packet[2] = level;
    packet[3] = (current_frequency >> 8) & 0xFF;
    packet[4] = current_frequency & 0xFF;
    packet[5] = current_waveform;
    packet[6] = 0;     // Reserved
    packet[7] = packet[1] ^ packet[2] ^ packet[3] ^ 
                packet[4] ^ packet[5];  // Checksum
    
    HAL_UART_Transmit(&huart1, packet, 8, 100);
}

void ProcessCommand(uint8_t* data) {
    uint8_t cmd = data[2];
    uint16_t param = (data[3] << 8) | data[4];
    
    switch (cmd) {
        case 0x01:  // Set Frequency
            current_frequency = param;
            TIM6_Init(current_frequency);
            break;
            
        case 0x02:  // Set Waveform
            current_waveform = (WaveformType)(param & 0x03);
            GenerateWaveform(current_waveform);
            break;
            
        case 0x03:  // Set Volume
            volume = param & 0xFF;
            GenerateWaveform(current_waveform);
            break;
            
        case 0x04:  // Play/Stop
            playing = param & 0x01;
            if (playing) {
                HAL_TIM_Base_Start(&htim6);
            } else {
                HAL_TIM_Base_Stop(&htim6);
            }
            break;
    }
}

// Additional init functions...
void DAC_Init(void) {
    // Full DAC initialization with DMA
    // (Same as jobsheet implementation)
}

void UART_Init(void) {
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_9;  // TX
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = GPIO_PIN_10;  // RX
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    HAL_UART_Init(&huart1);
}

void SystemClock_Config(void) {
    // 72MHz configuration
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);
    
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}
```

### ESP32 - Visual Controller (main.cpp)

```cpp
/**
 * ESP32 Visual Controller
 * PWM-based LED and motor control with WiFi
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

// Pin Definitions
#define LED_RED_PIN     25
#define LED_GREEN_PIN   26
#define LED_BLUE_PIN    27
#define MOTOR_PWM_PIN   32
#define MOTOR_DIR_A     33
#define MOTOR_DIR_B     14
#define SERVO_PIN       13
#define BTN_MODE_PIN    4

// PWM Channels
#define RED_CHANNEL     0
#define GREEN_CHANNEL   1
#define BLUE_CHANNEL    2
#define MOTOR_CHANNEL   3

// OLED
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Servo
Servo visualServo;

// WiFi Configuration
const char* ssid = "AudioVisual_Controller";
const char* password = "12345678";
WebServer server(80);

// Audio data from STM32
struct AudioData {
    uint8_t level;
    uint16_t frequency;
    uint8_t waveform;
    uint32_t lastUpdate;
} audioData;

// Visual settings
struct VisualSettings {
    uint8_t mode;           // 0=Manual, 1=Preset, 2=WiFi, 3=Reactive
    uint8_t brightness;
    uint8_t motorSpeed;
    uint8_t servoAngle;
    bool motorEnabled;
} settings;

// Function prototypes
void setupWiFi(void);
void setupWebServer(void);
void processSTM32Data(void);
void updateVisuals(void);
void updateDisplay(void);
void handleRoot(void);
void handleAPI(void);

// HTML Page
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Audio-Visual Controller</title>
    <style>
        body { font-family: Arial; text-align: center; margin: 20px; background: #1a1a2e; color: white; }
        .container { max-width: 600px; margin: auto; }
        .card { background: #16213e; border-radius: 10px; padding: 20px; margin: 10px 0; }
        .slider { width: 100%; height: 30px; }
        .btn { background: #e94560; border: none; color: white; padding: 15px 30px; 
               border-radius: 5px; font-size: 16px; cursor: pointer; margin: 5px; }
        .btn:hover { background: #ff6b6b; }
        .status { padding: 10px; background: #0f3460; border-radius: 5px; }
        h1 { color: #e94560; }
        h3 { color: #00d9ff; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🎵 Audio-Visual Controller</h1>
        
        <div class="card status">
            <h3>Status</h3>
            <p>Audio Level: <span id="level">--</span></p>
            <p>Frequency: <span id="freq">--</span> Hz</p>
            <p>Waveform: <span id="wave">--</span></p>
        </div>
        
        <div class="card">
            <h3>Audio Control</h3>
            <p>Frequency: <span id="freqVal">440</span> Hz</p>
            <input type="range" min="20" max="2000" value="440" 
                   class="slider" id="freqSlider" onchange="setFreq(this.value)">
            <br><br>
            <button class="btn" onclick="setWave(0)">Sine</button>
            <button class="btn" onclick="setWave(1)">Square</button>
            <button class="btn" onclick="setWave(2)">Triangle</button>
            <button class="btn" onclick="setWave(3)">Sawtooth</button>
        </div>
        
        <div class="card">
            <h3>Visual Control</h3>
            <p>Brightness: <span id="brightVal">100</span>%</p>
            <input type="range" min="0" max="100" value="100" 
                   class="slider" id="brightSlider" onchange="setBright(this.value)">
            <br><br>
            <p>Motor Speed: <span id="motorVal">0</span>%</p>
            <input type="range" min="0" max="100" value="0" 
                   class="slider" id="motorSlider" onchange="setMotor(this.value)">
        </div>
        
        <div class="card">
            <h3>Mode</h3>
            <button class="btn" onclick="setMode(0)">Manual</button>
            <button class="btn" onclick="setMode(1)">Preset</button>
            <button class="btn" onclick="setMode(2)">WiFi</button>
            <button class="btn" onclick="setMode(3)">Reactive</button>
        </div>
    </div>
    
    <script>
        function updateStatus() {
            fetch('/api?cmd=status')
                .then(r => r.json())
                .then(d => {
                    document.getElementById('level').textContent = d.level;
                    document.getElementById('freq').textContent = d.frequency;
                    const waves = ['Sine', 'Square', 'Triangle', 'Sawtooth'];
                    document.getElementById('wave').textContent = waves[d.waveform];
                });
        }
        function setFreq(v) { 
            document.getElementById('freqVal').textContent = v;
            fetch('/api?cmd=freq&val=' + v); 
        }
        function setWave(v) { fetch('/api?cmd=wave&val=' + v); }
        function setBright(v) { 
            document.getElementById('brightVal').textContent = v;
            fetch('/api?cmd=bright&val=' + v); 
        }
        function setMotor(v) { 
            document.getElementById('motorVal').textContent = v;
            fetch('/api?cmd=motor&val=' + v); 
        }
        function setMode(v) { fetch('/api?cmd=mode&val=' + v); }
        setInterval(updateStatus, 500);
    </script>
</body>
</html>
)rawliteral";

void setup() {
    Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, 16, 17);  // RX2, TX2
    
    // Initialize PWM
    ledcSetup(RED_CHANNEL, 5000, 8);
    ledcSetup(GREEN_CHANNEL, 5000, 8);
    ledcSetup(BLUE_CHANNEL, 5000, 8);
    ledcSetup(MOTOR_CHANNEL, 20000, 10);
    
    ledcAttachPin(LED_RED_PIN, RED_CHANNEL);
    ledcAttachPin(LED_GREEN_PIN, GREEN_CHANNEL);
    ledcAttachPin(LED_BLUE_PIN, BLUE_CHANNEL);
    ledcAttachPin(MOTOR_PWM_PIN, MOTOR_CHANNEL);
    
    // Motor direction pins
    pinMode(MOTOR_DIR_A, OUTPUT);
    pinMode(MOTOR_DIR_B, OUTPUT);
    pinMode(BTN_MODE_PIN, INPUT_PULLUP);
    
    // Servo
    visualServo.attach(SERVO_PIN, 500, 2500);
    visualServo.write(90);
    
    // OLED
    Wire.begin();
    if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 0);
        display.println("Audio-Visual");
        display.println("Controller");
        display.display();
    }
    
    // WiFi
    setupWiFi();
    setupWebServer();
    
    // Initialize settings
    settings.mode = 0;
    settings.brightness = 100;
    settings.motorSpeed = 0;
    settings.servoAngle = 90;
    settings.motorEnabled = false;
    
    Serial.println("ESP32 Visual Controller Ready!");
}

void loop() {
    server.handleClient();
    processSTM32Data();
    updateVisuals();
    updateDisplay();
    
    // Mode button check
    static uint32_t lastBtnCheck = 0;
    if (millis() - lastBtnCheck > 200) {
        if (digitalRead(BTN_MODE_PIN) == LOW) {
            settings.mode = (settings.mode + 1) % 4;
        }
        lastBtnCheck = millis();
    }
    
    delay(10);
}

void setupWiFi() {
    WiFi.softAP(ssid, password);
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
}

void setupWebServer() {
    server.on("/", handleRoot);
    server.on("/api", handleAPI);
    server.begin();
}

void handleRoot() {
    server.send(200, "text/html", htmlPage);
}

void handleAPI() {
    String cmd = server.arg("cmd");
    String val = server.arg("val");
    
    if (cmd == "status") {
        String json = "{\"level\":" + String(audioData.level) +
                     ",\"frequency\":" + String(audioData.frequency) +
                     ",\"waveform\":" + String(audioData.waveform) + "}";
        server.send(200, "application/json", json);
        return;
    }
    
    if (cmd == "freq") {
        sendCommandToSTM32(0x01, val.toInt());
    } else if (cmd == "wave") {
        sendCommandToSTM32(0x02, val.toInt());
    } else if (cmd == "bright") {
        settings.brightness = val.toInt();
    } else if (cmd == "motor") {
        settings.motorSpeed = val.toInt();
    } else if (cmd == "mode") {
        settings.mode = val.toInt();
    }
    
    server.send(200, "text/plain", "OK");
}

void processSTM32Data() {
    if (Serial2.available() >= 8) {
        uint8_t buffer[8];
        Serial2.readBytes(buffer, 8);
        
        if (buffer[0] == 0xAA && buffer[1] == 0x01) {
            // Verify checksum
            uint8_t checksum = buffer[1] ^ buffer[2] ^ buffer[3] ^ 
                              buffer[4] ^ buffer[5];
            if (checksum == buffer[7]) {
                audioData.level = buffer[2];
                audioData.frequency = (buffer[3] << 8) | buffer[4];
                audioData.waveform = buffer[5];
                audioData.lastUpdate = millis();
            }
        }
    }
}

void sendCommandToSTM32(uint8_t cmd, uint16_t param) {
    uint8_t packet[8];
    packet[0] = 0x55;
    packet[1] = 0x02;
    packet[2] = cmd;
    packet[3] = (param >> 8) & 0xFF;
    packet[4] = param & 0xFF;
    packet[5] = 0;
    packet[6] = 0;
    packet[7] = packet[1] ^ packet[2] ^ packet[3] ^ packet[4];
    
    Serial2.write(packet, 8);
}

void updateVisuals() {
    uint8_t r, g, b;
    
    switch (settings.mode) {
        case 0:  // Manual mode
            r = g = b = (settings.brightness * 255) / 100;
            break;
            
        case 1:  // Preset patterns
            // Rotating colors based on time
            {
                int hue = (millis() / 20) % 360;
                hsvToRgb(hue, 255, settings.brightness * 255 / 100, r, g, b);
            }
            break;
            
        case 2:  // WiFi controlled - use settings
            r = g = b = (settings.brightness * 255) / 100;
            break;
            
        case 3:  // Reactive mode
            // Color based on frequency
            {
                int hue = map(audioData.frequency, 20, 2000, 0, 360);
                int brightness = map(audioData.level, 0, 255, 0, 
                                    settings.brightness * 255 / 100);
                hsvToRgb(hue, 255, brightness, r, g, b);
            }
            break;
    }
    
    // Apply to LEDs
    ledcWrite(RED_CHANNEL, r);
    ledcWrite(GREEN_CHANNEL, g);
    ledcWrite(BLUE_CHANNEL, b);
    
    // Motor control
    if (settings.mode == 3) {
        // Reactive: motor speed follows audio level
        int speed = map(audioData.level, 0, 255, 0, 1023);
        ledcWrite(MOTOR_CHANNEL, speed);
    } else {
        ledcWrite(MOTOR_CHANNEL, (settings.motorSpeed * 1023) / 100);
    }
    
    digitalWrite(MOTOR_DIR_A, HIGH);
    digitalWrite(MOTOR_DIR_B, LOW);
    
    // Servo follows waveform phase in reactive mode
    if (settings.mode == 3) {
        int angle = map(audioData.level, 0, 255, 0, 180);
        visualServo.write(angle);
    } else {
        visualServo.write(settings.servoAngle);
    }
}

void hsvToRgb(int h, int s, int v, uint8_t& r, uint8_t& g, uint8_t& b) {
    int i = h / 60;
    int f = h % 60;
    int p = (v * (255 - s)) / 255;
    int q = (v * (255 - (s * f) / 60)) / 255;
    int t = (v * (255 - (s * (60 - f)) / 60)) / 255;
    
    switch (i % 6) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
    }
}

void updateDisplay() {
    static uint32_t lastUpdate = 0;
    if (millis() - lastUpdate < 100) return;
    lastUpdate = millis();
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.printf("Mode: %d\n", settings.mode);
    display.printf("Freq: %d Hz\n", audioData.frequency);
    display.printf("Level: %d\n", audioData.level);
    display.printf("Wave: %d\n", audioData.waveform);
    display.printf("Motor: %d%%\n", settings.motorSpeed);
    display.printf("IP: %s", WiFi.softAPIP().toString().c_str());
    display.display();
}
```

---

## 📊 Deliverables

### 1. Source Code
- [ ] STM32 project lengkap (main.c + header files)
- [ ] ESP32 project lengkap (main.cpp + libraries)
- [ ] Konfigurasi PlatformIO (platformio.ini)
- [ ] Dokumentasi kode (comments)

### 2. Hardware
- [ ] Skema rangkaian (Fritzing/KiCad)
- [ ] PCB layout (opsional, bonus)
- [ ] Foto prototype
- [ ] Daftar komponen (BOM)

### 3. Dokumentasi
- [ ] Laporan teknis (10-15 halaman)
- [ ] User manual
- [ ] Diagram blok sistem
- [ ] Flowchart program

### 4. Video Demonstrasi
- [ ] Video demo (5-7 menit)
- [ ] Penjelasan fitur
- [ ] Troubleshooting yang dihadapi
- [ ] Future improvements

---

## 📅 Timeline Pengerjaan

| Minggu | Task | Deliverable |
|--------|------|-------------|
| 1 | Persiapan & Design | Skema, flowchart |
| 1 | STM32 DAC Implementation | Audio generator working |
| 2 | ESP32 PWM Implementation | Visual controller working |
| 2 | UART Communication | Protokol terintegrasi |
| 3 | WiFi & Web Interface | Remote control working |
| 3 | Testing & Documentation | Laporan, video |

---

## 🏆 Kriteria Penilaian

| Komponen | Bobot | Detail |
|----------|-------|--------|
| Fungsionalitas | 35% | Semua fitur bekerja |
| Kode & Dokumentasi | 25% | Clean code, comments, laporan |
| Integrasi Dual-MCU | 20% | Komunikasi UART reliable |
| Kreativitas | 10% | Fitur tambahan, UI/UX |
| Presentasi | 10% | Video demo, penjelasan |

---

## 💡 Tips Pengerjaan

1. **Mulai dari yang simple** - Test DAC dan PWM terpisah dulu
2. **Test komunikasi UART** - Pastikan protokol benar sebelum integrasi
3. **Gunakan Serial Monitor** - Debug dengan print statements
4. **Incremental development** - Tambah fitur satu per satu
5. **Version control** - Gunakan Git untuk tracking changes

---

## 🔗 Resources

- [STM32 DAC Application Note](https://www.st.com/resource/en/application_note/an3126.pdf)
- [ESP32 LEDC Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html)
- [ESP32 WebServer Example](https://github.com/espressif/arduino-esp32/tree/master/libraries/WebServer)
- [Audio Synthesis Fundamentals](https://www.soundonsound.com/techniques/synth-secrets)
