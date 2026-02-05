# ============================================================
# PROGRAM: 01_translasi.py
# PRAKTIKUM COMPUTER VISION - BAB 2: PEMBENTUKAN CITRA
# ============================================================
# Deskripsi: Program ini mendemonstrasikan translasi (pergeseran)
#            gambar menggunakan transformation matrix
# 
# Tujuan Pembelajaran:
#   1. Memahami konsep translasi dalam transformasi geometri
#   2. Memahami penggunaan cv2.warpAffine()
#   3. Memahami struktur translation matrix
# ============================================================

# ====================
# IMPORT LIBRARY
# ====================
import cv2
import numpy as np
import matplotlib
matplotlib.use('Agg')  # Gunakan backend non-GUI
import matplotlib.pyplot as plt
import os
# REFERENSI: Lihat CV2_FUNCTIONS_REFERENCE.py untuk dokumentasi lengkap cv2 functions


# ============================================================
# PANDUAN FUNGSI (RINGKAS) - ARTI PARAMETER
# ============================================================
# cv2.imread(path)
#   - path : lokasi file gambar
#
# cv2.resize(img, dsize/None, fx, fy)
#   - dsize : (lebar, tinggi) output, atau None
#   - fx, fy: skala horizontal/vertikal jika dsize=None
#
# cv2.warpAffine(src, M, dsize, borderMode, borderValue)
#   - src        : gambar input
#   - M          : matriks transformasi 2x3
#   - dsize      : (lebar, tinggi) output
#   - borderMode : mode pengisian area kosong
#   - borderValue: warna area kosong jika mode CONSTANT
#
# cv2.rectangle(img, pt1, pt2, color, thickness)
#   - pt1/pt2 : sudut kiri-atas & kanan-bawah
#   - color   : warna BGR
#   - thickness: ketebalan; -1 artinya isi penuh
#
# cv2.circle(img, center, radius, color, thickness)
#   - center : titik pusat
#   - radius : jari-jari lingkaran
#
# cv2.putText(img, text, org, fontFace, fontScale, color, thickness)
#   - org : posisi teks (x, y)
#
# matplotlib.pyplot.savefig(path, dpi, bbox_inches)
#   - path : lokasi simpan gambar

# ============================================================
# VARIABEL YANG BISA DIUBAH-UBAH (EKSPERIMEN)
# ============================================================

# 1. File gambar yang akan diproses
NAMA_FILE_GAMBAR = "portrait.jpg"

# 2. Nilai translasi dalam piksel
#    tx: pergeseran horizontal (positif = ke kanan)
#    ty: pergeseran vertikal (positif = ke bawah)
TX = 100  # Coba ubah: -100, 0, 50, 150
TY = 50   # Coba ubah: -50, 0, 30, 100

# 3. Mode border (apa yang mengisi area kosong)
#    cv2.BORDER_CONSTANT: Warna solid (hitam default)
#    cv2.BORDER_REPLICATE: Replikasi piksel tepi
#    cv2.BORDER_REFLECT: Cermin
MODE_BORDER = cv2.BORDER_CONSTANT

# 4. Warna border jika BORDER_CONSTANT (BGR)
WARNA_BORDER = (50, 50, 50)  # Abu-abu gelap

# ============================================================
# KONFIGURASI PATH
# ============================================================

# Dapatkan direktori script (praktiukm folder)
DIR_SCRIPT = os.path.dirname(os.path.abspath(__file__))
DIR_DATA = os.path.join(DIR_SCRIPT, "data", "images")
DIR_OUTPUT = os.path.join(DIR_SCRIPT, "output", "output1")

# Pastikan folder output ada
os.makedirs(DIR_OUTPUT, exist_ok=True)

# ============================================================
# FUNGSI HELPER
# ============================================================

# Keterangan: Menentukan path file gambar dari beberapa lokasi kandidat.
def dapatkan_path_gambar(nama_file):
    """Mendapatkan path lengkap file gambar.

    Parameter:
    - nama_file (str): nama file gambar.

    Return:
    - str: path absolut/relatif yang ditemukan.
    """
    direktori_script = os.path.dirname(os.path.abspath(__file__))
    # Bentuk path lengkap file
    path_data = os.path.join(direktori_script, "data", "images", nama_file)
    
    # Cek kondisi logis
    if not os.path.exists(path_data):
        # Coba folder Bab-01
        path_data = os.path.join(direktori_script, "..", "..", 
                                  "Bab-01-Pendahuluan", "data", "images", nama_file)
    
    # Cek kondisi logis
    if not os.path.exists(path_data):
        # Bentuk path lengkap file
        path_data = os.path.join(direktori_script, nama_file)
    
    # Kembalikan hasil dari fungsi
    return path_data


# Keterangan: Membuat gambar sintetis untuk demo translasi.
def buat_gambar_sample():
    """Membuat gambar sample dengan shapes untuk demonstrasi.

    Return:
    - np.ndarray: gambar buatan berformat BGR.
    """
    # Inisialisasi kanvas gambar kosong (hitam) dengan ukuran 400x600 BGR
    gambar = np.zeros((400, 600, 3), dtype=np.uint8)
    
    # Buat gradient warna vertikal pada background untuk efek visual
    for i in range(400):
        gambar[i, :] = [int(i/2), int(100 + i/4), int(200 - i/4)]
    
    # Gambar persegi putih (isi penuh) sebagai elemen referensi posisi
    cv2.rectangle(gambar, (50, 50), (150, 150), (255, 255, 255), -1)
    # Gambar lingkaran cyan (isi penuh) sebagai elemen referensi
    cv2.circle(gambar, (300, 200), 60, (0, 255, 255), -1)
    # Tambahkan teks "OpenCV" untuk memudahkan melihat pergeseran
    cv2.putText(gambar, "OpenCV", (200, 350), 
                cv2.FONT_HERSHEY_SIMPLEX, 1.5, (255, 255, 255), 2)
    
    # Gambar garis horizontal abu-abu sebagai referensi sumbu-x
    cv2.line(gambar, (0, 200), (600, 200), (128, 128, 128), 1)
    # Gambar garis vertikal abu-abu sebagai referensi sumbu-y
    cv2.line(gambar, (300, 0), (300, 400), (128, 128, 128), 1)
    
    # Kembalikan gambar yang telah dibuat
    return gambar


# ============================================================
# FUNGSI TRANSLASI
# ============================================================

# Keterangan: Melakukan translasi (pergeseran) gambar.
def translasi_gambar(gambar, tx, ty, border_mode=cv2.BORDER_CONSTANT,
                     border_value=(0, 0, 0)):
    """Melakukan translasi (pergeseran) gambar.

    Parameter:
    - gambar (np.ndarray): gambar input (BGR).
    - tx (int/float): pergeseran horizontal (piksel).
    - ty (int/float): pergeseran vertikal (piksel).
    - border_mode (int): mode pengisian area kosong (cv2.BORDER_*).
    - border_value (tuple[int, int, int]): warna border jika BORDER_CONSTANT.

    Return:
    - np.ndarray: gambar yang sudah ditranslasi.
    - np.ndarray: matriks translasi 2x3.
    """
    # Ambil dimensi gambar (tinggi dan lebar)
    tinggi, lebar = gambar.shape[:2]
    
    # Translation Matrix:
    # | 1  0  tx |
    # | 0  1  ty |
    #
    # Persamaan:
    # x' = 1*x + 0*y + tx = x + tx
    # y' = 0*x + 1*y + ty = y + ty
    
    # Buat matriks translasi 2x3 dengan tipe float32
    translation_matrix = np.float32([
        [1, 0, tx],  # x' = x + tx
        [0, 1, ty]   # y' = y + ty
    ])
    
    # Terapkan transformasi affine menggunakan matriks translasi
    hasil = cv2.warpAffine(
        gambar,                        # Gambar input yang akan ditranslasi
        translation_matrix,            # Matriks transformasi 2x3
        (lebar, tinggi),              # Ukuran output (width, height)
        borderMode=border_mode,       # Modo pengisian area kosong
        borderValue=border_value      # Warna untuk border jika BORDER_CONSTANT
    )
    
    # Kembalikan gambar hasil dan matriks transformasi
    return hasil, translation_matrix


# Keterangan: Menampilkan translasi ke berbagai arah.
def demo_translasi_berbagai_arah(gambar):
    """Mendemonstrasikan translasi ke berbagai arah.

    Parameter:
    - gambar (np.ndarray): gambar input (BGR).
    """
    # Tampilkan header informasi demo
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("DEMO TRANSLASI KE BERBAGAI ARAH")
    # Cetak informasi ke console
    print("=" * 60)
    
    # Tentukan daftar translasi yang akan dicoba dengan label
    translasi_list = [
        (0, 0, "Original"),
        (100, 0, "Kanan (+tx)"),
        (-100, 0, "Kiri (-tx)"),
        (0, 80, "Bawah (+ty)"),
        (0, -80, "Atas (-ty)"),
        (100, 80, "Kanan-Bawah"),
        (-100, -80, "Kiri-Atas"),
    ]
    
    # Siapkan kanvas plot 2x4 (8 subplots) untuk menampilkan hasil
    fig, axes = plt.subplots(2, 4, figsize=(16, 8))
    axes = axes.flatten()
    
    # Iterasi melalui setiap kombinasi translasi
    for i, (tx, ty, label) in enumerate(translasi_list):
        # Terapkan translasi pada gambar dengan parameter tx, ty
        hasil, _ = translasi_gambar(gambar, tx, ty, 
                                    border_mode=cv2.BORDER_CONSTANT,
                                    border_value=WARNA_BORDER)
        
        # Konversi gambar dari BGR ke RGB untuk matplotlib display
        axes[i].imshow(cv2.cvtColor(hasil, cv2.COLOR_BGR2RGB))
        # Set judul subplot dengan label dan nilai translasi
        axes[i].set_title(f"{label}\ntx={tx}, ty={ty}")
        # Nonaktifkan axis untuk tampilan lebih rapi
        axes[i].axis('off')
        
        # Cetak informasi translasi ke console
        print(f"[{i+1}] {label}: tx={tx}, ty={ty}")
    
    # Kosongkan subplot terakhir (karena hanya 7 item, subplot ke-8 kosong)
    axes[7].axis('off')
    
    # Set judul keseluruhan figure
    plt.suptitle("Translasi Gambar ke Berbagai Arah", fontsize=14)
    # Atur spacing antar subplot agar tidak overlap
    plt.tight_layout()
    # Bentuk path lengkap untuk menyimpan output
    output_path = os.path.join(DIR_OUTPUT, 'demo_arah.png')
    # Simpan figure ke file dengan DPI tertentu
    plt.savefig(output_path, dpi=100, bbox_inches='tight')
    # Cetak pesan sukses dengan path file
    print(f"[SAVED] Gambar disimpan ke: {output_path}")
    # Tutup figure untuk menghemat memory
    plt.close()


# Keterangan: Membandingkan beberapa mode border pada translasi.
def demo_border_modes(gambar):
    """Mendemonstrasikan berbagai mode border pada translasi.

    Parameter:
    - gambar (np.ndarray): gambar input (BGR).
    """
    # Tampilkan header informasi demo border modes
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("DEMO BERBAGAI MODE BORDER")
    # Cetak informasi ke console
    print("=" * 60)
    
    # Tentukan nilai translasi yang cukup besar agar efek border terlihat jelas
    tx, ty = 150, 100
    
    # Daftar mode border yang akan diuji (mode, nilai warna, label)
    border_modes = [
        (cv2.BORDER_CONSTANT, (0, 0, 0), "BORDER_CONSTANT (Hitam)"),
        (cv2.BORDER_CONSTANT, (255, 0, 0), "BORDER_CONSTANT (Biru)"),
        (cv2.BORDER_REPLICATE, None, "BORDER_REPLICATE"),
        (cv2.BORDER_REFLECT, None, "BORDER_REFLECT"),
        (cv2.BORDER_WRAP, None, "BORDER_WRAP"),
        (cv2.BORDER_REFLECT_101, None, "BORDER_REFLECT_101"),
    ]
    
    # Siapkan kanvas plot 2x3 untuk menampilkan 6 mode border
    fig, axes = plt.subplots(2, 3, figsize=(15, 10))
    axes = axes.flatten()
    
    # Iterasi melalui setiap mode border
    for i, (mode, value, label) in enumerate(border_modes):
        # Jika value None, gunakan warna default hitam
        if value is None:
            value = (0, 0, 0)
        
        # Terapkan translasi dengan mode border tertentu
        hasil, _ = translasi_gambar(gambar, tx, ty, 
                                    border_mode=mode,
                                    border_value=value)
        
        # Tampilkan hasil translasi pada subplot
        axes[i].imshow(cv2.cvtColor(hasil, cv2.COLOR_BGR2RGB))
        # Set judul subplot dengan nama mode border
        axes[i].set_title(f"{label}")
        # Nonaktifkan axis untuk tampilan lebih rapi
        axes[i].axis('off')
        
        # Cetak informasi mode border ke console
        print(f"[{i+1}] {label}")
    
    # Set judul keseluruhan figure dengan nilai translasi
    plt.suptitle(f"Perbandingan Border Mode (tx={tx}, ty={ty})", fontsize=14)
    # Atur spacing antar subplot
    plt.tight_layout()
    # Bentuk path lengkap untuk menyimpan output
    output_path = os.path.join(DIR_OUTPUT, 'demo_border_modes.png')
    # Simpan figure ke file
    plt.savefig(output_path, dpi=100, bbox_inches='tight')
    # Cetak pesan sukses
    print(f"[SAVED] Gambar disimpan ke: {output_path}")
    # Tutup figure
    plt.close()


# Keterangan: Memvisualisasikan kerja matriks translasi.
def visualisasi_matrix_translasi(gambar):
    """Visualisasi cara kerja translation matrix.

    Parameter:
    - gambar (np.ndarray): gambar input (BGR).
    """
    # Tampilkan header informasi visualisasi matriks
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("VISUALISASI TRANSLATION MATRIX")
    # Cetak informasi ke console
    print("=" * 60)
    
    # Cetak penjelasan teori translation matrix
    print("""
TRANSLATION MATRIX:
    
    | 1  0  tx |     | x |     | x + tx |
    |          |  x  |   |  =  |        |
    | 0  1  ty |     | y |     | y + ty |
    
Dimana:
- (x, y) adalah koordinat piksel asli
- (tx, ty) adalah nilai pergeseran
- Hasil adalah koordinat baru (x + tx, y + ty)

CONTOH:
Jika tx = 100, ty = 50
- Piksel di (0, 0) → pindah ke (100, 50)
- Piksel di (50, 30) → pindah ke (150, 80)
- Piksel di (100, 100) → pindah ke (200, 150)
""")
    
    # Cetak matriks translasi untuk nilai saat ini (TX, TY)
    print(f"\nTranslation Matrix untuk tx={TX}, ty={TY}:")
    # Cetak informasi ke console
    print(f"    | 1  0  {TX:3d} |")
    # Cetak informasi ke console
    print(f"    | 0  1  {TY:3d} |")
    
    # Siapkan kanvas plot 1x2 untuk perbandingan sebelum-sesudah
    fig, axes = plt.subplots(1, 2, figsize=(14, 6))
    
    # Buat salinan gambar untuk visualisasi gambar asli
    gambar_ref = gambar.copy()
    # Tentukan 4 titik referensi (sudut persegi) untuk demo
    points = [(100, 100), (300, 100), (300, 300), (100, 300)]
    # Tentukan warna untuk setiap titik (RGB)
    colors = [(255, 0, 0), (0, 255, 0), (0, 0, 255), (255, 255, 0)]
    
    # Iterasi melalui setiap titik referensi
    for pt, color in zip(points, colors):
        # Gambar lingkaran pada setiap titik referensi
        cv2.circle(gambar_ref, pt, 10, color, -1)
        # Tambahkan teks koordinat di dekat setiap titik
        cv2.putText(gambar_ref, f"({pt[0]},{pt[1]})", (pt[0]+15, pt[1]), 
                   cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)
    
    # Tampilkan gambar asli dengan titik referensi pada subplot pertama
    axes[0].imshow(cv2.cvtColor(gambar_ref, cv2.COLOR_BGR2RGB))
    # Set judul untuk subplot
    axes[0].set_title("Gambar Asli dengan Titik Referensi")
    # Nonaktifkan atau atur axis pada subplot
    axes[0].axis('off')
    
    # Terapkan translasi pada gambar
    hasil, _ = translasi_gambar(gambar, TX, TY, cv2.BORDER_CONSTANT, WARNA_BORDER)
    
    # Iterasi melalui setiap titik untuk menunjukkan posisi baru setelah translasi
    for pt, color in zip(points, colors):
        # Hitung posisi baru berdasarkan matriks translasi
        # Hitung posisi baru berdasarkan matriks translasi
        new_pt = (pt[0] + TX, pt[1] + TY)
        # Cek apakah titik baru masih dalam batas gambar
        if 0 <= new_pt[0] < hasil.shape[1] and 0 <= new_pt[1] < hasil.shape[0]:
            # Gambar lingkaran pada posisi baru titik
            cv2.circle(hasil, new_pt, 10, color, -1)
            # Tambahkan teks koordinat baru di dekat titik
            cv2.putText(hasil, f"({new_pt[0]},{new_pt[1]})", (new_pt[0]+15, new_pt[1]), 
                       cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)
    
    # Tampilkan gambar hasil translasi dengan titik baru pada subplot kedua
    axes[1].imshow(cv2.cvtColor(hasil, cv2.COLOR_BGR2RGB))
    # Set judul untuk subplot
    axes[1].set_title(f"Setelah Translasi (tx={TX}, ty={TY})")
    # Nonaktifkan atau atur axis pada subplot
    axes[1].axis('off')
    
    # Set judul keseluruhan figure
    plt.suptitle("Pergerakan Titik Referensi Setelah Translasi", fontsize=14)
    # Atur spacing antar subplot
    plt.tight_layout()
    # Bentuk path lengkap untuk menyimpan output
    output_path = os.path.join(DIR_OUTPUT, 'demo_matrix_visualization.png')
    # Simpan figure ke file
    plt.savefig(output_path, dpi=100, bbox_inches='tight')
    # Cetak pesan sukses
    print(f"[SAVED] Gambar disimpan ke: {output_path}")
    # Tutup figure
    plt.close()


# ============================================================
# PROGRAM UTAMA
# ============================================================

# Keterangan: Menjalankan seluruh rangkaian demo translasi.
def main():
    """Fungsi utama program.

    Menjalankan seluruh demo translasi dan menyimpan hasil.
    """
    
    # Cetak header program utama
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("PRAKTIKUM: TRANSLASI GAMBAR")
    # Cetak informasi ke console
    print("Bab 2 - Pembentukan Citra")
    # Cetak informasi ke console
    print("=" * 60)
    
    # Dapatkan path gambar dari berbagai lokasi kandidat
    path_gambar = dapatkan_path_gambar(NAMA_FILE_GAMBAR)
    # Baca gambar dari file
    gambar = cv2.imread(path_gambar)
    
    # Cek apakah gambar berhasil dimuat
    if gambar is None:
        # Jika tidak ditemukan, cetak warning
        print(f"[WARNING] Gambar {NAMA_FILE_GAMBAR} tidak ditemukan")
        # Gunakan gambar sample yang dibuat secara sintetis
        print("[INFO] Menggunakan gambar sample...")
        gambar = buat_gambar_sample()
    else:
        # Cetak informasi sukses loading gambar
        print(f"[SUKSES] Gambar dimuat: {path_gambar}")
        # Resize gambar jika lebar > 800 piksel untuk performa
        if gambar.shape[1] > 800:
            # Hitung skala untuk resize
            scale = 800 / gambar.shape[1]
            # Resize dengan interpolasi bilinear
            gambar = cv2.resize(gambar, None, fx=scale, fy=scale)
    
    # Cetak dimensi gambar yang akan diproses
    print(f"[INFO] Ukuran gambar: {gambar.shape}")
    
    # Demo 1: Terapkan translasi sederhana dengan nilai TX dan TY
    print("\n[1] Translasi dengan nilai tx={}, ty={}".format(TX, TY))
    hasil, matrix = translasi_gambar(gambar, TX, TY, MODE_BORDER, WARNA_BORDER)
    
    # Siapkan kanvas plot 1x2 untuk menampilkan perbandingan
    fig, axes = plt.subplots(1, 2, figsize=(12, 5))
    # Tampilkan gambar asli pada subplot pertama
    axes[0].imshow(cv2.cvtColor(gambar, cv2.COLOR_BGR2RGB))
    # Set judul untuk subplot
    axes[0].set_title("Gambar Asli")
    # Nonaktifkan atau atur axis pada subplot
    axes[0].axis('off')
    
    # Tampilkan gambar hasil translasi pada subplot kedua
    axes[1].imshow(cv2.cvtColor(hasil, cv2.COLOR_BGR2RGB))
    axes[1].set_title(f"Setelah Translasi\ntx={TX}, ty={TY}")
    # Nonaktifkan atau atur axis pada subplot
    axes[1].axis('off')
    
    # Set judul keseluruhan figure
    plt.suptitle("Demonstrasi Translasi Gambar", fontsize=14)
    # Atur spacing antar subplot
    plt.tight_layout()
    # Bentuk path lengkap untuk menyimpan output
    output_path = os.path.join(DIR_OUTPUT, 'demo_translasi_simple.png')
    # Simpan figure ke file
    plt.savefig(output_path, dpi=100, bbox_inches='tight')
    # Cetak pesan sukses
    print(f"[SAVED] Gambar disimpan ke: {output_path}")
    # Tutup figure untuk menghemat memory
    plt.close()
    
    # Demo 2: Jalankan demo translasi ke berbagai arah
    demo_translasi_berbagai_arah(gambar)
    
    # Demo 3: Jalankan demo berbagai mode border
    demo_border_modes(gambar)
    
    # Demo 4: Jalankan visualisasi matriks translasi
    visualisasi_matrix_translasi(gambar)
    
    # Cetak ringkasan materi pembelajaran
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("RINGKASAN TRANSLASI GAMBAR")
    # Cetak informasi ke console
    print("=" * 60)
    # Cetak informasi ke console
    print("""
TRANSLATION MATRIX:
    M = | 1  0  tx |
        | 0  1  ty |

PENGGUNAAN:
    cv2.warpAffine(src, M, (width, height), 
                   borderMode=..., borderValue=...)

BORDER MODES:
├── cv2.BORDER_CONSTANT  : Warna solid
├── cv2.BORDER_REPLICATE : Replikasi tepi
├── cv2.BORDER_REFLECT   : Cermin
├── cv2.BORDER_WRAP      : Wrapping
└── cv2.BORDER_REFLECT_101: Cermin tanpa duplikat tepi

TIPS:
- tx positif → gambar geser ke kanan
- tx negatif → gambar geser ke kiri
- ty positif → gambar geser ke bawah
- ty negatif → gambar geser ke atas
""")


# Jalankan program utama
if __name__ == "__main__":
    main()
