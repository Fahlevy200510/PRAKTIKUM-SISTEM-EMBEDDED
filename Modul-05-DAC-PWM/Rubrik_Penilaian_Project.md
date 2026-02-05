# Rubrik Penilaian Project
## Modul 05: DAC & PWM - Smart Audio-Visual Controller

### 📋 Informasi Penilaian

| Item | Detail |
|------|--------|
| **Nama Project** | Smart Audio-Visual Controller |
| **Platform** | STM32F103 + ESP32 (Dual MCU) |
| **Bobot dalam Nilai Akhir** | 30% |
| **Penilai** | Dosen Pengampu + Asisten Lab |

---

## 🎯 Komponen Penilaian

### A. Fungsionalitas Sistem (35%)

#### A1. DAC Audio Generator - STM32 (15%)

| Kriteria | Excellent (4) | Good (3) | Fair (2) | Poor (1) | Score |
|----------|---------------|----------|----------|----------|-------|
| **Waveform Generation** | Semua 4 waveform (sine, square, triangle, sawtooth) bekerja sempurna | 3 waveform bekerja | 2 waveform bekerja | 1 atau tidak ada | |
| **Frequency Control** | Range 20Hz-2kHz, smooth transition | Range terbatas tapi berfungsi | Frequency tetap | Tidak berfungsi | |
| **Volume Control** | Level 0-100% dengan resolusi baik | Volume control ada, resolusi terbatas | Volume hanya on/off | Tidak ada kontrol | |
| **DMA Operation** | DAC dengan DMA, CPU-free | DAC dengan polling tapi smooth | DAC dengan delay | Tidak ada output | |
| **Audio Quality** | Output bersih, minimal noise | Sedikit noise | Noise cukup banyak | Sangat noisy/distorsi | |

**Subtotal A1:** ___/20 × 0.75 = ___/15

#### A2. PWM Visual Controller - ESP32 (12%)

| Kriteria | Excellent (4) | Good (3) | Fair (2) | Poor (1) | Score |
|----------|---------------|----------|----------|----------|-------|
| **LED RGB Control** | Full RGB dengan smooth color transition | RGB bekerja, transisi kurang smooth | Hanya single color | Tidak berfungsi | |
| **Motor Control** | Speed 0-100% dengan soft-start | Speed control ada | Motor hanya on/off | Tidak berfungsi | |
| **Servo Control** | 0-180° dengan posisi akurat | Servo bergerak tapi kurang akurat | Servo terbatas | Tidak berfungsi | |
| **Hardware Fade** | Menggunakan LEDC hardware fade | Software fade smooth | Software fade jerky | Tidak ada fade | |

**Subtotal A2:** ___/16 × 0.75 = ___/12

#### A3. Komunikasi & Integrasi (8%)

| Kriteria | Excellent (4) | Good (3) | Fair (2) | Poor (1) | Score |
|----------|---------------|----------|----------|----------|-------|
| **UART Protocol** | Protokol custom dengan checksum, reliable | Protokol sederhana, reliable | Komunikasi kadang error | Tidak bisa berkomunikasi | |
| **Real-time Sync** | Visual sync dengan audio < 50ms latency | Sync dengan latency noticeable | Sync tapi tidak real-time | Tidak sync | |

**Subtotal A3:** ___/8

---

### B. Kode & Dokumentasi (25%)

#### B1. Kualitas Kode (15%)

| Kriteria | Excellent (4) | Good (3) | Fair (2) | Poor (1) | Score |
|----------|---------------|----------|----------|----------|-------|
| **Code Structure** | Modular, fungsi terpisah jelas, header files | Struktur baik, kurang modular | Semua dalam satu file | Kode berantakan | |
| **Naming Convention** | Konsisten, deskriptif, mengikuti standard | Cukup konsisten | Inkonsisten | Tidak deskriptif | |
| **Comments** | Setiap fungsi ada doxygen comments | Comment cukup lengkap | Comment minimal | Tidak ada comment | |
| **Error Handling** | Semua error ditangani, graceful degradation | Error handling ada | Minimal error handling | Tidak ada | |
| **Efficiency** | Optimal, tidak ada blocking yang tidak perlu | Cukup efisien | Ada blocking | Banyak busy-wait | |

**Subtotal B1:** ___/20 × 0.75 = ___/15

#### B2. Dokumentasi (10%)

| Kriteria | Excellent (4) | Good (3) | Fair (2) | Poor (1) | Score |
|----------|---------------|----------|----------|----------|-------|
| **Laporan Teknis** | Lengkap 10-15 halaman, format profesional | Lengkap, format kurang rapi | Kurang lengkap | Tidak ada/sangat minim | |
| **Diagram & Skema** | Skema rangkaian, block diagram, flowchart semua ada | 2 dari 3 ada | 1 ada | Tidak ada | |
| **User Manual** | Manual penggunaan lengkap | Manual singkat | Hanya readme | Tidak ada | |

**Subtotal B2:** ___/12 × 0.83 = ___/10

---

### C. Integrasi Dual-MCU (20%)

| Kriteria | Excellent (4) | Good (3) | Fair (2) | Poor (1) | Score |
|----------|---------------|----------|----------|----------|-------|
| **Hardware Integration** | Wiring rapi, koneksi reliable | Wiring fungsional | Ada loose connection | Tidak terhubung dengan benar | |
| **Protocol Design** | Protokol well-designed, extensible | Protokol berfungsi | Protokol minimal | Tidak ada protokol | |
| **Bidirectional Comm** | Data dan command dua arah bekerja | Satu arah bekerja sempurna | Satu arah kadang error | Tidak berkomunikasi | |
| **Error Recovery** | Auto-reconnect, data validation | Basic error detection | Tidak ada recovery | Crash saat error | |
| **Timing Synchronization** | Perfect sync, no visible delay | Slight delay acceptable | Noticeable delay | Not synchronized | |

**Subtotal C:** ___/20

---

### D. Kreativitas & Inovasi (10%)

| Kriteria | Excellent (4) | Good (3) | Fair (2) | Poor (1) | Score |
|----------|---------------|----------|----------|----------|-------|
| **Fitur Tambahan** | 3+ fitur tambahan bermakna | 2 fitur tambahan | 1 fitur tambahan | Sesuai spesifikasi saja | |
| **UI/UX Web Interface** | Desain menarik, responsive, intuitif | Desain baik, kurang polish | Desain basic | Tidak ada web interface | |
| **Problem Solving** | Solusi kreatif untuk challenge | Solusi standard | Solusi copy-paste | Tidak ada solusi | |

**Subtotal D:** ___/12 × 0.83 = ___/10

---

### E. Presentasi & Demo (10%)

| Kriteria | Excellent (4) | Good (3) | Fair (2) | Poor (1) | Score |
|----------|---------------|----------|----------|----------|-------|
| **Video Quality** | HD, audio jelas, editing profesional | Kualitas baik | Kualitas cukup | Kualitas buruk | |
| **Demonstration** | Semua fitur didemonstrasikan jelas | Mayoritas fitur didemonstrasikan | Beberapa fitur saja | Demo minimal | |
| **Explanation** | Penjelasan mendalam, menjawab pertanyaan | Penjelasan cukup | Penjelasan surface level | Tidak bisa menjelaskan | |

**Subtotal E:** ___/12 × 0.83 = ___/10

---

## 📊 Rekapitulasi Nilai

| Komponen | Bobot | Nilai | Weighted |
|----------|-------|-------|----------|
| A. Fungsionalitas | 35% | ___/35 | ___ |
| B. Kode & Dokumentasi | 25% | ___/25 | ___ |
| C. Integrasi Dual-MCU | 20% | ___/20 | ___ |
| D. Kreativitas | 10% | ___/10 | ___ |
| E. Presentasi | 10% | ___/10 | ___ |
| **TOTAL** | **100%** | | **___/100** |

---

## 📝 Konversi Nilai

| Range Nilai | Grade | Predikat |
|-------------|-------|----------|
| 85 - 100 | A | Excellent |
| 80 - 84 | A- | Very Good |
| 75 - 79 | B+ | Good |
| 70 - 74 | B | Above Average |
| 65 - 69 | B- | Average |
| 60 - 64 | C+ | Below Average |
| 55 - 59 | C | Fair |
| 50 - 54 | D | Poor |
| < 50 | E | Failed |

---

## 🏆 Bonus Points (Max +10)

| Kriteria Bonus | Points |
|----------------|--------|
| Implementasi beat detection | +3 |
| Multi-zone LED control | +2 |
| Mobile app (Android/iOS) | +5 |
| PCB custom design | +3 |
| 3D printed enclosure | +2 |
| Fitur preset save/load ke EEPROM | +2 |
| Spectrum analyzer display | +3 |

**Total Bonus:** ___/10

---

## ⚠️ Penalty Points

| Pelanggaran | Penalty |
|-------------|---------|
| Terlambat submit (per hari) | -5 |
| Plagiarisme kode (>50% similarity) | -50 atau 0 |
| Tidak ada source code | -30 |
| Tidak ada video demo | -15 |
| Hardware tidak berfungsi saat demo | -20 |

---

## 📋 Catatan Penilai

### Kelebihan:
```
_________________________________________________________________
_________________________________________________________________
_________________________________________________________________
```

### Kekurangan:
```
_________________________________________________________________
_________________________________________________________________
_________________________________________________________________
```

### Saran Improvement:
```
_________________________________________________________________
_________________________________________________________________
_________________________________________________________________
```

---

## ✅ Checklist Kelengkapan Submission

| Item | Ada | Tidak |
|------|-----|-------|
| Source code STM32 | ☐ | ☐ |
| Source code ESP32 | ☐ | ☐ |
| platformio.ini | ☐ | ☐ |
| Skema rangkaian | ☐ | ☐ |
| Laporan PDF | ☐ | ☐ |
| Video demo | ☐ | ☐ |
| README.md | ☐ | ☐ |

---

**Tanggal Penilaian:** ________________

**Penilai:** ________________________

**Tanda Tangan:** ___________________
