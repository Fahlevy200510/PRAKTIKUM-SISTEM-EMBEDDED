# 📋 Project: Sistem Koreksi Perspektif Dokumen Otomatis
## Bab 2: Pembentukan Citra (Image Formation)

---

## 🎯 Deskripsi Project

### Latar Belakang

Di era digital, kebutuhan untuk mengonversi dokumen fisik menjadi format digital sangat tinggi. Aplikasi scanner dokumen pada smartphone menjadi sangat populer karena memungkinkan pengguna untuk "memindai" dokumen hanya dengan mengambil foto. Namun, foto yang diambil seringkali memiliki perspektif yang miring atau tidak sejajar, sehingga diperlukan koreksi perspektif otomatis.

### Cerita/Skenario

Anda adalah developer di startup "QuickScan Indonesia" yang sedang mengembangkan aplikasi mobile untuk digitalisasi dokumen UMKM. Banyak pelaku UMKM yang masih menyimpan nota, kwitansi, dan faktur dalam bentuk fisik. Tugas Anda adalah membuat modul **koreksi perspektif otomatis** yang dapat:

1. Mendeteksi tepi dokumen dalam foto
2. Mengidentifikasi 4 sudut dokumen
3. Melakukan transformasi perspektif
4. Menghasilkan gambar dokumen yang rata dan proporsional

### Kegunaan di Dunia Nyata

- **Scanning dokumen**: CamScanner, Adobe Scan, Microsoft Lens
- **Digitalisasi arsip**: Perpustakaan, kantor pemerintah
- **E-commerce**: Foto produk dari berbagai sudut
- **Pendidikan**: Foto papan tulis/whiteboard

---

## 📝 Spesifikasi Project

### Fitur Wajib (Minimum Viable Product)

1. **Input Handling**
   - Dapat membaca gambar dari file
   - Dapat menerima input dari kamera (live capture)
   - Support format: JPG, PNG, BMP

2. **Deteksi Dokumen**
   - Deteksi tepi menggunakan Canny edge detection
   - Menemukan kontur terbesar (dokumen)
   - Identifikasi 4 titik sudut dokumen

3. **Koreksi Perspektif**
   - Transformasi perspektif ke view bird's eye
   - Output dengan rasio aspek yang benar
   - Ukuran output dapat dikonfigurasi (A4, Letter, dll)

4. **Output**
   - Simpan hasil dalam format yang dipilih
   - Tampilkan preview sebelum/sesudah
   - Option untuk grayscale atau berwarna

### Fitur Tambahan (Bonus)

- [ ] Mode batch processing (multiple dokumen)
- [ ] Enhancement otomatis (brightness, contrast)
- [ ] OCR integration untuk ekstrak teks
- [ ] GUI sederhana dengan Tkinter/PyQt
- [ ] Export ke PDF

---

## 🔧 Panduan Implementasi

### Tahap 1: Persiapan (Week 1)

```
1. Setup environment
2. Siapkan gambar test (foto dokumen dari berbagai sudut)
3. Pelajari fungsi-fungsi OpenCV yang diperlukan:
   - cv2.Canny()
   - cv2.findContours()
   - cv2.approxPolyDP()
   - cv2.getPerspectiveTransform()
   - cv2.warpPerspective()

**Referensi Program Praktikum** (sebagai bahan belajar/validasi):
- `05_perspektif_transform.py` (homography dasar)
- `06_document_scanner.py` (pipeline lengkap scanner)
- `10_lens_distortion.py` (distorsi dan koreksi)
- `12_color_spaces.py` (analisis warna untuk enhancement)
```

### Tahap 2: Implementasi Core (Week 2)

```python
# Pseudocode alur utama

def process_document(image_path):
    # 1. Load dan preprocessing
    image = load_image(image_path)
    preprocessed = preprocess(image)  # grayscale, blur, dll
    
    # 2. Deteksi tepi
    edges = detect_edges(preprocessed)
    
    # 3. Temukan kontur dokumen
    contours = find_contours(edges)
    document_contour = find_document(contours)
    
    # 4. Dapatkan 4 sudut
    corners = get_corners(document_contour)
    
    # 5. Transformasi perspektif
    warped = perspective_transform(image, corners)
    
    # 6. Post-processing (optional)
    enhanced = enhance_document(warped)
    
    return enhanced
```

### Tahap 3: Pengujian dan Refinement (Week 3)

1. Test dengan berbagai kondisi:
   - Pencahayaan berbeda
   - Sudut berbeda
   - Warna background berbeda
   - Ukuran dokumen berbeda

2. Handle edge cases:
   - Dokumen tidak terdeteksi
   - Multiple dokumen dalam satu foto
   - Dokumen dengan sudut terpotong

---

## 📊 Kriteria Penilaian

### Rubrik Penilaian

| Komponen | Bobot | Kriteria |
|----------|-------|----------|
| **Fungsionalitas** | 40% | Program berjalan sesuai spesifikasi |
| **Akurasi** | 20% | Deteksi dan transformasi akurat |
| **Kode** | 20% | Clean code, dokumentasi, struktur |
| **Laporan** | 10% | Dokumentasi lengkap |
| **Kreativitas** | 10% | Fitur tambahan, UI, improvement |

### Detail Penilaian Fungsionalitas

| Fitur | Poin |
|-------|------|
| Load gambar dari file | 5 |
| Deteksi tepi berfungsi | 10 |
| Deteksi 4 sudut berfungsi | 15 |
| Transformasi perspektif berfungsi | 15 |
| Simpan output berfungsi | 5 |
| Preview sebelum/sesudah | 5 |
| Capture dari kamera | 10 |
| Error handling | 10 |
| User interface | 10 |
| Fitur bonus (per fitur) | 5-15 |

---

## 📁 Struktur Folder Project

```
Bab-02-Project/
├── main.py                 # Entry point program
├── document_scanner.py     # Modul utama scanner
├── utils/
│   ├── __init__.py
│   ├── image_utils.py      # Fungsi utilitas gambar
│   └── geometry.py         # Fungsi geometri
├── tests/
│   ├── test_images/        # Gambar untuk testing
│   └── test_scanner.py     # Unit tests
├── output/                 # Folder hasil scan
├── requirements.txt        # Dependencies
└── README.md              # Dokumentasi project
```

**Catatan Output**:
- Simpan hasil ke folder `output/` project dan sertakan preview sebelum/ sesudah.

---

## 📚 Referensi dan Hints

### Fungsi OpenCV yang Berguna

```python
# Edge Detection
edges = cv2.Canny(gray, 50, 150)

# Find Contours
contours, _ = cv2.findContours(edges, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

# Approximate Polygon
epsilon = 0.02 * cv2.arcLength(contour, True)
approx = cv2.approxPolyDP(contour, epsilon, True)

# Perspective Transform
M = cv2.getPerspectiveTransform(src_pts, dst_pts)
warped = cv2.warpPerspective(image, M, (width, height))
```

### Tips Mengurutkan Sudut

```
Urutan sudut yang benar:
    0 (top-left) -------- 1 (top-right)
         |                      |
         |                      |
    3 (bottom-left) ---- 2 (bottom-right)
```

### Algorithm untuk Sort Corners

```python
def order_points(pts):
    # Urutkan berdasarkan x+y (top-left punya sum terkecil)
    # Urutkan berdasarkan y-x (top-right punya diff terkecil)
    # ... implementasi
    return ordered_pts
```

---

## 📅 Timeline Pengerjaan

| Minggu | Aktivitas | Deliverable |
|--------|-----------|-------------|
| 1 | Setup, riset, desain | Rencana & pseudocode |
| 2 | Implementasi core | MVP berfungsi |
| 3 | Testing & refinement | Program final |
| 4 | Dokumentasi & video | Laporan + demo |

---

## 📤 Deliverables

1. **Source Code**
   - File Python yang berfungsi
   - Komentar yang jelas
   - README dengan instruksi

2. **Laporan**
   - Pendahuluan dan tujuan
   - Metodologi/algoritma
   - Hasil dan pembahasan
   - Kesimpulan

3. **Demo Video**
   - Durasi: 3-5 menit
   - Tunjukkan semua fitur
   - Jelaskan cara kerja

4. **Presentasi**
   - Slide (5-10 halaman)
   - Live demo

---

## ❓ FAQ

**Q: Bagaimana jika dokumen tidak terdeteksi?**
A: Implementasikan fallback ke mode manual di mana user dapat mengklik 4 sudut.

**Q: Bagaimana menangani dokumen dengan warna mirip background?**
A: Coba gunakan adaptive thresholding atau tambahkan preprocessing dengan morphological operations.

**Q: Boleh menggunakan library lain selain OpenCV?**
A: Boleh, tapi core computer vision harus menggunakan OpenCV. Library pendukung (GUI, PDF) boleh berbeda.

---

## 🏆 Contoh Project Serupa untuk Inspirasi

1. **OpenCV Document Scanner** - PyImageSearch
2. **DocScanner** - GitHub open source
3. **Simple Scanner** - Contoh di dokumentasi OpenCV

---

*Project ini adalah bagian dari Praktikum Computer Vision Bab 2: Pembentukan Citra*
