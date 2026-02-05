/**
 * @file main.cpp
 * @brief Program 02: Stack Overflow Detection pada STM32
 * 
 * Deskripsi:
 * Demonstrasi mekanisme deteksi stack overflow di FreeRTOS.
 * Menunjukkan 2 metode deteksi dan hook function.
 * 
 * Metode Deteksi:
 * 1. Method 1: Quick check - hanya cek pattern di akhir stack
 * 2. Method 2: Comprehensive - cek seluruh watermark area
 * 
 * @author Praktikum Sistem Embedded
 * @date 2026
 */

#include <Arduino.h>
#include <STM32FreeRTOS.h>

// ==================== KONFIGURASI ====================
#define LED_PIN         PC13
#define SERIAL_BAUD     115200

// Stack sizes untuk testing
#define SAFE_STACK_SIZE     256
#define SMALL_STACK_SIZE    128
#define TINY_STACK_SIZE     64   // Sengaja kecil untuk trigger overflow

// ==================== VARIABEL GLOBAL ====================
TaskHandle_t xSafeTask = NULL;
TaskHandle_t xRiskyTask = NULL;
TaskHandle_t xMonitorTask = NULL;

volatile bool overflowDetected = false;
volatile char overflowTaskName[16] = {0};

// ==================== STACK OVERFLOW HOOK ====================
/**
 * @brief Hook function yang dipanggil saat stack overflow terdeteksi
 * 
 * PENTING: Function ini dipanggil dari ISR context!
 * - Jangan gunakan blocking calls
 * - Jangan gunakan printf/Serial.print langsung
 * - Simpan info dan handle di task lain
 * 
 * @param xTask Handle task yang overflow
 * @param pcTaskName Nama task
 */
extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    // Set flag untuk ditangani di task lain
    overflowDetected = true;
    
    // Copy task name (safe string copy)
    for (int i = 0; i < 15 && pcTaskName[i] != '\0'; i++) {
        overflowTaskName[i] = pcTaskName[i];
    }
    
    // Blink LED cepat sebagai indikator darurat
    // (Direct register access untuk safety di ISR)
    GPIOC->BSRR = GPIO_BSRR_BR13;  // LED ON
    for (volatile int i = 0; i < 100000; i++) {}
    GPIOC->BSRR = GPIO_BSRR_BS13;  // LED OFF
}

/**
 * @brief Malloc failed hook
 */
extern "C" void vApplicationMallocFailedHook(void) {
    Serial.println("\n[CRITICAL] MALLOC FAILED!");
    while (1) {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        delay(100);
    }
}

// ==================== HELPER FUNCTIONS ====================

/**
 * @brief Menampilkan stack usage info
 */
void printStackInfo(const char* taskName, TaskHandle_t task) {
    if (task != NULL) {
        UBaseType_t hwm = uxTaskGetStackHighWaterMark(task);
        Serial.printf("  %s: High Water Mark = %u words (%u bytes free)\n",
                     taskName, hwm, hwm * 4);
        
        // Warning jika stack rendah
        if (hwm < 32) {
            Serial.printf("  ⚠️  WARNING: %s stack critically low!\n", taskName);
        } else if (hwm < 64) {
            Serial.printf("  ⚡ CAUTION: %s stack getting low\n", taskName);
        }
    }
}

/**
 * @brief Fungsi rekursif untuk simulasi penggunaan stack
 */
void recursiveFunction(int depth, int maxDepth) {
    // Alokasi di stack - gunakan volatile agar tidak dioptimize
    volatile char localBuffer[32];
    memset((void*)localBuffer, depth & 0xFF, 32);
    
    if (depth < maxDepth) {
        recursiveFunction(depth + 1, maxDepth);
    }
    
    // Prevent optimization
    if (localBuffer[0] == 0xFF) {
        Serial.print("X");
    }
}

// ==================== TASKS ====================

/**
 * @brief Task dengan stack aman - operasi normal
 */
void vSafeTask(void *pvParameters) {
    Serial.println("[SafeTask] Started with adequate stack");
    
    char buffer[64];  // Alokasi lokal yang aman
    int counter = 0;
    
    for (;;) {
        // Operasi normal dengan buffer lokal
        snprintf(buffer, sizeof(buffer), "Safe counter: %d", counter++);
        
        // Simulate some work
        vTaskDelay(pdMS_TO_TICKS(2000));
        
        // Print status tanpa overflow
        Serial.println("[SafeTask] Operating normally");
        Serial.printf("[SafeTask] %s\n", buffer);
        
        // Report stack usage
        UBaseType_t hwm = uxTaskGetStackHighWaterMark(NULL);
        Serial.printf("[SafeTask] Stack HWM: %u words\n", hwm);
    }
}

/**
 * @brief Task yang deliberately menggunakan banyak stack
 * 
 * PERINGATAN: Task ini akan menyebabkan stack overflow
 * setelah beberapa iterasi!
 */
void vRiskyTask(void *pvParameters) {
    Serial.println("[RiskyTask] Started - will attempt to use excessive stack");
    
    int iteration = 0;
    
    vTaskDelay(pdMS_TO_TICKS(3000));  // Let safe task run first
    
    for (;;) {
        iteration++;
        Serial.printf("\n[RiskyTask] Iteration %d - attempting deep recursion...\n", 
                     iteration);
        
        // Progressively deeper recursion - will eventually overflow
        int depth = iteration * 2;  // Semakin dalam tiap iterasi
        
        Serial.printf("[RiskyTask] Recursion depth: %d\n", depth);
        
        // Report before attempting
        UBaseType_t hwmBefore = uxTaskGetStackHighWaterMark(NULL);
        Serial.printf("[RiskyTask] Stack HWM before: %u words\n", hwmBefore);
        
        // Ini akan menyebabkan overflow ketika depth cukup besar
        recursiveFunction(0, depth);
        
        // Jika sampai sini, belum overflow
        Serial.println("[RiskyTask] Survived! Waiting before next attempt...");
        
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

/**
 * @brief Monitor task - memantau status dan handle overflow
 */
void vMonitorTask(void *pvParameters) {
    Serial.println("[Monitor] Task started");
    
    for (;;) {
        Serial.println("\n╔════════════════════════════════════════════╗");
        Serial.println("║     STACK OVERFLOW DETECTION MONITOR       ║");
        Serial.println("╠════════════════════════════════════════════╣");
        
        // Check overflow flag
        if (overflowDetected) {
            Serial.println("║ 🔴 STACK OVERFLOW DETECTED!                ║");
            Serial.printf("║    Task: %-32s ║\n", (char*)overflowTaskName);
            Serial.println("║                                            ║");
            Serial.println("║    Action: System should be reset!         ║");
            Serial.println("╚════════════════════════════════════════════╝");
            
            // Blink LED pattern untuk overflow
            for (int i = 0; i < 20; i++) {
                digitalWrite(LED_PIN, LOW);  // ON
                vTaskDelay(pdMS_TO_TICKS(50));
                digitalWrite(LED_PIN, HIGH); // OFF
                vTaskDelay(pdMS_TO_TICKS(50));
            }
            
            // Reset flag
            overflowDetected = false;
        } else {
            Serial.println("║ ✓ System status: NORMAL                   ║");
        }
        
        Serial.println("╠════════════════════════════════════════════╣");
        Serial.println("║ Task Stack Status:                         ║");
        
        printStackInfo("SafeTask", xSafeTask);
        printStackInfo("RiskyTask", xRiskyTask);
        printStackInfo("Monitor", xMonitorTask);
        
        Serial.println("╚════════════════════════════════════════════╝");
        
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// ==================== SETUP & LOOP ====================

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(2000);
    
    Serial.println("\n════════════════════════════════════════════════════════════");
    Serial.println("        MODUL 12: STACK OVERFLOW DETECTION (STM32)          ");
    Serial.println("════════════════════════════════════════════════════════════");
    Serial.println("\nKonfigurasi:");
    Serial.println("  configCHECK_FOR_STACK_OVERFLOW = 2 (comprehensive check)");
    Serial.println("  configUSE_MALLOC_FAILED_HOOK = 1");
    Serial.println("  configRECORD_STACK_HIGH_WATER_MARK = 1");
    Serial.println("");
    Serial.println("Stack Overflow Detection Methods:");
    Serial.println("  Method 1: Quick check at context switch");
    Serial.println("  Method 2: Fill pattern verification");
    Serial.println("════════════════════════════════════════════════════════════\n");
    
    // Initialize LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);  // OFF
    
    // Create tasks with different stack sizes
    xTaskCreate(
        vSafeTask,
        "SafeTask",
        SAFE_STACK_SIZE,  // Adequate stack
        NULL,
        1,
        &xSafeTask
    );
    
    xTaskCreate(
        vRiskyTask,
        "RiskyTask",
        TINY_STACK_SIZE,  // Very small stack - will overflow!
        NULL,
        1,
        &xRiskyTask
    );
    
    xTaskCreate(
        vMonitorTask,
        "Monitor",
        256,
        NULL,
        2,  // Higher priority
        &xMonitorTask
    );
    
    Serial.println("Tasks created:");
    Serial.printf("  SafeTask:  %d words stack\n", SAFE_STACK_SIZE);
    Serial.printf("  RiskyTask: %d words stack (deliberately small!)\n", TINY_STACK_SIZE);
    Serial.printf("  Monitor:   256 words stack\n");
    Serial.println("\n⚠️  RiskyTask WILL cause stack overflow - this is intentional!");
    Serial.println("Watch for overflow detection in action...\n");
    
    // Start scheduler
    vTaskStartScheduler();
    
    // Should never reach here
    Serial.println("[ERROR] Scheduler failed to start!");
}

void loop() {
    // Empty - FreeRTOS handles everything
}

/**
 * PENJELASAN:
 * 
 * 1. Stack Overflow Detection di FreeRTOS:
 *    - configCHECK_FOR_STACK_OVERFLOW=1: Quick check saat context switch
 *    - configCHECK_FOR_STACK_OVERFLOW=2: Check + fill pattern verification
 * 
 * 2. vApplicationStackOverflowHook():
 *    - Dipanggil oleh kernel saat overflow terdeteksi
 *    - Berjalan di ISR context - hindari blocking calls!
 *    - Biasanya digunakan untuk logging/reset
 * 
 * 3. High Water Mark:
 *    - uxTaskGetStackHighWaterMark() mengembalikan minimum free stack
 *    - Berguna untuk optimasi ukuran stack
 * 
 * 4. Best Practices:
 *    - Selalu test dengan configCHECK_FOR_STACK_OVERFLOW=2 saat development
 *    - Monitor HWM untuk fine-tune stack sizes
 *    - Tambahkan margin 20-30% di production
 * 
 * EXPECTED OUTPUT:
 * 
 * [SafeTask] Operating normally
 * [SafeTask] Stack HWM: 180 words
 * 
 * [RiskyTask] Iteration 3 - attempting deep recursion...
 * [RiskyTask] Recursion depth: 6
 * 🔴 STACK OVERFLOW DETECTED!
 *    Task: RiskyTask
 */
