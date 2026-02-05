# Project Modul 12: FreeRTOS Memory Management & Advanced Features

## 🎯 Judul Project

### **"Smart Industrial Data Logger dengan Memory Optimization dan Power Management"**

**Integrasi ESP32 + STM32 untuk Sistem Logging Data Industrial dengan Manajemen Memori Efisien**

---

## 📋 Deskripsi Project

Membangun sistem data logger industrial yang mengintegrasikan **ESP32** sebagai gateway/hub dan **STM32** sebagai sensor node. Sistem harus mengimplementasikan teknik memory management advanced, event-driven architecture, dan power optimization untuk operasi jangka panjang.

### Skenario Aplikasi

Sebuah pabrik membutuhkan sistem monitoring yang:
1. Mengumpulkan data dari multiple sensor (STM32 nodes)
2. Mengirim data ke central gateway (ESP32)
3. Menyimpan data saat offline
4. Beroperasi dengan battery backup selama mungkin
5. Memberikan alert real-time untuk kondisi abnormal

---

## 🔧 Arsitektur Sistem

```
┌─────────────────────────────────────────────────────────────────────┐
│                      INDUSTRIAL DATA LOGGER                          │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   ┌─────────────┐         UART          ┌─────────────────────┐    │
│   │   STM32     │◄─────────────────────►│       ESP32         │    │
│   │  Sensor     │    Stream Buffer       │      Gateway        │    │
│   │   Node      │                        │                     │    │
│   │             │    Message Buffer      │   ┌─────────────┐   │    │
│   │ - Temperature│◄────────────────────►│   │  WiFi/MQTT  │   │    │
│   │ - Pressure  │                        │   └─────────────┘   │    │
│   │ - Vibration │    Event Groups        │                     │    │
│   │             │◄────────────────────►│   ┌─────────────┐   │    │
│   │ Low Power   │                        │   │   SD Card   │   │    │
│   │   Mode      │                        │   │   Logger    │   │    │
│   └─────────────┘                        │   └─────────────┘   │    │
│                                          │                     │    │
│   ┌─────────────┐                        │   ┌─────────────┐   │    │
│   │   Memory    │                        │   │   Memory    │   │    │
│   │   Pool      │                        │   │   Stats     │   │    │
│   │  (Sensors)  │                        │   │  Dashboard  │   │    │
│   └─────────────┘                        │   └─────────────┘   │    │
│                                          └─────────────────────┘    │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 📝 Spesifikasi Teknis

### STM32 Sensor Node Requirements

| Komponen | Spesifikasi |
|----------|-------------|
| MCU | STM32F103C8T6 Blue Pill |
| Sensors | Temperature (NTC/DHT11), Vibration (SW-420), Potentiometer |
| Communication | UART ke ESP32 |
| Memory | Static allocation untuk semua tasks |
| Power | Tickless idle, sleep between readings |
| Stack Monitoring | High water mark per task |
| Protocol | Custom message buffer format |

### ESP32 Gateway Requirements

| Komponen | Spesifikasi |
|----------|-------------|
| MCU | ESP32 DevKit V1 |
| Communication | UART (dari STM32), WiFi (ke cloud) |
| Storage | SD Card untuk local logging |
| Memory | heap_4 dengan statistics monitoring |
| Features | Event groups untuk state management |
| Protocol | MQTT untuk cloud, Message Buffer untuk local |
| Power | Light sleep saat idle |

---

## 🎯 Fitur Wajib (Minimum Requirements)

### 1. Memory Management (30%)

#### STM32 - Static Allocation
```c
// Semua task dan resource harus static
static StaticTask_t xSensorTaskBuffer;
static StackType_t xSensorTaskStack[256];

static StaticTask_t xCommTaskBuffer;
static StackType_t xCommTaskStack[512];

// Memory pool untuk sensor data
#define SENSOR_POOL_SIZE    16
#define SENSOR_DATA_SIZE    32
static MemoryPool_t sensorPool;
```

#### ESP32 - Dynamic dengan Monitoring
```c
// Heap statistics setiap 10 detik
void memoryMonitorTask(void *pvParameters) {
    HeapStats_t stats;
    for(;;) {
        vPortGetHeapStats(&stats);
        logMemoryStats(&stats);
        
        if (stats.xAvailableHeapSpaceInBytes < HEAP_WARNING_THRESHOLD) {
            xEventGroupSetBits(xSystemEvents, EVENT_LOW_MEMORY);
        }
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
```

### 2. Stack Overflow Protection (15%)

```c
// STM32 & ESP32
#define configCHECK_FOR_STACK_OVERFLOW 2

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    // Log to flash/EEPROM
    logCriticalError(ERROR_STACK_OVERFLOW, pcTaskName);
    
    // Attempt recovery or reset
    #ifdef PRODUCTION
        NVIC_SystemReset();
    #else
        while(1) { debugBlink(); }
    #endif
}
```

### 3. Event Groups untuk State Management (20%)

```c
// System-wide events
#define EVENT_SENSOR_READY      (1 << 0)
#define EVENT_WIFI_CONNECTED    (1 << 1)
#define EVENT_MQTT_CONNECTED    (1 << 2)
#define EVENT_SD_READY          (1 << 3)
#define EVENT_LOW_BATTERY       (1 << 4)
#define EVENT_ALARM_ACTIVE      (1 << 5)
#define EVENT_DATA_PENDING      (1 << 6)
#define EVENT_LOW_MEMORY        (1 << 7)

EventGroupHandle_t xSystemEvents;

// Wait for system ready
xEventGroupWaitBits(xSystemEvents, 
    EVENT_SENSOR_READY | EVENT_WIFI_CONNECTED | EVENT_SD_READY,
    pdFALSE, pdTRUE, pdMS_TO_TICKS(30000));
```

### 4. Stream/Message Buffer Communication (20%)

#### STM32 → ESP32 Communication
```c
// Message format
typedef struct {
    uint8_t  nodeId;
    uint8_t  msgType;
    uint32_t timestamp;
    float    temperature;
    float    pressure;
    float    vibration;
    uint16_t batteryMv;
    uint8_t  checksum;
} __attribute__((packed)) SensorMessage_t;

// STM32: Send via Message Buffer
MessageBufferHandle_t xTxBuffer;

void sendSensorData(SensorMessage_t *data) {
    data->checksum = calculateChecksum(data);
    xMessageBufferSend(xTxBuffer, data, sizeof(SensorMessage_t), pdMS_TO_TICKS(100));
}

// ESP32: Receive via Stream Buffer (UART interrupt)
StreamBufferHandle_t xRxStream;

void UART_RxCallback(uint8_t byte) {
    xStreamBufferSendFromISR(xRxStream, &byte, 1, NULL);
}
```

### 5. Low Power Implementation (15%)

```c
// STM32: Tickless idle dengan sleep between readings
void sensorTask(void *pvParameters) {
    for(;;) {
        // Wake up, read sensors
        readAllSensors();
        
        // Send data
        sendSensorData(&data);
        
        // Sleep for 5 seconds (tickless idle akan aktif)
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// ESP32: Light sleep saat idle
void configurePowerManagement(void) {
    esp_pm_config_esp32_t pm_config = {
        .max_freq_mhz = 240,
        .min_freq_mhz = 80,
        .light_sleep_enable = true
    };
    esp_pm_configure(&pm_config);
}
```

---

## 🌟 Fitur Tambahan (Bonus Points)

### Bonus 1: Memory Leak Detection (+10%)
```c
// Track allocations in debug mode
typedef struct {
    void *ptr;
    size_t size;
    const char *file;
    int line;
    TickType_t tick;
} AllocationRecord_t;

#define TRACKED_MALLOC(size) trackedMalloc(size, __FILE__, __LINE__)
```

### Bonus 2: Heap Fragmentation Monitor (+10%)
```c
// Calculate fragmentation index
float calculateFragmentation(HeapStats_t *stats) {
    float fragIndex = 1.0 - ((float)stats->xSizeOfLargestFreeBlockInBytes / 
                             stats->xAvailableHeapSpaceInBytes);
    return fragIndex * 100;  // 0% = no frag, 100% = severe frag
}
```

### Bonus 3: Adaptive Sleep Duration (+10%)
```c
// Adjust sleep based on battery and activity
uint32_t calculateSleepDuration(void) {
    uint32_t baseSleep = 5000;  // 5 seconds
    
    if (batteryLevel < 20) baseSleep *= 4;      // Low battery: sleep longer
    if (alarmActive) baseSleep = 1000;          // Alarm: wake frequently
    if (noActivityCount > 100) baseSleep *= 2;  // Inactive: sleep longer
    
    return baseSleep;
}
```

### Bonus 4: Remote Memory Statistics via MQTT (+10%)
```c
// Publish memory stats to cloud
void publishMemoryStats(void) {
    char json[256];
    snprintf(json, sizeof(json),
        "{\"heap_free\":%d,\"heap_min\":%d,\"stack_hwm\":%d,\"frag\":%.1f}",
        xPortGetFreeHeapSize(),
        xPortGetMinimumEverFreeHeapSize(),
        uxTaskGetStackHighWaterMark(NULL),
        calculateFragmentation(&stats)
    );
    mqttPublish("device/memory", json);
}
```

---

## 📊 Deliverables

### 1. Source Code
- [ ] STM32 firmware dengan static allocation
- [ ] ESP32 firmware dengan dynamic allocation dan monitoring
- [ ] Shared header files untuk protocol
- [ ] Dokumentasi kode (comments)

### 2. Hardware Setup
- [ ] Foto rangkaian lengkap
- [ ] Wiring diagram
- [ ] Bill of Materials (BOM)

### 3. Documentation
- [ ] System architecture diagram
- [ ] Memory map analysis (sebelum dan sesudah optimasi)
- [ ] State diagram menggunakan event groups
- [ ] Flowchart untuk setiap subsystem

### 4. Testing Results
- [ ] Heap usage graph over 24 hours
- [ ] Stack high water mark per task
- [ ] Power consumption measurements
- [ ] Stress test results (max messages/second)

### 5. Video Demonstration
- [ ] Durasi: 5-10 menit
- [ ] System overview dan arsitektur
- [ ] Live demo dengan Serial Monitor
- [ ] Memory statistics demonstration
- [ ] Event-driven behavior showcase
- [ ] Power measurement demo

---

## 🔌 Wiring Diagram

### STM32 Blue Pill
```
STM32F103C8T6
┌──────────────────────────────────────┐
│                                      │
│  PC13 ──────[LED Status]             │
│                                      │
│  PA0  ──────[Button - Wake/Test]     │
│                                      │
│  PA1  ──────[Temperature Sensor]     │
│  PA2  ──────[Vibration Sensor]       │
│  PA3  ──────[Potentiometer]          │
│                                      │
│  PA9  ──────TX──────────┐            │
│  PA10 ──────RX──────────┼──► ESP32   │
│                         │            │
│  3.3V ─────────────────►│            │
│  GND  ─────────────────►│            │
└──────────────────────────────────────┘
```

### ESP32 DevKit
```
ESP32
┌──────────────────────────────────────┐
│                                      │
│  GPIO2  ────[LED Status]             │
│  GPIO4  ────[LED WiFi]               │
│  GPIO5  ────[LED Alarm]              │
│                                      │
│  GPIO16 ────TX──────────┐            │
│  GPIO17 ────RX──────────┼──► STM32   │
│                         │            │
│  GPIO18 ────SCK ────────┤            │
│  GPIO19 ────MISO────────┼──► SD Card │
│  GPIO23 ────MOSI────────┤            │
│  GPIO5  ────CS ─────────┘            │
│                                      │
│  WiFi: Internal                      │
└──────────────────────────────────────┘
```

---

## 📈 Kriteria Penilaian Detail

| Aspek | Bobot | Excellent (90-100) | Good (70-89) | Fair (50-69) | Poor (<50) |
|-------|-------|-------------------|--------------|--------------|------------|
| **Memory Management** | 30% | Static alloc STM32, monitoring ESP32, pool implemented | Partial implementation | Basic implementation | Tidak implement |
| **Stack Protection** | 15% | Detection method 2, hook dengan recovery | Method 1, basic hook | Basic detection | Tidak ada |
| **Event Groups** | 20% | Multi-event sync, rendezvous, proper cleanup | Basic event sync | Simple set/wait | Tidak implement |
| **Buffer Communication** | 20% | Stream + Message buffer, ISR safe | Salah satu buffer | Basic queue | Direct UART |
| **Low Power** | 15% | Tickless + adaptive sleep + measurement | Basic tickless | Delay-based sleep | No optimization |
| **Bonus Features** | +40% | Semua bonus | 3 bonus | 1-2 bonus | Tidak ada |

---

## 📅 Timeline Pengerjaan

| Minggu | Aktivitas |
|--------|-----------|
| 1 | Setup hardware, basic communication STM32-ESP32 |
| 2 | Implement memory management (static, pool, monitoring) |
| 3 | Implement event groups dan buffer communication |
| 4 | Implement low power, testing, documentation |
| 5 | Finalisasi, video demo, submission |

---

## 💡 Tips Pengerjaan

1. **Start Simple:** Mulai dengan komunikasi UART basic, lalu tambahkan buffer
2. **Monitor Early:** Pasang heap/stack monitoring dari awal development
3. **Static First:** Implementasikan static allocation di STM32 terlebih dahulu
4. **Test Incrementally:** Test setiap fitur sebelum integrasi
5. **Document As You Go:** Catat memory usage di setiap tahap

---

## ⚠️ Common Pitfalls

1. **Stack Too Small:** Gunakan high water mark untuk sizing
2. **Buffer Overflow:** Validasi ukuran message sebelum send
3. **Event Bit Collision:** Dokumentasikan semua event bits
4. **Power Measurement:** Lepas debugger saat mengukur power
5. **Timing Issues:** Perhatikan baudrate dan buffer timing

---

## 📚 Referensi Project

1. [FreeRTOS Memory Management](https://freertos.org/a00111.html)
2. [ESP32 Power Management](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/power_management.html)
3. [STM32 Low Power Modes](https://www.st.com/resource/en/application_note/an4621.pdf)
4. [FreeRTOS Event Groups](https://freertos.org/FreeRTOS-Event-Groups.html)
