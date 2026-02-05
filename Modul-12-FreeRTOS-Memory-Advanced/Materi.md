# BAB 12: FreeRTOS Memory Management & Advanced Features

## 🎯 Capaian Pembelajaran

Setelah menyelesaikan bab ini, mahasiswa diharapkan mampu:

1. **Memahami Memory Management di FreeRTOS** - Heap allocation schemes (heap_1 hingga heap_5)
2. **Mengimplementasikan Stack Overflow Detection** - Metode checking dan hook functions
3. **Menerapkan Event Groups** - Sinkronisasi multi-task menggunakan event bits
4. **Menggunakan Stream Buffer dan Message Buffer** - Komunikasi data streaming
5. **Melakukan Memory Debugging dan Optimization** - Teknik analisis dan optimasi memori
6. **Menerapkan Low Power Features** - Tickless idle mode dan sleep management
7. **Mengimplementasikan Static Allocation** - Task dan resource tanpa dynamic heap

---

## 📚 Materi Pembelajaran

### 12.1 Pendahuluan Memory Management FreeRTOS

Memory management adalah aspek kritis dalam embedded systems karena sumber daya memori yang terbatas. FreeRTOS menyediakan beberapa skema alokasi memori yang dapat dipilih sesuai kebutuhan aplikasi.

#### Mengapa Memory Management Penting?

```
┌─────────────────────────────────────────────────────────────┐
│                    MEMORY CONSTRAINTS                        │
├─────────────────────────────────────────────────────────────┤
│  Microcontroller      │  RAM       │  Flash     │  Notes    │
├───────────────────────┼────────────┼────────────┼───────────┤
│  STM32F103C8T6        │  20 KB     │  64 KB     │  Blue Pill│
│  ESP32-WROOM-32       │  520 KB    │  4 MB      │  DevKit   │
│  STM32F411            │  128 KB    │  512 KB    │  Black Pill│
│  ESP32-S3             │  512 KB    │  8 MB      │  + PSRAM  │
└─────────────────────────────────────────────────────────────┘
```

#### Memory Layout Microcontroller

```
┌─────────────────────────────────────────────────────────────┐
│                    MEMORY MAP STM32                          │
├─────────────────────────────────────────────────────────────┤
│  0x0800 0000  ┌──────────────────────┐  Flash Start         │
│               │                      │                       │
│               │     CODE (Text)      │  ← Program Code       │
│               │                      │                       │
│               ├──────────────────────┤                       │
│               │   Const Data (.rodata)│ ← String literals    │
│  0x0800 FFFF  └──────────────────────┘  Flash End (64KB)    │
│                                                              │
│  0x2000 0000  ┌──────────────────────┐  RAM Start           │
│               │   .data (initialized) │                      │
│               ├──────────────────────┤                       │
│               │   .bss (zero-init)   │                       │
│               ├──────────────────────┤                       │
│               │        HEAP          │  ← FreeRTOS Heap      │
│               │       ↓ grows        │                       │
│               │                      │                       │
│               │       ↑ grows        │                       │
│               │        STACK         │  ← MSP (Main Stack)   │
│  0x2000 4FFF  └──────────────────────┘  RAM End (20KB)      │
└─────────────────────────────────────────────────────────────┘
```

### 12.2 FreeRTOS Heap Allocation Schemes

FreeRTOS menyediakan 5 skema alokasi heap yang berbeda. Pemilihan scheme bergantung pada kebutuhan aplikasi.

#### 12.2.1 Heap_1 - Simplest, No Free

```c
/**
 * heap_1.c Characteristics:
 * - Paling sederhana dan deterministic
 * - TIDAK mendukung vPortFree() - memory tidak bisa dibebaskan
 * - Cocok untuk aplikasi yang membuat semua task/queue saat startup
 * - Tidak ada fragmentasi
 * - Thread safe
 * - Paling kecil ukuran code
 */

// Konfigurasi di FreeRTOSConfig.h
#define configTOTAL_HEAP_SIZE    ((size_t)(10 * 1024))  // 10KB heap

// Memory layout dengan heap_1
//
// ┌─────────────────────────────────────┐
// │           HEAP AREA                 │
// ├─────────────────────────────────────┤
// │  Allocation 1  │  Allocation 2  │   │
// │    (Task A)    │   (Queue B)    │...│
// ├────────────────┴────────────────┼───┤
// │         USED MEMORY             │FREE│
// └─────────────────────────────────────┘
//          ↑ Never freed

// Use case: Sistem dimana semua resources dibuat saat startup
void systemInit(void) {
    // Semua task, queue, semaphore dibuat disini
    xTaskCreate(taskA, "TaskA", 256, NULL, 1, NULL);
    xTaskCreate(taskB, "TaskB", 256, NULL, 2, NULL);
    xQueueCreate(10, sizeof(Data_t));
    // Setelah ini, tidak ada alokasi lagi
}
```

#### 12.2.2 Heap_2 - Best Fit, No Coalescence

```c
/**
 * heap_2.c Characteristics:
 * - Mendukung pvPortMalloc() dan vPortFree()
 * - Menggunakan best fit algorithm
 * - TIDAK menggabungkan adjacent free blocks
 * - Dapat menyebabkan fragmentasi
 * - Cocok untuk alokasi/dealokasi dengan ukuran sama
 */

// Best fit algorithm visualization
//
// Free List (linked list of free blocks):
// [32B] → [64B] → [128B] → [48B] → NULL
//
// Request: 50 bytes
// Best fit: [64B] block (smallest block that fits)
//
// Setelah alokasi:
// [32B] → [14B leftover] → [128B] → [48B] → NULL

// Use case: Task yang create/delete dengan ukuran sama
void temporaryTask(void *pvParameters) {
    // Alokasi buffer tetap
    uint8_t *buffer = pvPortMalloc(128);
    
    // Process...
    processData(buffer);
    
    // Free dengan ukuran sama - tidak fragmentasi
    vPortFree(buffer);
    vTaskDelete(NULL);
}
```

#### 12.2.3 Heap_3 - Wrapped Standard Library

```c
/**
 * heap_3.c Characteristics:
 * - Wrapper untuk standard malloc()/free()
 * - Heap size ditentukan oleh linker, bukan configTOTAL_HEAP_SIZE
 * - Thread safety via suspending scheduler
 * - Bergantung pada implementasi library
 * - Cocok jika sudah menggunakan malloc di code lain
 */

// Implementasi internal heap_3
void *pvPortMalloc(size_t xWantedSize) {
    void *pvReturn;
    
    vTaskSuspendAll();  // Thread safety - suspend scheduler
    {
        pvReturn = malloc(xWantedSize);
    }
    xTaskResumeAll();
    
    #if configUSE_MALLOC_FAILED_HOOK == 1
    if (pvReturn == NULL) {
        vApplicationMallocFailedHook();
    }
    #endif
    
    return pvReturn;
}

void vPortFree(void *pv) {
    if (pv != NULL) {
        vTaskSuspendAll();
        {
            free(pv);
        }
        xTaskResumeAll();
    }
}
```

#### 12.2.4 Heap_4 - First Fit with Coalescence (RECOMMENDED)

```c
/**
 * heap_4.c Characteristics:
 * - Mendukung pvPortMalloc() dan vPortFree()
 * - Menggunakan first fit algorithm
 * - MENGGABUNGKAN adjacent free blocks (coalescence)
 * - Mengurangi fragmentasi
 * - Paling umum digunakan - RECOMMENDED
 * - Deterministic time untuk allocation
 */

// Coalescence process visualization
//
// BEFORE FREE:
// ┌──────────┬──────────┬──────────┐
// │ Used 64B │ Used 32B │ Used 64B │
// └──────────┴──────────┴──────────┘
//
// AFTER FREE middle block:
// ┌──────────┬──────────┬──────────┐
// │ Used 64B │ FREE 32B │ Used 64B │
// └──────────┴──────────┴──────────┘
//
// AFTER FREE first block (coalescence terjadi):
// ┌─────────────────────┬──────────┐
// │ FREE 96B (merged)   │ Used 64B │
// └─────────────────────┴──────────┘

// Konfigurasi
#define configTOTAL_HEAP_SIZE    ((size_t)(15 * 1024))

// Contoh penggunaan
void *allocateBuffer(size_t size) {
    void *ptr = pvPortMalloc(size);
    if (ptr == NULL) {
        // Handle allocation failure
        printf("Malloc failed! Free heap: %u\n", xPortGetFreeHeapSize());
    }
    return ptr;
}
```

#### 12.2.5 Heap_5 - Multiple Non-Contiguous Regions

```c
/**
 * heap_5.c Characteristics:
 * - Dapat mengelola multiple memory regions
 * - Berguna untuk MCU dengan RAM terpisah (internal + external)
 * - WAJIB diinisialisasi sebelum digunakan (sebelum scheduler start)
 * - Mendukung coalescence seperti heap_4
 */

// Contoh: STM32F4 dengan CCM RAM dan SRAM
// SRAM: 0x20000000, 128KB - general purpose
// CCM:  0x10000000, 64KB  - core-coupled, no DMA

// Definisi regions
HeapRegion_t xHeapRegions[] = {
    { (uint8_t *)0x20000000, 0x20000 },  // 128KB SRAM
    { (uint8_t *)0x10000000, 0x10000 },  // 64KB CCM RAM
    { NULL, 0 }                           // Terminator - WAJIB
};

// Inisialisasi SEBELUM scheduler start
int main(void) {
    // 1. Hardware init
    SystemInit();
    
    // 2. WAJIB: Define heap regions SEBELUM kernel/task API
    vPortDefineHeapRegions(xHeapRegions);
    
    // 3. Sekarang bisa create tasks, queues, dll
    xTaskCreate(myTask, "Task", 256, NULL, 1, NULL);
    
    // 4. Start scheduler
    vTaskStartScheduler();
    
    while(1);
}

// Contoh ESP32 dengan PSRAM
// Internal: ~320KB, PSRAM: 4MB (jika tersedia)
#ifdef CONFIG_SPIRAM_SUPPORT
HeapRegion_t xHeapRegions[] = {
    { (uint8_t *)0x3FFB0000, 0x50000 },  // Internal RAM
    { (uint8_t *)0x3F800000, 0x400000 }, // PSRAM 4MB
    { NULL, 0 }
};
#endif
```

### 12.3 Perbandingan Heap Schemes

```
┌──────────┬───────────┬───────────┬──────────────┬─────────────────────────┐
│  Scheme  │   Free    │ Coalesc.  │ Determinism  │  Use Case               │
├──────────┼───────────┼───────────┼──────────────┼─────────────────────────┤
│  heap_1  │    No     │    N/A    │   Highest    │ Static only, safety-    │
│          │           │           │              │ critical systems        │
├──────────┼───────────┼───────────┼──────────────┼─────────────────────────┤
│  heap_2  │    Yes    │    No     │   Medium     │ Same-size alloc/free,   │
│          │           │           │              │ audio buffers           │
├──────────┼───────────┼───────────┼──────────────┼─────────────────────────┤
│  heap_3  │    Yes    │  Library  │   Lowest     │ Mixed FreeRTOS +        │
│          │           │           │              │ standard malloc code    │
├──────────┼───────────┼───────────┼──────────────┼─────────────────────────┤
│  heap_4  │    Yes    │    Yes    │   Medium     │ General purpose,        │
│          │           │           │              │ RECOMMENDED             │
├──────────┼───────────┼───────────┼──────────────┼─────────────────────────┤
│  heap_5  │    Yes    │    Yes    │   Medium     │ Multi-region RAM,       │
│          │           │           │              │ external memory         │
└──────────┴───────────┴───────────┴──────────────┴─────────────────────────┘
```

### 12.4 Stack Overflow Detection

Stack overflow adalah salah satu bug paling berbahaya dalam embedded systems karena dapat menyebabkan perilaku tidak terduga dan sulit di-debug.

#### 12.4.1 Metode 1 - Context Switch Checking

```c
// FreeRTOSConfig.h
#define configCHECK_FOR_STACK_OVERFLOW  1

/**
 * Metode 1: Check saat context switch
 * - Memeriksa stack pointer masih dalam batas
 * - Ringan (low overhead)
 * - Bisa miss overflow antara context switch
 */

// Hook function - WAJIB diimplementasikan
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    // Task handle dan nama task yang overflow
    printf("!!! STACK OVERFLOW in task: %s !!!\n", pcTaskName);
    
    // Log informasi untuk debugging
    #ifdef DEBUG
        // Simpan ke non-volatile memory
        logError(ERROR_STACK_OVERFLOW, pcTaskName);
    #endif
    
    // Opsi 1: System reset (production)
    NVIC_SystemReset();
    
    // Opsi 2: Halt untuk debugging (development)
    // while(1) { __BKPT(0); }
}
```

#### 12.4.2 Metode 2 - Pattern Checking (RECOMMENDED)

```c
// FreeRTOSConfig.h
#define configCHECK_FOR_STACK_OVERFLOW  2

/**
 * Metode 2: Pattern checking
 * - Mengisi 20 bytes terakhir stack dengan pattern 0xA5
 * - Check pattern saat context switch
 * - Lebih reliable dari metode 1
 * - Sedikit lebih berat (check 20 bytes)
 */

// Visualisasi stack dengan pattern
//
// ┌─────────────────────────────────────────────────┐
// │                    STACK AREA                    │
// ├─────────────────────────────────────────────────┤
// │ Stack Top (High Address)                         │
// │ ┌─────────────────────────────────────────────┐ │
// │ │  Return addresses, local variables, etc.    │ │
// │ │         Active Stack Data                   │ │
// │ │              ↓ grows downward               │ │
// │ │                                             │ │
// │ ├─────────────────────────────────────────────┤ │
// │ │  0xA5 0xA5 0xA5 0xA5 ... (20 bytes)        │ │
// │ │  [Canary Zone - jika rusak = overflow]     │ │
// │ └─────────────────────────────────────────────┘ │
// │ Stack Bottom (Low Address)                       │
// └─────────────────────────────────────────────────┘

// Hook implementation dengan more info
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    // Disable interrupts
    taskDISABLE_INTERRUPTS();
    
    // Get task info
    TaskStatus_t xTaskStatus;
    vTaskGetInfo(xTask, &xTaskStatus, pdTRUE, eInvalid);
    
    printf("\n========= STACK OVERFLOW DETECTED =========\n");
    printf("Task Name: %s\n", pcTaskName);
    printf("Task Priority: %u\n", xTaskStatus.uxCurrentPriority);
    printf("Stack Base: 0x%08X\n", (unsigned int)xTaskStatus.pxStackBase);
    printf("High Water Mark: %u words\n", xTaskStatus.usStackHighWaterMark);
    printf("==========================================\n");
    
    // Blink LED untuk visual indication
    while(1) {
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
        for(volatile int i = 0; i < 1000000; i++);
    }
}
```

### 12.5 Memory Statistics dan Debugging

#### 12.5.1 Heap Statistics API

```c
// FreeRTOSConfig.h requirements
#define configUSE_TRACE_FACILITY    1

// Mendapatkan informasi heap detail
void printHeapStats(void) {
    HeapStats_t xHeapStats;
    vPortGetHeapStats(&xHeapStats);
    
    printf("\n========== HEAP STATISTICS ==========\n");
    printf("Total Heap Size:      %u bytes\n", configTOTAL_HEAP_SIZE);
    printf("Available Heap:       %u bytes (%.1f%%)\n", 
           xHeapStats.xAvailableHeapSpaceInBytes,
           (float)xHeapStats.xAvailableHeapSpaceInBytes / configTOTAL_HEAP_SIZE * 100);
    printf("Minimum Ever Free:    %u bytes\n", xHeapStats.xMinimumEverFreeBytesRemaining);
    printf("Largest Free Block:   %u bytes\n", xHeapStats.xSizeOfLargestFreeBlockInBytes);
    printf("Smallest Free Block:  %u bytes\n", xHeapStats.xSizeOfSmallestFreeBlockInBytes);
    printf("Number Free Blocks:   %u\n", xHeapStats.xNumberOfFreeBlocks);
    printf("Successful Allocs:    %u\n", xHeapStats.xNumberOfSuccessfulAllocations);
    printf("Successful Frees:     %u\n", xHeapStats.xNumberOfSuccessfulFrees);
    printf("=====================================\n");
    
    // Fragmentasi indicator
    if (xHeapStats.xNumberOfFreeBlocks > 5 && 
        xHeapStats.xSizeOfLargestFreeBlockInBytes < xHeapStats.xAvailableHeapSpaceInBytes / 2) {
        printf("WARNING: Heap fragmentation detected!\n");
    }
}

// Quick heap check
void checkHeap(void) {
    size_t freeHeap = xPortGetFreeHeapSize();
    size_t minEver = xPortGetMinimumEverFreeHeapSize();
    
    printf("Free: %u, Min Ever: %u bytes\n", freeHeap, minEver);
    
    // Warning threshold
    if (freeHeap < 1024) {
        printf("CRITICAL: Low heap space!\n");
    }
}
```

#### 12.5.2 Task Stack High Water Mark

```c
// Memeriksa penggunaan stack setiap task
void printTaskStackUsage(void) {
    UBaseType_t uxArraySize = uxTaskGetNumberOfTasks();
    TaskStatus_t *pxTaskStatusArray;
    
    // Alokasi array untuk task info
    pxTaskStatusArray = pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));
    if (pxTaskStatusArray == NULL) {
        printf("Failed to allocate memory for task info\n");
        return;
    }
    
    // Get task info
    uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, NULL);
    
    printf("\n======== TASK STACK USAGE ========\n");
    printf("%-16s %8s %12s\n", "Task", "Priority", "StackFree");
    printf("-----------------------------------------\n");
    
    for (UBaseType_t i = 0; i < uxArraySize; i++) {
        // High water mark dalam WORDS (4 bytes di 32-bit)
        UBaseType_t hwm = pxTaskStatusArray[i].usStackHighWaterMark;
        
        printf("%-16s %8u %9u wd\n",
               pxTaskStatusArray[i].pcTaskName,
               pxTaskStatusArray[i].uxCurrentPriority,
               hwm);
        
        // Warning jika stack hampir penuh
        if (hwm < 50) {
            printf("  ^ WARNING: Low stack space!\n");
        }
    }
    printf("====================================\n");
    
    vPortFree(pxTaskStatusArray);
}

// Monitor untuk single task
void monitorMyStack(void) {
    UBaseType_t hwm = uxTaskGetStackHighWaterMark(NULL);  // NULL = current task
    printf("My stack high water mark: %u words (%u bytes)\n", hwm, hwm * 4);
}
```

#### 12.5.3 Runtime Statistics

```c
// FreeRTOSConfig.h
#define configGENERATE_RUN_TIME_STATS        1
#define configUSE_STATS_FORMATTING_FUNCTIONS 1

// Timer setup untuk high-resolution timing
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() configureTimerForRunTimeStats()
#define portGET_RUN_TIME_COUNTER_VALUE()         getRunTimeCounterValue()

// STM32: Gunakan DWT cycle counter atau timer
volatile uint32_t ulHighFrequencyTimerTicks = 0;

void configureTimerForRunTimeStats(void) {
    // Option 1: DWT Cycle Counter (paling akurat)
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t getRunTimeCounterValue(void) {
    return DWT->CYCCNT / (SystemCoreClock / 10000);  // 0.1ms resolution
}

// Print runtime stats
void printRuntimeStats(void) {
    char buffer[1024];
    
    printf("\n======= RUNTIME STATISTICS =======\n");
    printf("Task            Abs Time      %% Time\n");
    printf("-----------------------------------\n");
    
    vTaskGetRunTimeStats(buffer);
    printf("%s", buffer);
    
    printf("==================================\n");
}

// Alternative: Task list with all info
void printTaskList(void) {
    char buffer[512];
    
    printf("\n======== TASK LIST ========\n");
    printf("Task          State  Prio  Stack  Num\n");
    printf("--------------------------------------\n");
    
    vTaskList(buffer);
    printf("%s", buffer);
    
    // State legend: R=Running, B=Blocked, S=Suspended, D=Deleted
    printf("--------------------------------------\n");
    printf("State: R=Running B=Blocked S=Suspended\n");
}
```

### 12.6 Event Groups

Event groups memungkinkan sinkronisasi multi-task menggunakan bit flags. Sangat berguna untuk koordinasi antar task.

#### 12.6.1 Konsep Event Groups

```c
/**
 * Event Group adalah bitmap 8 atau 24 bit (tergantung konfigurasi)
 * - 8 bits jika configUSE_16_BIT_TICKS = 1
 * - 24 bits jika configUSE_16_BIT_TICKS = 0 (default)
 * 
 * Setiap bit dapat diset/clear oleh task manapun
 * Task dapat wait untuk kombinasi bit (AND/OR)
 */

// Define event bits
#define EVENT_SENSOR_READY     (1 << 0)  // Bit 0: Sensor initialized
#define EVENT_WIFI_CONNECTED   (1 << 1)  // Bit 1: WiFi connected
#define EVENT_DATA_READY       (1 << 2)  // Bit 2: Data ready to process
#define EVENT_BUTTON_PRESSED   (1 << 3)  // Bit 3: User button pressed
#define EVENT_TIMER_EXPIRED    (1 << 4)  // Bit 4: Timer event

// Combined events
#define EVENT_SYSTEM_READY    (EVENT_SENSOR_READY | EVENT_WIFI_CONNECTED)
#define EVENT_ALL_INIT        (EVENT_SENSOR_READY | EVENT_WIFI_CONNECTED | EVENT_DATA_READY)

// Visualisasi Event Group (24 bits)
//
// Bit: 23 22 21 20 ... 7 6 5 4 3 2 1 0
//       0  0  0  0 ... 0 0 0 1 1 1 1 1
//                          │ │ │ │ │ └─ SENSOR_READY (1)
//                          │ │ │ │ └─── WIFI_CONNECTED (1)
//                          │ │ │ └───── DATA_READY (1)
//                          │ │ └─────── BUTTON_PRESSED (1)
//                          │ └───────── TIMER_EXPIRED (1)
//                          └─────────── Reserved (top 8 bits by FreeRTOS)
```

#### 12.6.2 Event Group API

```c
#include "freertos/event_groups.h"

// Create event group
EventGroupHandle_t xEventGroup;

void setup(void) {
    xEventGroup = xEventGroupCreate();
    if (xEventGroup == NULL) {
        printf("Failed to create event group!\n");
    }
}

// ========== SETTING BITS ==========

// Set bits dari task
void sensorTask(void *pvParameters) {
    // Initialize sensor...
    initSensor();
    
    // Signal that sensor is ready
    xEventGroupSetBits(xEventGroup, EVENT_SENSOR_READY);
    printf("Sensor ready, event bit set\n");
    
    for(;;) {
        // Read sensor and set data ready
        readSensor();
        xEventGroupSetBits(xEventGroup, EVENT_DATA_READY);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Set bits dari ISR
void EXTI0_IRQHandler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    // Set button pressed event from ISR
    xEventGroupSetBitsFromISR(
        xEventGroup,
        EVENT_BUTTON_PRESSED,
        &xHigherPriorityTaskWoken
    );
    
    // Context switch if higher priority task woken
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// ========== WAITING FOR BITS ==========

// Wait for single event (OR logic)
void waitAnySingle(void) {
    EventBits_t bits = xEventGroupWaitBits(
        xEventGroup,
        EVENT_BUTTON_PRESSED | EVENT_TIMER_EXPIRED,  // Wait for either
        pdTRUE,              // Clear bits on exit
        pdFALSE,             // Wait for ANY (OR logic)
        portMAX_DELAY        // Wait forever
    );
    
    if (bits & EVENT_BUTTON_PRESSED) {
        printf("Button was pressed\n");
    }
    if (bits & EVENT_TIMER_EXPIRED) {
        printf("Timer expired\n");
    }
}

// Wait for multiple events (AND logic)
void waitAllEvents(void) {
    EventBits_t bits = xEventGroupWaitBits(
        xEventGroup,
        EVENT_SYSTEM_READY,  // Wait for sensor AND wifi
        pdTRUE,              // Clear bits on exit
        pdTRUE,              // Wait for ALL (AND logic)
        pdMS_TO_TICKS(10000) // 10 second timeout
    );
    
    if ((bits & EVENT_SYSTEM_READY) == EVENT_SYSTEM_READY) {
        printf("System fully initialized!\n");
    } else {
        printf("Timeout waiting for system init\n");
        if (!(bits & EVENT_SENSOR_READY)) printf("  - Sensor not ready\n");
        if (!(bits & EVENT_WIFI_CONNECTED)) printf("  - WiFi not connected\n");
    }
}

// ========== SYNCHRONIZATION (RENDEZVOUS) ==========

// Multiple tasks sync at a point
void task1(void *pvParameters) {
    printf("Task1: Doing initialization...\n");
    vTaskDelay(pdMS_TO_TICKS(100));
    
    printf("Task1: Waiting at sync point...\n");
    xEventGroupSync(
        xEventGroup,
        (1 << 0),                    // Set bit 0 when reaching sync
        (1 << 0) | (1 << 1) | (1 << 2),  // Wait for bits 0,1,2
        portMAX_DELAY
    );
    
    printf("Task1: All tasks synchronized!\n");
    // Continue with synchronized operation...
}

void task2(void *pvParameters) {
    printf("Task2: Doing initialization...\n");
    vTaskDelay(pdMS_TO_TICKS(200));
    
    printf("Task2: Waiting at sync point...\n");
    xEventGroupSync(xEventGroup, (1 << 1), (1 << 0) | (1 << 1) | (1 << 2), portMAX_DELAY);
    
    printf("Task2: All tasks synchronized!\n");
}

void task3(void *pvParameters) {
    printf("Task3: Doing initialization...\n");
    vTaskDelay(pdMS_TO_TICKS(300));
    
    printf("Task3: Waiting at sync point...\n");
    xEventGroupSync(xEventGroup, (1 << 2), (1 << 0) | (1 << 1) | (1 << 2), portMAX_DELAY);
    
    printf("Task3: All tasks synchronized!\n");
}
```

### 12.7 Stream Buffers dan Message Buffers

#### 12.7.1 Stream Buffer - Byte-Oriented

```c
#include "freertos/stream_buffer.h"

/**
 * Stream Buffer karakteristik:
 * - Byte-oriented continuous stream
 * - Tidak ada framing/message boundaries
 * - Single writer, single reader (optimal)
 * - Cocok untuk: UART data, audio streaming, sensor data
 */

// Create stream buffer
StreamBufferHandle_t xStreamBuffer;

void setup(void) {
    xStreamBuffer = xStreamBufferCreate(
        256,    // Total buffer size in bytes
        1       // Trigger level - wake reader when N bytes available
    );
}

// Producer task - write data
void uartRxTask(void *pvParameters) {
    uint8_t rxByte;
    
    for(;;) {
        // Receive byte from UART
        if (HAL_UART_Receive(&huart1, &rxByte, 1, 10) == HAL_OK) {
            // Send to stream buffer
            xStreamBufferSend(
                xStreamBuffer,
                &rxByte,
                1,
                pdMS_TO_TICKS(10)
            );
        }
    }
}

// Consumer task - read data
void processTask(void *pvParameters) {
    uint8_t buffer[64];
    size_t bytesReceived;
    
    for(;;) {
        // Wait and receive data
        bytesReceived = xStreamBufferReceive(
            xStreamBuffer,
            buffer,
            sizeof(buffer),
            portMAX_DELAY  // Block until data available
        );
        
        if (bytesReceived > 0) {
            printf("Received %d bytes: ", bytesReceived);
            for(int i = 0; i < bytesReceived; i++) {
                printf("%02X ", buffer[i]);
            }
            printf("\n");
        }
    }
}

// ISR version
void USART1_IRQHandler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint8_t data = USART1->DR;
    
    xStreamBufferSendFromISR(
        xStreamBuffer,
        &data,
        1,
        &xHigherPriorityTaskWoken
    );
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// Utility functions
size_t getBytesInBuffer(void) {
    return xStreamBufferBytesAvailable(xStreamBuffer);
}

size_t getFreeSpace(void) {
    return xStreamBufferSpacesAvailable(xStreamBuffer);
}

void resetBuffer(void) {
    xStreamBufferReset(xStreamBuffer);
}
```

#### 12.7.2 Message Buffer - Discrete Messages

```c
#include "freertos/message_buffer.h"

/**
 * Message Buffer karakteristik:
 * - Discrete messages dengan length prefix
 * - Setiap message adalah unit terpisah
 * - Automatic framing (4-byte length header)
 * - Single writer, single reader (lockless)
 * - Cocok untuk: Command/response, packets, structured data
 */

// Create message buffer
MessageBufferHandle_t xMessageBuffer;

void setup(void) {
    // Buffer size harus cukup untuk messages + length overhead
    // Setiap message butuh +4 bytes untuk length
    xMessageBuffer = xMessageBufferCreate(512);  // 512 bytes total
}

// Structured message
typedef struct {
    uint8_t  msgType;
    uint16_t sensorId;
    float    value;
    uint32_t timestamp;
} SensorMessage_t;

// Producer - send structured messages
void sensorTask(void *pvParameters) {
    SensorMessage_t msg;
    
    for(;;) {
        msg.msgType = MSG_SENSOR_DATA;
        msg.sensorId = 1;
        msg.value = readTemperature();
        msg.timestamp = xTaskGetTickCount();
        
        // Send complete message
        size_t bytesSent = xMessageBufferSend(
            xMessageBuffer,
            &msg,
            sizeof(msg),
            pdMS_TO_TICKS(100)
        );
        
        if (bytesSent != sizeof(msg)) {
            printf("Failed to send message - buffer full?\n");
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Consumer - receive complete messages
void processorTask(void *pvParameters) {
    SensorMessage_t msg;
    size_t bytesReceived;
    
    for(;;) {
        // Receive complete message
        bytesReceived = xMessageBufferReceive(
            xMessageBuffer,
            &msg,
            sizeof(msg),
            portMAX_DELAY
        );
        
        if (bytesReceived == sizeof(msg)) {
            printf("Message received:\n");
            printf("  Type: %d\n", msg.msgType);
            printf("  Sensor: %d\n", msg.sensorId);
            printf("  Value: %.2f\n", msg.value);
            printf("  Time: %lu\n", msg.timestamp);
        }
    }
}

// Variable length messages
void sendCommand(const char *cmd) {
    xMessageBufferSend(
        xMessageBuffer,
        cmd,
        strlen(cmd) + 1,  // Include null terminator
        portMAX_DELAY
    );
}

void receiveCommand(void) {
    char buffer[128];
    size_t len = xMessageBufferReceive(
        xMessageBuffer,
        buffer,
        sizeof(buffer),
        portMAX_DELAY
    );
    
    if (len > 0) {
        printf("Command: %s\n", buffer);
    }
}
```

### 12.8 Low Power Features

#### 12.8.1 Tickless Idle Mode

```c
/**
 * Tickless Idle menghentikan tick interrupt saat idle
 * - MCU dapat masuk sleep mode lebih dalam
 * - Hemat power untuk battery-powered devices
 * - FreeRTOS menghitung ticks yang "missed" saat sleep
 */

// FreeRTOSConfig.h
#define configUSE_TICKLESS_IDLE               1
#define configEXPECTED_IDLE_TIME_BEFORE_SLEEP 2  // Min ticks before sleep

// STM32 Low Power Implementation
void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime) {
    // 1. Stop SysTick
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    
    // 2. Calculate actual sleep time in ms
    uint32_t sleepTimeMs = xExpectedIdleTime * portTICK_PERIOD_MS;
    
    // 3. Configure wakeup timer (RTC atau LPTIM)
    HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, sleepTimeMs, RTC_WAKEUPCLOCK_CK_SPRE_16BITS);
    
    // 4. Enter sleep mode
    __DSB();  // Data Synchronization Barrier
    __WFI();  // Wait For Interrupt - CPU sleeps here
    
    // 5. Woken up! Calculate actual sleep time
    uint32_t actualSleepTicks = getActualSleepTime() / portTICK_PERIOD_MS;
    
    // 6. Correct tick count
    vTaskStepTick(actualSleepTicks);
    
    // 7. Restart SysTick
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

// ESP32 Automatic Light Sleep
#include "esp_pm.h"

void configureLowPower(void) {
    esp_pm_config_esp32_t pm_config = {
        .max_freq_mhz = 240,     // Max CPU frequency
        .min_freq_mhz = 80,      // Min CPU frequency  
        .light_sleep_enable = true
    };
    
    ESP_ERROR_CHECK(esp_pm_configure(&pm_config));
    printf("Power management configured\n");
}
```

#### 12.8.2 Deep Sleep dengan FreeRTOS

```c
// ESP32 Deep Sleep
void enterDeepSleep(uint32_t sleepSeconds) {
    printf("Entering deep sleep for %lu seconds...\n", sleepSeconds);
    
    // Konfigurasi wakeup source
    esp_sleep_enable_timer_wakeup(sleepSeconds * 1000000ULL);  // microseconds
    
    // GPIO wakeup (optional)
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0);  // Wake on GPIO0 LOW
    
    // Simpan data ke RTC memory (survive deep sleep)
    RTC_DATA_ATTR static uint32_t bootCount = 0;
    bootCount++;
    
    // Enter deep sleep - FreeRTOS state NOT preserved
    esp_deep_sleep_start();
    
    // Code after this won't execute - MCU resets after deep sleep
}

// Check wakeup reason after boot
void checkWakeupReason(void) {
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    
    switch(cause) {
        case ESP_SLEEP_WAKEUP_TIMER:
            printf("Wakeup: Timer\n");
            break;
        case ESP_SLEEP_WAKEUP_EXT0:
            printf("Wakeup: External GPIO\n");
            break;
        case ESP_SLEEP_WAKEUP_TOUCHPAD:
            printf("Wakeup: Touchpad\n");
            break;
        default:
            printf("Wakeup: Power on / Reset\n");
            break;
    }
}

// STM32 Standby Mode
void enterStandbyMode(uint32_t wakeupSeconds) {
    // Save important data to backup registers
    HAL_PWR_EnableBkUpAccess();
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, bootCount);
    
    // Configure RTC wakeup
    HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, wakeupSeconds, RTC_WAKEUPCLOCK_CK_SPRE_16BITS);
    
    // Enter standby mode
    HAL_PWR_EnterSTANDBYMode();
    
    // Code won't reach here - MCU resets
}
```

### 12.9 Static Allocation

```c
/**
 * Static allocation - semua memory dialokasikan compile-time
 * Keuntungan:
 * - Tidak ada runtime allocation failure
 * - Memory usage known at compile time
 * - Sedikit lebih cepat
 * - Tidak ada fragmentasi
 */

// FreeRTOSConfig.h
#define configSUPPORT_STATIC_ALLOCATION  1
#define configSUPPORT_DYNAMIC_ALLOCATION 0  // Optional: disable dynamic

// Static Task
static StaticTask_t xTaskBuffer;
static StackType_t xTaskStack[256];

void createStaticTask(void) {
    TaskHandle_t xHandle = xTaskCreateStatic(
        vTaskFunction,
        "StaticTask",
        256,                // Stack size in words
        NULL,               // Parameters
        2,                  // Priority
        xTaskStack,         // Stack buffer
        &xTaskBuffer        // Task buffer
    );
}

// Static Queue
static StaticQueue_t xQueueBuffer;
static uint8_t ucQueueStorage[10 * sizeof(uint32_t)];

void createStaticQueue(void) {
    QueueHandle_t xQueue = xQueueCreateStatic(
        10,                 // Queue length
        sizeof(uint32_t),   // Item size
        ucQueueStorage,     // Storage buffer
        &xQueueBuffer       // Queue buffer
    );
}

// Static Semaphore
static StaticSemaphore_t xSemaphoreBuffer;

void createStaticSemaphore(void) {
    SemaphoreHandle_t xSem = xSemaphoreCreateBinaryStatic(&xSemaphoreBuffer);
    SemaphoreHandle_t xMutex = xSemaphoreCreateMutexStatic(&xSemaphoreBuffer);
}

// Static Event Group
static StaticEventGroup_t xEventGroupBuffer;

void createStaticEventGroup(void) {
    EventGroupHandle_t xEvents = xEventGroupCreateStatic(&xEventGroupBuffer);
}

// Static Timer
static StaticTimer_t xTimerBuffer;

void createStaticTimer(void) {
    TimerHandle_t xTimer = xTimerCreateStatic(
        "StaticTimer",
        pdMS_TO_TICKS(1000),
        pdTRUE,             // Auto-reload
        NULL,               // Timer ID
        vTimerCallback,
        &xTimerBuffer
    );
}

// WAJIB: Provide memory for idle task dan timer task (jika static only)
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize) {
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];
    
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     uint32_t *pulTimerTaskStackSize) {
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];
    
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
```

### 12.10 Memory Pool Implementation

```c
/**
 * Custom memory pool untuk:
 * - Deterministic allocation time (O(1))
 * - Tidak ada fragmentasi
 * - Cocok untuk frequent same-size alloc/free
 */

#define POOL_BLOCK_SIZE    64
#define POOL_BLOCK_COUNT   16

typedef struct PoolBlock {
    struct PoolBlock *next;
    uint8_t data[POOL_BLOCK_SIZE - sizeof(struct PoolBlock *)];
} PoolBlock_t;

typedef struct {
    PoolBlock_t blocks[POOL_BLOCK_COUNT];
    PoolBlock_t *freeList;
    SemaphoreHandle_t mutex;
    uint32_t allocCount;
    uint32_t freeCount;
} MemoryPool_t;

// Initialize pool
void poolInit(MemoryPool_t *pool) {
    pool->freeList = &pool->blocks[0];
    pool->allocCount = 0;
    pool->freeCount = 0;
    pool->mutex = xSemaphoreCreateMutex();
    
    // Link all blocks
    for (int i = 0; i < POOL_BLOCK_COUNT - 1; i++) {
        pool->blocks[i].next = &pool->blocks[i + 1];
    }
    pool->blocks[POOL_BLOCK_COUNT - 1].next = NULL;
}

// Allocate from pool (thread-safe)
void *poolAlloc(MemoryPool_t *pool) {
    void *ptr = NULL;
    
    if (xSemaphoreTake(pool->mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        if (pool->freeList != NULL) {
            ptr = pool->freeList;
            pool->freeList = pool->freeList->next;
            pool->allocCount++;
        }
        xSemaphoreGive(pool->mutex);
    }
    
    return ptr;
}

// Return to pool (thread-safe)
void poolFree(MemoryPool_t *pool, void *ptr) {
    if (ptr == NULL) return;
    
    if (xSemaphoreTake(pool->mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        PoolBlock_t *block = (PoolBlock_t *)ptr;
        block->next = pool->freeList;
        pool->freeList = block;
        pool->freeCount++;
        xSemaphoreGive(pool->mutex);
    }
}

// Get statistics
void poolStats(MemoryPool_t *pool) {
    int freeBlocks = 0;
    PoolBlock_t *current = pool->freeList;
    
    while (current != NULL) {
        freeBlocks++;
        current = current->next;
    }
    
    printf("Pool Stats:\n");
    printf("  Total blocks: %d\n", POOL_BLOCK_COUNT);
    printf("  Free blocks: %d\n", freeBlocks);
    printf("  Used blocks: %d\n", POOL_BLOCK_COUNT - freeBlocks);
    printf("  Total allocs: %lu\n", pool->allocCount);
    printf("  Total frees: %lu\n", pool->freeCount);
}
```

---

## 📊 Perbandingan STM32 vs ESP32

| Aspek | STM32F103 | ESP32 |
|-------|-----------|-------|
| Total RAM | 20 KB | 520 KB |
| Heap Scheme Default | heap_4 | ESP-IDF custom |
| Stack per Task | ~256-512 words | ~2048-4096 words |
| Event Groups | Standard | Extended (ESP-IDF) |
| Stream/Message Buffer | Standard | Standard |
| Low Power | Stop/Standby | Light/Deep sleep |
| External RAM | Tidak | PSRAM (optional) |
| Memory Protection | Tidak (Cortex-M3) | Ya (Cortex-M0+) |

---

## 📖 Referensi

1. FreeRTOS Memory Management - https://freertos.org/a00111.html
2. Mastering the FreeRTOS Real Time Kernel - Richard Barry
3. STM32F103 Reference Manual - Memory Organization
4. ESP32 Technical Reference Manual - Memory Management
5. FreeRTOS Event Groups - https://freertos.org/FreeRTOS-Event-Groups.html
6. FreeRTOS Stream/Message Buffers - https://freertos.org/RTOS-stream-message-buffers.html

---

## ❓ Pertanyaan Review

1. Jelaskan perbedaan heap_1, heap_2, heap_4, dan heap_5. Kapan masing-masing digunakan?
2. Apa perbedaan stack overflow detection metode 1 dan metode 2?
3. Bagaimana Event Groups berbeda dari Queue dan Semaphore? Berikan contoh use case!
4. Jelaskan perbedaan Stream Buffer dan Message Buffer!
5. Apa keuntungan static allocation dibanding dynamic allocation?
6. Bagaimana tickless idle mode menghemat power?
7. Mengapa memory pool berguna untuk real-time systems?
