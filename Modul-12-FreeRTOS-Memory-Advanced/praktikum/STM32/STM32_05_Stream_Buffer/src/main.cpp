/**
 * @file main.cpp
 * @brief Program 05: Stream Buffer Communication pada STM32
 * 
 * Deskripsi:
 * Stream Buffer adalah mekanisme komunikasi untuk transfer data
 * stream (aliran byte) antar task dengan efisien.
 * 
 * Perbedaan dengan Queue:
 * - Queue: Fixed-size items, overhead per item
 * - Stream Buffer: Continuous byte stream, minimal overhead
 * 
 * Use Case:
 * - Serial data reception
 * - Audio streaming
 * - Data logging
 * - Any continuous data flow
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include <stream_buffer.h>

// ==================== KONFIGURASI ====================
#define LED_PIN             PC13
#define SERIAL_BAUD         115200

// Stream Buffer Configuration
#define STREAM_BUFFER_SIZE  256     // Total buffer size
#define TRIGGER_LEVEL       32      // Minimum bytes before unblocking receiver

// ==================== VARIABEL GLOBAL ====================
StreamBufferHandle_t xDataStream = NULL;
StreamBufferHandle_t xLogStream = NULL;

TaskHandle_t xSenderTask = NULL;
TaskHandle_t xReceiverTask = NULL;
TaskHandle_t xLoggerTask = NULL;

// Statistics
volatile uint32_t bytesSent = 0;
volatile uint32_t bytesReceived = 0;
volatile uint32_t sendErrors = 0;

// ==================== STREAM BUFFER HELPER ====================

/**
 * @brief Print stream buffer statistics
 */
void printStreamStats(StreamBufferHandle_t stream, const char* name) {
    size_t spaceFree = xStreamBufferSpacesAvailable(stream);
    size_t bytesWaiting = xStreamBufferBytesAvailable(stream);
    bool isEmpty = xStreamBufferIsEmpty(stream);
    bool isFull = xStreamBufferIsFull(stream);
    
    Serial.printf("  %s: %3d bytes waiting, %3d free %s%s\n",
                 name, bytesWaiting, spaceFree,
                 isEmpty ? "[EMPTY]" : "",
                 isFull ? "[FULL]" : "");
}

// ==================== TASKS ====================

/**
 * @brief Sender task - mengirim data stream
 * 
 * Mensimulasikan data dari sensor atau peripheral
 */
void vSenderTask(void *pvParameters) {
    Serial.println("[Sender] Task started");
    
    char buffer[64];
    uint32_t sequence = 0;
    
    for (;;) {
        // Generate sensor-like data
        float temp = 20.0 + (random(0, 200) / 10.0);
        float hum = 40.0 + (random(0, 400) / 10.0);
        uint32_t timestamp = xTaskGetTickCount();
        
        // Format as CSV line (like data stream from sensor)
        int len = snprintf(buffer, sizeof(buffer), 
                          "%lu,%lu,%.1f,%.1f\n",
                          sequence++, timestamp, temp, hum);
        
        // Send to stream buffer
        size_t sent = xStreamBufferSend(xDataStream,
                                        buffer,
                                        len,
                                        pdMS_TO_TICKS(100));
        
        if (sent == len) {
            bytesSent += sent;
            // Serial.printf("[Sender] Sent %d bytes: %s", len, buffer);
        } else if (sent > 0) {
            bytesSent += sent;
            Serial.printf("[Sender] Partial send: %d/%d bytes\n", sent, len);
        } else {
            sendErrors++;
            Serial.println("[Sender] Buffer full - data lost!");
        }
        
        // Variable rate simulation
        vTaskDelay(pdMS_TO_TICKS(200 + random(0, 300)));
    }
}

/**
 * @brief Receiver task - menerima dan memproses data stream
 * 
 * Memproses data line-by-line
 */
void vReceiverTask(void *pvParameters) {
    Serial.println("[Receiver] Task started");
    Serial.printf("[Receiver] Will unblock when >= %d bytes available\n", TRIGGER_LEVEL);
    
    char buffer[128];
    char lineBuffer[64];
    int lineIndex = 0;
    
    for (;;) {
        // Receive data - blocks until trigger level reached
        size_t received = xStreamBufferReceive(xDataStream,
                                               buffer,
                                               sizeof(buffer),
                                               pdMS_TO_TICKS(1000));
        
        if (received > 0) {
            bytesReceived += received;
            
            // Process received bytes - parse into lines
            for (size_t i = 0; i < received; i++) {
                if (buffer[i] == '\n' || lineIndex >= sizeof(lineBuffer) - 1) {
                    lineBuffer[lineIndex] = '\0';
                    
                    // Parse CSV line
                    uint32_t seq, ts;
                    float temp, hum;
                    if (sscanf(lineBuffer, "%lu,%lu,%f,%f", &seq, &ts, &temp, &hum) == 4) {
                        Serial.printf("[Receiver] #%lu @ %lu: T=%.1f°C, H=%.1f%%\n",
                                     seq, ts, temp, hum);
                    }
                    
                    lineIndex = 0;
                } else {
                    lineBuffer[lineIndex++] = buffer[i];
                }
            }
        } else {
            Serial.println("[Receiver] Timeout - no data");
        }
    }
}

/**
 * @brief Logger task - demonstrasi multiple readers
 * menggunakan stream buffer terpisah
 */
void vLoggerTask(void *pvParameters) {
    Serial.println("[Logger] Task started");
    
    char logEntry[64];
    uint32_t logNum = 0;
    
    for (;;) {
        // Create log entry
        int len = snprintf(logEntry, sizeof(logEntry),
                          "[LOG #%lu] Sent=%lu Recv=%lu Err=%lu\n",
                          logNum++, bytesSent, bytesReceived, sendErrors);
        
        // Send to log stream
        xStreamBufferSend(xLogStream, logEntry, len, pdMS_TO_TICKS(50));
        
        // LED heartbeat
        digitalWrite(LED_PIN, LOW);
        vTaskDelay(pdMS_TO_TICKS(100));
        digitalWrite(LED_PIN, HIGH);
        
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

/**
 * @brief Monitor task - menampilkan status stream buffers
 */
void vMonitorTask(void *pvParameters) {
    Serial.println("[Monitor] Task started");
    
    char logBuffer[128];
    
    for (;;) {
        Serial.println("\n╔══════════════════════════════════════════════╗");
        Serial.println("║        STREAM BUFFER STATUS MONITOR          ║");
        Serial.println("╠══════════════════════════════════════════════╣");
        Serial.println("║ Buffer Status:                               ║");
        printStreamStats(xDataStream, "DataStream");
        printStreamStats(xLogStream, "LogStream ");
        Serial.println("╠══════════════════════════════════════════════╣");
        Serial.printf("║ Total Sent:     %8lu bytes              ║\n", bytesSent);
        Serial.printf("║ Total Received: %8lu bytes              ║\n", bytesReceived);
        Serial.printf("║ Send Errors:    %8lu                    ║\n", sendErrors);
        Serial.println("╚══════════════════════════════════════════════╝");
        
        // Read and display log entries if available
        size_t logBytes = xStreamBufferReceive(xLogStream,
                                               logBuffer,
                                               sizeof(logBuffer) - 1,
                                               0);  // Non-blocking
        if (logBytes > 0) {
            logBuffer[logBytes] = '\0';
            Serial.print(logBuffer);
        }
        
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// ==================== SETUP & LOOP ====================

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(2000);
    
    Serial.println("\n════════════════════════════════════════════════════════════");
    Serial.println("          MODUL 12: STREAM BUFFER COMMUNICATION             ");
    Serial.println("════════════════════════════════════════════════════════════\n");
    
    Serial.println("Stream Buffer Characteristics:");
    Serial.println("  • Variable-length byte stream");
    Serial.println("  • Single writer, single reader");
    Serial.println("  • Trigger level for efficient wakeup");
    Serial.println("  • Minimal overhead vs queues");
    Serial.println("");
    Serial.printf("Configuration: Buffer=%d bytes, Trigger=%d bytes\n\n",
                 STREAM_BUFFER_SIZE, TRIGGER_LEVEL);
    
    // Initialize LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    
    // Initialize random
    randomSeed(analogRead(PA0));
    
    // Create stream buffers
    xDataStream = xStreamBufferCreate(STREAM_BUFFER_SIZE, TRIGGER_LEVEL);
    xLogStream = xStreamBufferCreate(128, 1);  // Trigger at 1 byte
    
    if (xDataStream == NULL || xLogStream == NULL) {
        Serial.println("[ERROR] Failed to create stream buffers!");
        while (1);
    }
    
    Serial.println("Stream buffers created successfully");
    
    // Create tasks
    BaseType_t result;
    
    result = xTaskCreate(vSenderTask, "Sender", 256, NULL, 2, &xSenderTask);
    if (result != pdPASS) Serial.println("Failed to create Sender task");
    
    result = xTaskCreate(vReceiverTask, "Receiver", 256, NULL, 2, &xReceiverTask);
    if (result != pdPASS) Serial.println("Failed to create Receiver task");
    
    result = xTaskCreate(vLoggerTask, "Logger", 256, NULL, 1, &xLoggerTask);
    if (result != pdPASS) Serial.println("Failed to create Logger task");
    
    TaskHandle_t xMonitorTask = NULL;
    result = xTaskCreate(vMonitorTask, "Monitor", 256, NULL, 1, &xMonitorTask);
    if (result != pdPASS) Serial.println("Failed to create Monitor task");
    
    Serial.println("\nStarting scheduler...\n");
    
    vTaskStartScheduler();
    
    Serial.println("[ERROR] Scheduler failed!");
}

void loop() {
    // Empty
}

/**
 * PENJELASAN STREAM BUFFER:
 * 
 * 1. xStreamBufferCreate(size, triggerLevel):
 *    - size: Total buffer size in bytes
 *    - triggerLevel: Minimum bytes before reader unblocks
 * 
 * 2. xStreamBufferSend():
 *    - Mengirim byte stream ke buffer
 *    - Bisa partial send jika buffer hampir penuh
 *    - Returns jumlah bytes yang berhasil dikirim
 * 
 * 3. xStreamBufferReceive():
 *    - Menerima bytes dari buffer
 *    - Blocks sampai triggerLevel tercapai atau timeout
 *    - Returns jumlah bytes yang diterima
 * 
 * 4. Stream Buffer vs Queue:
 *    Stream Buffer:
 *    + Efficient untuk continuous data
 *    + Variable-length transfers
 *    + Lower overhead per byte
 *    - Single writer/reader only
 *    
 *    Queue:
 *    + Multiple writers/readers
 *    + Fixed-size items
 *    + Better for discrete messages
 *    - Higher overhead
 * 
 * EXPECTED OUTPUT:
 * 
 * [Receiver] #0 @ 1234: T=25.3°C, H=55.7%
 * [Receiver] #1 @ 1567: T=26.1°C, H=58.2%
 * 
 * ╔══════════════════════════════════════════════╗
 * ║        STREAM BUFFER STATUS MONITOR          ║
 * ║ Buffer Status:                               ║
 * ║   DataStream:  45 bytes waiting, 211 free    ║
 * ║ Total Sent:        1234 bytes                ║
 * ║ Total Received:    1189 bytes                ║
 * ╚══════════════════════════════════════════════╝
 */
