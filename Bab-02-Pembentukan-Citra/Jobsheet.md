# Jobsheet Praktikum
## Bab 2: Pembentukan Citra (Image Formation)

---

### 📋 Informasi Praktikum

| Item | Keterangan |
|------|------------|
| **Mata Kuliah** | Computer Vision |
| **Topik** | Pembentukan Citra dan Transformasi Geometri |
| **Waktu** | 3 x 50 menit |
| **Software** | Python 3.8+, OpenCV, NumPy, Matplotlib |

---

### 🎯 Tujuan Praktikum

Setelah menyelesaikan praktikum ini, mahasiswa mampu:
1. Memahami dan mengimplementasikan transformasi geometri dasar (translasi, rotasi, scaling)
2. Melakukan transformasi affine dan perspektif pada gambar
3. Mengimplementasikan aplikasi document scanner menggunakan transformasi perspektif
4. Memahami prinsip kalibrasi kamera dan koreksi distorsi lensa
5. Menganalisis perbedaan metode interpolasi dalam operasi resize
6. Memahami rotasi 3D (axis-angle) dan proyeksi 3D ke 2D (ortografik vs perspektif)
7. Memahami sampling & aliasing, color space, gamma correction, dan artefak kompresi
8. Memahami photometric image formation (Lambertian & Phong shading)

---

### 🔧 Alat dan Bahan

**Software:**
- Python 3.8 atau lebih baru
- Library: OpenCV, NumPy, Matplotlib
- IDE: VS Code / PyCharm / Jupyter Notebook

**File yang diperlukan:**
- Gambar sample dari folder `data/images/`
- Gambar dokumen/kertas untuk percobaan scanner (opsional)

**Output:**
- Semua hasil disimpan otomatis ke folder `praktikum/output/output1`, `output2`, dst.

**Hardware:**
- Komputer dengan kamera (opsional untuk kalibrasi)
- Checkerboard pattern untuk kalibrasi (opsional)

---

### 📝 Langkah Kerja

## Percobaan 1: Translasi Gambar ⬅️➡️

**Tujuan**: Memahami cara menggeser gambar menggunakan transformation matrix

**Teori Singkat**:
Transformasi translasi menggunakan matrix 2×3:
```
M = | 1  0  tx |
    | 0  1  ty |
```
- tx: pergeseran horizontal (+kanan, -kiri)
- ty: pergeseran vertikal (+bawah, -atas)

**Langkah-langkah**:
1. Jalankan program `01_translasi.py`
2. Amati pergerakan gambar saat nilai `tx` dan `ty` diubah
3. Perhatikan berbagai mode border yang tersedia
4. Catat hasilnya di tabel observasi

**Pertanyaan**:
- Apa yang terjadi jika `tx` positif vs negatif?
- Mode border mana yang paling cocok untuk aplikasi real-world?
- Mengapa ada area kosong setelah translasi?

**Tabel Observasi**:

| tx | ty | Arah Pergerakan | Border Mode | Observasi |
|----|----|-----------------| -------------|-----------|
| 100 | 0 | Kanan | CONSTANT | Gambar geser kanan, area kosong hitam |
| -100 | 0 | Kiri | CONSTANT | |
| 0 | 80 | Bawah | REPLICATE | |
| -50 | -50 | Kiri-Atas | REFLECT | |

---

## Percobaan 2: Rotasi Gambar 🔄

**Tujuan**: Memahami rotasi gambar dengan berbagai sudut dan titik pusat

**Teori Singkat**:
OpenCV menggunakan fungsi `cv2.getRotationMatrix2D()` yang menghasilkan matrix 2×3:
```
M = | α   β  (1-α)cx - β cy |
    | -β  α   β cx + (1-α)cy |

dimana: α = scale × cos(θ), β = scale × sin(θ)
```

**Langkah-langkah**:
1. Jalankan program `02_rotasi.py`
2. Eksperimen dengan berbagai sudut rotasi (30°, 45°, 90°, 180°)
3. Ubah titik pusat rotasi dan amati perbedaannya
4. Bandingkan hasil dengan dan tanpa perluasan canvas

**Pertanyaan**:
- Mengapa bagian gambar bisa terpotong saat rotasi?
- Apa perbedaan rotasi dari pusat vs dari sudut (0,0)?
- Kapan sebaiknya menggunakan expanded canvas?

**Tabel Observasi**:

| Sudut | Titik Pusat | Scale | Hasil | Observasi |
|-------|-------------|-------|-------|-----------|
| 45° | Tengah | 1.0 | ✓ | Gambar miring 45°, sebagian terpotong |
| 90° | Tengah | 1.0 | ✓ | |
| 45° | (0,0) | 1.0 | ✓ | |
| -30° | Tengah | 0.8 | ✓ | |

---

## Percobaan 3: Scaling (Resize) 🔍

**Tujuan**: Memahami berbagai metode interpolasi saat mengubah ukuran gambar

**Teori Singkat**:
Interpolasi adalah proses menghitung nilai piksel baru saat mengubah resolusi:
- **INTER_NEAREST**: Ambil piksel terdekat (cepat, pixelated)  
- **INTER_LINEAR**: Rata-rata 4 piksel terdekat (balanced)
- **INTER_CUBIC**: Rata-rata 16 piksel (smooth)
- **INTER_LANCZOS4**: Kualitas terbaik untuk upscaling
- **INTER_AREA**: Terbaik untuk downscaling

**Langkah-langkah**:
1. Jalankan program `03_scaling.py`
2. Bandingkan hasil berbagai metode interpolasi
3. Ukur waktu pemrosesan untuk setiap metode
4. Amati kualitas tepi dan detail

**Pertanyaan**:
- Metode mana yang paling cepat vs paling berkualitas?
- Kapan sebaiknya menggunakan INTER_AREA?
- Bagaimana trade-off antara kecepatan dan kualitas?

**Tabel Observasi**:

| Metode | Waktu (ms) | Kualitas Visual (1-5) | Cocok untuk |
|--------|------------|----------------------| ------------|
| INTER_NEAREST | ~1.8 | 2 | Pixel art, mask |
| INTER_LINEAR | ~1.6 | 3 | Real-time processing |
| INTER_CUBIC | ~4.3 | 4 | Photo upscaling |
| INTER_LANCZOS4 | ~24.1 | 5 | High quality upscaling |
| INTER_AREA | ~2.9 | 4 | Downscaling |

---

## Percobaan 4: Transformasi Affine 📐

**Tujuan**: Memahami transformasi affine yang menggabungkan berbagai transformasi dasar

**Teori Singkat**:
Transformasi Affine memiliki 6 DOF (Degree of Freedom) dan membutuhkan 3 pasang titik korespondensi:
```
Matrix 2×3: | a  b  tx |
           | c  d  ty |

Properti: - Garis lurus tetap lurus
         - Garis paralel tetap paralel  
         - Sudut dan jarak bisa berubah
```

**Langkah-langkah**:
1. Jalankan program `04_affine_transform.py`
2. Amati berbagai jenis transformasi affine (shear, kombinasi)
3. Eksperimen dengan mengubah koordinat titik tujuan
4. Pahami dekomposisi transformasi affine

**Pertanyaan**:
- Mengapa dibutuhkan tepat 3 titik untuk transformasi affine?
- Apa perbedaan antara shear horizontal dan vertikal?
- Transformasi apa saja yang bisa digabungkan dalam affine?

**Tabel Eksperimen**:

| Jenis Transformasi | Titik Tujuan | Efek yang Dihasilkan |
|--------------------|--------------|----------------------|
| Original | [(0,0), (399,0), (0,399)] | Tidak berubah |
| Shear Kanan | [(50,0), (449,0), (0,399)] | Gambar miring ke kanan |
| Shear Bawah | [(0,0), (399,50), (0,449)] | Gambar miring ke bawah |
| Mirror Horizontal | [(399,0), (0,0), (399,399)] | Gambar terbalik horizontal |

---

## Percobaan 5: Transformasi Perspektif 👁️

**Tujuan**: Memahami transformasi perspektif untuk koreksi sudut pandang (bird's eye view)

**Teori Singkat**:
Transformasi Perspektif (Homography) memiliki 8 DOF dan membutuhkan 4 pasang titik:
```
Matrix 3×3: | h00  h01  h02 |
           | h10  h11  h12 |
           | h20  h21  h22 |

Final: x' = x_homog/w, y' = y_homog/w
```

**Langkah-langkah**:
1. Jalankan program `05_perspektif_transform.py`
2. Amati berbagai sudut perspektif yang disimulasikan
3. Pahami perbedaan dengan transformasi affine
4. Eksperimen dengan inverse transform

**Pertanyaan**:
- Mengapa dibutuhkan 4 titik untuk transformasi perspektif?
- Apa yang dimaksud dengan homogeneous coordinates?
- Kapan menggunakan perspektif vs affine transform?

**Analisis Perbandingan**:

| Aspek | Affine (3 titik) | Perspective (4 titik) |
|-------|------------------|-----------------------|
| DOF | 6 | 8 |
| Paralel lines | Tetap paralel | Bisa konvergen |
| Aplikasi | Rotasi, shear, scale | Koreksi perspektif |
| Kompleksitas | Sedang | Tinggi |

---

## Percobaan 6: Document Scanner 📄

**Tujuan**: Mengaplikasikan transformasi perspektif untuk membuat document scanner

**Teori Singkat**:
Document scanner menggunakan pipeline:
1. **Edge Detection** (Canny)
2. **Contour Detection** 
3. **4-Point Detection** (largest quadrilateral)
4. **Perspective Correction**
5. **Enhancement** (adaptive threshold, CLAHE)

**Langkah-langkah**:
1. Jalankan program `06_document_scanner.py`
2. Amati proses deteksi otomatis sudut dokumen
3. Pahami parameter Canny edge detection
4. Analisis hasil enhancement

**Pertanyaan**:
- Mengapa menggunakan Canny edge detection?
- Bagaimana cara mengurutkan 4 sudut dokumen?
- Apa tantangan dalam deteksi otomatis?

**Pipeline Evaluation**:

| Step | Input | Output | Success Rate |
|------|-------|--------|--------------|
| Edge Detection | Original | Edges | ⭐⭐⭐⭐ |
| Contour Detection | Edges | Contours | ⭐⭐⭐ |
| 4-Point Detection | Contours | Corners | ⭐⭐⭐ |
| Enhancement | Corrected | Final | ⭐⭐⭐⭐⭐ |

**Catatan**: Program akan menggunakan gambar simulasi jika tidak dapat mendeteksi dokumen pada gambar real.

---

## Percobaan 7: Kalibrasi Kamera 📸

**Tujuan**: Memahami proses kalibrasi kamera untuk mendapatkan parameter intrinsik dan ekstrinsik

**Teori Singkat**:
Kalibrasi kamera mengestimasi:
- **Intrinsic**: focal length (fx,fy), principal point (cx,cy)  
- **Distortion**: radial (k1,k2,k3), tangential (p1,p2)
- **Extrinsic**: rotation (R), translation (t) untuk setiap view

**Langkah-langkah**:
1. Jalankan program `07_kalibrasi_kamera.py`
2. Amati proses simulasi kalibrasi dengan checkerboard
3. Pahami hasil RMS reprojection error
4. Lihat koreksi distorsi

**Pertanyaan**:
- Apa arti RMS error < 1.0 pixel?
- Mengapa menggunakan pola checkerboard?
- Bagaimana menilai kualitas kalibrasi?

**Hasil Kalibrasi**:

| Parameter | Nilai Simulasi | Keterangan |
|-----------|----------------|------------|
| fx | ~3500 | Focal length X |
| fy | ~3500 | Focal length Y |
| cx | ~422 | Principal point X |
| cy | ~38 | Principal point Y |
| k1 | ~0.19 | Radial distortion |
| RMS Error | ~0.09 | Reprojection error |

---

## Percobaan 8: Rotasi 3D (Axis-Angle) 🧭

**Tujuan**: Memahami rotasi 3D menggunakan rumus Rodrigues (axis-angle)

**Program**: `08_3d_rotation.py`

**Langkah-langkah**:
1. Jalankan program dan amati perubahan wireframe kubus
2. Ubah `axis` dan `angle_deg`
3. Amati perbedaan arah dan besar rotasi

**Pertanyaan**:
- Apa perbedaan rotasi di sekitar sumbu X, Y, Z?
- Mengapa axis harus dinormalisasi?

---

## Percobaan 9: Proyeksi 3D ke 2D 👁️

**Tujuan**: Membandingkan proyeksi ortografik, scaled-orthographic, dan perspektif

**Program**: `09_projection_perspective.py`

**Langkah-langkah**:
1. Jalankan program dan bandingkan 3 tampilan
2. Ubah parameter `fx`, `fy`, `cx`, `cy`
3. Amati perubahan skala dan sudut pandang

**Pertanyaan**:
- Kapan ortografik lebih cocok daripada perspektif?
- Apa peran `fx` dan `fy`?

---

## Percobaan 10: Distorsi Lensa 🔍

**Tujuan**: Memahami distorsi radial (barrel/pincushion) dan koreksinya

**Program**: `10_lens_distortion.py`

**Langkah-langkah**:
1. Jalankan program dan lihat grid yang terdistorsi
2. Ubah nilai `dist` (k1, k2)
3. Bandingkan hasil sebelum dan sesudah koreksi

**Pertanyaan**:
- Bagaimana tanda k1 memengaruhi bentuk distorsi?
- Mengapa perlu koreksi distorsi pada aplikasi photogrammetry?

---

## Percobaan 11: Sampling & Aliasing 📉

**Tujuan**: Memahami efek aliasing saat downsampling

**Program**: `11_sampling_aliasing.py`

**Langkah-langkah**:
1. Jalankan program dan amati perbedaan metode downsampling
2. Bandingkan `INTER_NEAREST` vs `INTER_AREA`
3. Lihat efek blur sebelum downsample

**Pertanyaan**:
- Mengapa blur dapat mengurangi aliasing?
- Kapan `INTER_AREA` lebih baik digunakan?

---

## Percobaan 12: Color Spaces 🎨

**Tujuan**: Memahami representasi warna RGB, HSV, LAB, YCrCb, XYZ

**Program**: `12_color_spaces.py`

**Langkah-langkah**:
1. Jalankan program dan amati channel tiap color space
2. Bandingkan channel H/S/V vs L/a/b
3. Amati komponen XYZ (X,Y,Z)

**Pertanyaan**:
- Mengapa LAB lebih “perceptual”?
- Kapan memakai HSV dibanding RGB?

---

## Percobaan 13: Gamma Correction 🌗

**Tujuan**: Memahami perbedaan linear vs gamma-compressed

**Program**: `13_gamma_correction.py`

**Langkah-langkah**:
1. Jalankan program dan amati ramp intensitas
2. Bandingkan hasil encode vs decode

**Pertanyaan**:
- Mengapa gamma penting untuk tampilan?
- Apa dampaknya pada operasi linear (misal filter)?

---

## Percobaan 14: Photometric Shading 💡

**Tujuan**: Memahami komponen diffuse & specular (Lambertian + Phong)

**Program**: `14_photometric_shading.py`

**Langkah-langkah**:
1. Jalankan program dan amati diffuse, specular, combined
2. Ubah `shininess` dan `light_dir`

**Pertanyaan**:
- Bagaimana `shininess` mempengaruhi highlight?
- Mengapa diffuse tergantung dot(n, light)?

---

## Percobaan 15: Artefak Kompresi JPEG 🧩

**Tujuan**: Melihat efek kualitas JPEG terhadap detail dan blok

**Program**: `15_compression_artifacts.py`

**Langkah-langkah**:
1. Jalankan program dan bandingkan kualitas Q=95,70,40
2. Amati munculnya blockiness dan hilangnya detail

**Pertanyaan**:
- Pada kualitas berapa artefak mulai terlihat jelas?
- Mengapa teks dan tepi tajam paling terdampak?

---

### 📊 Laporan Hasil Praktikum

**Format Laporan**: PDF, maksimal 10 halaman

#### A. Bagian Wajib (80%)
1. **Cover & Identitas** (5%)
2. **Hasil Setiap Percobaan** (50%)
   - Screenshot program berjalan
   - Tabel observasi yang diisi lengkap
   - Jawaban pertanyaan analisis
3. **Analisis Perbandingan** (15%)
   - Perbandingan metode interpolasi
   - Kelebihan/kekurangan setiap transformasi
4. **Kesimpulan** (10%)

#### B. Bagian Tambahan (20%)
1. **Eksperimen Kreatif** (10%)
   - Modifikasi parameter program
   - Hasil dengan gambar sendiri
2. **Implementasi Modifikasi** (10%)
   - Tambahan fitur atau optimasi
   - Dokumentasi perubahan kode

---

### 🎯 Kriteria Penilaian

| Komponen | Bobot | Indikator Penilaian |
|----------|-------|---------------------|
| **Ketepatan Eksekusi** | 30% | Semua program berhasil dijalankan |
| **Kelengkapan Data** | 25% | Tabel observasi terisi lengkap |
| **Analisis & Jawaban** | 25% | Jawaban pertanyaan menunjukkan pemahaman |
| **Laporan & Dokumentasi** | 15% | Format rapi, struktur sistematis |
| **Kreativitas** | 5% | Eksperimen tambahan atau modifikasi |

---

### 💡 Tips Sukses

1. **Persiapan**: Pastikan semua library terinstall dengan benar
2. **Dokumentasi**: Screenshot setiap langkah penting
3. **Eksperimen**: Jangan takut ubah parameter dan amati hasilnya
4. **Analisis**: Hubungkan hasil dengan teori yang dipelajari
5. **Time Management**: Alokasikan waktu untuk setiap percobaan

---

### 🔗 Referensi

1. **Szeliski, R.** (2021). Computer Vision: Algorithms and Applications, 2nd Edition. Chapter 2.
2. **OpenCV Documentation**: Image Geometric Transformations
3. **Zhang, Z.** (2000). A Flexible New Technique for Camera Calibration

---

**Selamat Bereksperimen! 🚀**
- Mengapa dibutuhkan 4 titik untuk transformasi perspektif?
- Apa perbedaan dengan transformasi affine?

**Sketsa Titik-titik**:
```
Gambar Asli:              Hasil Transformasi:
    1──────2              1────────2
   /        \             │        │
  /          \      →     │        │
 4────────────3           4────────3
```

---

## Percobaan 6: Document Scanner

**Tujuan**: Mengaplikasikan transformasi perspektif untuk membuat document scanner

**Langkah-langkah**:
1. Jalankan program `06_document_scanner.py`
2. Load gambar dokumen dengan sudut miring
3. Klik 4 sudut dokumen secara berurutan
4. Lihat hasil dokumen yang sudah dikoreksi

**Pertanyaan**:
- Bagaimana cara otomatis mendeteksi sudut dokumen?
- Apa tantangan dalam membuat scanner otomatis?

**Evaluasi Hasil**:

| Aspek | Skor (1-5) | Catatan |
|-------|------------|---------|
| Kelurusan garis | | |
| Proporsi dokumen | | |
| Kualitas teks | | |

---

## Percobaan 7: Kalibrasi Kamera (Opsional)

**Tujuan**: Memahami proses kalibrasi kamera untuk mendapatkan parameter intrinsik

**Langkah-langkah**:
1. Print checkerboard pattern
2. Jalankan program `07_kalibrasi_kamera.py`
3. Ambil 10-15 foto checkerboard dari berbagai sudut
4. Lihat hasil matrix intrinsik dan distorsi

**Pertanyaan**:
- Apa fungsi checkerboard dalam kalibrasi?
- Bagaimana cara mengetahui kalibrasi sudah akurat?

**Hasil Kalibrasi**:
```
Camera Matrix (K):
fx = ____    fy = ____
cx = ____    cy = ____

Distortion Coefficients:
k1 = ____
k2 = ____
p1 = ____
p2 = ____
k3 = ____

Reprojection Error = ____ pixels
```

---

### 📊 Data Observasi

#### Tabel Perbandingan Transformasi

| Transformasi | Preservasi Garis Parallel | Preservasi Sudut | DOF | Contoh Penggunaan |
|--------------|---------------------------|------------------|-----|-------------------|
| Translasi | Ya | Ya | 2 | |
| Rotasi | Ya | Ya | 1 | |
| Scaling | Ya | Ya | 2 | |
| Affine | Ya | Tidak | 6 | |
| Perspektif | Tidak | Tidak | 8 | |

---

### 📈 Analisis

#### Pertanyaan Analisis

1. **Perbandingan Transformasi**
   - Jelaskan perbedaan utama antara transformasi affine dan perspektif
   - Kapan sebaiknya menggunakan masing-masing transformasi?

2. **Interpolasi**
   - Mengapa hasil resize dengan INTER_NEAREST terlihat "pixelated"?
   - Dalam kasus apa INTER_AREA lebih baik dari INTER_CUBIC?

3. **Aplikasi Praktis**
   - Sebutkan 3 aplikasi nyata dari transformasi perspektif
   - Bagaimana kalibrasi kamera membantu dalam augmented reality?

4. **Troubleshooting**
   - Apa yang menyebabkan gambar menjadi hitam setelah transformasi?
   - Bagaimana mengatasi artifact pada tepi gambar setelah rotasi?

---

### ✍️ Kesimpulan

*Tuliskan kesimpulan Anda di bawah ini:*

1. Hal baru yang dipelajari:
   
   _______________________________________________
   
   _______________________________________________

2. Kesulitan yang dihadapi:
   
   _______________________________________________
   
   _______________________________________________

3. Solusi yang ditemukan:
   
   _______________________________________________
   
   _______________________________________________

4. Aplikasi yang mungkin dibuat dengan pengetahuan ini:
   
   _______________________________________________
   
   _______________________________________________

---

### 📝 Tugas Mandiri

1. **Tugas 1**: Buat program yang dapat merotasi gambar berdasarkan deteksi wajah (auto-rotate portrait)

2. **Tugas 2**: Modifikasi document scanner untuk mendeteksi sudut dokumen secara otomatis menggunakan edge detection

3. **Tugas 3**: Buat aplikasi yang menggabungkan 2 gambar dengan transformasi perspektif (simple panorama)

---

### ✅ Checklist Penyelesaian

- [ ] Percobaan 1: Translasi selesai
- [ ] Percobaan 2: Rotasi selesai
- [ ] Percobaan 3: Scaling selesai
- [ ] Percobaan 4: Affine Transform selesai
- [ ] Percobaan 5: Perspective Transform selesai
- [ ] Percobaan 6: Document Scanner selesai
- [ ] Percobaan 7: Kalibrasi Kamera (opsional)
- [ ] Analisis dijawab lengkap
- [ ] Kesimpulan ditulis
- [ ] Tugas mandiri dikerjakan

---

**Catatan Asisten**:

| Aspek | Nilai | Catatan |
|-------|-------|---------|
| Persiapan | /10 | |
| Pelaksanaan | /30 | |
| Laporan | /30 | |
| Analisis | /20 | |
| Kesimpulan | /10 | |
| **Total** | **/100** | |

---

*Jobsheet ini adalah bagian dari Praktikum Computer Vision*
