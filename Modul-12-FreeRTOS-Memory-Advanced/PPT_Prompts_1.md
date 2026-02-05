# Prompt untuk Pembuatan PPT - Bagian 1
## Modul 12: FreeRTOS Memory Management & Advanced Features

### Instruksi Umum untuk AI Image Generator

Gunakan prompt berikut untuk membuat slide presentasi yang profesional dan edukatif. Setiap slide harus memiliki visual yang jelas dan konsisten.

---

## Slide 1: Judul

**Prompt:**
```
Create a professional presentation title slide for "FreeRTOS Memory Management & Advanced Features". Include:
- Modern tech aesthetic with blue and dark gray color scheme
- Subtle circuit board pattern in background
- Memory chip and microcontroller icons
- University/educational style layout
- Text placeholder for "Module 12" and "Embedded Systems Practicum"
Clean, minimal design with gradient effects.
```

---

## Slide 2: Learning Objectives

**Prompt:**
```
Create an educational slide showing learning objectives with:
- Numbered list design (1-7 items)
- Icons for each objective: memory chip, stack, events, buffer, debug, power, static
- Checkmark or target icons
- Light background with accent colors
- Modern infographic style
Title: "Capaian Pembelajaran"
```

---

## Slide 3: Memory Constraints in Embedded Systems

**Prompt:**
```
Create an infographic comparing memory constraints of different microcontrollers:
- STM32F103: 20KB RAM, 64KB Flash (small box)
- ESP32: 520KB RAM, 4MB Flash (medium box)
- Visual size comparison
- RAM vs Flash color coding (RAM=green, Flash=blue)
- Microcontroller chip images
- Scale/ruler visualization
Title: "Keterbatasan Memori Embedded Systems"
```

---

## Slide 4: Memory Map Visualization

**Prompt:**
```
Create a detailed memory map diagram for microcontroller showing:
- Flash section (Code, .rodata) at top - blue color
- RAM section below - green color
- RAM subdivisions: .data, .bss, HEAP (growing down), STACK (growing up)
- Address labels on the left
- Arrows showing growth direction
- Clear section boundaries
Professional technical diagram style.
Title: "Memory Map Microcontroller"
```

---

## Slide 5: FreeRTOS Heap Schemes Overview

**Prompt:**
```
Create a comparison infographic for 5 FreeRTOS heap schemes:
- heap_1: Simple box, no free (red X)
- heap_2: Multiple boxes, best fit
- heap_3: Standard library wrapper
- heap_4: Boxes merging together (coalescence)
- heap_5: Multiple memory regions
Each with icon, key feature, and use case
Table or card layout
Title: "FreeRTOS Heap Allocation Schemes"
```

---

## Slide 6: Heap_1 - No Free

**Prompt:**
```
Create a diagram showing heap_1 allocation:
- Linear memory bar
- Sequential allocations (Task A, Queue B, Timer C)
- No gaps, no fragmentation
- "Used" section growing, "Free" section shrinking
- Red X over "vPortFree()" 
- Checkmarks: Simple, Deterministic, No fragmentation
- X marks: Cannot free memory
Title: "Heap_1: Paling Sederhana"
```

---

## Slide 7: Heap_4 - First Fit with Coalescence

**Prompt:**
```
Create a step-by-step diagram showing heap_4 coalescence:
- Step 1: Three used blocks [A][B][C]
- Step 2: Free block B - [A][FREE][C]
- Step 3: Free block A - [FREE (merged)][C]
- Arrows showing merge process
- Highlight the coalescence benefit
- Before/After comparison
Title: "Heap_4: First Fit dengan Coalescence"
```

---

## Slide 8: Heap_5 - Multiple Regions

**Prompt:**
```
Create a diagram showing heap_5 with multiple memory regions:
- Two separate memory blocks (different addresses)
- Internal SRAM: 0x20000000
- CCM RAM: 0x10000000 (or External PSRAM for ESP32)
- Linked together conceptually
- Arrow showing "vPortDefineHeapRegions()"
- Benefits list: Use all available RAM
Title: "Heap_5: Multiple Memory Regions"
```

---

## Slide 9: Heap Schemes Comparison Table

**Prompt:**
```
Create a professional comparison table with 5 columns:
- Scheme | Free Support | Coalescence | Determinism | Best Use Case
- 5 rows for heap_1 through heap_5
- Color coding: Green=Yes, Red=No, Yellow=Depends
- Icons for each characteristic
- Clean table design with alternating row colors
Title: "Perbandingan Heap Schemes"
```

---

## Slide 10: Stack Overflow - The Silent Killer

**Prompt:**
```
Create a dramatic visualization of stack overflow:
- Stack growing downward into protected/heap area
- Warning symbols and red danger zone
- "Corrupted Data" explosion effect
- Stack canary concept (bird/canary icon)
- Before (safe) vs After (overflow) comparison
- Skull or bug icon for "undefined behavior"
Title: "Stack Overflow: Bug Tersembunyi"
```

---

## Slide 11: Stack Overflow Detection Methods

**Prompt:**
```
Create a comparison diagram of two detection methods:
Method 1 (left):
- Context switch checking
- Simple stack pointer check
- Light bulb icon (lightweight)

Method 2 (right):
- Pattern checking (0xA5A5A5A5)
- 20-byte canary zone at bottom
- Shield icon (more protection)

Central comparison: Speed vs Safety tradeoff
Title: "Metode Deteksi Stack Overflow"
```

---

## Slide 12: Stack with Canary Pattern

**Prompt:**
```
Create a detailed stack visualization with canary:
- Stack area from top (high address) to bottom (low address)
- Active stack data region
- Downward growth arrow
- 20-byte canary zone at bottom highlighted in yellow
- Pattern: 0xA5 repeated
- "If corrupted = OVERFLOW DETECTED" label
Title: "Stack dengan Canary Pattern"
```

---

## Slide 13: Memory Debugging APIs

**Prompt:**
```
Create an infographic showing FreeRTOS memory debugging APIs:
- xPortGetFreeHeapSize() - pie chart icon
- xPortGetMinimumEverFreeHeapSize() - graph trending
- uxTaskGetStackHighWaterMark() - water level gauge
- vTaskGetRunTimeStats() - CPU meter
- vPortGetHeapStats() - detailed report icon
Each with mini code snippet and visualization
Title: "API untuk Memory Debugging"
```

---

## Slide 14: Heap Statistics Visualization

**Prompt:**
```
Create a dashboard-style visualization showing heap statistics:
- Total Heap: Large circle/gauge
- Available: Percentage bar
- Minimum Ever Free: Historical low marker
- Largest Free Block: Size indicator
- Number of Free Blocks: Counter
- Fragmentation Warning indicator
Modern monitoring dashboard style
Title: "Heap Statistics Dashboard"
```

---

## Slide 15: Task Stack High Water Mark

**Prompt:**
```
Create a visualization of high water mark concept:
- Multiple vertical bars representing task stacks
- Water level showing "highest usage"
- Clear zone above water = "Available"
- Used zone below water
- Warning when water level too high
- Multiple tasks side by side comparison
Title: "Stack High Water Mark"
```

---

## Slide 16: Runtime Statistics Table

**Prompt:**
```
Create a sample runtime statistics display:
- Table with columns: Task, Abs Time, % CPU
- Bar chart visualization of CPU usage per task
- IDLE task showing largest percentage
- Color coding by usage level
- Pie chart alternative view
- Real-time monitoring aesthetic
Title: "Runtime Statistics - CPU Usage per Task"
```

---

## Slide 17: Event Groups Concept

**Prompt:**
```
Create a visualization of Event Groups:
- 24-bit register visualization (bit boxes)
- Different colored bits for different events
- Bit labels: SENSOR_READY, WIFI_OK, DATA_READY, etc.
- Multiple tasks pointing to same event group
- Set/Wait operations illustrated
- Binary representation: 00000111 = 3 events set
Title: "Event Groups: Sinkronisasi dengan Bit Flags"
```

---

## Slide 18: Event Groups - Wait Operations

**Prompt:**
```
Create a flowchart showing Event Group wait operations:
- Task waiting (sleeping)
- Event bits being set
- Wait for ANY (OR) - wake on first bit
- Wait for ALL (AND) - wake when all bits set
- Clear on exit option
- Timeout handling
Decision tree style diagram
Title: "Event Groups: Wait Operations"
```

---

## Slide 19: Event Groups - Synchronization (Rendezvous)

**Prompt:**
```
Create a diagram showing task synchronization:
- Three tasks (Task1, Task2, Task3) as lanes
- Each doing initialization (different times)
- Converging at "Sync Point" barrier
- All waiting until everyone arrives
- xEventGroupSync() highlighted
- After sync: All proceed together
Swimlane/timeline diagram style
Title: "Event Groups: Task Rendezvous"
```

---

## Slide 20: Stream Buffer Concept

**Prompt:**
```
Create a visualization of Stream Buffer:
- Circular buffer representation
- Write pointer (producer arrow)
- Read pointer (consumer arrow)
- Continuous byte stream visualization
- Single producer → Buffer → Single consumer
- Trigger level indicator
- UART data bytes flowing through
Title: "Stream Buffer: Byte-Oriented Streaming"
```

---

## Slide 21: Message Buffer Concept

**Prompt:**
```
Create a visualization of Message Buffer:
- Buffer with discrete message packets
- Each packet has: [Length Header][Data]
- Different sized messages
- Clear message boundaries
- Producer sending structured data
- Consumer receiving complete messages
- Framing concept illustrated
Title: "Message Buffer: Discrete Messages"
```

---

## Slide 22: Stream vs Message Buffer Comparison

**Prompt:**
```
Create a side-by-side comparison:
Stream Buffer (left):
- Continuous flow (water pipe analogy)
- No boundaries
- Best for: UART, audio

Message Buffer (right):
- Discrete packets (mail analogy)
- Length prefix
- Best for: Commands, structured data

Visual comparison with icons and use cases
Title: "Stream Buffer vs Message Buffer"
```

---

## Slide 23: Summary Slide

**Prompt:**
```
Create a summary infographic with key takeaways:
- Memory Management: Choose right heap scheme
- Stack Safety: Always enable overflow detection
- Event Groups: Efficient multi-task sync
- Buffers: Stream for bytes, Message for packets
- Monitoring: Regular heap/stack checks
- Best Practices checklist
Modern summary layout with icons
Title: "Ringkasan Materi"
```

---

## Slide 24: Q&A Slide

**Prompt:**
```
Create a Q&A slide with:
- Large question mark icon
- "Questions?" or "Pertanyaan?" text
- Subtle background with memory/embedded theme
- Contact information placeholder
- Light, inviting design
- Discussion bubble icons
Title: "Sesi Tanya Jawab"
```

---

## Catatan Desain

1. **Warna Konsisten:** Gunakan palet biru-hijau untuk memory, merah untuk warning/error
2. **Font:** Sans-serif modern (Roboto, Open Sans)
3. **Diagram:** Gunakan gaya flat design dengan sedikit shadow
4. **Code:** Gunakan monospace font dengan syntax highlighting
5. **Icon:** Gunakan icon set yang konsisten (Material, Feather)
