/**
 * @file main.cpp
 * @brief Program 05: ESP32 Stream Buffer dengan ISR
 * 
 * Demonstrasi penggunaan Stream Buffer untuk komunikasi
 * antara ISR (interrupt) dan task.
 * 
 * Use case: Menerima data dari UART/GPIO interrupt
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/stream_buffer.h"
#include "driver/timer.h"

#define LED_PIN         2
#define BUTTON_PIN      0   // Boot button

// Stream Buffer
#define STREAM_SIZE     256
#define TRIGGER_LEVEL   16

StreamBufferHandle_t xISRStream = NULL;
StreamBufferHandle_t xDataStream = NULL;

TaskHandle_t xReceiverTask = NULL;
TaskHandle_t xSenderTask = NULL;

volatile uint32_t isrCounter = 0;
volatile uint32_t bytesFromISR = 0;

// Timer ISR - simulates periodic data arrival
hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer() {
    portENTER_CRITICAL_ISR(&timerMux);
    isrCounter++;
    portEXIT_CRITICAL_ISR(&timerMux);
    
    // Create data packet
    char data[16];
    int len = snprintf(data, sizeof(data), "ISR:%lu\n", isrCounter);
    
    // Send to stream buffer from ISR
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    size_t sent = xStreamBufferSendFromISR(
        xISRStream,
        data,
        len,
        &xHigherPriorityTaskWoken
    );
    
    if (sent > 0) {
        bytesFromISR += sent;
    }
    
    // Yield if higher priority task woken
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

// Button ISR
volatile uint32_t buttonPresses = 0;

void IRAM_ATTR onButton() {
    buttonPresses++;
    
    char msg[32];
    int len = snprintf(msg, sizeof(msg), "BTN:%lu\n", buttonPresses);
    
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xStreamBufferSendFromISR(xISRStream, msg, len, &xHigherPriorityTaskWoken);
    
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

void vReceiverTask(void *pvParameters) {
    Serial.println("[Receiver] Started - waiting for ISR data");
    
    char buffer[64];
    
    for (;;) {
        // Receive from ISR stream buffer
        size_t received = xStreamBufferReceive(
            xISRStream,
            buffer,
            sizeof(buffer) - 1,
            pdMS_TO_TICKS(1000)
        );
        
        if (received > 0) {
            buffer[received] = '\0';
            Serial.printf("[Receiver] Got %d bytes: %s", received, buffer);
        }
    }
}

void vSenderTask(void *pvParameters) {
    Serial.println("[Sender] Started - sending task-to-task data");
    
    char data[64];
    uint32_t seq = 0;
    
    for (;;) {
        int len = snprintf(data, sizeof(data),
                          "TASK:%lu,heap:%lu\n",
                          seq++, (unsigned long)ESP.getFreeHeap());
        
        // Send to data stream (task to task)
        size_t sent = xStreamBufferSend(
            xDataStream,
            data,
            len,
            pdMS_TO_TICKS(100)
        );
        
        if (sent < len) {
            Serial.println("[Sender] Buffer full!");
        }
        
        // LED blink
        digitalWrite(LED_PIN, HIGH);
        vTaskDelay(pdMS_TO_TICKS(50));
        digitalWrite(LED_PIN, LOW);
        
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void vMonitorTask(void *pvParameters) {
    Serial.println("[Monitor] Started");
    
    char buffer[128];
    
    for (;;) {
        Serial.println("\n┌─────────────────────────────────────────┐");
        Serial.println("│     STREAM BUFFER ISR STATISTICS        │");
        Serial.println("├─────────────────────────────────────────┤");
        Serial.printf("│ ISR Counter:      %8lu             │\n", isrCounter);
        Serial.printf("│ Bytes from ISR:   %8lu             │\n", bytesFromISR);
        Serial.printf("│ Button presses:   %8lu             │\n", buttonPresses);
        Serial.println("├─────────────────────────────────────────┤");
        Serial.printf("│ ISR Stream:                             │\n");
        Serial.printf("│   Bytes waiting: %4d                   │\n",
                     xStreamBufferBytesAvailable(xISRStream));
        Serial.printf("│   Space free:    %4d                   │\n",
                     xStreamBufferSpacesAvailable(xISRStream));
        Serial.printf("│ Data Stream:                            │\n");
        Serial.printf("│   Bytes waiting: %4d                   │\n",
                     xStreamBufferBytesAvailable(xDataStream));
        Serial.println("└─────────────────────────────────────────┘");
        
        // Read from data stream if available
        size_t avail = xStreamBufferBytesAvailable(xDataStream);
        if (avail > 0) {
            size_t read = xStreamBufferReceive(xDataStream, buffer, 
                                               sizeof(buffer) - 1, 0);
            if (read > 0) {
                buffer[read] = '\0';
                Serial.printf("[Monitor] Data stream: %s", buffer);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n═══════════════════════════════════════════════════════");
    Serial.println("       ESP32 STREAM BUFFER WITH ISR DEMONSTRATION");
    Serial.println("═══════════════════════════════════════════════════════\n");
    
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    
    // Create stream buffers
    xISRStream = xStreamBufferCreate(STREAM_SIZE, TRIGGER_LEVEL);
    xDataStream = xStreamBufferCreate(STREAM_SIZE, 1);
    
    if (xISRStream == NULL || xDataStream == NULL) {
        Serial.println("Failed to create stream buffers!");
        while (1);
    }
    
    Serial.println("Stream Buffers created:");
    Serial.printf("  ISR Stream: %d bytes, trigger=%d\n", STREAM_SIZE, TRIGGER_LEVEL);
    Serial.printf("  Data Stream: %d bytes\n", STREAM_SIZE);
    
    // Setup hardware timer (timer 0, divider 80 = 1MHz)
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 500000, true);  // 500ms
    timerAlarmEnable(timer);
    Serial.println("Timer ISR enabled (500ms interval)");
    
    // Setup button interrupt
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), onButton, FALLING);
    Serial.println("Button ISR enabled (GPIO0)");
    
    // Create tasks
    xTaskCreatePinnedToCore(vReceiverTask, "Receiver", 4096, NULL, 2, &xReceiverTask, 0);
    xTaskCreatePinnedToCore(vSenderTask, "Sender", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(vMonitorTask, "Monitor", 4096, NULL, 1, NULL, 0);
    
    Serial.println("\nPress BOOT button to generate button events\n");
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}

/**
 * KEY POINTS:
 * 
 * 1. xStreamBufferSendFromISR():
 *    - Safe to call from interrupt context
 *    - Non-blocking
 *    - Returns bytes actually sent
 * 
 * 2. Trigger Level:
 *    - Receiver unblocks when trigger level reached
 *    - Balance between latency and efficiency
 * 
 * 3. ISR Best Practices:
 *    - Keep ISR short
 *    - Use IRAM_ATTR for ESP32
 *    - Use FromISR variants of FreeRTOS functions
 *    - Check xHigherPriorityTaskWoken
 */
