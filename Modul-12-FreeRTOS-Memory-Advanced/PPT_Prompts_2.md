# Prompt untuk Pembuatan PPT - Bagian 2
## Modul 12: FreeRTOS Memory Management & Advanced Features

### Lanjutan Slide untuk Materi Advanced

---

## Slide 25: Static vs Dynamic Allocation

**Prompt:**
```
Create a comparison diagram of static vs dynamic allocation:

Dynamic (left side):
- Runtime allocation visualization
- Heap pool with varying blocks
- "pvPortMalloc()" arrow
- Risk: Allocation failure, fragmentation

Static (right side):
- Compile-time fixed blocks
- Pre-allocated arrays
- "xTaskCreateStatic()" arrow
- Benefits: Deterministic, no failure

Balance scale showing tradeoffs
Title: "Static vs Dynamic Allocation"
```

---

## Slide 26: Static Task Creation

**Prompt:**
```
Create a code visualization showing static task creation:
- StaticTask_t buffer (TCB)
- StackType_t array (Stack)
- xTaskCreateStatic() function call
- Arrow pointing: "Memory allocated at compile time"
- Visual of task control block structure
- Comparison: No heap fragmentation
Title: "Membuat Task Secara Statis"
```

---

## Slide 27: Static Resource Overview

**Prompt:**
```
Create an infographic showing all static resources:
- Static Task (xTaskCreateStatic)
- Static Queue (xQueueCreateStatic)
- Static Semaphore (xSemaphoreCreateBinaryStatic)
- Static Mutex (xSemaphoreCreateMutexStatic)
- Static Event Group (xEventGroupCreateStatic)
- Static Timer (xTimerCreateStatic)
Each with icon and memory requirement
Grid layout with consistent styling
Title: "Static Allocation untuk Semua Resource"
```

---

## Slide 28: Idle and Timer Task Memory

**Prompt:**
```
Create a diagram showing required memory callbacks:
- vApplicationGetIdleTaskMemory()
- vApplicationGetTimerTaskMemory()
- Required when configSUPPORT_STATIC_ALLOCATION = 1
- Show TCB buffer and Stack buffer for each
- Arrow from FreeRTOS kernel to user-provided memory
- Warning icon: "WAJIB diimplementasikan!"
Title: "Memory untuk Idle dan Timer Task"
```

---

## Slide 29: Memory Pool Concept

**Prompt:**
```
Create a visualization of custom memory pool:
- Fixed-size blocks arranged in grid
- Free list linking empty blocks
- "poolAlloc()" taking from free list
- "poolFree()" returning to free list
- O(1) allocation time indicator
- No fragmentation benefit
- Block diagram with arrows
Title: "Memory Pool: Custom Allocator"
```

---

## Slide 30: Memory Pool Operations

**Prompt:**
```
Create an animated-style sequence showing pool operations:
1. Initial: All blocks free (green)
2. Alloc Block 1: First block used (red)
3. Alloc Block 2: Second block used
4. Free Block 1: Returns to free list
5. Alloc Block 3: Reuses freed block
Show free list pointer movement
Title: "Memory Pool Operations"
```

---

## Slide 31: Low Power Introduction

**Prompt:**
```
Create an introduction slide for low power features:
- Battery icon with percentage
- Power consumption graph
- IoT device needing long battery life
- Sleep mode icons (light sleep, deep sleep)
- Years of battery life goal
- Smart watch, sensor node examples
Title: "Low Power: Hemat Energi untuk Battery-Powered Devices"
```

---

## Slide 32: Tickless Idle Concept

**Prompt:**
```
Create a timing diagram showing tickless idle:
Regular mode:
- Continuous tick interrupts (every 1ms)
- CPU wakes for each tick

Tickless mode:
- Long sleep period
- Tick interrupt stopped
- Wake only when needed
- vTaskStepTick() compensation

Power graph showing savings
Title: "Tickless Idle Mode"
```

---

## Slide 33: Sleep Modes Hierarchy

**Prompt:**
```
Create a pyramid/hierarchy diagram of sleep modes:
Top (least power save):
- Active Mode: Full power
- Sleep Mode: CPU halted, peripherals on
- Stop Mode: Most clocks stopped
- Standby Mode: Minimal power
Bottom (most power save):
- Off Mode: Only RTC

Power consumption and wake time for each level
Color gradient from red (high power) to green (low power)
Title: "Hirarki Sleep Modes"
```

---

## Slide 34: ESP32 Light Sleep

**Prompt:**
```
Create a diagram showing ESP32 light sleep:
- esp_pm_config configuration
- Automatic light sleep when idle
- WiFi connection maintained (optional)
- GPIO wakeup sources
- Timer wakeup
- Touchpad wakeup
- Power vs functionality tradeoff
Title: "ESP32 Light Sleep dengan FreeRTOS"
```

---

## Slide 35: Deep Sleep with RTC Memory

**Prompt:**
```
Create a diagram showing deep sleep with data preservation:
- Main RAM: Powered off (crossed out)
- RTC RAM: Preserved (highlighted)
- RTC_DATA_ATTR variables
- Boot count example
- Wake from: Timer, GPIO, Touchpad
- Reset on wake (not resume)
Title: "Deep Sleep dengan RTC Memory"
```

---

## Slide 36: STM32 vs ESP32 Memory Comparison

**Prompt:**
```
Create a detailed comparison table/infographic:
Feature | STM32F103 | ESP32
- Total RAM | 20KB | 520KB
- Heap Scheme | heap_4 | ESP-IDF custom
- Stack Direction | Descending | Descending
- External RAM | No | PSRAM support
- Low Power Modes | Stop/Standby | Light/Deep sleep
- DMA Memory | Any SRAM | Special regions

Visual icons for each platform
Title: "Perbandingan STM32 vs ESP32"
```

---

## Slide 37: Best Practices - Stack Sizing

**Prompt:**
```
Create a best practices guide for stack sizing:
1. Start conservative (large)
2. Monitor high water mark
3. Reduce gradually
4. Keep 20-30% margin
5. Consider printf impact

Visual: Stack bar with safety margin highlighted
Guidelines table:
- Minimal: 128 words
- Small: 256 words
- Medium: 512 words
- Large: 1024 words
Title: "Best Practice: Sizing Task Stacks"
```

---

## Slide 38: Best Practices - Avoiding Memory Leaks

**Prompt:**
```
Create an infographic about memory leak prevention:
1. Prefer static allocation
2. Track all allocations
3. Use RAII-like patterns
4. Debug build tracking
5. Regular heap monitoring

Warning signs:
- Decreasing free heap over time
- Allocation failures
- High fragmentation

Checklist format with icons
Title: "Best Practice: Menghindari Memory Leak"
```

---

## Slide 39: Debugging Memory Issues

**Prompt:**
```
Create a troubleshooting flowchart:
Problem: Application crash/hang

Is stack overflow? → Check hook
Is heap exhausted? → Check free heap
Is fragmentation? → Check largest free block
Memory corruption? → Check buffer bounds

Solution paths for each issue
Diagnostic commands for each step
Title: "Debugging Memory Issues"
```

---

## Slide 40: Code Example - Heap Statistics

**Prompt:**
```
Create a slide showing code example:
- Code snippet for vPortGetHeapStats()
- Formatted output example
- Dashboard visualization of the stats
- Interpretation guide:
  - If fragmentation high: consider heap_4
  - If min free low: increase heap size
Syntax highlighted code block
Title: "Code Example: Heap Statistics"
```

---

## Slide 41: Code Example - Event Groups

**Prompt:**
```
Create a code walkthrough slide:
- Event bit definitions (#define)
- Create event group
- Task setting bits
- Task waiting for bits
- Visual flow of bit operations
- Output showing sync moment
Split: Code on left, diagram on right
Title: "Code Example: Event Groups"
```

---

## Slide 42: Code Example - Stream Buffer

**Prompt:**
```
Create a code example slide for stream buffer:
- xStreamBufferCreate()
- Producer task: xStreamBufferSend()
- Consumer task: xStreamBufferReceive()
- ISR version: xStreamBufferSendFromISR()
- Visual: Data flowing through buffer
- UART use case scenario
Title: "Code Example: Stream Buffer"
```

---

## Slide 43: Hands-On Exercise 1

**Prompt:**
```
Create a lab exercise slide:
Exercise: "Monitor Heap dan Stack Usage"
Steps:
1. Create 3 tasks with different stack sizes
2. Monitor heap after each creation
3. Track high water mark
4. Print periodic statistics
5. Optimize stack sizes

Expected output sample
Time estimate: 30 minutes
Title: "Latihan 1: Memory Monitoring"
```

---

## Slide 44: Hands-On Exercise 2

**Prompt:**
```
Create a lab exercise slide:
Exercise: "Event Groups untuk Sensor Fusion"
Scenario:
- 3 sensor tasks (temp, humidity, pressure)
- Each sets ready bit when data available
- Processing task waits for ALL three
- Calculate and display fused data

Block diagram of the system
Time estimate: 45 minutes
Title: "Latihan 2: Event Groups Sensor Fusion"
```

---

## Slide 45: Hands-On Exercise 3

**Prompt:**
```
Create a lab exercise slide:
Exercise: "Implement Low Power Mode"
Tasks:
1. Configure tickless idle on STM32
2. Add button wakeup source
3. Measure current consumption
4. Compare with non-tickless mode

Power measurement setup diagram
Expected power savings graph
Time estimate: 45 minutes
Title: "Latihan 3: Low Power Implementation"
```

---

## Slide 46: Project Ideas

**Prompt:**
```
Create a project ideas slide with 4 mini-cards:
1. Smart Power Monitor
   - Track heap/stack usage
   - Alert on low memory
   
2. Event-Driven Data Logger
   - Multiple sensors with event groups
   - Stream buffer to SD card
   
3. Low Power Weather Station
   - Deep sleep between readings
   - RTC memory for data
   
4. Memory Pool Audio Buffer
   - Fixed-size audio chunks
   - Zero fragmentation

Each card with icon and difficulty level
Title: "Ide Project Lanjutan"
```

---

## Slide 47: Common Mistakes

**Prompt:**
```
Create a "common mistakes" warning slide:
1. ❌ Stack too small → Use high water mark
2. ❌ Heap too small → Monitor free heap
3. ❌ Forgetting to free → Track allocations
4. ❌ Using heap in ISR → Use FromISR variants
5. ❌ Not checking return values → Always verify

Before/After code examples
Red warning style design
Title: "Kesalahan Umum dan Solusinya"
```

---

## Slide 48: Performance Comparison

**Prompt:**
```
Create a performance comparison chart:
Operation | Dynamic | Static | Pool
Task Create | ~50μs | ~20μs | N/A
Malloc | ~10μs | N/A | ~1μs
Free | ~5μs | N/A | ~1μs
Fragmentation | Yes | No | No

Bar chart visualization
"Choose based on your requirements" conclusion
Title: "Perbandingan Performa Alokasi"
```

---

## Slide 49: Key Takeaways

**Prompt:**
```
Create a final key takeaways slide:
🎯 Choose heap scheme based on needs
🎯 Always enable stack overflow detection
🎯 Monitor memory in development
🎯 Consider static allocation for critical systems
🎯 Event Groups for complex synchronization
🎯 Buffers for efficient data transfer
🎯 Low power for battery devices

Bullet points with icons
Highlighted/bordered design
Title: "Poin-Poin Penting"
```

---

## Slide 50: Next Module Preview

**Prompt:**
```
Create a preview slide for next module:
"Module 13: Network Communication"
Topics preview:
- WiFi connectivity
- HTTP/HTTPS requests
- MQTT protocol
- WebSocket
- Bluetooth communication

Exciting tech icons (WiFi, cloud, IoT)
"Coming up next..." style
Title: "Preview: Modul Selanjutnya"
```

---

## Catatan Tambahan untuk Presenter

### Tips Presentasi:
1. Demo live heap monitoring jika memungkinkan
2. Tunjukkan stack overflow secara controlled
3. Visualisasikan event groups dengan LED
4. Bandingkan power consumption real-time

### Alat Demo:
- Serial monitor untuk statistics
- Multimeter untuk power measurement
- Logic analyzer untuk timing

### Durasi Rekomendasi:
- Teori: 45 menit
- Demo: 20 menit
- Latihan: 45 menit
- Diskusi: 10 menit
