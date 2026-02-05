/**
 * @file main.cpp
 * @brief Program 06: ESP32 Message Buffer Multi-Core
 * 
 * Message Buffer untuk komunikasi antar core ESP32.
 * Demonstrasi message passing yang type-safe.
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/message_buffer.h"

#define LED_PIN     2

// Message Buffer
#define MSG_BUFFER_SIZE     512

MessageBufferHandle_t xCore0ToCore1 = NULL;
MessageBufferHandle_t xCore1ToCore0 = NULL;

// Message types
typedef enum {
    MSG_SENSOR_DATA = 1,
    MSG_COMMAND,
    MSG_RESPONSE,
    MSG_HEARTBEAT
} MsgType_t;

typedef struct __attribute__((packed)) {
    MsgType_t type;
    uint8_t sender;
    uint32_t timestamp;
    union {
        struct {
            float temperature;
            float humidity;
        } sensor;
        struct {
            uint8_t cmd;
            uint8_t param;
        } command;
        struct {
            uint8_t status;
            uint32_t value;
        } response;
        uint32_t heartbeat;
    } data;
} Message_t;

// Statistics
volatile uint32_t msgsSent0to1 = 0;
volatile uint32_t msgsSent1to0 = 0;
volatile uint32_t msgsReceived0 = 0;
volatile uint32_t msgsReceived1 = 0;

const char* getMsgTypeName(MsgType_t type) {
    switch (type) {
        case MSG_SENSOR_DATA: return "SENSOR";
        case MSG_COMMAND: return "COMMAND";
        case MSG_RESPONSE: return "RESPONSE";
        case MSG_HEARTBEAT: return "HEARTBEAT";
        default: return "UNKNOWN";
    }
}

// Core 0 tasks - sensor simulation
void vCore0SensorTask(void *pvParameters) {
    Serial.println("[Core0-Sensor] Started");
    
    for (;;) {
        Message_t msg;
        msg.type = MSG_SENSOR_DATA;
        msg.sender = 0;
        msg.timestamp = millis();
        msg.data.sensor.temperature = 20.0 + random(0, 150) / 10.0;
        msg.data.sensor.humidity = 40.0 + random(0, 400) / 10.0;
        
        size_t sent = xMessageBufferSend(xCore0ToCore1, &msg, sizeof(msg), 
                                         pdMS_TO_TICKS(100));
        if (sent == sizeof(msg)) {
            msgsSent0to1++;
            Serial.printf("[Core0] Sent sensor: T=%.1f H=%.1f\n",
                         msg.data.sensor.temperature, msg.data.sensor.humidity);
        }
        
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

void vCore0ReceiverTask(void *pvParameters) {
    Serial.println("[Core0-Receiver] Started");
    
    Message_t msg;
    
    for (;;) {
        size_t received = xMessageBufferReceive(xCore1ToCore0, &msg, sizeof(msg),
                                                pdMS_TO_TICKS(2000));
        
        if (received > 0) {
            msgsReceived0++;
            Serial.printf("[Core0] Received %s from Core%d\n",
                         getMsgTypeName(msg.type), msg.sender);
            
            if (msg.type == MSG_RESPONSE) {
                Serial.printf("[Core0] Response: status=%d value=%lu\n",
                             msg.data.response.status, msg.data.response.value);
            }
        }
    }
}

// Core 1 tasks - processing
void vCore1ProcessorTask(void *pvParameters) {
    Serial.println("[Core1-Processor] Started");
    
    Message_t rxMsg, txMsg;
    
    for (;;) {
        size_t received = xMessageBufferReceive(xCore0ToCore1, &rxMsg, sizeof(rxMsg),
                                                pdMS_TO_TICKS(2000));
        
        if (received > 0) {
            msgsReceived1++;
            
            Serial.printf("[Core1] Processing %s message\n", getMsgTypeName(rxMsg.type));
            
            // Process and send response
            if (rxMsg.type == MSG_SENSOR_DATA) {
                // Simulate processing
                vTaskDelay(pdMS_TO_TICKS(50));
                
                // Send response
                txMsg.type = MSG_RESPONSE;
                txMsg.sender = 1;
                txMsg.timestamp = millis();
                txMsg.data.response.status = 0x01;  // OK
                txMsg.data.response.value = (uint32_t)(rxMsg.data.sensor.temperature * 100);
                
                size_t sent = xMessageBufferSend(xCore1ToCore0, &txMsg, sizeof(txMsg),
                                                 pdMS_TO_TICKS(100));
                if (sent == sizeof(txMsg)) {
                    msgsSent1to0++;
                }
            }
        }
    }
}

void vCore1HeartbeatTask(void *pvParameters) {
    Serial.println("[Core1-Heartbeat] Started");
    
    Message_t msg;
    
    for (;;) {
        msg.type = MSG_HEARTBEAT;
        msg.sender = 1;
        msg.timestamp = millis();
        msg.data.heartbeat = xTaskGetTickCount();
        
        xMessageBufferSend(xCore1ToCore0, &msg, sizeof(msg), pdMS_TO_TICKS(50));
        msgsSent1to0++;
        
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void vMonitorTask(void *pvParameters) {
    Serial.println("[Monitor] Started");
    
    for (;;) {
        Serial.println("\n╔══════════════════════════════════════════════════════╗");
        Serial.println("║      MESSAGE BUFFER MULTI-CORE STATISTICS            ║");
        Serial.println("╠══════════════════════════════════════════════════════╣");
        Serial.printf("║ Core0 -> Core1: %5lu messages sent                  ║\n", msgsSent0to1);
        Serial.printf("║ Core1 -> Core0: %5lu messages sent                  ║\n", msgsSent1to0);
        Serial.printf("║ Core0 received: %5lu messages                       ║\n", msgsReceived0);
        Serial.printf("║ Core1 received: %5lu messages                       ║\n", msgsReceived1);
        Serial.println("╠══════════════════════════════════════════════════════╣");
        Serial.printf("║ Buffer 0->1: %4d bytes free                         ║\n",
                     xMessageBufferSpacesAvailable(xCore0ToCore1));
        Serial.printf("║ Buffer 1->0: %4d bytes free                         ║\n",
                     xMessageBufferSpacesAvailable(xCore1ToCore0));
        Serial.println("╠══════════════════════════════════════════════════════╣");
        Serial.printf("║ Message_t size: %d bytes                             ║\n", sizeof(Message_t));
        Serial.printf("║ Free heap: %lu bytes                                 ║\n", 
                     (unsigned long)ESP.getFreeHeap());
        Serial.println("╚══════════════════════════════════════════════════════╝");
        
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n═══════════════════════════════════════════════════════");
    Serial.println("     ESP32 MESSAGE BUFFER MULTI-CORE COMMUNICATION");
    Serial.println("═══════════════════════════════════════════════════════\n");
    
    pinMode(LED_PIN, OUTPUT);
    
    // Create message buffers for bidirectional communication
    xCore0ToCore1 = xMessageBufferCreate(MSG_BUFFER_SIZE);
    xCore1ToCore0 = xMessageBufferCreate(MSG_BUFFER_SIZE);
    
    if (xCore0ToCore1 == NULL || xCore1ToCore0 == NULL) {
        Serial.println("Failed to create message buffers!");
        while (1);
    }
    
    Serial.println("Message Buffers created for inter-core communication");
    Serial.printf("Message size: %d bytes\n\n", sizeof(Message_t));
    
    // Core 0 tasks
    xTaskCreatePinnedToCore(vCore0SensorTask, "C0Sensor", 4096, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(vCore0ReceiverTask, "C0Recv", 4096, NULL, 2, NULL, 0);
    
    // Core 1 tasks
    xTaskCreatePinnedToCore(vCore1ProcessorTask, "C1Proc", 4096, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(vCore1HeartbeatTask, "C1HB", 4096, NULL, 1, NULL, 1);
    
    // Monitor on Core 0
    xTaskCreatePinnedToCore(vMonitorTask, "Monitor", 4096, NULL, 1, NULL, 0);
    
    Serial.println("All tasks created. Multi-core messaging active.\n");
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
