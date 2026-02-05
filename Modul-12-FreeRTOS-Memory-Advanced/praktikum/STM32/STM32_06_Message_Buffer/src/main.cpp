/**
 * @file main.cpp
 * @brief Program 06: Message Buffer Communication pada STM32
 * 
 * Deskripsi:
 * Message Buffer adalah stream buffer dengan penambahan framing.
 * Setiap message disimpan dengan length prefix untuk menjaga
 * integritas batas message.
 * 
 * Perbedaan dengan Stream Buffer:
 * - Stream Buffer: Raw byte stream, no framing
 * - Message Buffer: Discrete messages with length header
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include <message_buffer.h>

// ==================== KONFIGURASI ====================
#define LED_PIN             PC13
#define SERIAL_BAUD         115200

// Message Buffer Configuration
#define MSG_BUFFER_SIZE     512     // Total buffer size

// ==================== MESSAGE TYPES ====================

// Enum untuk tipe message
typedef enum {
    MSG_TYPE_SENSOR_DATA = 0x01,
    MSG_TYPE_SYSTEM_STATUS = 0x02,
    MSG_TYPE_COMMAND = 0x03,
    MSG_TYPE_ACK = 0x04,
    MSG_TYPE_ERROR = 0xFF
} MessageType_t;

// Header untuk semua messages
typedef struct __attribute__((packed)) {
    uint8_t type;
    uint8_t sender;
    uint16_t sequence;
    uint32_t timestamp;
} MessageHeader_t;

// Sensor data message
typedef struct __attribute__((packed)) {
    MessageHeader_t header;
    float temperature;
    float humidity;
    float pressure;
    uint8_t status;
} SensorMessage_t;

// System status message
typedef struct __attribute__((packed)) {
    MessageHeader_t header;
    uint32_t uptime;
    uint32_t freeHeap;
    uint8_t cpuLoad;
    uint8_t taskCount;
} StatusMessage_t;

// Command message
typedef struct __attribute__((packed)) {
    MessageHeader_t header;
    uint8_t command;
    uint8_t params[8];
} CommandMessage_t;

// ==================== VARIABEL GLOBAL ====================
MessageBufferHandle_t xSensorMsgBuffer = NULL;
MessageBufferHandle_t xStatusMsgBuffer = NULL;
MessageBufferHandle_t xCommandMsgBuffer = NULL;

TaskHandle_t xSensorTask = NULL;
TaskHandle_t xStatusTask = NULL;
TaskHandle_t xProcessorTask = NULL;
TaskHandle_t xControllerTask = NULL;

volatile uint16_t msgSequence = 0;

// ==================== HELPER FUNCTIONS ====================

/**
 * @brief Get message type name
 */
const char* getMsgTypeName(MessageType_t type) {
    switch (type) {
        case MSG_TYPE_SENSOR_DATA: return "SENSOR_DATA";
        case MSG_TYPE_SYSTEM_STATUS: return "SYS_STATUS";
        case MSG_TYPE_COMMAND: return "COMMAND";
        case MSG_TYPE_ACK: return "ACK";
        case MSG_TYPE_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Print message buffer stats
 */
void printMsgBufferStats() {
    Serial.println("\n┌─────────────────────────────────────────────┐");
    Serial.println("│       MESSAGE BUFFER STATISTICS             │");
    Serial.println("├─────────────────────────────────────────────┤");
    
    Serial.printf("│ SensorBuffer:  %3d bytes free              │\n",
                 xMessageBufferSpacesAvailable(xSensorMsgBuffer));
    Serial.printf("│ StatusBuffer:  %3d bytes free              │\n",
                 xMessageBufferSpacesAvailable(xStatusMsgBuffer));
    Serial.printf("│ CommandBuffer: %3d bytes free              │\n",
                 xMessageBufferSpacesAvailable(xCommandMsgBuffer));
    
    Serial.println("└─────────────────────────────────────────────┘");
}

// ==================== TASKS ====================

/**
 * @brief Sensor task - produces sensor data messages
 */
void vSensorTask(void *pvParameters) {
    Serial.println("[Sensor] Task started");
    
    for (;;) {
        SensorMessage_t msg;
        
        // Fill message
        msg.header.type = MSG_TYPE_SENSOR_DATA;
        msg.header.sender = 1;
        msg.header.sequence = msgSequence++;
        msg.header.timestamp = xTaskGetTickCount();
        
        msg.temperature = 20.0 + (random(0, 200) / 10.0);
        msg.humidity = 40.0 + (random(0, 400) / 10.0);
        msg.pressure = 1000.0 + (random(0, 500) / 10.0);
        msg.status = 0x01;
        
        // Send complete message atomically
        size_t sent = xMessageBufferSend(xSensorMsgBuffer,
                                         &msg,
                                         sizeof(msg),
                                         pdMS_TO_TICKS(100));
        
        if (sent == sizeof(msg)) {
            Serial.printf("[Sensor] Sent #%d: T=%.1f H=%.1f P=%.1f\n",
                         msg.header.sequence, msg.temperature,
                         msg.humidity, msg.pressure);
        } else {
            Serial.println("[Sensor] Failed to send - buffer full!");
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
 * @brief Status task - produces system status messages
 */
void vStatusTask(void *pvParameters) {
    Serial.println("[Status] Task started");
    
    for (;;) {
        StatusMessage_t msg;
        
        msg.header.type = MSG_TYPE_SYSTEM_STATUS;
        msg.header.sender = 2;
        msg.header.sequence = msgSequence++;
        msg.header.timestamp = xTaskGetTickCount();
        
        msg.uptime = millis() / 1000;
        msg.freeHeap = xPortGetFreeHeapSize();
        msg.cpuLoad = random(10, 60);  // Simulated
        msg.taskCount = uxTaskGetNumberOfTasks();
        
        size_t sent = xMessageBufferSend(xStatusMsgBuffer,
                                         &msg,
                                         sizeof(msg),
                                         pdMS_TO_TICKS(100));
        
        if (sent == sizeof(msg)) {
            Serial.printf("[Status] Sent #%d: Uptime=%lu Heap=%lu Tasks=%d\n",
                         msg.header.sequence, msg.uptime,
                         msg.freeHeap, msg.taskCount);
        }
        
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

/**
 * @brief Processor task - receives and processes all messages
 */
void vProcessorTask(void *pvParameters) {
    Serial.println("[Processor] Task started");
    
    uint8_t buffer[64];  // General receive buffer
    
    for (;;) {
        // Try to receive sensor message (non-blocking check)
        size_t received = xMessageBufferReceive(xSensorMsgBuffer,
                                                buffer,
                                                sizeof(buffer),
                                                pdMS_TO_TICKS(500));
        
        if (received > 0) {
            SensorMessage_t *sensorMsg = (SensorMessage_t*)buffer;
            Serial.println("\n╔═════════════════════════════════════════╗");
            Serial.println("║     PROCESSING SENSOR MESSAGE           ║");
            Serial.println("╠═════════════════════════════════════════╣");
            Serial.printf("║ Type: %s                       ║\n", 
                         getMsgTypeName((MessageType_t)sensorMsg->header.type));
            Serial.printf("║ Seq:  %5d  Sender: %d  Time: %lu      ║\n",
                         sensorMsg->header.sequence,
                         sensorMsg->header.sender,
                         sensorMsg->header.timestamp);
            Serial.printf("║ Temp: %.1f°C  Hum: %.1f%%              ║\n",
                         sensorMsg->temperature, sensorMsg->humidity);
            Serial.printf("║ Pressure: %.1f hPa                    ║\n",
                         sensorMsg->pressure);
            Serial.println("╚═════════════════════════════════════════╝");
        }
        
        // Try to receive status message
        received = xMessageBufferReceive(xStatusMsgBuffer,
                                         buffer,
                                         sizeof(buffer),
                                         pdMS_TO_TICKS(500));
        
        if (received > 0) {
            StatusMessage_t *statusMsg = (StatusMessage_t*)buffer;
            Serial.println("\n┌─────────────────────────────────────────┐");
            Serial.println("│     PROCESSING STATUS MESSAGE           │");
            Serial.println("├─────────────────────────────────────────┤");
            Serial.printf("│ Seq: %5d  Uptime: %lu sec             │\n",
                         statusMsg->header.sequence, statusMsg->uptime);
            Serial.printf("│ Free Heap: %lu bytes                   │\n",
                         statusMsg->freeHeap);
            Serial.printf("│ CPU Load: %d%%  Tasks: %d               │\n",
                         statusMsg->cpuLoad, statusMsg->taskCount);
            Serial.println("└─────────────────────────────────────────┘");
        }
        
        // LED indicator
        digitalWrite(LED_PIN, LOW);
        vTaskDelay(pdMS_TO_TICKS(50));
        digitalWrite(LED_PIN, HIGH);
    }
}

/**
 * @brief Controller task - sends command messages (simulated user input)
 */
void vControllerTask(void *pvParameters) {
    Serial.println("[Controller] Task started");
    
    uint8_t cmdNum = 0;
    
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(10000));  // Send commands periodically
        
        CommandMessage_t cmd;
        cmd.header.type = MSG_TYPE_COMMAND;
        cmd.header.sender = 3;
        cmd.header.sequence = msgSequence++;
        cmd.header.timestamp = xTaskGetTickCount();
        cmd.command = cmdNum++;
        memset(cmd.params, 0, sizeof(cmd.params));
        cmd.params[0] = random(0, 255);
        
        size_t sent = xMessageBufferSend(xCommandMsgBuffer,
                                         &cmd,
                                         sizeof(cmd),
                                         pdMS_TO_TICKS(100));
        
        if (sent == sizeof(cmd)) {
            Serial.printf("[Controller] Sent CMD #%d param=%d\n",
                         cmd.command, cmd.params[0]);
        }
    }
}

// ==================== SETUP & LOOP ====================

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(2000);
    
    Serial.println("\n════════════════════════════════════════════════════════════");
    Serial.println("         MODUL 12: MESSAGE BUFFER COMMUNICATION             ");
    Serial.println("════════════════════════════════════════════════════════════\n");
    
    Serial.println("Message Buffer vs Stream Buffer:");
    Serial.println("  Stream Buffer:");
    Serial.println("    - Raw byte stream without framing");
    Serial.println("    - Receiver must parse boundaries");
    Serial.println("");
    Serial.println("  Message Buffer:");
    Serial.println("    - Each message has length header");
    Serial.println("    - Atomic message send/receive");
    Serial.println("    - Maintains message boundaries");
    Serial.println("");
    
    // Initialize LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    
    // Initialize random
    randomSeed(analogRead(PA0));
    
    // Create message buffers
    xSensorMsgBuffer = xMessageBufferCreate(MSG_BUFFER_SIZE);
    xStatusMsgBuffer = xMessageBufferCreate(256);
    xCommandMsgBuffer = xMessageBufferCreate(128);
    
    if (xSensorMsgBuffer == NULL || xStatusMsgBuffer == NULL || 
        xCommandMsgBuffer == NULL) {
        Serial.println("[ERROR] Failed to create message buffers!");
        while (1);
    }
    
    Serial.println("Message structures:");
    Serial.printf("  SensorMessage_t:  %d bytes\n", sizeof(SensorMessage_t));
    Serial.printf("  StatusMessage_t:  %d bytes\n", sizeof(StatusMessage_t));
    Serial.printf("  CommandMessage_t: %d bytes\n", sizeof(CommandMessage_t));
    Serial.println("");
    
    // Create tasks
    xTaskCreate(vSensorTask, "Sensor", 256, NULL, 1, &xSensorTask);
    xTaskCreate(vStatusTask, "Status", 256, NULL, 1, &xStatusTask);
    xTaskCreate(vProcessorTask, "Processor", 512, NULL, 2, &xProcessorTask);
    xTaskCreate(vControllerTask, "Controller", 256, NULL, 1, &xControllerTask);
    
    Serial.println("Starting scheduler...\n");
    
    vTaskStartScheduler();
    
    Serial.println("[ERROR] Scheduler failed!");
}

void loop() {
    // Empty
}

/**
 * PENJELASAN MESSAGE BUFFER:
 * 
 * 1. xMessageBufferCreate(size):
 *    - Membuat message buffer dengan kapasitas tertentu
 *    - Actual capacity = size - 4 (4 bytes untuk length header)
 * 
 * 2. xMessageBufferSend(buf, data, length, timeout):
 *    - Mengirim message lengkap dengan atomic operation
 *    - Returns: sizeof(message) jika sukses, 0 jika gagal
 *    - Message disimpan dengan length prefix
 * 
 * 3. xMessageBufferReceive(buf, data, maxLen, timeout):
 *    - Menerima SATU message lengkap
 *    - Returns: actual message length yang diterima
 *    - Receiver selalu dapat complete message
 * 
 * 4. Message Buffer Overhead:
 *    - 4 bytes per message untuk length field
 *    - Consider ini saat sizing buffer
 * 
 * BEST PRACTICES:
 * 
 * 1. Use packed structs untuk efficient memory usage
 * 2. Include header dengan type, sender, sequence, timestamp
 * 3. Validate message type sebelum casting
 * 4. Size buffer untuk worst-case scenario
 * 
 * EXPECTED OUTPUT:
 * 
 * [Sensor] Sent #0: T=25.3 H=55.7 P=1015.2
 * 
 * ╔═════════════════════════════════════════╗
 * ║     PROCESSING SENSOR MESSAGE           ║
 * ╠═════════════════════════════════════════╣
 * ║ Type: SENSOR_DATA                       ║
 * ║ Seq:     0  Sender: 1  Time: 1234       ║
 * ║ Temp: 25.3°C  Hum: 55.7%                ║
 * ║ Pressure: 1015.2 hPa                    ║
 * ╚═════════════════════════════════════════╝
 */
