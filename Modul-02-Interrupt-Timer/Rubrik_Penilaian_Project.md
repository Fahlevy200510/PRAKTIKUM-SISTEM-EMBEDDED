# Rubrik Penilaian Project
## Modul 02: Sistem Monitoring Keamanan Real-Time

### Informasi Project
| Item | Keterangan |
|------|------------|
| **Judul Project** | Real-Time Security Monitoring System |
| **Platform** | STM32F103C8T6 + ESP32 DevKitC |
| **Durasi** | 2 Minggu |
| **Bobot Nilai** | 30% dari Nilai Modul |

---

## A. Implementasi Kode (40%)

### A1. Firmware STM32 - Edge Controller (15%)

| Skor | Kriteria | Deskripsi |
|------|----------|-----------|
| **14-15** | Excellent | Semua EXTI dikonfigurasi dengan benar, NVIC priority management optimal, semua timer berfungsi (timestamp, debounce, heartbeat), interrupt handler efisien (<100µs), komunikasi UART reliable |
| **11-13** | Good | Mayoritas fitur berfungsi, minor issue pada priority atau timing, dokumentasi code memadai |
| **8-10** | Satisfactory | Fitur dasar berfungsi (interrupt dan timer basic), beberapa bug minor, struktur code perlu improvement |
| **5-7** | Needs Improvement | Hanya sebagian fitur berfungsi, terdapat bug signifikan, code tidak terstruktur |
| **0-4** | Poor | Tidak compile atau crash, mayoritas fitur tidak berfungsi |

**Checklist Detail:**
- [ ] EXTI0 (PIR) dengan rising edge detection
- [ ] EXTI1 (Door) dengan change detection
- [ ] EXTI2 (Window) dengan change detection
- [ ] EXTI3 (Panic) dengan falling edge dan highest priority
- [ ] TIM2 untuk timestamp (µs precision)
- [ ] TIM3 untuk debouncing (50ms)
- [ ] TIM4 untuk heartbeat (1Hz)
- [ ] ISR clear flag properly
- [ ] Volatile variables untuk shared data
- [ ] UART TX dengan format protocol yang benar

### A2. Firmware ESP32 - Hub Controller (15%)

| Skor | Kriteria | Deskripsi |
|------|----------|-----------|
| **14-15** | Excellent | Event logging lengkap dengan timestamp, alert management multi-level berfungsi sempurna, UART reception robust dengan checksum validation, timer-based monitoring efektif |
| **11-13** | Good | Mayoritas fitur berfungsi, minor issue pada logging atau alert timing |
| **8-10** | Satisfactory | Fitur dasar berfungsi, alert system basic, logging tanpa analysis |
| **5-7** | Needs Improvement | Hanya menerima data tanpa processing yang memadai |
| **0-4** | Poor | Tidak bisa receive data atau tidak compile |

**Checklist Detail:**
- [ ] UART reception dengan buffer management
- [ ] Checksum validation untuk setiap message
- [ ] Event logging dengan timestamp
- [ ] Alert level management (5 levels)
- [ ] LED indicator sesuai alert level
- [ ] Buzzer control untuk alert/emergency
- [ ] Timer untuk timeout detection
- [ ] IRAM_ATTR untuk semua ISR
- [ ] Critical section untuk shared data
- [ ] Serial output untuk monitoring

### A3. Protokol Komunikasi (5%)

| Skor | Kriteria | Deskripsi |
|------|----------|-----------|
| **5** | Excellent | Protocol well-defined dengan format $EVT dan $ACK, checksum berfungsi, error handling robust |
| **4** | Good | Protocol berfungsi dengan minor issues |
| **3** | Satisfactory | Komunikasi basic tanpa checksum atau ACK |
| **1-2** | Needs Improvement | Komunikasi tidak reliable |
| **0** | Poor | Tidak ada komunikasi |

### A4. Struktur dan Kualitas Code (5%)

| Skor | Kriteria | Deskripsi |
|------|----------|-----------|
| **5** | Excellent | Modular design (separate files), naming convention konsisten, comprehensive comments, no hardcoded values |
| **4** | Good | Struktur baik dengan minor inconsistencies |
| **3** | Satisfactory | Single file tapi readable, beberapa comments |
| **1-2** | Needs Improvement | Code tidak terstruktur, minimal comments |
| **0** | Poor | Tidak readable, no organization |

---

## B. Dokumentasi Teknis (25%)

### B1. Arsitektur Sistem (8%)

| Skor | Kriteria | Deskripsi |
|------|----------|-----------|
| **7-8** | Excellent | Block diagram lengkap dan akurat, data flow jelas, semua komponen terdokumentasi, wiring diagram professional |
| **5-6** | Good | Diagram lengkap dengan minor missing details |
| **3-4** | Satisfactory | Diagram basic, beberapa komponen tidak dijelaskan |
| **1-2** | Needs Improvement | Diagram tidak lengkap atau tidak akurat |
| **0** | Poor | Tidak ada dokumentasi arsitektur |

**Checklist:**
- [ ] System block diagram
- [ ] Detailed wiring diagram
- [ ] Pin assignment table
- [ ] Communication flow diagram
- [ ] Component list dengan specifications

### B2. Timing Analysis (10%)

| Skor | Kriteria | Deskripsi |
|------|----------|-----------|
| **9-10** | Excellent | Timing diagram lengkap untuk setiap interrupt, latency measurement data, response time analysis, optimization discussion |
| **7-8** | Good | Timing diagram memadai, data pengukuran ada |
| **5-6** | Satisfactory | Timing basic tanpa detailed analysis |
| **3-4** | Needs Improvement | Minimal timing documentation |
| **0-2** | Poor | Tidak ada timing analysis |

**Checklist:**
- [ ] Interrupt latency diagram
- [ ] Timer accuracy measurement
- [ ] Response time data per sensor
- [ ] Worst-case scenario analysis
- [ ] ISR execution time measurement

### B3. Protocol Specification (7%)

| Skor | Kriteria | Deskripsi |
|------|----------|-----------|
| **7** | Excellent | Format message lengkap, state diagram, error handling documented, example messages |
| **5-6** | Good | Specification memadai |
| **3-4** | Satisfactory | Basic format tanpa detail |
| **1-2** | Needs Improvement | Minimal specification |
| **0** | Poor | Tidak ada specification |

---

## C. Laporan Project (20%)

### C1. Abstrak dan Pendahuluan (4%)

| Skor | Kriteria | Deskripsi |
|------|----------|-----------|
| **4** | Excellent | Abstrak 200 kata mencakup tujuan, metodologi, hasil. Pendahuluan dengan background, problem statement, objectives jelas |
| **3** | Good | Lengkap dengan minor improvement needed |
| **2** | Satisfactory | Cukup tapi kurang detail |
| **1** | Needs Improvement | Tidak lengkap |
| **0** | Poor | Tidak ada atau copy-paste |

### C2. Metodologi (5%)

| Skor | Kriteria | Deskripsi |
|------|----------|-----------|
| **5** | Excellent | Design process lengkap, tools & materials, step-by-step implementation, testing methodology |
| **4** | Good | Metodologi jelas dengan minor gaps |
| **3** | Satisfactory | Basic methodology |
| **1-2** | Needs Improvement | Tidak structured |
| **0** | Poor | Tidak ada methodology |

### C3. Hasil dan Analisis (8%)

| Skor | Kriteria | Deskripsi |
|------|----------|-----------|
| **7-8** | Excellent | Data lengkap dengan tabel/grafik, analysis mendalam, comparison STM32 vs ESP32, discussion tentang findings |
| **5-6** | Good | Hasil lengkap dengan analysis memadai |
| **3-4** | Satisfactory | Hasil ada tanpa deep analysis |
| **1-2** | Needs Improvement | Hasil minimal |
| **0** | Poor | Tidak ada hasil |

### C4. Kesimpulan (3%)

| Skor | Kriteria | Deskripsi |
|------|----------|-----------|
| **3** | Excellent | Summary pembelajaran, challenges overcome, future improvements suggestions |
| **2** | Good | Kesimpulan memadai |
| **1** | Satisfactory | Kesimpulan singkat |
| **0** | Poor | Tidak ada kesimpulan |

---

## D. Video Demonstrasi (15%)

### D1. Konten Video (8%)

| Skor | Kriteria | Deskripsi |
|------|----------|-----------|
| **7-8** | Excellent | System overview (1-2 min), hardware demo (2-3 min), interrupt demo dengan priority test (2-3 min), timer accuracy demo (1-2 min), full integration (2-3 min). Total 8-13 menit |
| **5-6** | Good | Semua section ada dengan minor gaps |
| **3-4** | Satisfactory | Demo basic tanpa detailed explanation |
| **1-2** | Needs Improvement | Terlalu singkat atau tidak lengkap |
| **0** | Poor | Tidak ada video |

**Required Demo Sections:**
- [ ] System architecture explanation
- [ ] Hardware connection walkthrough
- [ ] Interrupt priority demonstration
- [ ] Timer accuracy verification
- [ ] Full system operation

### D2. Kualitas Teknis (4%)

| Skor | Kriteria | Deskripsi |
|------|----------|-----------|
| **4** | Excellent | Video 720p+, audio clear, good lighting, close-up saat diperlukan |
| **3** | Good | Quality memadai |
| **2** | Satisfactory | Acceptable quality |
| **1** | Needs Improvement | Poor quality tapi masih bisa dipahami |
| **0** | Poor | Tidak bisa dilihat/didengar |

### D3. Presentasi (3%)

| Skor | Kriteria | Deskripsi |
|------|----------|-----------|
| **3** | Excellent | Penjelasan jelas, tempo appropriate, semua anggota contribute |
| **2** | Good | Penjelasan memadai |
| **1** | Satisfactory | Basic explanation |
| **0** | Poor | Tidak ada narration |

---

## E. Bonus Points (max +10%)

| Bonus | Poin | Kriteria |
|-------|------|----------|
| Extra Feature | +3 | Implementasi fitur tambahan (WiFi alert, LCD display, etc.) |
| Code Optimization | +2 | Demonstrasi interrupt latency optimization |
| Extended Testing | +2 | Stress test dan stability analysis |
| Professional Documentation | +2 | LaTeX report, professional diagrams |
| Early Submission | +1 | Submit 3+ hari sebelum deadline |

---

## Ringkasan Bobot Penilaian

| Komponen | Bobot | Max Skor |
|----------|-------|----------|
| A. Implementasi Kode | 40% | 40 |
| B. Dokumentasi Teknis | 25% | 25 |
| C. Laporan Project | 20% | 20 |
| D. Video Demonstrasi | 15% | 15 |
| **Total** | **100%** | **100** |
| E. Bonus | +10% | +10 |

---

## Kriteria Kelulusan

| Grade | Range | Keterangan |
|-------|-------|------------|
| A | 85-100 | Excellent - Semua requirement terpenuhi dengan kualitas tinggi |
| B | 70-84 | Good - Mayoritas requirement terpenuhi dengan baik |
| C | 55-69 | Satisfactory - Minimum requirement terpenuhi |
| D | 40-54 | Needs Improvement - Perlu perbaikan signifikan |
| E | <40 | Failed - Tidak memenuhi minimum requirement |

**Minimum untuk lulus: C (55 poin)**

---

## Catatan Penting

1. **Plagiarisme**: Copy-paste code tanpa pemahaman = 0 untuk seluruh project
2. **Late Submission**: -10% per hari keterlambatan (max -30%)
3. **Tidak ada video**: Maximum nilai 70%
4. **Code tidak compile**: Maximum nilai untuk implementasi 50%
5. **Demonstrasi gagal**: Wajib troubleshoot saat demo atau reschedule

---

## Form Penilaian

**Nama/NIM:** _______________________________________________

**Tanggal Penilaian:** _______________________________________

| Komponen | Skor | Catatan |
|----------|------|---------|
| A1. STM32 Firmware | /15 | |
| A2. ESP32 Firmware | /15 | |
| A3. Protokol | /5 | |
| A4. Kualitas Code | /5 | |
| B1. Arsitektur | /8 | |
| B2. Timing Analysis | /10 | |
| B3. Protocol Spec | /7 | |
| C1. Abstrak | /4 | |
| C2. Metodologi | /5 | |
| C3. Hasil | /8 | |
| C4. Kesimpulan | /3 | |
| D1. Konten Video | /8 | |
| D2. Kualitas Teknis | /4 | |
| D3. Presentasi | /3 | |
| **Subtotal** | **/100** | |
| Bonus | /+10 | |
| Penalty | | |
| **TOTAL** | | |

**Penilai:** _______________________________________________

**Tanda Tangan:** _________________________________________
