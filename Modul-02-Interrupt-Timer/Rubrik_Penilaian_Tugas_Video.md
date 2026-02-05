# Rubrik Penilaian Tugas Video
## Modul 02: Interrupt dan Timer

### Informasi Tugas
| Item | Keterangan |
|------|------------|
| **Judul Tugas** | Video Tutorial: Interrupt dan Timer |
| **Durasi Video** | 8-12 menit |
| **Deadline** | 1 minggu setelah praktikum |
| **Format** | MP4, minimal 720p |
| **Bobot Nilai** | 20% dari Nilai Modul |

---

## A. Konten dan Penjelasan (40%)

### A1. Penjelasan Konsep Teori (15%)

| Skor | Kriteria | Deskripsi |
|------|----------|-----------|
| **14-15** | Excellent | Menjelaskan perbedaan polling vs interrupt dengan analogi yang tepat, arsitektur NVIC/Interrupt Matrix, konsep timer dengan rumus perhitungan, perbandingan STM32 vs ESP32 |
| **11-13** | Good | Menjelaskan konsep utama dengan benar, minor gaps dalam detail |
| **8-10** | Satisfactory | Penjelasan dasar, beberapa konsep kurang mendalam |
| **5-7** | Needs Improvement | Penjelasan tidak lengkap atau terdapat kesalahan konsep |
| **0-4** | Poor | Tidak ada penjelasan teori atau banyak kesalahan |

**Checklist Konten Teori:**
- [ ] Definisi dan kegunaan interrupt
- [ ] Polling vs Interrupt (dengan diagram/animasi)
- [ ] Alur proses interrupt (context save → ISR → restore)
- [ ] Jenis trigger: Rising, Falling, Change
- [ ] Konsep timer: Prescaler, Counter, Auto-reload
- [ ] Rumus perhitungan timer
- [ ] IRAM_ATTR dan mengapa diperlukan (ESP32)
- [ ] Perbandingan implementasi STM32 vs ESP32

### A2. Demonstrasi Praktikum (15%)

| Skor | Kriteria | Deskripsi |
|------|----------|-----------|
| **14-15** | Excellent | Demo lengkap external interrupt, timer interrupt, debouncing, dan timing measurement. Menunjukkan hasil di serial monitor dengan penjelasan setiap output |
| **11-13** | Good | Demo mayoritas fitur dengan penjelasan memadai |
| **8-10** | Satisfactory | Demo basic dengan penjelasan singkat |
| **5-7** | Needs Improvement | Demo tidak lengkap atau tanpa penjelasan |
| **0-4** | Poor | Tidak ada demonstrasi praktis |

**Checklist Demo:**
- [ ] Demo external interrupt dengan button → LED toggle
- [ ] Demo timer interrupt dengan LED blink tanpa delay()
- [ ] Demo debouncing (before vs after)
- [ ] Demo multi-button handler
- [ ] Serial monitor output explanation
- [ ] Code walkthrough dengan highlight bagian penting

### A3. Analisis dan Kesimpulan (10%)

| Skor | Kriteria | Deskripsi |
|------|----------|-----------|
| **9-10** | Excellent | Analisis response time dengan data, comparison STM32 vs ESP32 performance, best practices summary, challenges faced dan solutions |
| **7-8** | Good | Analisis memadai dengan data pendukung |
| **5-6** | Satisfactory | Analisis basic tanpa data kuantitatif |
| **3-4** | Needs Improvement | Minimal analysis |
| **0-2** | Poor | Tidak ada analisis atau kesimpulan |

**Checklist Analisis:**
- [ ] Data response time measurement
- [ ] Perbandingan accuracy timer
- [ ] Best practices yang dipelajari
- [ ] Common mistakes dan cara menghindari
- [ ] Ringkasan pembelajaran

---

## B. Kualitas Teknis Video (25%)

### B1. Kualitas Visual (10%)

| Skor | Kriteria | Deskripsi |
|------|----------|-----------|
| **9-10** | Excellent | Resolusi minimal 720p, framing tepat, lighting bagus, close-up saat menunjukkan detail, screen recording jelas terbaca |
| **7-8** | Good | Kualitas baik dengan minor issues (sedikit blur, lighting kurang optimal) |
| **5-6** | Satisfactory | Video acceptable, konten masih bisa dipahami |
| **3-4** | Needs Improvement | Kualitas rendah tapi masih bisa dilihat |
| **0-2** | Poor | Tidak bisa dilihat dengan jelas |

**Technical Requirements:**
- Resolusi: Minimal 720p (1280x720)
- Frame rate: Minimal 24fps
- Format: MP4 (H.264)
- Screen recording: Teks code harus readable

### B2. Kualitas Audio (10%)

| Skor | Kriteria | Deskripsi |
|------|----------|-----------|
| **9-10** | Excellent | Audio jernih tanpa noise, volume konsisten, tidak ada distorsi, narasi mudah didengar |
| **7-8** | Good | Audio jelas dengan background noise minimal |
| **5-6** | Satisfactory | Audio acceptable, kadang kurang jelas |
| **3-4** | Needs Improvement | Audio ada tapi banyak noise |
| **0-2** | Poor | Tidak bisa didengar dengan jelas |

**Audio Requirements:**
- Clear narration tanpa echo
- Konsisten volume level
- Minimal background noise
- Gunakan microphone eksternal jika memungkinkan

### B3. Editing dan Produksi (5%)

| Skor | Kriteria | Deskripsi |
|------|----------|-----------|
| **5** | Excellent | Smooth transitions, text overlays untuk poin penting, intro/outro, timestamps, zoom effect saat perlu |
| **4** | Good | Editing rapi dengan beberapa enhancement |
| **3** | Satisfactory | Basic editing, potong bagian tidak perlu |
| **2** | Needs Improvement | Minimal editing |
| **0-1** | Poor | Tidak ada editing, raw footage |

**Editing Checklist:**
- [ ] Intro dengan judul dan nama
- [ ] Timestamps/chapter markers
- [ ] Text overlay untuk poin penting
- [ ] Zoom/highlight saat menunjukkan detail
- [ ] Smooth transitions antar section
- [ ] Outro dengan kesimpulan

---

## C. Penyampaian dan Presentasi (25%)

### C1. Kejelasan Penyampaian (10%)

| Skor | Kriteria | Deskripsi |
|------|----------|-----------|
| **9-10** | Excellent | Artikulasi jelas, pace appropriate (tidak terlalu cepat/lambat), penjelasan mudah dipahami, menggunakan bahasa yang tepat |
| **7-8** | Good | Penyampaian jelas dengan minor issues |
| **5-6** | Satisfactory | Cukup jelas tapi perlu improvement |
| **3-4** | Needs Improvement | Sulit dipahami, terlalu cepat/lambat |
| **0-2** | Poor | Tidak bisa dipahami |

### C2. Struktur dan Organisasi (10%)

| Skor | Kriteria | Deskripsi |
|------|----------|-----------|
| **9-10** | Excellent | Alur logis: Intro → Teori → Demo → Analisis → Kesimpulan. Transisi smooth, setiap section jelas |
| **7-8** | Good | Struktur baik dengan minor jumps |
| **5-6** | Satisfactory | Struktur ada tapi kurang organized |
| **3-4** | Needs Improvement | Tidak terstruktur, jumping topics |
| **0-2** | Poor | Tidak ada struktur jelas |

**Suggested Structure:**
1. **Opening (30-60 detik)**: Intro, overview
2. **Teori (2-3 menit)**: Konsep interrupt dan timer
3. **Demo STM32 (2-3 menit)**: Hardware setup, code, hasil
4. **Demo ESP32 (2-3 menit)**: Hardware setup, code, hasil
5. **Analisis (1-2 menit)**: Comparison, findings
6. **Closing (30-60 detik)**: Summary, references

### C3. Durasi dan Time Management (5%)

| Skor | Kriteria | Deskripsi |
|------|----------|-----------|
| **5** | Excellent | 8-12 menit, setiap section proporsional, tidak ada filler content |
| **4** | Good | Sedikit over/under time tapi proporsional |
| **3** | Satisfactory | Durasi acceptable tapi distribusi tidak merata |
| **2** | Needs Improvement | Terlalu panjang (>15 min) atau pendek (<6 min) |
| **0-1** | Poor | Durasi sangat tidak sesuai (<4 min atau >20 min) |

---

## D. Kreativitas dan Inovasi (10%)

| Skor | Kriteria | Deskripsi |
|------|----------|-----------|
| **9-10** | Excellent | Menggunakan animasi/diagram custom, real-world analogy yang tepat, demo aplikasi tambahan, oscilloscope/logic analyzer untuk timing verification |
| **7-8** | Good | Beberapa elemen kreatif yang menambah pemahaman |
| **5-6** | Satisfactory | Standard presentation tanpa elemen tambahan |
| **3-4** | Needs Improvement | Monoton tanpa variasi |
| **0-2** | Poor | Tidak ada usaha untuk engage audience |

**Creative Elements yang Diapresiasi:**
- Custom animation untuk menjelaskan interrupt flow
- Real hardware oscilloscope demo untuk timing
- Side-by-side comparison STM32 vs ESP32
- Practical application demo (stopwatch, frequency counter)
- Interactive elements atau quiz questions
- Professional graphics dan diagrams

---

## Ringkasan Bobot Penilaian

| Komponen | Sub-komponen | Bobot | Max Skor |
|----------|--------------|-------|----------|
| **A. Konten** | | **40%** | **40** |
| | A1. Penjelasan Teori | 15% | 15 |
| | A2. Demo Praktikum | 15% | 15 |
| | A3. Analisis | 10% | 10 |
| **B. Teknis** | | **25%** | **25** |
| | B1. Visual | 10% | 10 |
| | B2. Audio | 10% | 10 |
| | B3. Editing | 5% | 5 |
| **C. Penyampaian** | | **25%** | **25** |
| | C1. Kejelasan | 10% | 10 |
| | C2. Struktur | 10% | 10 |
| | C3. Durasi | 5% | 5 |
| **D. Kreativitas** | | **10%** | **10** |
| **TOTAL** | | **100%** | **100** |

---

## Bonus Points (max +5%)

| Bonus | Poin | Kriteria |
|-------|------|----------|
| Oscilloscope Demo | +2 | Demonstrasi timing dengan oscilloscope/logic analyzer |
| Bilingual | +2 | Video dalam Bahasa Indonesia dan English subtitle |
| Advanced Demo | +1 | Demo aplikasi lanjutan (frequency counter, reaction time, dll) |

---

## Penalti

| Pelanggaran | Penalti |
|-------------|---------|
| Late submission | -10% per hari (max -30%) |
| Durasi <5 menit | -10% |
| Durasi >15 menit | -5% |
| No face/voice (screen only) | -10% |
| Plagiarism (copy video lain) | -100% (0 total) |
| Portrait orientation | -5% |

---

## Kriteria Kelulusan Video

| Grade | Range | Keterangan |
|-------|-------|------------|
| A | 85-100 | Video berkualitas tinggi, dapat dijadikan referensi |
| B | 70-84 | Video baik, memenuhi semua requirement utama |
| C | 55-69 | Video acceptable, perlu improvement di beberapa area |
| D | 40-54 | Video di bawah standar, major improvement needed |
| E | <40 | Video tidak memenuhi requirement minimum |

**Minimum untuk lulus: C (55 poin)**

---

## Submission Requirements

1. **File Format**: MP4 (H.264 codec)
2. **Resolusi**: Minimal 720p (1280x720)
3. **Durasi**: 8-12 menit
4. **File Name**: `Modul02_NIM_Nama.mp4`
5. **Upload**: Google Drive/YouTube (unlisted) dengan link sharing
6. **Deadline**: 1 minggu setelah praktikum

---

## Form Penilaian Video

**Nama/NIM:** _______________________________________________

**Link Video:** _____________________________________________

**Durasi:** _________________________________________________

| Komponen | Skor | Catatan |
|----------|------|---------|
| A1. Penjelasan Teori | /15 | |
| A2. Demo Praktikum | /15 | |
| A3. Analisis | /10 | |
| B1. Kualitas Visual | /10 | |
| B2. Kualitas Audio | /10 | |
| B3. Editing | /5 | |
| C1. Kejelasan | /10 | |
| C2. Struktur | /10 | |
| C3. Durasi | /5 | |
| D. Kreativitas | /10 | |
| **Subtotal** | **/100** | |
| Bonus | /+5 | |
| Penalti | | |
| **TOTAL** | | |

**Catatan Penilai:**

___________________________________________________________

___________________________________________________________

**Tanggal Penilaian:** ______________________________________

**Penilai:** _______________________________________________
