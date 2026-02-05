# ============================================================
# PROGRAM: 11_sampling_aliasing.py
# PRAKTIKUM COMPUTER VISION - BAB 2: PEMBENTUKAN CITRA
# ============================================================
# Deskripsi: Demonstrasi sampling dan aliasing saat downsampling
#
# Tujuan Pembelajaran:
#   1. Memahami efek aliasing
#   2. Membandingkan downsampling dengan/ tanpa anti-aliasing
# ============================================================

import os
import numpy as np
import cv2
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

# ============================================================
# PANDUAN FUNGSI (RINGKAS) - ARTI PARAMETER
# ============================================================
# cv2.resize(img, dsize/None, fx, fy, interpolation)
#   - INTER_NEAREST : tanpa smoothing (aliasing jelas)
#   - INTER_AREA    : downsampling yang lebih halus
#
# cv2.GaussianBlur(img, ksize, sigmaX)
#   - ksize: ukuran kernel blur (ganjil)
#
# matplotlib.pyplot.savefig(path, dpi, bbox_inches)
#   - path : lokasi simpan gambar

# ============================================================
# KONFIGURASI PATH
# ============================================================
DIR_SCRIPT = os.path.dirname(os.path.abspath(__file__))
DIR_OUTPUT = os.path.join(DIR_SCRIPT, "output", "output11")
# Buat folder jika belum ada
os.makedirs(DIR_OUTPUT, exist_ok=True)


# Keterangan: Membuat pola checkerboard untuk melihat aliasing.
def buat_checkerboard(size=512, block=4):
    """Membuat gambar checkerboard untuk demo aliasing.

    Parameter:
    - size (int): ukuran sisi gambar.
    - block (int): ukuran blok checkerboard (piksel).

    Return:
    - np.ndarray: gambar checkerboard (BGR).
    """
    # Inisialisasi array numpy dengan nilai nol
    img = np.zeros((size, size), dtype=np.uint8)
    # Iterasi melalui range
    for y in range(size):
        # Iterasi melalui range
        for x in range(size):
            if ((x // block) + (y // block)) % 2 == 0:
                img[y, x] = 255
    # Kembalikan hasil dari fungsi
    return cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)


# Keterangan: Menjalankan demo sampling dan aliasing.
def main():
    """Fungsi utama program.

    Menjalankan demo sampling dan aliasing.
    """
    # Cetak informasi ke console
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("PRAKTIKUM: SAMPLING & ALIASING")
    # Cetak informasi ke console
    print("Bab 2 - Pembentukan Citra")
    # Cetak informasi ke console
    print("=" * 60)

    img = buat_checkerboard(size=512, block=4)

    # Downsampling 4x
    nearest = cv2.resize(img, None, fx=0.25, fy=0.25, interpolation=cv2.INTER_NEAREST)
    # Resize gambar ke ukuran baru
    area = cv2.resize(img, None, fx=0.25, fy=0.25, interpolation=cv2.INTER_AREA)

    # Anti-aliasing (Gaussian blur sebelum downsample)
    blurred = cv2.GaussianBlur(img, (7, 7), 0)
    # Resize gambar ke ukuran baru
    blurred_down = cv2.resize(blurred, None, fx=0.25, fy=0.25, interpolation=cv2.INTER_NEAREST)

    # Siapkan kanvas plot untuk menampilkan hasil
    fig, axes = plt.subplots(1, 4, figsize=(14, 4))
    # Tampilkan gambar pada subplot tertentu
    axes[0].imshow(cv2.cvtColor(img, cv2.COLOR_BGR2RGB))
    # Set judul untuk subplot
    axes[0].set_title("Original")
    # Nonaktifkan atau atur axis pada subplot
    axes[0].axis('off')

    # Tampilkan gambar pada subplot tertentu
    axes[1].imshow(cv2.cvtColor(nearest, cv2.COLOR_BGR2RGB))
    # Set judul untuk subplot
    axes[1].set_title("Nearest (aliasing)")
    # Nonaktifkan atau atur axis pada subplot
    axes[1].axis('off')

    # Tampilkan gambar pada subplot tertentu
    axes[2].imshow(cv2.cvtColor(area, cv2.COLOR_BGR2RGB))
    # Set judul untuk subplot
    axes[2].set_title("INTER_AREA")
    # Nonaktifkan atau atur axis pada subplot
    axes[2].axis('off')

    # Tampilkan gambar pada subplot tertentu
    axes[3].imshow(cv2.cvtColor(blurred_down, cv2.COLOR_BGR2RGB))
    # Set judul untuk subplot
    axes[3].set_title("Blur + Nearest")
    # Nonaktifkan atau atur axis pada subplot
    axes[3].axis('off')

    # Set judul keseluruhan figure
    plt.suptitle("Downsampling dan Aliasing")
    # Atur spacing antar subplot
    plt.tight_layout()
    output_path = os.path.join(DIR_OUTPUT, "output.png")
    # Simpan figure ke file dengan kualitas DPI tertentu
    plt.savefig(output_path, dpi=120, bbox_inches="tight")
    # Tutup figure untuk menghemat memory
    plt.close()

    # Cetak informasi ke console
    print(f"[SAVED] {output_path}")


if __name__ == "__main__":
    main()
