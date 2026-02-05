# Rubrik Penilaian Project
## Modul 06: I2C Bus dan Sensor Integration - Smart Environmental Monitor

---

## 📋 Informasi Penilaian

| Item | Keterangan |
|------|------------|
| **Nama Project** | Smart Environmental Monitor dengan Dual-MCU I2C |
| **Bobot Total** | 100 poin |
| **Passing Grade** | 55 poin |

---

## 📊 Komponen Penilaian

### A. Hardware dan Assembly (15 poin)

| No | Kriteria | Poin Max | Skor |
|----|----------|----------|------|
| 1 | Rangkaian I2C sesuai skematik (pull-up resistor, wiring benar) | 5 | |
| 2 | Koneksi STM32-ESP32 terhubung dengan baik | 4 | |
| 3 | Semua komponen terpasang (BME280, SSD1306, DS3231, EEPROM) | 4 | |
| 4 | Kerapian dan keamanan wiring | 2 | |
| | **Subtotal A** | **15** | |

**Panduan Penilaian:**
- 5 poin: Semua kriteria terpenuhi sempurna
- 3-4 poin: Sebagian besar terpenuhi dengan minor issues
- 1-2 poin: Banyak issues tetapi masih fungsional
- 0 poin: Tidak fungsional

---

### B. STM32 Sensor Hub Implementation (20 poin)

| No | Kriteria | Poin Max | Skor |
|----|----------|----------|------|
| 1 | I2C Master untuk sensor berfungsi (BME280/DS3231 terbaca) | 6 | |
| 2 | I2C Slave mode implementasi (address 0x08 merespon) | 6 | |
| 3 | Data structure dan register map sesuai spesifikasi | 4 | |
| 4 | Checksum calculation dan validation | 2 | |
| 5 | Error handling dan status reporting | 2 | |
| | **Subtotal B** | **20** | |

**Panduan Penilaian I2C Slave:**
- 6 poin: Merespon read/write dengan data correct
- 4-5 poin: Merespon tetapi ada data inconsistency
- 2-3 poin: Hanya merespon address, data incomplete
- 0-1 poin: Tidak merespon atau crash

---

### C. ESP32 Display Controller (20 poin)

| No | Kriteria | Poin Max | Skor |
|----|----------|----------|------|
| 1 | I2C Master request ke STM32 berhasil | 5 | |
| 2 | OLED Display menampilkan data dengan format baik | 5 | |
| 3 | Multiple screen UI (Dashboard, History, Statistics) | 5 | |
| 4 | Button navigation berfungsi | 3 | |
| 5 | User interface responsif dan informatif | 2 | |
| | **Subtotal C** | **20** | |

**Panduan Penilaian Display:**
- 5 poin: All screens implemented dengan UI yang baik
- 3-4 poin: Main screens work, minor UI issues
- 1-2 poin: Basic display works, limited functionality
- 0 poin: Display tidak berfungsi

---

### D. Data Communication - I2C antar MCU (15 poin)

| No | Kriteria | Poin Max | Skor |
|----|----------|----------|------|
| 1 | Komunikasi Master-Slave stabil (tidak error/hang) | 5 | |
| 2 | Data transfer rate sesuai (1 second update) | 3 | |
| 3 | Checksum validation berfungsi | 3 | |
| 4 | Protokol register-based access benar | 2 | |
| 5 | Bus arbitration handling (jika multi-master) | 2 | |
| | **Subtotal D** | **15** | |

**Panduan Penilaian Communication:**
- 5 poin: >95% success rate, no errors
- 3-4 poin: 80-95% success rate
- 1-2 poin: 50-80% success rate
- 0 poin: <50% atau tidak berfungsi

---

### E. Data Logging (10 poin)

| No | Kriteria | Poin Max | Skor |
|----|----------|----------|------|
| 1 | EEPROM write berfungsi (data tersimpan) | 3 | |
| 2 | EEPROM read berfungsi (data terbaca kembali) | 3 | |
| 3 | Circular buffer implementation | 2 | |
| 4 | Data persistent setelah power cycle | 2 | |
| | **Subtotal E** | **10** | |

---

### F. Alert System (5 poin)

| No | Kriteria | Poin Max | Skor |
|----|----------|----------|------|
| 1 | Temperature threshold detection | 2 | |
| 2 | Humidity threshold detection | 1 | |
| 3 | LED/Buzzer alert berfungsi | 2 | |
| | **Subtotal F** | **5** | |

---

### G. Error Handling (5 poin)

| No | Kriteria | Poin Max | Skor |
|----|----------|----------|------|
| 1 | I2C bus recovery mechanism | 2 | |
| 2 | Sensor failure detection dan reporting | 1 | |
| 3 | Graceful degradation (sistem tetap berjalan partial) | 2 | |
| | **Subtotal G** | **5** | |

---

### H. Dokumentasi (5 poin)

| No | Kriteria | Poin Max | Skor |
|----|----------|----------|------|
| 1 | README dengan instruksi lengkap | 2 | |
| 2 | Schematic/wiring diagram | 1 | |
| 3 | Code comments dan struktur | 1 | |
| 4 | Laporan pengujian | 1 | |
| | **Subtotal H** | **5** | |

---

### I. Video Demonstrasi (5 poin)

| No | Kriteria | Poin Max | Skor |
|----|----------|----------|------|
| 1 | Hardware overview jelas | 1 | |
| 2 | Semua fitur didemonstrasikan | 2 | |
| 3 | I2C communication proof (serial/analyzer) | 1 | |
| 4 | Durasi sesuai (5-10 menit) | 1 | |
| | **Subtotal I** | **5** | |

---

## 🌟 Bonus Points (Maximum +15 poin)

| No | Kriteria Bonus | Poin | Skor |
|----|----------------|------|------|
| 1 | Sensor tambahan terintegrasi (AHT20, BH1750, dll) | +5 | |
| 2 | WiFi connectivity dengan web dashboard | +5 | |
| 3 | Power management (sleep modes) | +3 | |
| 4 | Custom PCB design | +2 | |
| | **Total Bonus** | **(+15)** | |

---

## 📈 Rekapitulasi Nilai

| Komponen | Bobot | Skor | Nilai |
|----------|-------|------|-------|
| A. Hardware dan Assembly | 15 | | |
| B. STM32 Sensor Hub | 20 | | |
| C. ESP32 Display Controller | 20 | | |
| D. Data Communication | 15 | | |
| E. Data Logging | 10 | | |
| F. Alert System | 5 | | |
| G. Error Handling | 5 | | |
| H. Dokumentasi | 5 | | |
| I. Video Demonstrasi | 5 | | |
| **Total Base** | **100** | | |
| Bonus Points | (+15) | | |
| **TOTAL AKHIR** | **Max 115** | | |

---

## 📊 Konversi Grade

| Skor | Grade | Predikat |
|------|-------|----------|
| 85-100+ | A | Excellent - Semua fitur + bonus |
| 75-84 | B+ | Very Good |
| 70-74 | B | Good - Semua fitur utama |
| 65-69 | C+ | Above Average |
| 55-64 | C | Average - Memenuhi minimum |
| 45-54 | D | Below Average |
| <45 | E | Fail |

---

## 📝 Catatan Penilai

**Kekuatan:**
```
[Tuliskan aspek yang dikerjakan dengan baik]
```

**Area Perbaikan:**
```
[Tuliskan aspek yang perlu ditingkatkan]
```

**Feedback:**
```
[Berikan feedback konstruktif untuk mahasiswa]
```

---

| | |
|----------|------------|
| **Penilai** | _________________ |
| **Tanggal** | _________________ |
| **Tanda Tangan** | _________________ |
