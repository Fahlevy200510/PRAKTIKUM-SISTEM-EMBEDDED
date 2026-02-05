# ============================================================
# PROGRAM: 10_lens_distortion.py
# PRAKTIKUM COMPUTER VISION - BAB 2: PEMBENTUKAN CITRA
# ============================================================
# Deskripsi: Demonstrasi distorsi radial lensa dan koreksinya
#
# Tujuan Pembelajaran:
#   1. Memahami distorsi radial (barrel/pincushion)
#   2. Menggunakan cv2.undistort() untuk koreksi
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
# cv2.undistort(img, K, distCoeffs)
#   - K          : intrinsic matrix 3x3
#   - distCoeffs : [k1, k2, p1, p2, k3]
#
# cv2.line(img, pt1, pt2, color, thickness)
# cv2.rectangle(img, pt1, pt2, color, thickness)
#
# matplotlib.pyplot.savefig(path, dpi, bbox_inches)
#   - path : lokasi simpan gambar

# ============================================================
# KONFIGURASI PATH
# ============================================================
DIR_SCRIPT = os.path.dirname(os.path.abspath(__file__))
DIR_OUTPUT = os.path.join(DIR_SCRIPT, "output", "output10")
# Buat folder jika belum ada
os.makedirs(DIR_OUTPUT, exist_ok=True)

# ============================================================
# FUNGSI BANTU
# ============================================================

# Keterangan: Membuat gambar grid sebagai acuan distorsi.
def buat_grid_image(w=600, h=400, step=40):
    """Membuat gambar grid untuk visualisasi distorsi.

    Parameter:
    - w (int): lebar gambar.
    - h (int): tinggi gambar.
    - step (int): jarak antar garis grid.

    Return:
    - np.ndarray: gambar grid (BGR).
    """
    img = np.ones((h, w, 3), dtype=np.uint8) * 255
    # Iterasi melalui range
    for x in range(0, w, step):
        # Gambar garis pada gambar
        cv2.line(img, (x, 0), (x, h - 1), (0, 0, 0), 1)
    # Iterasi melalui range
    for y in range(0, h, step):
        # Gambar garis pada gambar
        cv2.line(img, (0, y), (w - 1, y), (0, 0, 0), 1)
    # Gambar persegi panjang pada gambar
    cv2.rectangle(img, (50, 50), (w - 50, h - 50), (0, 0, 255), 2)
    # Kembalikan hasil dari fungsi
    return img


# Keterangan: Mensimulasikan distorsi dengan membalik koefisien.
def simulasi_distorsi(img, K, dist_coeffs):
    """Simulasi distorsi (pendekatan) dengan koefisien negatif.

    Parameter:
    - img (np.ndarray): gambar input (BGR).
    - K (np.ndarray): matriks intrinsik 3x3.
    - dist_coeffs (np.ndarray): koefisien distorsi.

    Return:
    - np.ndarray: gambar dengan distorsi simulasi.
    """
    # Kembalikan hasil dari fungsi
    return cv2.undistort(img, K, -dist_coeffs)


# ============================================================
# PROGRAM UTAMA
# ============================================================

# Keterangan: Menjalankan demo distorsi lensa dan koreksinya.
def main():
    """Fungsi utama program.

    Menjalankan demo distorsi radial dan koreksi.
    """
    # Cetak informasi ke console
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("PRAKTIKUM: DISTORSI LENSA")
    # Cetak informasi ke console
    print("Bab 2 - Pembentukan Citra")
    # Cetak informasi ke console
    print("=" * 60)

    img = buat_grid_image()
    h, w = img.shape[:2]

    # Camera intrinsics sederhana
    K = np.array([
        [400, 0, w / 2],
        [0, 400, h / 2],
        [0, 0, 1]
    ], dtype=np.float32)

    # Distorsi radial (k1, k2, p1, p2, k3)
    dist = np.array([0.25, 0.10, 0.0, 0.0, 0.0], dtype=np.float32)

    distorted = simulasi_distorsi(img, K, dist)
    corrected = cv2.undistort(distorted, K, dist)

    # Siapkan kanvas plot untuk menampilkan hasil
    fig, axes = plt.subplots(1, 3, figsize=(12, 4))
    # Tampilkan gambar pada subplot tertentu
    axes[0].imshow(cv2.cvtColor(img, cv2.COLOR_BGR2RGB))
    # Set judul untuk subplot
    axes[0].set_title("Original")
    # Nonaktifkan atau atur axis pada subplot
    axes[0].axis('off')

    # Tampilkan gambar pada subplot tertentu
    axes[1].imshow(cv2.cvtColor(distorted, cv2.COLOR_BGR2RGB))
    # Set judul untuk subplot
    axes[1].set_title("Distorted (simulasi)")
    # Nonaktifkan atau atur axis pada subplot
    axes[1].axis('off')

    # Tampilkan gambar pada subplot tertentu
    axes[2].imshow(cv2.cvtColor(corrected, cv2.COLOR_BGR2RGB))
    # Set judul untuk subplot
    axes[2].set_title("Undistorted")
    # Nonaktifkan atau atur axis pada subplot
    axes[2].axis('off')

    # Set judul keseluruhan figure
    plt.suptitle("Distorsi Radial dan Koreksi")
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
