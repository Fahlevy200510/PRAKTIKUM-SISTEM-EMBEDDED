# Rubrik Penilaian Project
## Modul 01: GPIO Digital I/O - Smart Home Control Panel

---

## 📋 Informasi Penilaian

| Item | Keterangan |
|------|------------|
| **Nama Project** | Smart Home Control Panel |
| **Total Skor Maksimum** | 100 poin |
| **Passing Grade** | 60 poin (C) |
| **Penilaian** | Kelompok |

---

## 📊 Komponen Penilaian

| Komponen | Bobot | Poin Maksimum |
|----------|-------|---------------|
| Fungsionalitas | 40% | 40 poin |
| Kualitas Kode | 20% | 20 poin |
| Dokumentasi | 15% | 15 poin |
| Video Demonstrasi | 15% | 15 poin |
| Fitur Bonus | 10% | 10 poin |
| **Total** | **100%** | **100 poin** |

---

## 📝 Rubrik Detail

### A. Fungsionalitas (40 poin)

#### A.1 Level 1 - Fitur Dasar (24 poin)

| No | Fitur | Poin | Kriteria Penilaian |
|----|-------|------|-------------------|
| A.1.1 | LED Control via Button | 8 | **8:** Semua 4 room bekerja sempurna<br>**6:** 3 room bekerja<br>**4:** 2 room bekerja<br>**2:** 1 room bekerja<br>**0:** Tidak ada yang bekerja |
| A.1.2 | Software Debouncing | 4 | **4:** Semua button di-debounce dengan baik<br>**2:** Sebagian button di-debounce<br>**0:** Tidak ada debouncing |
| A.1.3 | Toggle Behavior | 4 | **4:** Toggle berfungsi konsisten di semua LED<br>**2:** Toggle berfungsi sebagian<br>**0:** Tidak berfungsi |
| A.1.4 | Emergency Stop | 4 | **4:** E-Stop mematikan semua LED instantly<br>**2:** E-Stop bekerja dengan delay<br>**0:** Tidak ada E-Stop |
| A.1.5 | Serial Debug Output | 2 | **2:** Log informatif dan terstruktur<br>**1:** Log minimal<br>**0:** Tidak ada log |
| A.1.6 | Heartbeat LED ESP32 | 2 | **2:** Blink konsisten 1Hz<br>**1:** Blink tidak konsisten<br>**0:** Tidak ada heartbeat |

#### A.2 Level 2 - Fitur Intermediate (16 poin)

| No | Fitur | Poin | Kriteria Penilaian |
|----|-------|------|-------------------|
| A.2.1 | DIP Switch Room Selector | 4 | **4:** 4 kombinasi bekerja sempurna<br>**3:** 3 kombinasi bekerja<br>**2:** 2 kombinasi bekerja<br>**1:** 1 kombinasi bekerja<br>**0:** Tidak bekerja |
| A.2.2 | Inter-MCU Serial Communication | 6 | **6:** Komunikasi bidirectional sempurna<br>**4:** Unidirectional bekerja<br>**2:** Komunikasi partial<br>**0:** Tidak ada komunikasi |
| A.2.3 | Remote Control via Serial | 4 | **4:** Semua command dikenali dan dieksekusi<br>**3:** 75% command bekerja<br>**2:** 50% command bekerja<br>**1:** 25% command bekerja<br>**0:** Tidak bekerja |
| A.2.4 | Status Reporting | 2 | **2:** Status lengkap dan terformat<br>**1:** Status partial<br>**0:** Tidak ada status |

### B. Kualitas Kode (20 poin)

| No | Aspek | Poin | Kriteria Penilaian |
|----|-------|------|-------------------|
| B.1 | Struktur & Modularitas | 6 | **6:** Kode terpisah dalam modul/file yang logis, fungsi-fungsi reusable<br>**4:** Struktur cukup baik, beberapa fungsi modular<br>**2:** Struktur minimal, banyak kode repetitif<br>**0:** Semua dalam satu file tanpa struktur |
| B.2 | Naming Convention | 4 | **4:** Nama variabel/fungsi deskriptif dan konsisten<br>**2:** Sebagian nama deskriptif<br>**0:** Nama tidak deskriptif (x, y, temp1, dll) |
| B.3 | Comments & Documentation | 4 | **4:** Komentar menjelaskan logic penting, header file ada<br>**2:** Komentar minimal<br>**0:** Tidak ada komentar |
| B.4 | Error Handling | 3 | **3:** Handle edge cases dan input validation<br>**2:** Sebagian error di-handle<br>**0:** Tidak ada error handling |
| B.5 | Efisiensi | 3 | **3:** Non-blocking code, resource efficient<br>**2:** Beberapa blocking delay<br>**0:** Banyak blocking code, inefficient |

### C. Dokumentasi (15 poin)

| No | Aspek | Poin | Kriteria Penilaian |
|----|-------|------|-------------------|
| C.1 | README | 4 | **4:** Lengkap (deskripsi, setup, usage, troubleshooting)<br>**3:** Cukup lengkap<br>**2:** Minimal<br>**0:** Tidak ada |
| C.2 | Wiring Diagram | 4 | **4:** Skematik profesional + foto jelas<br>**3:** Diagram hand-drawn + foto<br>**2:** Foto saja atau diagram saja<br>**0:** Tidak ada |
| C.3 | Flowchart Program | 3 | **3:** Flowchart detail untuk kedua MCU<br>**2:** Flowchart sederhana atau hanya 1 MCU<br>**0:** Tidak ada flowchart |
| C.4 | Laporan Teknis | 4 | **4:** Analisis mendalam, kendala & solusi jelas<br>**3:** Analisis cukup<br>**2:** Deskriptif saja tanpa analisis<br>**0:** Tidak ada laporan |

### D. Video Demonstrasi (15 poin)

| No | Aspek | Poin | Kriteria Penilaian |
|----|-------|------|-------------------|
| D.1 | Kualitas Video | 3 | **3:** HD, stabil, audio jelas<br>**2:** Kualitas cukup<br>**1:** Kualitas rendah tapi bisa dilihat<br>**0:** Tidak bisa dilihat/didengar |
| D.2 | Kelengkapan Demo | 6 | **6:** Semua fitur didemonstrasikan<br>**5:** 90% fitur didemonstrasikan<br>**4:** 75% fitur didemonstrasikan<br>**3:** 50% fitur didemonstrasikan<br>**0:** Tidak ada demo |
| D.3 | Penjelasan | 4 | **4:** Penjelasan jelas dan terstruktur<br>**3:** Penjelasan cukup<br>**2:** Penjelasan minimal<br>**0:** Tidak ada penjelasan |
| D.4 | Durasi | 2 | **2:** Sesuai ketentuan (3-5 menit)<br>**1:** Terlalu pendek/panjang (±2 menit)<br>**0:** Jauh dari ketentuan |

### E. Fitur Bonus - Level 3 (10 poin)

| No | Fitur | Poin | Kriteria Penilaian |
|----|-------|------|-------------------|
| E.1 | Scene Mode | 4 | **4:** 3+ scene dengan transisi smooth<br>**2:** 1-2 scene basic<br>**0:** Tidak ada scene mode |
| E.2 | Long Press Detection | 3 | **3:** Long/short press dengan feedback<br>**2:** Deteksi bekerja tapi tidak reliable<br>**0:** Tidak ada |
| E.3 | State Recovery | 3 | **3:** State persist dan restore dengan benar<br>**2:** Partial recovery<br>**0:** Tidak ada recovery |

---

## 📈 Konversi Nilai

| Rentang Poin | Huruf | Keterangan |
|--------------|-------|------------|
| 85 - 100 | A | Sangat Baik |
| 80 - 84 | A- | |
| 75 - 79 | B+ | Baik |
| 70 - 74 | B | |
| 65 - 69 | B- | |
| 60 - 64 | C+ | Cukup |
| 55 - 59 | C | |
| 50 - 54 | C- | |
| 40 - 49 | D | Kurang |
| 0 - 39 | E | Tidak Lulus |

---

## ⚠️ Penalti

| Pelanggaran | Penalti |
|-------------|---------|
| Terlambat submit (per hari) | -5 poin |
| Plagiarisme kode (copy-paste tanpa modifikasi) | -50% total nilai |
| Video tidak bisa diputar | -10 poin |
| Kode tidak dapat di-compile | -20 poin |
| Hardware tidak berfungsi saat demo | -15 poin |

---

## ✅ Checklist Pengumpulan

### File yang Harus Dikumpulkan:

```
□ Source Code STM32 (folder lengkap dengan platformio.ini)
□ Source Code ESP32 (folder lengkap dengan platformio.ini)
□ README.md
□ Wiring Diagram (gambar/PDF)
□ Flowchart (gambar/PDF)
□ Laporan Teknis (PDF, max 5 halaman)
□ Video Demonstrasi (MP4/link YouTube, 3-5 menit)
```

### Format Nama File:

```
Kelompok_[XX]_[Nama1]_[Nama2]_GPIO_Project.zip
Kelompok_[XX]_Video.mp4 atau link YouTube
```

---

## 📝 Form Penilaian

### Kelompok: _______________ | Tanggal: _______________

| Komponen | Poin Maks | Poin | Catatan |
|----------|-----------|------|---------|
| **A. Fungsionalitas** | | | |
| A.1.1 LED Control | 8 | | |
| A.1.2 Debouncing | 4 | | |
| A.1.3 Toggle | 4 | | |
| A.1.4 E-Stop | 4 | | |
| A.1.5 Serial Debug | 2 | | |
| A.1.6 Heartbeat | 2 | | |
| A.2.1 DIP Switch | 4 | | |
| A.2.2 Inter-MCU Comm | 6 | | |
| A.2.3 Remote Control | 4 | | |
| A.2.4 Status Report | 2 | | |
| **Subtotal A** | **40** | | |
| **B. Kualitas Kode** | | | |
| B.1 Struktur | 6 | | |
| B.2 Naming | 4 | | |
| B.3 Comments | 4 | | |
| B.4 Error Handling | 3 | | |
| B.5 Efisiensi | 3 | | |
| **Subtotal B** | **20** | | |
| **C. Dokumentasi** | | | |
| C.1 README | 4 | | |
| C.2 Wiring Diagram | 4 | | |
| C.3 Flowchart | 3 | | |
| C.4 Laporan | 4 | | |
| **Subtotal C** | **15** | | |
| **D. Video** | | | |
| D.1 Kualitas | 3 | | |
| D.2 Kelengkapan | 6 | | |
| D.3 Penjelasan | 4 | | |
| D.4 Durasi | 2 | | |
| **Subtotal D** | **15** | | |
| **E. Bonus** | | | |
| E.1 Scene Mode | 4 | | |
| E.2 Long Press | 3 | | |
| E.3 State Recovery | 3 | | |
| **Subtotal E** | **10** | | |
| **TOTAL** | **100** | | |
| **Penalti** | | | |
| **NILAI AKHIR** | | | |

### Tanda Tangan Penilai:

___________________________ | Tanggal: _______________

---

*Rubrik Penilaian Project Modul 01 - Praktikum Sistem Embedded*
*Versi 1.0 - Februari 2026*
