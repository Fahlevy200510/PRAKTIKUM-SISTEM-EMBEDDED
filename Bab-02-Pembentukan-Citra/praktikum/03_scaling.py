# ============================================================
# PROGRAM: 03_scaling.py
# PRAKTIKUM COMPUTER VISION - BAB 2: PEMBENTUKAN CITRA
# ============================================================
# Deskripsi: Program ini mendemonstrasikan scaling (resize)
#            gambar dengan berbagai metode interpolasi
# 
# Tujuan Pembelajaran:
#   1. Memahami berbagai metode interpolasi
#   2. Memilih metode yang tepat untuk kasus tertentu
#   3. Memahami trade-off antara kecepatan dan kualitas
# ============================================================

# ====================
# IMPORT LIBRARY
# ====================
import cv2
import numpy as np
import matplotlib
matplotlib.use('Agg')  # Gunakan backend non-GUI
import matplotlib.pyplot as plt
import time
import os

# ============================================================
# PANDUAN FUNGSI (RINGKAS) - ARTI PARAMETER
# ============================================================
# cv2.resize(img, dsize/None, fx, fy, interpolation)
#   - dsize        : (lebar, tinggi) output, atau None
#   - fx, fy       : skala horizontal/vertikal jika dsize=None
#   - interpolation: metode interpolasi (INTER_LINEAR, dst)
#
# cv2.copyMakeBorder(img, top, bottom, left, right, borderType, value)
#   - top/bottom/left/right: ukuran padding
#   - borderType           : tipe border (cv2.BORDER_*)
#   - value                : warna border jika BORDER_CONSTANT
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
DIR_OUTPUT = os.path.join(DIR_SCRIPT, "output", "output3")

# Pastikan folder output ada
os.makedirs(DIR_OUTPUT, exist_ok=True)


# 1. File gambar yang akan diproses
NAMA_FILE_GAMBAR = "portrait.jpg"

# 2. Faktor skala
SKALA_X = 2.0  # Skala horizontal (coba: 0.5, 1.5, 2.0, 3.0)
SKALA_Y = 2.0  # Skala vertikal (coba: 0.5, 1.5, 2.0, 3.0)

# 3. Atau gunakan ukuran spesifik (set None untuk menggunakan skala)
UKURAN_BARU = None  # Contoh: (640, 480) atau None

# 4. Metode interpolasi default
#    cv2.INTER_NEAREST  - Nearest neighbor (tercepat, kualitas rendah)
#    cv2.INTER_LINEAR   - Bilinear (default, balanced)
#    cv2.INTER_CUBIC    - Bicubic (lebih halus, lebih lambat)
#    cv2.INTER_LANCZOS4 - Lanczos (terbaik untuk upscaling, paling lambat)
#    cv2.INTER_AREA     - Resampling area (terbaik untuk downscaling)
METODE_INTERPOLASI = cv2.INTER_LINEAR

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


# Keterangan: Membuat gambar sintetis untuk demo interpolasi.
def buat_gambar_sample():
    """Membuat gambar sample untuk demonstrasi interpolasi.

    Return:
    - np.ndarray: gambar buatan berformat BGR.
    """
    # Inisialisasi kanvas gambar kosong (hitam) dengan dimensi tertentu
    gambar = np.zeros((200, 300, 3), dtype=np.uint8)
    
    # Checkerboard pattern (bagus untuk melihat aliasing)
    for i in range(0, 200, 20):
        # Iterasi melalui range
        for j in range(0, 300, 20):
            if (i // 20 + j // 20) % 2:
                gambar[i:i+20, j:j+20] = [200, 200, 200]
    
    # Diagonal lines (bagus untuk melihat smoothing)
    for i in range(200):
        gambar[i, i % 300] = [255, 0, 0]
        gambar[i, (i + 5) % 300] = [0, 255, 0]
    
    # Circles (bagus untuk melihat kurva)
    cv2.circle(gambar, (150, 100), 50, (0, 0, 255), 2)
    # Gambar lingkaran pada gambar
    cv2.circle(gambar, (150, 100), 30, (255, 255, 0), 1)
    
    # Text
    cv2.putText(gambar, "TEST", (100, 180), 
                cv2.FONT_HERSHEY_SIMPLEX, 0.8, (255, 255, 255), 2)
    
    # Kembalikan hasil dari fungsi
    return gambar


# ============================================================
# FUNGSI SCALING
# ============================================================

# Keterangan: Resize gambar menggunakan faktor skala (fx, fy).
def resize_dengan_skala(gambar, skala_x, skala_y, interpolasi=cv2.INTER_LINEAR):
    """Resize gambar menggunakan faktor skala.

    Parameter:
    - gambar (np.ndarray): gambar input (BGR).
    - skala_x (float): faktor skala horizontal.
    - skala_y (float): faktor skala vertikal.
    - interpolasi (int): metode interpolasi (cv2.INTER_*).

    Return:
    - np.ndarray: gambar hasil resize.
    """
    # cv2.resize dengan fx dan fy
    hasil = cv2.resize(gambar, None, fx=skala_x, fy=skala_y, 
                       interpolation=interpolasi)
    # Kembalikan hasil dari fungsi
    return hasil


# Keterangan: Resize gambar ke ukuran target tertentu.
def resize_ke_ukuran(gambar, lebar_baru, tinggi_baru, interpolasi=cv2.INTER_LINEAR):
    """Resize gambar ke ukuran spesifik.

    Parameter:
    - gambar (np.ndarray): gambar input (BGR).
    - lebar_baru (int): lebar target.
    - tinggi_baru (int): tinggi target.
    - interpolasi (int): metode interpolasi (cv2.INTER_*).

    Return:
    - np.ndarray: gambar hasil resize.
    """
    # cv2.resize dengan dsize
    hasil = cv2.resize(gambar, (lebar_baru, tinggi_baru), 
                       interpolation=interpolasi)
    # Kembalikan hasil dari fungsi
    return hasil


# Keterangan: Resize gambar sambil menjaga aspect ratio.
def resize_menjaga_aspek_rasio(gambar, max_lebar=None, max_tinggi=None,
                               interpolasi=cv2.INTER_LINEAR):
    """Resize gambar dengan menjaga aspect ratio.

    Parameter:
    - gambar (np.ndarray): gambar input (BGR).
    - max_lebar (int | None): batas lebar maksimum.
    - max_tinggi (int | None): batas tinggi maksimum.
    - interpolasi (int): metode interpolasi (cv2.INTER_*).

    Return:
    - np.ndarray: gambar hasil resize.
    """
    tinggi, lebar = gambar.shape[:2]
    
    # Cek apakah variabel kosong/None
    if max_lebar is None and max_tinggi is None:
        # Kembalikan hasil dari fungsi
        return gambar
    
    if max_lebar is not None and max_tinggi is not None:
        # Pilih skala terkecil agar muat di batas
        skala_w = max_lebar / lebar
        skala_h = max_tinggi / tinggi
        skala = min(skala_w, skala_h)
    elif max_lebar is not None:
        skala = max_lebar / lebar
    else:
        skala = max_tinggi / tinggi
    
    lebar_baru = int(lebar * skala)
    tinggi_baru = int(tinggi * skala)
    
    # Resize gambar ke ukuran baru
    hasil = cv2.resize(gambar, (lebar_baru, tinggi_baru), interpolation=interpolasi)
    # Kembalikan hasil dari fungsi
    return hasil


# Keterangan: Membandingkan berbagai metode interpolasi.
def bandingkan_metode_interpolasi(gambar, skala):
    """Membandingkan berbagai metode interpolasi.

    Parameter:
    - gambar (np.ndarray): gambar input (BGR).
    - skala (float): faktor skala resize.

    Return:
    - list[tuple]: daftar (hasil, nama, deskripsi).
    - list[float]: daftar waktu proses (ms).
    """
    # Cetak informasi ke console
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("PERBANDINGAN METODE INTERPOLASI")
    # Cetak informasi ke console
    print("=" * 60)
    
    metode_list = [
        (cv2.INTER_NEAREST, "INTER_NEAREST", "Nearest Neighbor"),
        (cv2.INTER_LINEAR, "INTER_LINEAR", "Bilinear"),
        (cv2.INTER_CUBIC, "INTER_CUBIC", "Bicubic"),
        (cv2.INTER_LANCZOS4, "INTER_LANCZOS4", "Lanczos"),
        (cv2.INTER_AREA, "INTER_AREA", "Area (Resampling)"),
    ]
    
    hasil_list = []
    waktu_list = []
    
    # Cetak informasi ke console
    print(f"\nSkala: {skala}x")
    # Cetak informasi ke console
    print(f"Ukuran asli: {gambar.shape[1]} x {gambar.shape[0]}")
    # Cetak informasi ke console
    print("\n{:<20} {:<15} {:<15}".format("Metode", "Waktu (ms)", "Ukuran Output"))
    # Cetak informasi ke console
    print("-" * 50)
    
    for metode, nama, deskripsi in metode_list:
        # Ukur waktu
        start = time.time()
        hasil = resize_dengan_skala(gambar, skala, skala, metode)
        elapsed = (time.time() - start) * 1000
        
        hasil_list.append((hasil, nama, deskripsi))
        waktu_list.append(elapsed)
        
        # Cetak informasi ke console
        print(f"{nama:<20} {elapsed:<15.3f} {hasil.shape[1]}x{hasil.shape[0]}")
    
    # Kembalikan hasil dari fungsi
    return hasil_list, waktu_list


# Keterangan: Menampilkan efek upscaling dengan beberapa metode.
def demo_upscaling(gambar):
    """Mendemonstrasikan upscaling (memperbesar gambar).

    Parameter:
    - gambar (np.ndarray): gambar input (BGR).
    """
    # Cetak informasi ke console
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("DEMO UPSCALING (MEMPERBESAR GAMBAR)")
    # Cetak informasi ke console
    print("=" * 60)
    
    # Cetak informasi ke console
    print("""
UPSCALING: Memperbesar gambar dari ukuran kecil ke besar.
Tantangan: Menciptakan piksel baru yang tidak ada di gambar asli.

Rekomendasi metode untuk UPSCALING:
1. INTER_LANCZOS4 - Kualitas terbaik, tapi lambat
2. INTER_CUBIC    - Keseimbangan kualitas/kecepatan
3. INTER_LINEAR   - Cepat, kualitas cukup
4. INTER_NEAREST  - Tercepat, tapi pixelated
    """)
    
    # Upscale 3x
    skala = 3.0
    hasil_list, _ = bandingkan_metode_interpolasi(gambar, skala)
    
    # Tampilkan
    fig, axes = plt.subplots(2, 3, figsize=(15, 10))
    axes = axes.flatten()
    
    # Tampilkan gambar pada subplot tertentu
    axes[0].imshow(cv2.cvtColor(gambar, cv2.COLOR_BGR2RGB))
    axes[0].set_title(f"Original\n{gambar.shape[1]}x{gambar.shape[0]}")
    # Nonaktifkan atau atur axis pada subplot
    axes[0].axis('off')
    
    for i, (hasil, nama, deskripsi) in enumerate(hasil_list):
        # Tampilkan gambar pada subplot tertentu
        axes[i+1].imshow(cv2.cvtColor(hasil, cv2.COLOR_BGR2RGB))
        axes[i+1].set_title(f"{nama}\n{deskripsi}")
        # Nonaktifkan atau atur axis pada subplot
        axes[i+1].axis('off')
    
    # Set judul keseluruhan figure
    plt.suptitle(f"Perbandingan Metode Interpolasi - Upscaling {skala}x", fontsize=14)
    # Atur spacing antar subplot
    plt.tight_layout()
    output_path = os.path.join(DIR_OUTPUT, "output.png")

    # Simpan figure ke file dengan kualitas DPI tertentu
    plt.savefig(output_path, dpi=100, bbox_inches="tight")

    # Cetak informasi ke console
    print(f"[SAVED] {output_path}")

    # Tutup figure untuk menghemat memory
    plt.close()


# Keterangan: Menampilkan efek downscaling dengan beberapa metode.
def demo_downscaling(gambar):
    """Mendemonstrasikan downscaling (memperkecil gambar).

    Parameter:
    - gambar (np.ndarray): gambar input (BGR).
    """
    # Cetak informasi ke console
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("DEMO DOWNSCALING (MEMPERKECIL GAMBAR)")
    # Cetak informasi ke console
    print("=" * 60)
    
    # Cetak informasi ke console
    print("""
DOWNSCALING: Memperkecil gambar dari ukuran besar ke kecil.
Tantangan: Menghilangkan detail tanpa aliasing.

Rekomendasi metode untuk DOWNSCALING:
1. INTER_AREA    - Terbaik! Menggunakan pixel area relation
2. INTER_CUBIC   - Bagus untuk pengurangan moderat
3. INTER_LINEAR  - Cepat, hasil cukup baik
    """)
    
    # Downscale 0.25x
    skala = 0.25
    hasil_list, _ = bandingkan_metode_interpolasi(gambar, skala)
    
    # Tampilkan
    fig, axes = plt.subplots(2, 3, figsize=(15, 10))
    axes = axes.flatten()
    
    # Tampilkan gambar pada subplot tertentu
    axes[0].imshow(cv2.cvtColor(gambar, cv2.COLOR_BGR2RGB))
    axes[0].set_title(f"Original\n{gambar.shape[1]}x{gambar.shape[0]}")
    # Nonaktifkan atau atur axis pada subplot
    axes[0].axis('off')
    
    for i, (hasil, nama, deskripsi) in enumerate(hasil_list):
        # Perbesar untuk visualisasi yang lebih jelas
        hasil_display = cv2.resize(hasil, (gambar.shape[1], gambar.shape[0]), 
                                   interpolation=cv2.INTER_NEAREST)
        # Tampilkan gambar pada subplot tertentu
        axes[i+1].imshow(cv2.cvtColor(hasil_display, cv2.COLOR_BGR2RGB))
        axes[i+1].set_title(f"{nama}\n{deskripsi}\n(diperbesar untuk preview)")
        # Nonaktifkan atau atur axis pada subplot
        axes[i+1].axis('off')
    
    # Set judul keseluruhan figure
    plt.suptitle(f"Perbandingan Metode Interpolasi - Downscaling {skala}x", fontsize=14)
    # Atur spacing antar subplot
    plt.tight_layout()
    output_path = os.path.join(DIR_OUTPUT, "output.png")

    # Simpan figure ke file dengan kualitas DPI tertentu
    plt.savefig(output_path, dpi=100, bbox_inches="tight")

    # Cetak informasi ke console
    print(f"[SAVED] {output_path}")

    # Tutup figure untuk menghemat memory
    plt.close()


# Keterangan: Visualisasi detail cara kerja interpolasi.
def demo_detail_interpolasi():
    """Mendemonstrasikan cara kerja interpolasi secara visual."""
    # Cetak informasi ke console
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("VISUALISASI CARA KERJA INTERPOLASI")
    # Cetak informasi ke console
    print("=" * 60)
    
    # Cetak informasi ke console
    print("""
CARA KERJA INTERPOLASI:

1. NEAREST NEIGHBOR:
   - Mengambil nilai piksel terdekat
   - Tercepat, tapi menghasilkan "piksel kotak-kotak"
   
2. BILINEAR (LINEAR):
   - Rata-rata tertimbang dari 4 piksel terdekat
   - Keseimbangan kecepatan dan kualitas
   
3. BICUBIC (CUBIC):
   - Rata-rata tertimbang dari 16 piksel terdekat
   - Lebih halus, lebih lambat
   
4. LANCZOS:
   - Menggunakan fungsi sinc untuk 64 piksel terdekat
   - Kualitas terbaik untuk upscaling
   
5. AREA:
   - Menghitung rata-rata area piksel
   - Terbaik untuk downscaling
    """)
    
    # Buat gambar sangat kecil untuk zoom
    mini = np.array([
        [[255, 0, 0], [0, 255, 0]],
        [[0, 0, 255], [255, 255, 0]]
    ], dtype=np.uint8)
    
    skala = 50  # Perbesar 50x untuk visualisasi
    
    metode_list = [
        (cv2.INTER_NEAREST, "NEAREST"),
        (cv2.INTER_LINEAR, "LINEAR"),
        (cv2.INTER_CUBIC, "CUBIC"),
        (cv2.INTER_LANCZOS4, "LANCZOS4"),
    ]
    
    # Siapkan kanvas plot untuk menampilkan hasil
    fig, axes = plt.subplots(1, 5, figsize=(15, 3))
    
    # Tampilkan gambar pada subplot tertentu
    axes[0].imshow(cv2.cvtColor(mini, cv2.COLOR_BGR2RGB))
    # Set judul untuk subplot
    axes[0].set_title("Original 2x2")
    # Nonaktifkan atau atur axis pada subplot
    axes[0].axis('off')
    
    for i, (metode, nama) in enumerate(metode_list):
        # Resize gambar ke ukuran baru
        hasil = cv2.resize(mini, (mini.shape[1]*skala, mini.shape[0]*skala), 
                          interpolation=metode)
        # Tampilkan gambar pada subplot tertentu
        axes[i+1].imshow(cv2.cvtColor(hasil, cv2.COLOR_BGR2RGB))
        # Set judul untuk subplot
        axes[i+1].set_title(f"{nama}")
        # Nonaktifkan atau atur axis pada subplot
        axes[i+1].axis('off')
    
    # Set judul keseluruhan figure
    plt.suptitle(f"Upscaling Gambar 2x2 sebesar {skala}x", fontsize=14)
    # Atur spacing antar subplot
    plt.tight_layout()
    output_path = os.path.join(DIR_OUTPUT, "output.png")

    # Simpan figure ke file dengan kualitas DPI tertentu
    plt.savefig(output_path, dpi=100, bbox_inches="tight")

    # Cetak informasi ke console
    print(f"[SAVED] {output_path}")

    # Tutup figure untuk menghemat memory
    plt.close()


# Keterangan: Perbandingan resize dengan/ tanpa menjaga aspect ratio.
def demo_aspect_ratio(gambar):
    """Mendemonstrasikan resize dengan dan tanpa aspect ratio.

    Parameter:
    - gambar (np.ndarray): gambar input (BGR).
    """
    # Cetak informasi ke console
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("DEMO ASPECT RATIO")
    # Cetak informasi ke console
    print("=" * 60)
    
    tinggi, lebar = gambar.shape[:2]
    
    # Target ukuran yang berbeda aspect ratio
    target = (400, 400)  # Square
    
    # 1. Resize langsung (aspect ratio berubah)
    hasil_stretch = cv2.resize(gambar, target)
    
    # 2. Resize menjaga aspect ratio dengan padding
    hasil_fit = resize_menjaga_aspek_rasio(gambar, max_lebar=400, max_tinggi=400)
    
    # Tambahkan padding untuk membuat square
    delta_w = target[0] - hasil_fit.shape[1]
    delta_h = target[1] - hasil_fit.shape[0]
    top = delta_h // 2
    bottom = delta_h - top
    left = delta_w // 2
    right = delta_w - left
    # Buat salinan dari array/gambar
    hasil_padded = cv2.copyMakeBorder(hasil_fit, top, bottom, left, right,
                                       cv2.BORDER_CONSTANT, value=(50, 50, 50))
    
    # 3. Crop center (untuk fit ke square)
    scale = max(target[0]/lebar, target[1]/tinggi)
    # Resize gambar ke ukuran baru
    resized = cv2.resize(gambar, None, fx=scale, fy=scale)
    center_y = resized.shape[0] // 2
    center_x = resized.shape[1] // 2
    hasil_crop = resized[center_y-200:center_y+200, center_x-200:center_x+200]
    
    # Cetak informasi ke console
    print(f"Original: {lebar} x {tinggi}")
    # Cetak informasi ke console
    print(f"Target: {target[0]} x {target[1]}")
    # Cetak informasi ke console
    print(f"\nMetode:")
    # Cetak informasi ke console
    print(f"1. Stretch: Langsung resize, aspect ratio berubah")
    # Cetak informasi ke console
    print(f"2. Fit: Menjaga aspect ratio, tambah padding")
    # Cetak informasi ke console
    print(f"3. Crop: Menjaga aspect ratio, potong bagian luar")
    
    # Tampilkan
    fig, axes = plt.subplots(1, 4, figsize=(16, 4))
    
    # Tampilkan gambar pada subplot tertentu
    axes[0].imshow(cv2.cvtColor(gambar, cv2.COLOR_BGR2RGB))
    axes[0].set_title(f"Original\n{lebar}x{tinggi}")
    # Nonaktifkan atau atur axis pada subplot
    axes[0].axis('off')
    
    # Tampilkan gambar pada subplot tertentu
    axes[1].imshow(cv2.cvtColor(hasil_stretch, cv2.COLOR_BGR2RGB))
    axes[1].set_title(f"Stretch\n{target[0]}x{target[1]}")
    # Nonaktifkan atau atur axis pada subplot
    axes[1].axis('off')
    
    # Tampilkan gambar pada subplot tertentu
    axes[2].imshow(cv2.cvtColor(hasil_padded, cv2.COLOR_BGR2RGB))
    axes[2].set_title(f"Fit + Padding\n{target[0]}x{target[1]}")
    # Nonaktifkan atau atur axis pada subplot
    axes[2].axis('off')
    
    # Tampilkan gambar pada subplot tertentu
    axes[3].imshow(cv2.cvtColor(hasil_crop, cv2.COLOR_BGR2RGB))
    axes[3].set_title(f"Center Crop\n{target[0]}x{target[1]}")
    # Nonaktifkan atau atur axis pada subplot
    axes[3].axis('off')
    
    # Set judul keseluruhan figure
    plt.suptitle("Berbagai Strategi Resize ke Target Size", fontsize=14)
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

# Keterangan: Menjalankan seluruh rangkaian demo scaling.
def main():
    """Fungsi utama program.

    Menjalankan seluruh demo scaling dan menyimpan hasil.
    """
    
    # Cetak informasi ke console
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("PRAKTIKUM: SCALING GAMBAR")
    # Cetak informasi ke console
    print("Bab 2 - Pembentukan Citra")
    # Cetak informasi ke console
    print("=" * 60)
    
    # Muat gambar
    path_gambar = dapatkan_path_gambar(NAMA_FILE_GAMBAR)
    # Baca gambar dari file
    gambar = cv2.imread(path_gambar)
    
    # Cek apakah variabel kosong/None
    if gambar is None:
        # Cetak informasi ke console
        print(f"[WARNING] Gambar {NAMA_FILE_GAMBAR} tidak ditemukan")
        # Cetak informasi ke console
        print("[INFO] Menggunakan gambar sample...")
        gambar = buat_gambar_sample()
    else:
        # Cetak informasi ke console
        print(f"[SUKSES] Gambar dimuat: {path_gambar}")
        # Resize jika terlalu besar untuk demo
        if gambar.shape[1] > 400:
            scale = 400 / gambar.shape[1]
            # Resize gambar ke ukuran baru
            gambar = cv2.resize(gambar, None, fx=scale, fy=scale)
    
    # Cetak informasi ke console
    print(f"[INFO] Ukuran gambar: {gambar.shape}")
    
    # 1. Demo detail interpolasi
    demo_detail_interpolasi()
    
    # 2. Demo upscaling
    demo_upscaling(gambar)
    
    # 3. Demo downscaling
    demo_downscaling(gambar)
    
    # 4. Demo aspect ratio
    demo_aspect_ratio(gambar)
    
    # Ringkasan
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("RINGKASAN SCALING GAMBAR")
    # Cetak informasi ke console
    print("=" * 60)
    # Cetak informasi ke console
    print("""
FUNGSI RESIZE:
    # Resize gambar ke ukuran baru dengan metode interpolasi tertentu
    cv2.resize(src, dsize, fx, fy, interpolation)
    
PARAMETER:
├── dsize: ukuran output (width, height) atau None
├── fx, fy: faktor skala jika dsize=None
└── interpolation: metode interpolasi

METODE INTERPOLASI:
┌───────────────────┬─────────────┬───────────────────────┐
│ Metode            │ Kecepatan   │ Kasus Penggunaan      │
├───────────────────┼─────────────┼───────────────────────┤
│ INTER_NEAREST     │ ★★★★★       │ Pixel art, mask       │
│ INTER_LINEAR      │ ★★★★☆       │ Default, balanced     │
│ INTER_CUBIC       │ ★★★☆☆       │ Upscaling foto        │
│ INTER_LANCZOS4    │ ★★☆☆☆       │ Upscaling berkualitas │
│ INTER_AREA        │ ★★★☆☆       │ Downscaling           │
└───────────────────┴─────────────┴───────────────────────┘

REKOMENDASI:
├── Upscaling (memperbesar)  : INTER_LANCZOS4 atau INTER_CUBIC
├── Downscaling (memperkecil): INTER_AREA
├── Real-time processing     : INTER_LINEAR
└── Preserve sharp edges     : INTER_NEAREST

STRATEGI RESIZE:
├── Stretch: Langsung ke target size (aspect ratio berubah)
├── Fit: Menjaga aspect ratio + padding
└── Crop: Menjaga aspect ratio + potong bagian luar
""")


# Jalankan program utama
if __name__ == "__main__":
    main()
