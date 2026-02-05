/**
 * @file main.cpp
 * @brief Program 04: Event Groups - System Initialization - STM32F103C8T6
 * 
 * Deskripsi:
 * Demonstrasi penggunaan Event Groups untuk sinkronisasi
 * inisialisasi sistem multi-task.
 * 
 * Skenario:
 * - Main task menunggu semua subsystem ready
 * - Sensor task melakukan init dan set event bit
 * - Communication task melakukan init dan set event bit
 * - Storage task melakukan init dan set event bit
 * - Main task mulai operasi setelah semua ready
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include <event_groups.h>

// ==================== EVENT BITS DEFINITION ====================
#define EVENT_SENSOR_READY      (1 << 0)    // Bit 0: Sensor initialized
#define EVENT_COMM_READY        (1 << 1)    // Bit 1: Communication ready
#define EVENT_STORAGE_READY     (1 << 2)    // Bit 2: Storage ready
#define EVENT_USER_INPUT        (1 << 3)    // Bit 3: User input detected
#define EVENT_ALARM_ACTIVE      (1 << 4)    // Bit 4: Alarm condition
#define EVENT_LOW_BATTERY       (1 << 5)    // Bit 5: Low battery warning
#define EVENT_DATA_PENDING      (1 << 6)    // Bit 6: Data ready to process
#define EVENT_ERROR_OCCURRED    (1 << 7)    // Bit 7: Error flag

// Combined events for system ready
#define EVENT_ALL_SUBSYSTEMS    (EVENT_SENSOR_READY | EVENT_COMM_READY | EVENT_STORAGE_READY)

// ==================== KONFIGURASI ====================
#define LED_PIN             PC13
#define BUTTON_PIN          PA0
#define SERIAL_BAUD         115200

// ==================== VARIABEL GLOBAL ====================
EventGroupHandle_t xSystemEvents = NULL;

// ==================== FUNGSI HELPER ====================

/**
 * @brief Mencetak status event bits
 */
void printEventBits(EventBits_t bits) {
    Serial.print("Event Bits: 0b");
    for (int i = 7; i >= 0; i--) {
        Serial.print((bits >> i) & 1);
    }
    Serial.printf(" (0x%02X)\n", bits);
    
    Serial.println("  Status:");
    if (bits & EVENT_SENSOR_READY)   Serial.println("    ✓ Sensor Ready");
    if (bits & EVENT_COMM_READY)     Serial.println("    ✓ Communication Ready");
    if (bits & EVENT_STORAGE_READY)  Serial.println("    ✓ Storage Ready");
    if (bits & EVENT_USER_INPUT)     Serial.println("    ✓ User Input");
    if (bits & EVENT_ALARM_ACTIVE)   Serial.println("    ! Alarm Active");
    if (bits & EVENT_LOW_BATTERY)    Serial.println("    ⚡ Low Battery");
    if (bits & EVENT_DATA_PENDING)   Serial.println("    📊 Data Pending");
    if (bits & EVENT_ERROR_OCCURRED) Serial.println("    ❌ Error Occurred");
}

// ==================== TASKS ====================

/**
 * @brief Sensor initialization task
 */
void vSensorTask(void *pvParameters) {
    Serial.println("[Sensor] Starting initialization...");
    
    // Simulate sensor initialization (random delay 1-3 seconds)
    uint32_t initTime = 1000 + (rand() % 2000);
    vTaskDelay(pdMS_TO_TICKS(initTime));
    
    // Simulate sensor self-test
    Serial.println("[Sensor] Running self-test...");
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Initialization complete - set event bit
    Serial.printf("[Sensor] Initialization complete after %lu ms\n", initTime + 500);
    xEventGroupSetBits(xSystemEvents, EVENT_SENSOR_READY);
    
    // Continue with normal operation
    for (;;) {
        // Simulate sensor reading
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // Occasionally generate data
        if (rand() % 5 == 0) {
            Serial.println("[Sensor] New data available");
            xEventGroupSetBits(xSystemEvents, EVENT_DATA_PENDING);
        }
    }
}

/**
 * @brief Communication initialization task
 */
void vCommTask(void *pvParameters) {
    Serial.println("[Comm] Starting initialization...");
    
    // Simulate connection establishment
    uint32_t initTime = 1500 + (rand() % 2500);
    
    Serial.println("[Comm] Connecting to network...");
    vTaskDelay(pdMS_TO_TICKS(initTime));
    
    // Initialization complete
    Serial.printf("[Comm] Connected after %lu ms\n", initTime);
    xEventGroupSetBits(xSystemEvents, EVENT_COMM_READY);
    
    // Continue with normal operation
    for (;;) {
        // Wait for data pending event
        EventBits_t bits = xEventGroupWaitBits(
            xSystemEvents,
            EVENT_DATA_PENDING,
            pdTRUE,             // Clear bit on exit
            pdFALSE,            // Wait for any
            pdMS_TO_TICKS(5000) // 5 second timeout
        );
        
        if (bits & EVENT_DATA_PENDING) {
            Serial.println("[Comm] Sending data...");
            vTaskDelay(pdMS_TO_TICKS(200));
            Serial.println("[Comm] Data sent successfully");
        }
    }
}

/**
 * @brief Storage initialization task
 */
void vStorageTask(void *pvParameters) {
    Serial.println("[Storage] Starting initialization...");
    
    // Simulate storage initialization
    Serial.println("[Storage] Checking filesystem...");
    vTaskDelay(pdMS_TO_TICKS(800));
    
    Serial.println("[Storage] Mounting storage...");
    vTaskDelay(pdMS_TO_TICKS(1200));
    
    // Initialization complete
    Serial.println("[Storage] Initialization complete");
    xEventGroupSetBits(xSystemEvents, EVENT_STORAGE_READY);
    
    // Continue with normal operation
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

/**
 * @brief Main system task - waits for all subsystems
 */
void vMainTask(void *pvParameters) {
    Serial.println("[Main] Waiting for all subsystems...");
    Serial.println("[Main] Required: Sensor + Comm + Storage");
    
    uint32_t startTime = millis();
    
    // Wait for ALL subsystems to be ready
    EventBits_t bits = xEventGroupWaitBits(
        xSystemEvents,
        EVENT_ALL_SUBSYSTEMS,   // Wait for these bits
        pdFALSE,                // Don't clear on exit
        pdTRUE,                 // Wait for ALL bits (AND logic)
        pdMS_TO_TICKS(30000)    // 30 second timeout
    );
    
    uint32_t waitTime = millis() - startTime;
    
    Serial.println("\n╔══════════════════════════════════════════╗");
    
    if ((bits & EVENT_ALL_SUBSYSTEMS) == EVENT_ALL_SUBSYSTEMS) {
        Serial.println("║   ✓ SYSTEM INITIALIZATION COMPLETE!     ║");
        Serial.printf("║   Total init time: %lu ms              ║\n", waitTime);
        
        // LED solid on to indicate ready
        digitalWrite(LED_PIN, LOW);  // LED on (active low)
    } else {
        Serial.println("║   ❌ SYSTEM INITIALIZATION FAILED!       ║");
        Serial.println("║   Missing subsystems:                    ║");
        
        if (!(bits & EVENT_SENSOR_READY))  Serial.println("║     - Sensor not ready                  ║");
        if (!(bits & EVENT_COMM_READY))    Serial.println("║     - Communication not ready           ║");
        if (!(bits & EVENT_STORAGE_READY)) Serial.println("║     - Storage not ready                 ║");
    }
    
    Serial.println("╠══════════════════════════════════════════╣");
    Serial.println("║   Current Event Status:                  ║");
    Serial.println("╚══════════════════════════════════════════╝");
    
    printEventBits(bits);
    
    // Continue with main operation
    Serial.println("\n[Main] System operational - entering main loop");
    
    for (;;) {
        // Monitor system events
        bits = xEventGroupGetBits(xSystemEvents);
        
        // Check for alarm conditions
        if (bits & EVENT_ALARM_ACTIVE) {
            Serial.println("[Main] !!! ALARM DETECTED !!!");
            xEventGroupClearBits(xSystemEvents, EVENT_ALARM_ACTIVE);
            
            // Blink LED rapidly
            for (int i = 0; i < 10; i++) {
                digitalWrite(LED_PIN, !digitalRead(LED_PIN));
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        }
        
        // Check for errors
        if (bits & EVENT_ERROR_OCCURRED) {
            Serial.println("[Main] ERROR condition detected!");
            xEventGroupClearBits(xSystemEvents, EVENT_ERROR_OCCURRED);
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
 * @brief Button monitoring task (simulates user input and alarm)
 */
void vButtonTask(void *pvParameters) {
    bool lastButtonState = HIGH;
    uint32_t pressCount = 0;
    
    for (;;) {
        bool buttonState = digitalRead(BUTTON_PIN);
        
        if (buttonState == LOW && lastButtonState == HIGH) {
            // Button pressed
            pressCount++;
            Serial.printf("[Button] Press detected (#%lu)\n", pressCount);
            
            // Set user input event
            xEventGroupSetBits(xSystemEvents, EVENT_USER_INPUT);
            
            // Every 3rd press, trigger alarm
            if (pressCount % 3 == 0) {
                Serial.println("[Button] Triggering ALARM!");
                xEventGroupSetBits(xSystemEvents, EVENT_ALARM_ACTIVE);
            }
            
            // Every 5th press, trigger error
            if (pressCount % 5 == 0) {
                Serial.println("[Button] Triggering ERROR!");
                xEventGroupSetBits(xSystemEvents, EVENT_ERROR_OCCURRED);
            }
            
            vTaskDelay(pdMS_TO_TICKS(200));  // Debounce
        }
        
        lastButtonState = buttonState;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// ==================== HOOK FUNCTIONS ====================

extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    Serial.printf("\n!!! STACK OVERFLOW in task: %s !!!\n", pcTaskName);
    while (1);
}

// ==================== SETUP & LOOP ====================

void setup() {
    Serial.begin(SERIAL_BAUD);
    while (!Serial && millis() < 3000);
    
    Serial.println("\n════════════════════════════════════════════");
    Serial.println("   MODUL 12: EVENT GROUPS - SYSTEM INIT    ");
    Serial.println("   STM32F103C8T6 - FreeRTOS               ");
    Serial.println("════════════════════════════════════════════\n");
    
    // Initialize pins
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    digitalWrite(LED_PIN, HIGH);  // LED off
    
    // Seed random number generator
    srand(analogRead(PA0));
    
    // Create Event Group
    xSystemEvents = xEventGroupCreate();
    if (xSystemEvents == NULL) {
        Serial.println("ERROR: Failed to create event group!");
        while (1);
    }
    Serial.println("Event Group created successfully");
    
    // Print initial event status
    printEventBits(xEventGroupGetBits(xSystemEvents));
    
    Serial.println("\nCreating tasks...");
    
    // Create tasks
    xTaskCreate(vSensorTask, "Sensor", 192, NULL, 2, NULL);
    xTaskCreate(vCommTask, "Comm", 192, NULL, 2, NULL);
    xTaskCreate(vStorageTask, "Storage", 192, NULL, 2, NULL);
    xTaskCreate(vMainTask, "Main", 256, NULL, 3, NULL);
    xTaskCreate(vButtonTask, "Button", 128, NULL, 1, NULL);
    
    Serial.println("All tasks created. Starting scheduler...\n");
    Serial.println("════════════════════════════════════════════");
    Serial.println("Press button to generate events:");
    Serial.println("  1st press: User Input event");
    Serial.println("  3rd press: Alarm event");
    Serial.println("  5th press: Error event");
    Serial.println("════════════════════════════════════════════\n");
    
    vTaskStartScheduler();
    
    Serial.println("ERROR: Scheduler failed!");
    while (1);
}

void loop() {
    // Not used with FreeRTOS
}
