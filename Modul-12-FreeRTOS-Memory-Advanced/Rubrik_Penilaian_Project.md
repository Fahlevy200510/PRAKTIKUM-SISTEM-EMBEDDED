# Rubrik Penilaian Project
## Modul 12: FreeRTOS Memory Management & Advanced Features

### 📋 Informasi Project

| Item | Keterangan |
|------|------------|
| **Nama Project** | Smart Industrial Data Logger dengan Memory Optimization |
| **Platform** | STM32F103C8T6 + ESP32 DevKit V1 |
| **Bobot Nilai** | 40% dari total nilai modul |
| **Deadline** | Minggu ke-5 setelah modul dimulai |

---

## 📊 Rubrik Penilaian Detail

### A. Memory Management Implementation (30%)

| Kriteria | Excellent (90-100%) | Good (70-89%) | Fair (50-69%) | Poor (<50%) |
|----------|---------------------|---------------|---------------|-------------|
| **Static Allocation STM32** | Semua task, queue, semaphore menggunakan static allocation. Memory pool diimplementasikan dengan benar | Static allocation untuk task saja. Pool tidak ada atau partial | Hanya 1-2 resource static. Tidak ada pool | Dynamic allocation semua |
| **Heap Monitoring ESP32** | HeapStats lengkap, periodic monitoring, alert pada low memory, fragmentation tracking | Basic heap monitoring, periodic logging | Hanya free heap size | Tidak ada monitoring |
| **Memory Pool** | Pool diimplementasikan dengan mutex, O(1) allocation, statistics tracking | Pool berfungsi tapi tidak thread-safe | Basic pool tanpa thread safety | Tidak diimplementasikan |

**Poin Detail:**
- Static Task Creation: 10 poin
- Static Queue/Semaphore: 5 poin
- Memory Pool Implementation: 10 poin
- Heap Monitoring Dashboard: 5 poin

---

### B. Stack Protection (15%)

| Kriteria | Excellent (90-100%) | Good (70-89%) | Fair (50-69%) | Poor (<50%) |
|----------|---------------------|---------------|---------------|-------------|
| **Detection Method** | Method 2 (pattern checking) dengan custom hook yang melakukan logging dan recovery | Method 2 dengan basic hook | Method 1 dengan basic hook | Tidak ada detection |
| **Hook Implementation** | Hook logs ke non-volatile memory, notifies user, attempts graceful recovery | Hook logs dan blinks LED | Hook hanya print message | Hook kosong |
| **Stack Sizing** | High water mark monitoring untuk semua task, dokumentasi sizing rationale | High water mark untuk beberapa task | Basic sizing tanpa monitoring | Default sizing tanpa analisis |

**Poin Detail:**
- Detection Configuration: 5 poin
- Hook Function Quality: 5 poin
- High Water Mark Analysis: 5 poin

---

### C. Event Groups Implementation (20%)

| Kriteria | Excellent (90-100%) | Good (70-89%) | Fair (50-69%) | Poor (<50%) |
|----------|---------------------|---------------|---------------|-------------|
| **Event Definition** | Minimal 8 event bits terdefinisi dengan jelas, dokumentasi setiap bit | 5-7 event bits | 3-4 event bits | <3 events |
| **Wait Operations** | Menggunakan wait ALL dan wait ANY sesuai kebutuhan, proper timeout handling | Menggunakan keduanya tapi tidak optimal | Hanya satu jenis wait | Tidak ada wait |
| **Synchronization** | Task rendezvous diimplementasikan, multiple sync points | Basic synchronization | Simple set/wait pattern | Tidak ada sync |
| **State Machine** | Event-driven state machine lengkap dengan diagram | Partial state machine | Basic state handling | Tidak ada state |

**Poin Detail:**
- Event Bit Architecture: 5 poin
- Wait Operations: 5 poin
- Task Sync (Rendezvous): 5 poin
- State Management: 5 poin

---

### D. Buffer Communication (20%)

| Kriteria | Excellent (90-100%) | Good (70-89%) | Fair (50-69%) | Poor (<50%) |
|----------|---------------------|---------------|---------------|-------------|
| **Stream Buffer** | Stream buffer untuk UART data dengan ISR-safe operations, proper trigger level | Stream buffer berfungsi, basic ISR | Stream buffer tanpa ISR | Tidak digunakan |
| **Message Buffer** | Structured messages dengan checksum, variable length support | Fixed size messages, no checksum | Basic message passing | Tidak digunakan |
| **Protocol Design** | Protokol lengkap dengan header, payload, checksum, error handling | Protokol dengan checksum | Basic message format | No protocol |
| **Throughput** | Documented throughput testing, optimized for use case | Basic throughput measurement | No throughput analysis | Poor performance |

**Poin Detail:**
- Stream Buffer Usage: 5 poin
- Message Buffer Usage: 5 poin
- Protocol Design: 5 poin
- ISR Integration: 5 poin

---

### E. Low Power Implementation (15%)

| Kriteria | Excellent (90-100%) | Good (70-89%) | Fair (50-69%) | Poor (<50%) |
|----------|---------------------|---------------|---------------|-------------|
| **Tickless Idle STM32** | Tickless idle dengan proper timer compensation, measured power savings | Tickless enabled, basic operation | Tickless enabled tanpa verification | Tidak enabled |
| **Light Sleep ESP32** | Light sleep dengan WiFi maintained, GPIO wakeup configured | Light sleep basic | Light sleep tanpa wakeup config | Tidak diimplementasikan |
| **Adaptive Sleep** | Sleep duration adaptive berdasarkan battery level dan activity | Fixed sleep with battery consideration | Fixed sleep duration | No sleep optimization |
| **Power Measurement** | Actual current measurements documented, before/after comparison | Basic power measurement | Estimated power savings | No measurement |

**Poin Detail:**
- STM32 Tickless Idle: 4 poin
- ESP32 Light Sleep: 4 poin
- Adaptive Algorithm: 4 poin
- Power Documentation: 3 poin

---

## 🌟 Bonus Points (Maksimal +40%)

### Bonus 1: Memory Leak Detection (+10%)
| Kriteria | Full Points | Partial | None |
|----------|-------------|---------|------|
| Implementation | Tracking system dengan file/line info, leak report function | Basic tracking | 0 |

### Bonus 2: Fragmentation Monitor (+10%)
| Kriteria | Full Points | Partial | None |
|----------|-------------|---------|------|
| Implementation | Fragmentation index calculation, alert system | Basic fragmentation check | 0 |

### Bonus 3: Adaptive Sleep (+10%)
| Kriteria | Full Points | Partial | None |
|----------|-------------|---------|------|
| Implementation | Multiple factors (battery, activity, time) | Single factor adaptive | 0 |

### Bonus 4: MQTT Memory Stats (+10%)
| Kriteria | Full Points | Partial | None |
|----------|-------------|---------|------|
| Implementation | Real-time stats to cloud, dashboard integration | Basic MQTT publish | 0 |

---

## 📝 Dokumentasi (Wajib)

| Komponen | Bobot | Kriteria Lengkap |
|----------|-------|------------------|
| README.md | 5% | Setup instructions, dependencies, wiring |
| Architecture Diagram | 5% | System overview, data flow, communication |
| Memory Analysis | 5% | Before/after optimization, heap graphs |
| State Diagram | 5% | Event-driven states, transitions |
| Testing Report | 5% | Test cases, results, screenshots |

---

## 🎬 Video Demonstrasi (Wajib)

| Aspek | Bobot | Kriteria |
|-------|-------|----------|
| System Overview | 5% | Clear explanation of architecture |
| Live Demo | 10% | Working system dengan Serial Monitor |
| Memory Stats | 5% | Show heap/stack monitoring |
| Event Demo | 5% | Demonstrate event-driven behavior |
| Power Demo | 5% | Show power measurement |

**Durasi:** 5-10 menit
**Format:** MP4 atau link YouTube (unlisted OK)

---

## 📊 Perhitungan Nilai Akhir

```
Nilai Project = (A×0.30 + B×0.15 + C×0.20 + D×0.20 + E×0.15) + Bonus

Dimana:
A = Memory Management Score (0-100)
B = Stack Protection Score (0-100)
C = Event Groups Score (0-100)
D = Buffer Communication Score (0-100)
E = Low Power Score (0-100)
Bonus = 0-40 additional points

Nilai Maksimal = 100 + 40 = 140 (dinormalisasi ke 100 jika >100)
```

---

## 📅 Milestone dan Checkpoint

| Minggu | Checkpoint | Deliverable |
|--------|------------|-------------|
| 1 | Hardware Setup | Foto rangkaian, basic UART test |
| 2 | Memory Implementation | Static allocation working, heap monitoring |
| 3 | Event & Buffer | Event groups, buffer communication working |
| 4 | Low Power & Integration | Full system integration |
| 5 | Final | Video, documentation, code submission |

---

## ⚠️ Penalti

| Pelanggaran | Penalti |
|-------------|---------|
| Terlambat 1-3 hari | -10% |
| Terlambat 4-7 hari | -25% |
| Terlambat >7 hari | Tidak diterima |
| Plagiarisme | -100% + laporan |
| Tidak ada video | -30% |
| Tidak ada dokumentasi | -25% |

---

## 📋 Checklist Submission

- [ ] Source code STM32 (PlatformIO project)
- [ ] Source code ESP32 (PlatformIO project)
- [ ] Wiring diagram
- [ ] Foto hardware setup
- [ ] README.md dengan setup instructions
- [ ] Architecture diagram
- [ ] Memory analysis document
- [ ] Testing report dengan screenshots
- [ ] Video demonstrasi (5-10 menit)
- [ ] Presentasi slides (optional)
