# ============================================================
# PROGRAM: 02_rotasi.py
# PRAKTIKUM COMPUTER VISION - BAB 2: PEMBENTUKAN CITRA
# ============================================================
# Deskripsi: Program ini mendemonstrasikan rotasi gambar
#            dengan berbagai sudut dan titik pusat
# 
# Tujuan Pembelajaran:
#   1. Memahami konsep rotasi dalam transformasi geometri
#   2. Memahami rotation matrix dan cara kerjanya
#   3. Mengatasi masalah gambar terpotong saat rotasi
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

# ============================================================
# PANDUAN FUNGSI (RINGKAS) - ARTI PARAMETER
# ============================================================
# cv2.getRotationMatrix2D(center, angle, scale)
#   - center : titik pusat rotasi (x, y)
#   - angle  : sudut rotasi derajat (positif CCW)
#   - scale  : faktor skala
#
# cv2.warpAffine(src, M, dsize)
#   - src   : gambar input
#   - M     : matriks transformasi 2x3
#   - dsize : (lebar, tinggi) output
#
# cv2.resize(img, dsize/None, fx, fy)
#   - dsize : (lebar, tinggi) output, atau None
#   - fx, fy: skala horizontal/vertikal jika dsize=None
#
# cv2.putText(img, text, org, fontFace, fontScale, color, thickness)
#   - org : posisi teks (x, y)
#
# matplotlib.pyplot.savefig(path, dpi, bbox_inches)
#   - path : lokasi simpan gambar

# ============================================================
# VARIABEL YANG BISA DIUBAH-UBAH (EKSPERIMEN)
# ============================================================

# ============================================================
# KONFIGURASI PATH
# ============================================================

# Dapatkan direktori script (praktikum folder)
DIR_SCRIPT = os.path.dirname(os.path.abspath(__file__))
DIR_DATA = os.path.join(DIR_SCRIPT, "data", "images")
DIR_OUTPUT = os.path.join(DIR_SCRIPT, "output", "output2")

# Pastikan folder output ada
os.makedirs(DIR_OUTPUT, exist_ok=True)


# 1. File gambar yang akan diproses
NAMA_FILE_GAMBAR = "portrait.jpg"

# 2. Sudut rotasi dalam derajat
#    Positif = berlawanan arah jarum jam (counter-clockwise)
#    Negatif = searah jarum jam (clockwise)
SUDUT_ROTASI = 45  # Coba ubah: 15, 30, 45, 90, 180, -45

# 3. Skala saat rotasi (1.0 = ukuran asli)
SKALA = 1.0  # Coba ubah: 0.5, 0.8, 1.0, 1.2, 1.5

# 4. Titik pusat rotasi (None = tengah gambar)
#    Contoh: (0, 0) untuk sudut kiri atas
TITIK_PUSAT = None  # Coba ubah: (100, 100), (0, 0)

# 5. Apakah ingin bounding box diperbesar untuk menampung seluruh gambar?
AUTO_EXPAND = True

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
        # Bentuk path lengkap file
        path_data = os.path.join(direktori_script, "..", "..", 
                                  "Bab-01-Pendahuluan", "data", "images", nama_file)
    
    # Cek kondisi logis
    if not os.path.exists(path_data):
        # Bentuk path lengkap file
        path_data = os.path.join(direktori_script, nama_file)
    
    # Kembalikan hasil dari fungsi
    return path_data


# Keterangan: Membuat gambar sintetis untuk demo rotasi.
def buat_gambar_sample():
    """Membuat gambar sample dengan shapes untuk demonstrasi.

    Return:
    - np.ndarray: gambar buatan berformat BGR.
    """
    # Inisialisasi kanvas gambar hitam berukuran 400x600 dengan 3 channel BGR
    gambar = np.zeros((400, 600, 3), dtype=np.uint8)
    
    # Buat gradient warna vertikal pada background untuk efek visual menarik
    for i in range(400):
        gambar[i, :] = [int(100 + i/4), int(150 - i/8), int(200 - i/4)]
    
    # Gambar persegi luar (outline) putih sebagai referensi batas rotasi
    cv2.rectangle(gambar, (100, 100), (500, 300), (255, 255, 255), 2)
    # Gambar persegi dalam (outline) cyan untuk membedakan area
    cv2.rectangle(gambar, (120, 120), (480, 280), (0, 255, 255), 2)
    
    # Tambahkan teks "ROTASI" yang akan menunjukkan efek rotasi dengan jelas
    cv2.putText(gambar, "ROTASI", (200, 220), 
                cv2.FONT_HERSHEY_SIMPLEX, 2, (255, 255, 255), 3)
    
    # Gambar titik pusat rotasi dengan lingkaran merah kecil
    cv2.circle(gambar, (300, 200), 5, (0, 0, 255), -1)
    # Tambahkan label "Pusat" di dekat titik pusat
    cv2.putText(gambar, "Pusat", (310, 200), 
                cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 1)
    
    # Kembalikan gambar yang telah dibuat
    return gambar


# ============================================================
# FUNGSI ROTASI
# ============================================================

# Keterangan: Rotasi gambar tanpa memperbesar canvas.
def rotasi_gambar(gambar, sudut, skala=1.0, pusat=None):
    """Melakukan rotasi gambar (tanpa expand).

    Parameter:
    - gambar (np.ndarray): gambar input (BGR).
    - sudut (float): sudut rotasi dalam derajat.
    - skala (float): faktor skala.
    - pusat (tuple[int, int] | None): titik pusat rotasi (default: tengah).

    Return:
    - np.ndarray: gambar hasil rotasi.
    - np.ndarray: rotation matrix 2x3.
    """
    # Ambil dimensi gambar (tinggi dan lebar)
    tinggi, lebar = gambar.shape[:2]
    
    # Jika pusat belum ditentukan, gunakan pusat gambar (tengah)
    if pusat is None:
        pusat = (lebar // 2, tinggi // 2)
    
    # Hitung rotation matrix 2x3 menggunakan fungsi OpenCV
    # Format: cv2.getRotationMatrix2D(center, angle, scale)
    rotation_matrix = cv2.getRotationMatrix2D(pusat, sudut, skala)
    
    # Terapkan transformasi affine (rotasi) pada gambar dengan ukuran asli
    hasil = cv2.warpAffine(gambar, rotation_matrix, (lebar, tinggi))
    
    # Kembalikan gambar hasil dan matriks transformasi
    return hasil, rotation_matrix


# Keterangan: Rotasi gambar dengan canvas diperbesar agar tidak terpotong.
def rotasi_gambar_expand(gambar, sudut, skala=1.0):
    """Melakukan rotasi gambar dengan memperbesar canvas.

    Parameter:
    - gambar (np.ndarray): gambar input (BGR).
    - sudut (float): sudut rotasi dalam derajat.
    - skala (float): faktor skala.

    Return:
    - np.ndarray: gambar hasil rotasi (ukuran lebih besar).
    - np.ndarray: rotation matrix 2x3 (terkoreksi translasi).
    """
    # Ambil dimensi gambar
    tinggi, lebar = gambar.shape[:2]
    # Tentukan pusat rotasi di tengah gambar
    pusat = (lebar // 2, tinggi // 2)
    
    # Hitung rotation matrix dengan pusat gambar
    rotation_matrix = cv2.getRotationMatrix2D(pusat, sudut, skala)
    
    # Ekstrak nilai cos dan sin dari rotation matrix untuk menghitung bounding box
    cos = np.abs(rotation_matrix[0, 0])
    sin = np.abs(rotation_matrix[0, 1])
    
    # Hitung lebar baru yang dibutuhkan untuk menampung gambar terrotasi penuh
    lebar_baru = int((tinggi * sin) + (lebar * cos))
    # Hitung tinggi baru yang dibutuhkan untuk menampung gambar terrotasi penuh
    tinggi_baru = int((tinggi * cos) + (lebar * sin))
    
    # Sesuaikan elemen translasi matriks (offset-x) agar gambar terpusat di canvas baru
    rotation_matrix[0, 2] += (lebar_baru / 2) - pusat[0]
    # Sesuaikan elemen translasi matriks (offset-y) agar gambar terpusat di canvas baru
    rotation_matrix[1, 2] += (tinggi_baru / 2) - pusat[1]
    
    # Terapkan transformasi rotasi dengan canvas baru yang lebih besar
    hasil = cv2.warpAffine(gambar, rotation_matrix, (lebar_baru, tinggi_baru))
    
    # Kembalikan gambar hasil dan matriks transformasi yang sudah dikoreksi
    return hasil, rotation_matrix


# Keterangan: Menampilkan hasil rotasi pada beberapa sudut.
def demo_rotasi_berbagai_sudut(gambar):
    """Mendemonstrasikan rotasi dengan berbagai sudut.

    Parameter:
    - gambar (np.ndarray): gambar input (BGR).
    """
    # Cetak header demo
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("DEMO ROTASI BERBAGAI SUDUT")
    # Cetak informasi ke console
    print("=" * 60)
    
    # Tentukan daftar sudut yang akan dicoba
    sudut_list = [0, 30, 45, 60, 90, 120, 180, 270]
    
    # Siapkan kanvas plot 2x4 untuk menampilkan 8 hasil rotasi
    fig, axes = plt.subplots(2, 4, figsize=(16, 8))
    axes = axes.flatten()
    
    # Iterasi melalui setiap sudut
    for i, sudut in enumerate(sudut_list):
        # Terapkan rotasi pada sudut tertentu
        hasil, _ = rotasi_gambar(gambar, sudut)
        
        # Tampilkan hasil rotasi pada subplot (konversi BGR ke RGB untuk matplotlib)
        axes[i].imshow(cv2.cvtColor(hasil, cv2.COLOR_BGR2RGB))
        # Set judul subplot dengan nilai sudut
        axes[i].set_title(f"Rotasi {sudut}°")
        # Nonaktifkan axis untuk tampilan lebih rapi
        axes[i].axis('off')
        
        # Cetak informasi ke console
        print(f"[{i+1}] Rotasi {sudut}°")
    
    # Set judul keseluruhan figure
    plt.suptitle("Rotasi Gambar dengan Berbagai Sudut (tanpa expand)", fontsize=14)
    # Atur spacing antar subplot
    plt.tight_layout()
    # Bentuk path lengkap untuk menyimpan output
    output_path = os.path.join(DIR_OUTPUT, "output.png")

    # Simpan figure ke file dengan kualitas DPI tertentu
    plt.savefig(output_path, dpi=100, bbox_inches="tight")

    # Cetak pesan sukses
    print(f"[SAVED] {output_path}")

    # Tutup figure untuk menghemat memory
    plt.close()


# Keterangan: Membandingkan rotasi normal vs expand.
def demo_rotasi_expand_vs_normal(gambar):
    """Membandingkan rotasi normal vs dengan expand.

    Parameter:
    - gambar (np.ndarray): gambar input (BGR).
    """
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("PERBANDINGAN ROTASI NORMAL VS EXPAND")
    # Cetak informasi ke console
    print("=" * 60)
    
    sudut = 45
    
    # Rotasi normal (gambar terpotong)
    hasil_normal, _ = rotasi_gambar(gambar, sudut)
    
    # Rotasi dengan expand (gambar utuh)
    hasil_expand, _ = rotasi_gambar_expand(gambar, sudut)
    
    # Cetak informasi ke console
    print(f"Sudut rotasi: {sudut}°")
    # Cetak informasi ke console
    print(f"Ukuran asli   : {gambar.shape[1]} x {gambar.shape[0]}")
    # Cetak informasi ke console
    print(f"Ukuran normal : {hasil_normal.shape[1]} x {hasil_normal.shape[0]}")
    # Cetak informasi ke console
    print(f"Ukuran expand : {hasil_expand.shape[1]} x {hasil_expand.shape[0]}")
    
    # Tampilkan
    fig, axes = plt.subplots(1, 3, figsize=(15, 5))
    
    # Tampilkan gambar pada subplot tertentu
    axes[0].imshow(cv2.cvtColor(gambar, cv2.COLOR_BGR2RGB))
    axes[0].set_title(f"Original\n{gambar.shape[1]}x{gambar.shape[0]}")
    # Nonaktifkan atau atur axis pada subplot
    axes[0].axis('off')
    
    # Tampilkan gambar pada subplot tertentu
    axes[1].imshow(cv2.cvtColor(hasil_normal, cv2.COLOR_BGR2RGB))
    axes[1].set_title(f"Rotasi {sudut}° (Normal)\n{hasil_normal.shape[1]}x{hasil_normal.shape[0]}\nSudut terpotong!")
    # Nonaktifkan atau atur axis pada subplot
    axes[1].axis('off')
    
    # Tampilkan gambar pada subplot tertentu
    axes[2].imshow(cv2.cvtColor(hasil_expand, cv2.COLOR_BGR2RGB))
    axes[2].set_title(f"Rotasi {sudut}° (Expand)\n{hasil_expand.shape[1]}x{hasil_expand.shape[0]}\nGambar utuh")
    # Nonaktifkan atau atur axis pada subplot
    axes[2].axis('off')
    
    # Set judul keseluruhan figure
    plt.suptitle("Perbandingan Rotasi Normal vs Expand", fontsize=14)
    # Atur spacing antar subplot
    plt.tight_layout()
    output_path = os.path.join(DIR_OUTPUT, "output.png")

    # Simpan figure ke file dengan kualitas DPI tertentu
    plt.savefig(output_path, dpi=100, bbox_inches="tight")

    # Cetak informasi ke console
    print(f"[SAVED] {output_path}")

    # Tutup figure untuk menghemat memory
    plt.close()


# Keterangan: Menunjukkan efek titik pusat rotasi yang berbeda.
def demo_rotasi_titik_pusat_berbeda(gambar):
    """Mendemonstrasikan rotasi dengan titik pusat berbeda.

    Parameter:
    - gambar (np.ndarray): gambar input (BGR).
    """
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("DEMO ROTASI DENGAN TITIK PUSAT BERBEDA")
    # Cetak informasi ke console
    print("=" * 60)
    
    tinggi, lebar = gambar.shape[:2]
    sudut = 45
    
    # Berbagai titik pusat
    pusat_list = [
        ((lebar//2, tinggi//2), "Tengah"),
        ((0, 0), "Kiri Atas (0,0)"),
        ((lebar-1, 0), "Kanan Atas"),
        ((0, tinggi-1), "Kiri Bawah"),
        ((lebar//4, tinggi//4), "1/4 dari kiri atas"),
    ]
    
    # Siapkan kanvas plot untuk menampilkan hasil
    fig, axes = plt.subplots(2, 3, figsize=(15, 10))
    axes = axes.flatten()
    
    # Original
    gambar_marked = gambar.copy()
    for pusat, _ in pusat_list:
        # Gambar lingkaran pada gambar
        cv2.circle(gambar_marked, pusat, 8, (0, 255, 0), -1)
    # Tampilkan gambar pada subplot tertentu
    axes[0].imshow(cv2.cvtColor(gambar_marked, cv2.COLOR_BGR2RGB))
    # Set judul untuk subplot
    axes[0].set_title("Original dengan titik pusat")
    # Nonaktifkan atau atur axis pada subplot
    axes[0].axis('off')
    
    for i, (pusat, label) in enumerate(pusat_list):
        hasil, _ = rotasi_gambar(gambar, sudut, pusat=pusat)
        
        # Tandai titik pusat pada hasil
        if 0 <= pusat[0] < hasil.shape[1] and 0 <= pusat[1] < hasil.shape[0]:
            # Gambar lingkaran pada gambar
            cv2.circle(hasil, pusat, 8, (0, 255, 0), -1)
        
        # Tampilkan gambar pada subplot tertentu
        axes[i+1].imshow(cv2.cvtColor(hasil, cv2.COLOR_BGR2RGB))
        # Set judul untuk subplot
        axes[i+1].set_title(f"Pusat: {label}")
        # Nonaktifkan atau atur axis pada subplot
        axes[i+1].axis('off')
        
        # Cetak informasi ke console
        print(f"[{i+1}] Rotasi {sudut}° dengan pusat di {label}")
    
    # Set judul keseluruhan figure
    plt.suptitle(f"Rotasi {sudut}° dengan Titik Pusat Berbeda", fontsize=14)
    # Atur spacing antar subplot
    plt.tight_layout()
    output_path = os.path.join(DIR_OUTPUT, "output.png")

    # Simpan figure ke file dengan kualitas DPI tertentu
    plt.savefig(output_path, dpi=100, bbox_inches="tight")

    # Cetak informasi ke console
    print(f"[SAVED] {output_path}")

    # Tutup figure untuk menghemat memory
    plt.close()


# Keterangan: Menjelaskan bentuk matriks rotasi dan rumus terkait.
def visualisasi_rotation_matrix(sudut):
    """Menjelaskan bentuk rotation matrix dan komponennya.

    Parameter:
    - sudut (float): sudut rotasi dalam derajat.
    """
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("PENJELASAN ROTATION MATRIX")
    # Cetak informasi ke console
    print("=" * 60)
    
    # Convert ke radian
    theta = np.radians(sudut)
    cos_t = np.cos(theta)
    sin_t = np.sin(theta)
    
    # Cetak informasi ke console
    print(f"""
ROTATION MATRIX (2D):

Untuk sudut θ = {sudut}°:

    | cos(θ)  -sin(θ) |     | {cos_t:7.4f}  {-sin_t:7.4f} |
R = |                 |  =  |                        |
    | sin(θ)   cos(θ) |     | {sin_t:7.4f}   {cos_t:7.4f} |

OpenCV Rotation Matrix (dengan translation dan scale):

    | α   β   (1-α)·cx - β·cy |
M = |                          |
    | -β  α   β·cx + (1-α)·cy |

Dimana:
    α = scale × cos(θ)
    β = scale × sin(θ)
    (cx, cy) = titik pusat rotasi

PERSAMAAN TRANSFORMASI:
    x' = α·x + β·y + (1-α)·cx - β·cy
    y' = -β·x + α·y + β·cx + (1-α)·cy

CATATAN:
- Sudut positif → rotasi counter-clockwise
- Sudut negatif → rotasi clockwise
- Scale = 1.0 → ukuran tetap
- Scale > 1.0 → diperbesar
- Scale < 1.0 → diperkecil
""")


# Keterangan: Menampilkan rotasi dengan kombinasi sudut dan skala.
def demo_rotasi_dengan_skala(gambar):
    """Mendemonstrasikan rotasi dengan berbagai skala.

    Parameter:
    - gambar (np.ndarray): gambar input (BGR).
    """
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("DEMO ROTASI DENGAN SKALA BERBEDA")
    # Cetak informasi ke console
    print("=" * 60)
    
    kombinasi = [
        (0, 1.0, "Original"),
        (45, 0.5, "Rotasi 45°, Scale 0.5"),
        (45, 1.0, "Rotasi 45°, Scale 1.0"),
        (45, 1.5, "Rotasi 45°, Scale 1.5"),
        (90, 0.8, "Rotasi 90°, Scale 0.8"),
        (180, 1.2, "Rotasi 180°, Scale 1.2"),
    ]
    
    # Siapkan kanvas plot untuk menampilkan hasil
    fig, axes = plt.subplots(2, 3, figsize=(15, 10))
    axes = axes.flatten()
    
    for i, (sudut, skala, label) in enumerate(kombinasi):
        hasil, _ = rotasi_gambar(gambar, sudut, skala)
        
        # Tampilkan gambar pada subplot tertentu
        axes[i].imshow(cv2.cvtColor(hasil, cv2.COLOR_BGR2RGB))
        # Set judul untuk subplot
        axes[i].set_title(f"{label}")
        # Nonaktifkan atau atur axis pada subplot
        axes[i].axis('off')
        
        # Cetak informasi ke console
        print(f"[{i+1}] {label}")
    
    # Set judul keseluruhan figure
    plt.suptitle("Rotasi dengan Berbagai Kombinasi Sudut dan Skala", fontsize=14)
    # Atur spacing antar subplot
    plt.tight_layout()
    output_path = os.path.join(DIR_OUTPUT, "output.png")

    # Simpan figure ke file dengan kualitas DPI tertentu
    plt.savefig(output_path, dpi=100, bbox_inches="tight")

    # Cetak informasi ke console
    print(f"[SAVED] {output_path}")

    # Tutup figure untuk menghemat memory
    plt.close()


# ============================================================
# PROGRAM UTAMA
# ============================================================

# Keterangan: Menjalankan seluruh rangkaian demo rotasi.
def main():
    """Fungsi utama program.

    Menjalankan seluruh demo rotasi dan menyimpan hasil.
    """
    
# Keterangan: Menjalankan seluruh rangkaian demo rotasi.
def main():
    """Fungsi utama program.

    Menjalankan seluruh demo rotasi dan menyimpan hasil.
    """
    
    # Cetak header program utama
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("PRAKTIKUM: ROTASI GAMBAR")
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
        # Cetak warning jika gambar tidak ditemukan
        print(f"[WARNING] Gambar {NAMA_FILE_GAMBAR} tidak ditemukan")
        # Gunakan gambar sample yang dibuat secara sintetis
        print("[INFO] Menggunakan gambar sample...")
        gambar = buat_gambar_sample()
    else:
        # Cetak informasi sukses loading gambar
        print(f"[SUKSES] Gambar dimuat: {path_gambar}")
        # Resize gambar jika lebar > 600 piksel untuk performa
        if gambar.shape[1] > 600:
            # Hitung skala untuk resize
            scale = 600 / gambar.shape[1]
            # Resize dengan interpolasi bilinear
            gambar = cv2.resize(gambar, None, fx=scale, fy=scale)
    
    # Cetak dimensi gambar yang akan diproses
    print(f"[INFO] Ukuran gambar: {gambar.shape}")
    
    # Demo 1: Tampilkan visualisasi teori rotation matrix
    visualisasi_rotation_matrix(SUDUT_ROTASI)
    
    # Demo 2: Lakukan rotasi sederhana
    print(f"\n[DEMO] Rotasi dengan sudut={SUDUT_ROTASI}°, skala={SKALA}")
    
    # Tentukan apakah menggunakan expand atau normal berdasarkan flag AUTO_EXPAND
    if AUTO_EXPAND:
        # Terapkan rotasi dengan canvas diperbesar agar gambar tidak terpotong
        hasil, matrix = rotasi_gambar_expand(gambar, SUDUT_ROTASI, SKALA)
    else:
        # Terapkan rotasi normal tanpa memperbesar canvas
        hasil, matrix = rotasi_gambar(gambar, SUDUT_ROTASI, SKALA, TITIK_PUSAT)
    
    # Siapkan kanvas plot 1x2 untuk perbandingan sebelum-sesudah
    fig, axes = plt.subplots(1, 2, figsize=(12, 6))
    # Tampilkan gambar asli pada subplot pertama
    axes[0].imshow(cv2.cvtColor(gambar, cv2.COLOR_BGR2RGB))
    # Set judul untuk subplot
    axes[0].set_title("Gambar Asli")
    # Nonaktifkan atau atur axis pada subplot
    axes[0].axis('off')
    
    # Tampilkan gambar hasil rotasi pada subplot kedua
    axes[1].imshow(cv2.cvtColor(hasil, cv2.COLOR_BGR2RGB))
    # Set judul untuk subplot
    axes[1].set_title(f"Rotasi {SUDUT_ROTASI}°, Skala {SKALA}")
    # Nonaktifkan atau atur axis pada subplot
    axes[1].axis('off')
    
    # Set judul keseluruhan figure
    plt.suptitle("Demonstrasi Rotasi Gambar", fontsize=14)
    # Atur spacing antar subplot
    plt.tight_layout()
    # Bentuk path lengkap untuk menyimpan output
    output_path = os.path.join(DIR_OUTPUT, "output.png")

    # Simpan figure ke file
    plt.savefig(output_path, dpi=100, bbox_inches="tight")

    # Cetak pesan sukses
    print(f"[SAVED] {output_path}")

    # Tutup figure untuk menghemat memory
    plt.close()
    
    # Demo 3: Jalankan demo rotasi dengan berbagai sudut
    demo_rotasi_berbagai_sudut(gambar)
    
    # Demo 4: Jalankan demo perbandingan expand vs normal
    demo_rotasi_expand_vs_normal(gambar)
    
    # Demo 5: Jalankan demo rotasi dengan titik pusat berbeda
    demo_rotasi_titik_pusat_berbeda(gambar)
    
    # Demo 6: Jalankan demo rotasi dengan skala berbeda
    demo_rotasi_dengan_skala(gambar)
    
    # Cetak ringkasan materi pembelajaran
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("RINGKASAN ROTASI GAMBAR")
    # Cetak informasi ke console
    print("=" * 60)
    # Cetak informasi ke console
    print("""
FUNGSI UTAMA:
    # Hitung matriks rotasi 2D
    M = cv2.getRotationMatrix2D(center, angle, scale)
    # Terapkan transformasi affine pada gambar
    hasil = cv2.warpAffine(src, M, (width, height))

PARAMETER:
├── center: titik pusat rotasi (x, y)
├── angle: sudut dalam derajat (positif = CCW)
└── scale: faktor skala

TIPS MENGHINDARI GAMBAR TERPOTONG:
1. Hitung bounding box baru berdasarkan cos dan sin
2. Sesuaikan translation pada matrix
3. Gunakan ukuran output yang lebih besar

RUMUS BOUNDING BOX BARU:
    new_width = height × |sin(θ)| + width × |cos(θ)|
    new_height = height × |cos(θ)| + width × |sin(θ)|

EKSPERIMEN:
- Ubah SUDUT_ROTASI dan SKALA di awal program
- Bandingkan hasil expand vs normal
- Coba berbagai titik pusat rotasi
""")


# Jalankan program utama
if __name__ == "__main__":
    main()
