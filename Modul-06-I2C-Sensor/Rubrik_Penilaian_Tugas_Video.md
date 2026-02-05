# Rubrik Penilaian Tugas Video
## Modul 06: I2C Bus dan Sensor Integration

---

## 📋 Informasi Tugas

| Item | Keterangan |
|------|------------|
| **Topik** | I2C Protocol dan Multi-Sensor Integration |
| **Platform** | STM32F103C8T6 + ESP32 DevKit V1 |
| **Durasi Video** | 8-12 menit |
| **Format** | MP4 (H.264), 720p minimum |
| **Bobot Total** | 100 poin |

---

## 📹 Struktur Video yang Diharapkan

### Timeline Rekomendasi

| Segmen | Durasi | Konten |
|--------|--------|--------|
| Opening | 0:30 | Intro, judul, nama praktikan |
| Teori I2C | 2:00 | Penjelasan protokol dan konsep |
| Hardware | 1:30 | Overview rangkaian dan komponen |
| STM32 Demo | 2:30 | Demonstrasi sensor reading |
| ESP32 Demo | 2:30 | Demonstrasi display & communication |
| Multi-Device | 1:30 | Demo integrasi semua device |
| Closing | 1:30 | Kesimpulan dan insights |
| **Total** | **~12:00** | |

---

## 📊 Komponen Penilaian

### A. Konten Teori I2C (25 poin)

| No | Kriteria | Poin Max | Skor |
|----|----------|----------|------|
| 1 | Penjelasan dasar protokol I2C (Master-Slave, SDA-SCL) | 5 | |
| 2 | Penjelasan address format (7-bit/10-bit) | 4 | |
| 3 | Penjelasan timing diagram (Start, Stop, ACK/NACK) | 5 | |
| 4 | Perbedaan I2C vs protokol lain (SPI, UART) | 4 | |
| 5 | Penjelasan pull-up resistor dan electrical requirements | 4 | |
| 6 | Konsep multi-device pada satu bus | 3 | |
| | **Subtotal A** | **25** | |

**Panduan Penilaian:**
- 5 poin: Penjelasan sangat jelas, contoh tepat, visual aid
- 3-4 poin: Penjelasan cukup jelas, minor inaccuracies
- 1-2 poin: Penjelasan dasar saja, ada kesalahan
- 0 poin: Tidak dijelaskan atau salah total

---

### B. Demonstrasi Praktikum (35 poin)

| No | Kriteria | Poin Max | Skor |
|----|----------|----------|------|
| 1 | I2C Scanner - mendeteksi address device | 4 | |
| 2 | BME280 sensor reading (temperature, humidity, pressure) | 6 | |
| 3 | SSD1306 OLED display output | 6 | |
| 4 | DS3231 RTC reading (time, date) | 5 | |
| 5 | 24LC256 EEPROM read/write | 5 | |
| 6 | Multi-device integration demo | 5 | |
| 7 | Serial monitor output terlihat jelas | 4 | |
| | **Subtotal B** | **35** | |

**Panduan Penilaian Demo:**
- Full poin: Demo berjalan lancar, output sesuai expectation
- 70% poin: Demo berjalan dengan minor issues
- 50% poin: Demo berjalan partial
- <50% poin: Demo gagal atau tidak ditunjukkan

---

### C. Kualitas Teknis Video (20 poin)

| No | Kriteria | Poin Max | Skor |
|----|----------|----------|------|
| 1 | Resolusi minimal 720p, fokus tajam | 5 | |
| 2 | Audio jelas, tidak ada noise berlebihan | 5 | |
| 3 | Pencahayaan cukup, komponen terlihat jelas | 4 | |
| 4 | Screen capture/recording berkualitas baik | 3 | |
| 5 | Editing smooth, transisi appropriate | 3 | |
| | **Subtotal C** | **20** | |

**Panduan Penilaian Teknis:**
| Aspek | Excellent (100%) | Good (75%) | Fair (50%) | Poor (<50%) |
|-------|-----------------|------------|------------|-------------|
| Video | 1080p, crystal clear | 720p, clear | 480p, acceptable | Blur, pixelated |
| Audio | Clear, professional | Clear, minor noise | Understandable | Hard to hear |
| Lighting | Perfect | Good | Acceptable | Too dark/bright |

---

### D. Penyampaian dan Komunikasi (15 poin)

| No | Kriteria | Poin Max | Skor |
|----|----------|----------|------|
| 1 | Narasi jelas dan mudah dipahami | 5 | |
| 2 | Sistematika penyampaian terstruktur | 4 | |
| 3 | Penggunaan istilah teknis tepat | 3 | |
| 4 | Tempo bicara appropriate (tidak terlalu cepat/lambat) | 3 | |
| | **Subtotal D** | **15** | |

---

### E. Kelengkapan dan Kreativitas (5 poin)

| No | Kriteria | Poin Max | Skor |
|----|----------|----------|------|
| 1 | Durasi sesuai (8-12 menit) | 2 | |
| 2 | Visual aids (diagram, animasi, overlay) | 2 | |
| 3 | Kesimpulan dan insights bermakna | 1 | |
| | **Subtotal E** | **5** | |

---

## 🌟 Bonus Points (Maximum +10 poin)

| No | Kriteria Bonus | Poin | Skor |
|----|----------------|------|------|
| 1 | Analisis timing dengan logic analyzer | +4 | |
| 2 | Perbandingan performa STM32 vs ESP32 | +3 | |
| 3 | Troubleshooting demo (recovery dari error) | +3 | |
| | **Total Bonus** | **(+10)** | |

---

## ⚠️ Penalty Points

| No | Pelanggaran | Pengurangan |
|----|-------------|-------------|
| 1 | Durasi kurang dari 5 menit | -15 |
| 2 | Durasi lebih dari 15 menit | -5 |
| 3 | Tidak ada demo hardware nyata | -20 |
| 4 | Plagiarism/copy video lain | -100 (Fail) |
| 5 | Audio/video quality sangat buruk | -10 |

---

## 📈 Rekapitulasi Nilai

| Komponen | Bobot | Skor | Nilai |
|----------|-------|------|-------|
| A. Konten Teori I2C | 25 | | |
| B. Demonstrasi Praktikum | 35 | | |
| C. Kualitas Teknis Video | 20 | | |
| D. Penyampaian | 15 | | |
| E. Kelengkapan & Kreativitas | 5 | | |
| **Total Base** | **100** | | |
| Bonus Points | (+10) | | |
| Penalty Points | (-) | | |
| **TOTAL AKHIR** | | | |

---

## 📊 Konversi Grade

| Skor | Grade | Predikat |
|------|-------|----------|
| 85-100+ | A | Excellent - Video berkualitas profesional |
| 75-84 | B+ | Very Good |
| 70-74 | B | Good - Semua aspek terpenuhi |
| 65-69 | C+ | Above Average |
| 55-64 | C | Average - Memenuhi minimum |
| 45-54 | D | Below Average |
| <45 | E | Fail |

---

## 📝 Checklist Sebelum Submit

### Konten
- [ ] Intro dengan identitas praktikan
- [ ] Penjelasan teori I2C
- [ ] Demo I2C Scanner
- [ ] Demo minimal 3 device I2C berbeda
- [ ] Demo pada STM32 DAN ESP32
- [ ] Kesimpulan dan lessons learned

### Teknis
- [ ] Format video MP4/MOV
- [ ] Resolusi minimal 720p
- [ ] Audio jelas
- [ ] File size reasonable (<500MB)
- [ ] Durasi 8-12 menit

### Etika
- [ ] Video adalah karya original
- [ ] Semua sumber disebutkan
- [ ] Tidak mengandung konten inappropriate

---

## 💬 Catatan Penilai

**Kekuatan Video:**
```
[Aspek yang baik dari video]
```

**Area Perbaikan:**
```
[Saran untuk video yang lebih baik]
```

**Feedback:**
```
[Feedback konstruktif untuk praktikan]
```

---

| | |
|----------|------------|
| **Penilai** | _________________ |
| **Tanggal** | _________________ |
| **Tanda Tangan** | _________________ |
