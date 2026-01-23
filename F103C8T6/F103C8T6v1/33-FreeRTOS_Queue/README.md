# Program 33: FreeRTOS Queue - Inter-Task Communication

## Deskripsi
Program ini mendemonstrasikan penggunaan **FreeRTOS Queue** untuk komunikasi antar task (Producer-Consumer pattern). Producer task membaca ADC dan mengirim data ke queue, sedangkan Consumer task mengambil data dari queue dan menampilkan via UART.

## Hardware Requirements
- STM32F103C8T6 Blue Pill board
- Potentiometer 10kΩ di PA0 (ADC input)
- LED built-in di PC13 (queue full indicator)
- USB-to-TTL Serial adapter
- ST-Link V2 programmer

## Pin Configuration
| Pin | Function | Description |
|-----|----------|-------------|
| PA0 | ADC1_IN0 | Potentiometer input (0-3.3V) |
| PA9 | USART1_TX | UART transmit |
| PA10 | USART1_RX | UART receive |
| PC13 | LED | Queue full indicator (active LOW) |

## Features
### Producer Task (Priority 2)
- Sampling ADC setiap 500ms (2Hz)
- Mengirim data ke queue (non-blocking)
- Mendeteksi queue full condition
- Menyalakan LED jika queue penuh

### Consumer Task (Priority 1)
- Menerima data dari queue (blocking)
- Menampilkan data via UART:
  - Timestamp
  - ADC value (0-4095)
  - Voltage (0-3.3V)
  - Queue depth
  - Statistics (produced, consumed, lost)
- Simulasi variable processing time

### Queue Configuration
- Length: 10 items
- Item size: sizeof(ADC_Data_t) = 12 bytes
- Total memory: 120 bytes

## Konsep FreeRTOS Queue

### Queue Operations
```c
// Create queue
QueueHandle_t xQueue = xQueueCreate(LENGTH, ITEM_SIZE);

// Send to queue (Producer)
xQueueSend(xQueue, &data, timeout);

// Receive from queue (Consumer)
xQueueReceive(xQueue, &data, timeout);

// Check queue status
uxQueueMessagesWaiting(xQueue);
```

### Producer-Consumer Pattern
1. **Producer**: Generates data dan push ke queue
2. **Consumer**: Pop data dari queue dan process
3. **Decoupling**: Tasks tidak perlu tahu tentang satu sama lain
4. **Buffering**: Queue acts as buffer untuk handle rate mismatch

## Cara Menggunakan

### 1. Build dan Upload
```bash
cd 33-FreeRTOS_Queue
pio run -t upload
```

### 2. Monitor Serial (115200 baud)
```bash
pio device monitor -b 115200
```

### 3. Testing Scenarios

#### Normal Operation (Slow Consumer)
- Putar potentiometer ke posisi rendah
- Consumer processing time: 100-120ms
- Producer rate: 500ms (2Hz)
- Queue tidak akan penuh

#### Queue Overflow (Fast Producer)
- Putar potentiometer ke posisi tinggi
- Consumer processing time: 190-200ms
- Producer rate: 500ms
- Queue akan penuh, LED menyala
- Data akan hilang

### Expected Output
```
=== FreeRTOS Queue Demo ===
Program 33: Inter-Task Communication
Queue Length: 10 items
Producer: ADC sampling @ 2Hz
Consumer: UART logging

Queue created successfully

[PRODUCER] Task started
[CONSUMER] Task started

[1023] ADC: 2048 | 1.650V | Queue: 0/10 | P:1 C:1 Lost:0
[1523] ADC: 2050 | 1.651V | Queue: 0/10 | P:2 C:2 Lost:0
[2023] ADC: 2052 | 1.652V | Queue: 1/10 | P:3 C:2 Lost:0
...
[PRODUCER] WARNING: Queue full! Data lost.
[5523] ADC: 2055 | 1.654V | Queue: 9/10 | P:15 C:10 Lost:4
```

## Learning Points

### 1. Queue Benefits
- ✅ Thread-safe communication
- ✅ Decouples producers and consumers
- ✅ Handles rate mismatch dengan buffering
- ✅ FIFO ordering guaranteed

### 2. Queue Full Handling
```c
// Non-blocking send (Producer)
if (xQueueSend(queue, &data, 0) != pdPASS) {
    // Queue full - handle error
    queue_full_count++;
}
```

### 3. Blocking Receive
```c
// Consumer waits indefinitely for data
xQueueReceive(queue, &data, portMAX_DELAY);
```

### 4. Queue Statistics
```c
UBaseType_t waiting = uxQueueMessagesWaiting(queue);
UBaseType_t spaces = uxQueueSpacesAvailable(queue);
```

## Troubleshooting

### Problem: Queue selalu penuh
**Solusi**:
- Consumer terlalu lambat, tambahkan priority atau kurangi processing time
- Producer terlalu cepat, kurangi sampling rate
- Tambah queue depth

### Problem: Data tidak sampai ke consumer
**Solusi**:
- Cek xQueueSend() return value
- Pastikan item size sesuai dengan struct
- Verify queue creation berhasil

### Problem: Stack overflow
**Solusi**:
- Increase task stack size di xTaskCreate()
- Reduce local variables atau gunakan static

## Advanced Modifications

### 1. Multiple Producers
```c
// Task 1: ADC producer
// Task 2: UART RX producer
// Task 3: Consumer logger
```

### 2. Priority Queue
Gunakan multiple queues dengan priority handling

### 3. Queue Set
Handle multiple queues di single consumer

### 4. Mailbox Pattern
Queue dengan depth=1 untuk latest value only

## Memory Usage
- Queue overhead: ~40 bytes
- Queue storage: 10 × 12 = 120 bytes
- Producer task stack: 256 × 4 = 1024 bytes
- Consumer task stack: 256 × 4 = 1024 bytes
- **Total**: ~2.2KB RAM

## Performance
- Queue operation: ~50 CPU cycles
- Context switch overhead: ~150 cycles
- Maximum throughput: ~100,000 messages/sec @ 72MHz

## References
- FreeRTOS Queue API: https://www.freertos.org/a00018.html
- Producer-Consumer Pattern
- "Mastering STM32" - Chapter 11: FreeRTOS

## License
Educational use - Based on "Mastering STM32" by Carmine Noviello
