# JOBSHEET BAB 12: FreeRTOS Memory Management & Advanced Features

## 📋 Informasi Praktikum

| Item | Keterangan |
|------|------------|
| **Topik** | FreeRTOS Memory Management & Advanced Features |
| **Platform** | STM32F103C8T6 (Blue Pill), ESP32 DevKit V1 |
| **Framework** | Arduino + FreeRTOS |
| **IDE** | PlatformIO / VS Code |
| **Jumlah Program STM32** | 10 Program |
| **Jumlah Program ESP32** | 10 Program |
| **Durasi** | 3 x 50 menit |

---

## 🎯 Tujuan Praktikum

Setelah menyelesaikan praktikum ini, mahasiswa mampu:

1. Memahami dan mengimplementasikan berbagai heap allocation schemes
2. Menerapkan stack overflow detection dan hook functions
3. Menggunakan Event Groups untuk sinkronisasi multi-task
4. Mengimplementasikan Stream Buffer dan Message Buffer
5. Melakukan memory debugging dan monitoring
6. Menerapkan static allocation untuk task dan resources
7. Mengimplementasikan low power features dengan tickless idle

---

## 🔧 Peralatan yang Dibutuhkan

### Hardware
- 1x STM32F103C8T6 Blue Pill
- 1x ESP32 DevKit V1
- 1x ST-Link V2 Programmer
- 2x LED (berbeda warna)
- 2x Resistor 220Ω
- 1x Push Button
- 1x Resistor 10kΩ (pull-up)
- 1x Potentiometer 10kΩ
- Breadboard dan kabel jumper
- Kabel USB

### Software
- VS Code + PlatformIO
- Serial Monitor

---

## 📝 PRAKTIKUM STM32

### Program 1: Heap Statistics Monitor

**Tujuan:** Memonitor penggunaan heap memory secara real-time

**Rangkaian:**
```
STM32F103C8T6
┌─────────────────────┐
│                     │
│ PC13 ────[LED]───GND│  (Built-in LED)
│                     │
│ PA9  ────TX→Serial  │
│ PA10 ────RX→Serial  │
└─────────────────────┘
```

### Program 2: Stack High Water Mark Monitor

**Tujuan:** Memantau penggunaan stack setiap task

### Program 3: Stack Overflow Detection

**Tujuan:** Mendeteksi dan menangani stack overflow

### Program 4: Event Groups - System Initialization

**Tujuan:** Sinkronisasi multiple tasks dengan event bits

### Program 5: Event Groups - Task Synchronization (Rendezvous)

**Tujuan:** Menyinkronkan multiple tasks di satu titik

### Program 6: Stream Buffer - UART Data Streaming

**Tujuan:** Menggunakan stream buffer untuk data UART

### Program 7: Message Buffer - Structured Data

**Tujuan:** Mengirim pesan terstruktur antar task

### Program 8: Static Allocation - Tasks and Queues

**Tujuan:** Membuat task dan queue secara statis

### Program 9: Memory Pool Implementation

**Tujuan:** Implementasi custom memory pool

### Program 10: Runtime Statistics

**Tujuan:** Menampilkan CPU usage per task

---

## 📝 PRAKTIKUM ESP32

### Program 1: ESP32 Heap Info and Multi-Heap

**Tujuan:** Memahami struktur heap ESP32 (Internal + PSRAM)

### Program 2: Task Stack Watermark Analysis

**Tujuan:** Menganalisis penggunaan stack task

### Program 3: Stack Overflow Handler ESP32

**Tujuan:** Implementasi stack overflow hook di ESP32

### Program 4: Event Groups - WiFi Connection Events

**Tujuan:** Mengelola event WiFi dengan Event Groups

### Program 5: Event Groups - Multi-Sensor Sync

**Tujuan:** Sinkronisasi pembacaan multiple sensor

### Program 6: Stream Buffer - Sensor Data Streaming

**Tujuan:** Streaming data sensor dengan stream buffer

### Program 7: Message Buffer - Command Parser

**Tujuan:** Parsing command dengan message buffer

### Program 8: Static Allocation ESP32

**Tujuan:** Menggunakan static allocation di ESP32

### Program 9: Light Sleep with FreeRTOS

**Tujuan:** Mengimplementasikan light sleep untuk hemat power

### Program 10: Deep Sleep with RTC Memory

**Tujuan:** Menyimpan data saat deep sleep

---

## 📊 Tabel Program Praktikum

### STM32 Programs

| No | Nama Program | File | Konsep |
|----|--------------|------|--------|
| 1 | Heap Statistics Monitor | `STM32_01_Heap_Statistics/` | Heap monitoring |
| 2 | Stack High Water Mark | `STM32_02_Stack_Watermark/` | Stack analysis |
| 3 | Stack Overflow Detection | `STM32_03_Stack_Overflow/` | Overflow hook |
| 4 | Event Groups Init | `STM32_04_EventGroup_Init/` | Event bits |
| 5 | Event Groups Sync | `STM32_05_EventGroup_Sync/` | Rendezvous |
| 6 | Stream Buffer UART | `STM32_06_StreamBuffer/` | Byte streaming |
| 7 | Message Buffer | `STM32_07_MessageBuffer/` | Message framing |
| 8 | Static Allocation | `STM32_08_Static_Alloc/` | Compile-time alloc |
| 9 | Memory Pool | `STM32_09_Memory_Pool/` | Custom allocator |
| 10 | Runtime Stats | `STM32_10_Runtime_Stats/` | CPU statistics |

### ESP32 Programs

| No | Nama Program | File | Konsep |
|----|--------------|------|--------|
| 1 | ESP32 Heap Info | `ESP32_01_Heap_Info/` | Multi-heap |
| 2 | Stack Watermark | `ESP32_02_Stack_Watermark/` | Stack monitor |
| 3 | Stack Overflow | `ESP32_03_Stack_Overflow/` | ESP32 hook |
| 4 | Event WiFi | `ESP32_04_Event_WiFi/` | WiFi events |
| 5 | Multi-Sensor Sync | `ESP32_05_MultiSensor_Sync/` | Sensor sync |
| 6 | Stream Buffer Sensor | `ESP32_06_StreamBuffer/` | Sensor streaming |
| 7 | Message Buffer CMD | `ESP32_07_MessageBuffer/` | Command parser |
| 8 | Static Allocation | `ESP32_08_Static_Alloc/` | Static resources |
| 9 | Light Sleep | `ESP32_09_Light_Sleep/` | Power saving |
| 10 | Deep Sleep RTC | `ESP32_10_Deep_Sleep/` | RTC memory |

---

## 📝 Tugas Praktikum

### Tugas 1: Implementasi dan Dokumentasi (40%)
Implementasikan minimal 5 program STM32 dan 5 program ESP32. Dokumentasikan:
- Screenshot hasil serial monitor
- Penjelasan alur program
- Analisis penggunaan memory

### Tugas 2: Analisis Memory (30%)
Buat tabel perbandingan:
- Heap usage sebelum dan sesudah membuat tasks
- Stack high water mark setiap task
- Perbedaan static vs dynamic allocation

### Tugas 3: Modifikasi dan Eksperimen (30%)
Pilih salah satu eksperimen:
1. Modifikasi stack size dan amati high water mark
2. Trigger stack overflow dan implementasi recovery
3. Bandingkan Stream Buffer vs Queue untuk throughput
4. Ukur power consumption dengan dan tanpa tickless idle

---

## 📊 Rubrik Penilaian

| Komponen | Bobot | Kriteria |
|----------|-------|----------|
| Implementasi | 40% | Program berjalan tanpa error |
| Dokumentasi | 25% | Screenshot, penjelasan lengkap |
| Analisis | 20% | Pemahaman konsep memory |
| Modifikasi | 15% | Kreativitas dan kedalaman eksperimen |

---

## ⚠️ Catatan Penting

1. **Stack Size:** Mulai dengan nilai konservatif (512 words), lalu optimize berdasarkan high water mark
2. **Heap Size:** Pastikan `configTOTAL_HEAP_SIZE` cukup untuk semua allocations
3. **Stack Overflow:** Selalu enable detection (metode 2) saat development
4. **Static Allocation:** Harus provide memory untuk idle dan timer task
5. **ESP32:** Perhatikan DMA-capable memory regions

---

## 📚 Referensi Tambahan

1. [FreeRTOS Memory Management](https://freertos.org/a00111.html)
2. [FreeRTOS Event Groups](https://freertos.org/FreeRTOS-Event-Groups.html)
3. [ESP32 Memory Types](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/mem_alloc.html)
4. [STM32 Low Power Modes](https://www.st.com/resource/en/application_note/an4621.pdf)
