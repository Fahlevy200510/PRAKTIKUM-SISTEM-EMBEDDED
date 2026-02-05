# Rubrik Penilaian Tugas Video
## Modul 12: FreeRTOS Memory Management & Advanced Features

### 📋 Informasi Tugas

| Item | Keterangan |
|------|------------|
| **Jenis Tugas** | Video Demonstrasi dan Penjelasan |
| **Durasi** | 5-10 menit |
| **Format** | MP4, MOV, atau link YouTube/Google Drive |
| **Bobot Nilai** | 20% dari total nilai modul |
| **Deadline** | 1 minggu setelah praktikum selesai |

---

## 🎯 Tujuan Tugas Video

1. Mendemonstrasikan pemahaman konsep memory management FreeRTOS
2. Menunjukkan kemampuan implementasi pada hardware nyata
3. Menganalisis dan menjelaskan hasil output program
4. Mendokumentasikan proses debugging dan troubleshooting

---

## 📹 Komponen Video yang Dinilai

### A. Pembukaan dan Perkenalan (10%)

| Kriteria | Excellent (90-100%) | Good (70-89%) | Fair (50-69%) | Poor (<50%) |
|----------|---------------------|---------------|---------------|-------------|
| **Identitas** | Nama, NIM, kelas, tanggal jelas di awal video | Identitas lengkap tapi kurang jelas | Identitas tidak lengkap | Tidak ada identitas |
| **Overview** | Penjelasan singkat tujuan dan apa yang akan didemonstrasikan | Overview ada tapi kurang terstruktur | Overview minimal | Langsung ke demo |

---

### B. Setup Hardware dan Koneksi (15%)

| Kriteria | Excellent (90-100%) | Good (70-89%) | Fair (50-69%) | Poor (<50%) |
|----------|---------------------|---------------|---------------|-------------|
| **Tampilan Hardware** | Close-up jelas semua komponen, labeling yang baik | Hardware terlihat, sebagian berlabel | Hardware terlihat tapi tidak jelas | Hardware tidak ditampilkan |
| **Penjelasan Wiring** | Menjelaskan setiap koneksi pin dengan alasannya | Menjelaskan koneksi utama | Sebagian koneksi dijelaskan | Tidak ada penjelasan |
| **Dokumentasi** | Menunjukkan diagram wiring yang dibuat | Ada diagram sederhana | Diagram tidak ditampilkan | Tidak ada diagram |

---

### C. Demonstrasi Program (35%)

| Kriteria | Excellent (90-100%) | Good (70-89%) | Fair (50-69%) | Poor (<50%) |
|----------|---------------------|---------------|---------------|-------------|
| **Heap Monitoring** | Demo heap stats lengkap, before/after allocation, fragmentation | Demo heap stats dasar | Hanya menampilkan free heap | Tidak demo heap |
| **Stack Overflow** | Demo trigger overflow (controlled), hook function berjalan | Demo detection tanpa trigger | Hanya menjelaskan konsep | Tidak demo stack |
| **Event Groups** | Demo multi-task sync, wait ALL/ANY, rendezvous | Demo basic event set/wait | Event groups minimal | Tidak demo events |
| **Stream/Message Buffer** | Demo keduanya, perbandingan, ISR integration | Demo salah satu dengan baik | Demo basic buffer | Tidak demo buffer |
| **Low Power** | Demo tickless idle, power measurement jika ada | Demo low power enable | Hanya menjelaskan | Tidak demo power |

---

### D. Penjelasan Konsep (25%)

| Kriteria | Excellent (90-100%) | Good (70-89%) | Fair (50-69%) | Poor (<50%) |
|----------|---------------------|---------------|---------------|-------------|
| **Heap Schemes** | Menjelaskan perbedaan heap_1-5, pemilihan untuk use case | Menjelaskan heap yang digunakan | Penjelasan superfisial | Tidak menjelaskan |
| **Stack Protection** | Menjelaskan method 1 vs 2, canary pattern, hook purpose | Menjelaskan detection method | Penjelasan dasar | Tidak menjelaskan |
| **Event Groups vs Queue** | Membandingkan kapan menggunakan masing-masing | Menjelaskan event groups | Penjelasan minimal | Tidak menjelaskan |
| **Static vs Dynamic** | Menjelaskan tradeoff, kapan menggunakan masing-masing | Menjelaskan salah satu | Penjelasan tidak lengkap | Tidak menjelaskan |

---

### E. Analisis Output (10%)

| Kriteria | Excellent (90-100%) | Good (70-89%) | Fair (50-69%) | Poor (<50%) |
|----------|---------------------|---------------|---------------|-------------|
| **Serial Monitor** | Menjelaskan setiap output, memory numbers, timing | Menjelaskan output utama | Hanya menampilkan output | Tidak analisis |
| **Interpretasi Data** | Menginterpretasi heap stats, high water mark, fragmentation | Interpretasi dasar | Interpretasi minimal | Tidak ada |
| **Problem Solving** | Menunjukkan dan menyelesaikan masalah yang ditemukan | Menunjukkan awareness masalah | Menyebutkan masalah | Tidak ada |

---

### F. Kualitas Produksi (5%)

| Kriteria | Excellent (90-100%) | Good (70-89%) | Fair (50-69%) | Poor (<50%) |
|----------|---------------------|---------------|---------------|-------------|
| **Audio** | Suara jelas, tidak ada noise, volume konsisten | Suara jelas dengan sedikit noise | Suara kadang tidak jelas | Tidak ada audio/sangat buruk |
| **Video** | Resolution minimal 720p, fokus baik, lighting cukup | Quality cukup, terkadang blur | Quality rendah | Sangat buruk |
| **Editing** | Transisi smooth, teks overlay informatif | Editing basic | Raw footage | Tidak ada |

---

## 📝 Struktur Video yang Direkomendasikan

```
0:00 - 0:30  │ Pembukaan dan perkenalan
0:30 - 1:30  │ Overview hardware setup
1:30 - 3:00  │ Demo Heap Monitoring
3:00 - 4:30  │ Demo Stack Overflow Detection
4:30 - 6:00  │ Demo Event Groups
6:00 - 7:30  │ Demo Stream/Message Buffer
7:30 - 8:30  │ Demo Low Power (jika ada)
8:30 - 9:30  │ Analisis dan Kesimpulan
9:30 - 10:00 │ Penutup
```

---

## 📊 Perhitungan Nilai

```
Nilai Video = A×0.10 + B×0.15 + C×0.35 + D×0.25 + E×0.10 + F×0.05

Dimana:
A = Pembukaan (0-100)
B = Setup Hardware (0-100)
C = Demonstrasi (0-100)
D = Penjelasan Konsep (0-100)
E = Analisis Output (0-100)
F = Kualitas Produksi (0-100)
```

---

## 🌟 Bonus Points (Maksimal +15%)

| Bonus | Poin | Kriteria |
|-------|------|----------|
| Perbandingan STM32 vs ESP32 | +5% | Demo dan jelaskan perbedaan implementasi |
| Live Debugging | +5% | Tunjukkan proses debug dan fix masalah |
| Creative Presentation | +5% | Animasi, diagram interaktif, penjelasan kreatif |

---

## ⚠️ Penalti

| Pelanggaran | Penalti |
|-------------|---------|
| Durasi < 3 menit | -20% |
| Durasi > 15 menit | -10% |
| Tidak ada demo hardware | -30% |
| Tidak ada penjelasan, hanya demo | -25% |
| File corrupt / tidak bisa dibuka | Resubmit required |
| Terlambat 1-3 hari | -10% |
| Terlambat > 3 hari | -25% |

---

## 📋 Checklist Sebelum Submit

### Konten
- [ ] Identitas lengkap di awal video
- [ ] Hardware terlihat jelas dengan penjelasan
- [ ] Demo heap monitoring dan statistics
- [ ] Demo stack overflow detection
- [ ] Demo event groups
- [ ] Demo stream atau message buffer
- [ ] Penjelasan konsep teoritis
- [ ] Analisis output Serial Monitor
- [ ] Kesimpulan dan lessons learned

### Teknis
- [ ] Durasi 5-10 menit
- [ ] Resolusi minimal 720p
- [ ] Audio jelas dan dapat didengar
- [ ] File dapat diputar (test sebelum submit)
- [ ] Nama file: `NIM_Nama_Modul12_Video.mp4`

---

## 📤 Format Submission

### Opsi 1: Upload Langsung
- Format: MP4 (recommended), MOV, AVI
- Max size: 500MB
- Upload ke LMS/Google Classroom

### Opsi 2: Link Sharing
- YouTube (Unlisted)
- Google Drive (Anyone with link)
- OneDrive (Anyone with link)
- Pastikan link accessible!

---

## 💡 Tips Membuat Video yang Baik

1. **Persiapan:**
   - Test semua program sebelum recording
   - Siapkan script/outline
   - Pastikan lighting cukup
   
2. **Recording:**
   - Gunakan screen recorder untuk Serial Monitor
   - Kombinasikan dengan camera untuk hardware
   - Bicara dengan jelas dan tidak terlalu cepat
   
3. **Editing:**
   - Potong bagian yang tidak perlu
   - Tambahkan text overlay untuk poin penting
   - Speed up bagian repetitif (2x)

4. **Review:**
   - Tonton ulang sebelum submit
   - Minta teman review
   - Test link jika menggunakan sharing

---

## 📚 Contoh Narasi

### Pembukaan
*"Assalamualaikum, nama saya [Nama], NIM [NIM], dari kelas [Kelas]. Hari ini saya akan mendemonstrasikan implementasi FreeRTOS Memory Management pada STM32 dan ESP32..."*

### Demo Heap
*"Sekarang kita lihat heap statistics. Free heap saat ini adalah 12.456 bytes dari total 15.000 bytes. Minimum ever free menunjukkan 10.234 bytes, artinya kita pernah menggunakan sekitar 5.000 bytes heap..."*

### Penutup
*"Dari praktikum ini, saya belajar bahwa memory management sangat penting untuk embedded systems dengan resource terbatas. Pemilihan heap scheme yang tepat dan monitoring yang konsisten dapat mencegah crash dan bug yang sulit di-debug..."*
