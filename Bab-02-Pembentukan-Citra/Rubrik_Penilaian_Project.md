# Rubrik Penilaian Project
## Bab 2: Sistem Koreksi Perspektif Dokumen Otomatis

---

## 📊 Komponen Penilaian

| Komponen | Bobot | Kriteria |
|----------|-------|----------|
| Fungsionalitas | 40% | Sistem berfungsi sesuai spesifikasi |
| Kualitas Kode | 20% | Clean code, dokumentasi, struktur |
| Inovasi & Kreativitas | 15% | Fitur tambahan di luar requirement |
| Laporan | 15% | Dokumentasi dan penjelasan |
| Demo & Presentasi | 10% | Kemampuan menjelaskan dan demo |

---

## 🎯 Rubrik Detail

### 1. Fungsionalitas (40%)

#### A. Deteksi Dokumen (15%)

| Skor | Kriteria |
|------|----------|
| **90-100** | Deteksi akurat pada berbagai kondisi (sudut, pencahayaan, background berbeda). Support multi-dokumen. Threshold adaptif. |
| **80-89** | Deteksi akurat pada kondisi ideal dan sebagian kondisi challenging. Handling dokumen tunggal dengan baik. |
| **70-79** | Deteksi berhasil pada kondisi ideal dengan kontras tinggi. Terkadang gagal pada sudut ekstrem. |
| **60-69** | Deteksi berhasil hanya pada kondisi sangat ideal. Sering memerlukan manual adjustment. |
| **< 60** | Deteksi tidak konsisten. Sering gagal bahkan pada kondisi ideal. |

#### B. Koreksi Perspektif (15%)

| Skor | Kriteria |
|------|----------|
| **90-100** | Koreksi perspektif sangat akurat. Hasil selalu tegak dan proporsional. Support berbagai ukuran output. |
| **80-89** | Koreksi perspektif akurat pada kebanyakan kasus. Hasil proporsional dengan distorsi minimal. |
| **70-79** | Koreksi perspektif berhasil tetapi kadang ada sedikit distorsi atau cropping yang tidak sempurna. |
| **60-69** | Koreksi perspektif sering tidak sempurna. Hasil masih memiliki distorsi yang terlihat. |
| **< 60** | Koreksi perspektif gagal atau hasil sangat tidak akurat. |

#### C. Enhancement & Output (10%)

| Skor | Kriteria |
|------|----------|
| **90-100** | Multiple enhancement options (B&W, color, high contrast). Auto-optimize. Multi-format output (JPEG, PNG, PDF). |
| **80-89** | Enhancement options lengkap. Output berkualitas tinggi dengan kompresi optimal. |
| **70-79** | Enhancement dasar berfungsi dengan baik. Output dalam format umum. |
| **60-69** | Enhancement minimal. Output hanya satu format dengan kualitas standar. |
| **< 60** | Tidak ada enhancement. Output dengan artifacts atau kualitas rendah. |

---

### 2. Kualitas Kode (20%)

#### A. Struktur dan Organisasi (8%)

| Skor | Kriteria |
|------|----------|
| **90-100** | Struktur modular sempurna. Separation of concerns. Design patterns yang tepat. Unit testable. |
| **80-89** | Struktur modular baik. Fungsi-fungsi terpisah dengan baik. Mudah di-maintain. |
| **70-79** | Struktur cukup terorganisir. Beberapa fungsi terlalu panjang atau kompleks. |
| **60-69** | Struktur kurang terorganisir. Banyak code repetition. |
| **< 60** | Tidak ada struktur jelas. Semua kode dalam satu file tanpa fungsi yang jelas. |

#### B. Dokumentasi Kode (6%)

| Skor | Kriteria |
|------|----------|
| **90-100** | Docstrings lengkap untuk semua fungsi. Type hints. README komprehensif. Inline comments yang membantu. |
| **80-89** | Docstrings untuk fungsi utama. README yang jelas. Comments pada bagian kompleks. |
| **70-79** | Dokumentasi dasar ada. Beberapa fungsi tanpa penjelasan. README minimal. |
| **60-69** | Dokumentasi minim. README tidak lengkap. Sulit memahami alur kode. |
| **< 60** | Tidak ada dokumentasi. Tidak ada README. |

#### C. Error Handling & Edge Cases (6%)

| Skor | Kriteria |
|------|----------|
| **90-100** | Error handling komprehensif. Graceful degradation. Informative error messages. Logging system. |
| **80-89** | Error handling untuk semua operasi I/O dan kasus umum. Pesan error yang jelas. |
| **70-79** | Error handling dasar. Program tidak crash pada input tidak valid. |
| **60-69** | Error handling minimal. Program kadang crash pada edge cases. |
| **< 60** | Tidak ada error handling. Program mudah crash. |

---

### 3. Inovasi & Kreativitas (15%)

| Skor | Kriteria |
|------|----------|
| **90-100** | Fitur sangat inovatif: ML-based detection, OCR integration, cloud sync, AR preview, batch processing dengan progress bar, koreksi distorsi otomatis, dll. |
| **80-89** | Fitur tambahan yang berguna: GUI interaktif, batch processing, PDF export, history/undo, preset profiles, enhancement warna/gamma. |
| **70-79** | Beberapa fitur tambahan: Preview sebelum save, basic settings, keyboard shortcuts. |
| **60-69** | Fitur minimal di luar requirement: Hanya memenuhi spesifikasi dasar dengan sedikit improvement. |
| **< 60** | Tidak ada fitur tambahan. Hanya memenuhi requirement minimum. |

---

### 4. Laporan (15%)

#### A. Analisis Algoritma (6%)

| Skor | Kriteria |
|------|----------|
| **90-100** | Analisis mendalam: kompleksitas waktu/ruang, trade-offs, perbandingan dengan alternatif, justifikasi pilihan algoritma. |
| **80-89** | Analisis baik dengan penjelasan mengapa algoritma tertentu dipilih. Ada perbandingan sederhana. |
| **70-79** | Penjelasan algoritma yang digunakan tanpa analisis mendalam. |
| **60-69** | Penjelasan superficial tentang algoritma. Tidak ada analisis. |
| **< 60** | Tidak ada penjelasan algoritma. |

#### B. Hasil Pengujian (5%)

| Skor | Kriteria |
|------|----------|
| **90-100** | Testing komprehensif: unit tests, integration tests, test dengan berbagai dataset, metrics (akurasi, waktu, dll). |
| **80-89** | Testing yang baik dengan berbagai skenario. Dokumentasi hasil yang jelas. |
| **70-79** | Testing dasar dengan beberapa contoh. Screenshot hasil. |
| **60-69** | Testing minimal. Hanya satu atau dua contoh. |
| **< 60** | Tidak ada bukti testing. |

#### C. Format dan Kelengkapan (4%)

| Skor | Kriteria |
|------|----------|
| **90-100** | Format profesional. Semua section lengkap. Gambar/diagram berkualitas. Referensi lengkap. |
| **80-89** | Format baik. Semua section ada. Beberapa gambar pendukung. |
| **70-79** | Format cukup. Sebagian besar section lengkap. |
| **60-69** | Format kurang rapi. Beberapa section tidak lengkap. |
| **< 60** | Format buruk. Banyak section tidak ada. |

---

### 5. Demo & Presentasi (10%)

| Skor | Kriteria |
|------|----------|
| **90-100** | Demo smooth tanpa error. Mampu menjawab semua pertanyaan teknis. Penjelasan sangat jelas dan terstruktur. |
| **80-89** | Demo berjalan baik dengan minor issues. Mampu menjawab sebagian besar pertanyaan. Penjelasan jelas. |
| **70-79** | Demo berhasil dengan beberapa kendala. Mampu menjawab pertanyaan dasar. Penjelasan cukup. |
| **60-69** | Demo dengan banyak kendala. Kesulitan menjawab pertanyaan teknis. Penjelasan kurang jelas. |
| **< 60** | Demo gagal. Tidak mampu menjawab pertanyaan. Tidak ada penjelasan yang bermakna. |

---

## 📋 Form Penilaian

### Identitas Mahasiswa
- **Nama**: _______________
- **NIM**: _______________
- **Kelompok**: _______________

### Skor Penilaian

| Komponen | Bobot | Skor (0-100) | Nilai Tertimbang |
|----------|-------|--------------|------------------|
| Fungsionalitas | 40% | _____ | _____ |
| Kualitas Kode | 20% | _____ | _____ |
| Inovasi & Kreativitas | 15% | _____ | _____ |
| Laporan | 15% | _____ | _____ |
| Demo & Presentasi | 10% | _____ | _____ |
| **TOTAL** | **100%** | | **_____** |

### Catatan Penilai

**Kelebihan:**
```
_________________________________________________________________
_________________________________________________________________
```

**Kekurangan:**
```
_________________________________________________________________
_________________________________________________________________
```

**Saran Perbaikan:**
```
_________________________________________________________________
_________________________________________________________________
```

---

## 📏 Konversi Nilai

| Skor | Huruf | Keterangan |
|------|-------|------------|
| 85-100 | A | Sangat Baik |
| 80-84 | A- | Baik Sekali |
| 75-79 | B+ | Baik |
| 70-74 | B | Cukup Baik |
| 65-69 | B- | Cukup |
| 60-64 | C+ | Kurang |
| 55-59 | C | Kurang Sekali |
| < 55 | D/E | Tidak Lulus |

---

## ⚠️ Ketentuan Khusus

### Pengurangan Nilai

| Pelanggaran | Pengurangan |
|-------------|-------------|
| Terlambat submit (per hari) | -5 poin |
| Plagiarisme | -50 s/d -100 poin |
| Tidak ada demo | -20 poin |
| Kode tidak bisa dijalankan | -30 poin |

### Bonus Nilai (maksimal +10)

| Pencapaian | Bonus |
|------------|-------|
| Deploy sebagai web app | +5 poin |
| Integrasi dengan cloud storage | +3 poin |
| Mobile app version | +5 poin |
| Performance optimization dengan profiling | +3 poin |
| Comprehensive test suite (>80% coverage) | +3 poin |

---

*Rubrik ini dapat disesuaikan sesuai kebijakan dosen pengampu*
