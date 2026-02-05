# ============================================================
# PROGRAM: 12_color_spaces.py
# PRAKTIKUM COMPUTER VISION - BAB 2: PEMBENTUKAN CITRA
# ============================================================
# Deskripsi: Demonstrasi konversi color space (RGB, HSV, LAB, YCrCb, XYZ)
#
# Tujuan Pembelajaran:
#   1. Memahami representasi warna berbeda
#   2. Melihat perbedaan channel antar color space
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
# cv2.cvtColor(img, code)
#   - BGR2RGB, BGR2HSV, BGR2LAB, BGR2YCrCb, HSV2BGR, dst
#
# np.linspace(start, stop, num)
#   - membuat gradasi nilai
#
# matplotlib.pyplot.savefig(path, dpi, bbox_inches)
#   - path : lokasi simpan gambar

# ============================================================
# KONFIGURASI PATH
# ============================================================
DIR_SCRIPT = os.path.dirname(os.path.abspath(__file__))
DIR_OUTPUT = os.path.join(DIR_SCRIPT, "output", "output12")
# Buat folder jika belum ada
os.makedirs(DIR_OUTPUT, exist_ok=True)


# Keterangan: Membuat gradasi HSV sebagai input demo.
def buat_gradient_hsv(w=512, h=256):
    """Membuat gambar gradasi HSV untuk demo color space.

    Parameter:
    - w (int): lebar gambar.
    - h (int): tinggi gambar.

    Return:
    - np.ndarray: gambar gradasi (BGR).
    """
    # Inisialisasi array numpy dengan nilai nol
    hsv = np.zeros((h, w, 3), dtype=np.uint8)
    hsv[..., 0] = np.tile(np.linspace(0, 179, w, dtype=np.uint8), (h, 1))
    hsv[..., 1] = 200
    hsv[..., 2] = np.tile(np.linspace(50, 255, h, dtype=np.uint8).reshape(-1, 1), (1, w))
    # Konversi gambar ke format warna berbeda
    bgr = cv2.cvtColor(hsv, cv2.COLOR_HSV2BGR)
    # Kembalikan hasil dari fungsi
    return bgr


# Keterangan: Mengonversi RGB (0..1) ke XYZ.
def rgb_to_xyz(rgb):
    """Konversi RGB (0..1) ke XYZ menggunakan matriks CIE (sesuai materi).

    Parameter:
    - rgb (np.ndarray): gambar RGB float32 rentang 0..1.

    Return:
    - np.ndarray: gambar XYZ.
    """
    # Buat array numpy dari data
    M = np.array([
        [0.49, 0.31, 0.20],
        [0.17697, 0.81240, 0.01063],
        [0.00, 0.01, 0.99]
    ], dtype=np.float32)
    h, w, _ = rgb.shape
    flat = rgb.reshape(-1, 3)
    xyz = (flat @ M.T).reshape(h, w, 3)
    # Kembalikan hasil dari fungsi
    return xyz


# Keterangan: Menormalkan channel ke rentang 0..1 untuk visualisasi.
def normalize_channel(ch):
    """Normalisasi channel ke rentang 0..1 untuk visualisasi.

    Parameter:
    - ch (np.ndarray): channel input.

    Return:
    - np.ndarray: channel ternormalisasi.
    """
    ch_min, ch_max = ch.min(), ch.max()
    if ch_max - ch_min < 1e-6:
        # Kembalikan hasil dari fungsi
        return np.zeros_like(ch)
    # Kembalikan hasil dari fungsi
    return (ch - ch_min) / (ch_max - ch_min)


# Keterangan: Menjalankan demo perbandingan color space.
def main():
    """Fungsi utama program.

    Menjalankan demo perbandingan color space.
    """
    # Cetak informasi ke console
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("PRAKTIKUM: COLOR SPACES")
    # Cetak informasi ke console
    print("Bab 2 - Pembentukan Citra")
    # Cetak informasi ke console
    print("=" * 60)

    bgr = buat_gradient_hsv()
    # Konversi gambar ke format warna berbeda
    rgb = cv2.cvtColor(bgr, cv2.COLOR_BGR2RGB)

    # Konversi gambar ke format warna berbeda
    hsv = cv2.cvtColor(bgr, cv2.COLOR_BGR2HSV)
    # Konversi gambar ke format warna berbeda
    lab = cv2.cvtColor(bgr, cv2.COLOR_BGR2LAB)
    # Konversi gambar ke format warna berbeda
    ycrcb = cv2.cvtColor(bgr, cv2.COLOR_BGR2YCrCb)

    rgb01 = rgb.astype(np.float32) / 255.0
    xyz = rgb_to_xyz(rgb01)

    # Siapkan kanvas plot untuk menampilkan hasil
    fig, axes = plt.subplots(3, 4, figsize=(12, 9))

    # RGB
    axes[0, 0].imshow(rgb)
    # Set judul untuk subplot
    axes[0, 0].set_title("RGB")
    # Nonaktifkan atau atur axis pada subplot
    axes[0, 0].axis('off')
    for i, ch in enumerate([rgb[..., 0], rgb[..., 1], rgb[..., 2]]):
        # Tampilkan gambar pada subplot tertentu
        axes[1, i].imshow(ch, cmap='gray')
        # Set judul untuk subplot
        axes[1, i].set_title(["R", "G", "B"][i])
        # Nonaktifkan atau atur axis pada subplot
        axes[1, i].axis('off')

    # HSV
    axes[0, 1].imshow(cv2.cvtColor(hsv, cv2.COLOR_HSV2RGB))
    # Set judul untuk subplot
    axes[0, 1].set_title("HSV")
    # Nonaktifkan atau atur axis pada subplot
    axes[0, 1].axis('off')
    for i, ch in enumerate([hsv[..., 0], hsv[..., 1], hsv[..., 2]]):
        # Tampilkan gambar pada subplot tertentu
        axes[2, i].imshow(ch, cmap='gray')
        # Set judul untuk subplot
        axes[2, i].set_title(["H", "S", "V"][i])
        # Nonaktifkan atau atur axis pada subplot
        axes[2, i].axis('off')

    # LAB preview
    axes[0, 2].imshow(cv2.cvtColor(lab, cv2.COLOR_LAB2RGB))
    # Set judul untuk subplot
    axes[0, 2].set_title("LAB")
    # Nonaktifkan atau atur axis pada subplot
    axes[0, 2].axis('off')

    # YCrCb preview
    axes[0, 3].imshow(cv2.cvtColor(ycrcb, cv2.COLOR_YCrCb2RGB))
    # Set judul untuk subplot
    axes[0, 3].set_title("YCrCb")
    # Nonaktifkan atau atur axis pada subplot
    axes[0, 3].axis('off')

    # XYZ channels
    for i, ch in enumerate([xyz[..., 0], xyz[..., 1], xyz[..., 2]]):
        # Tampilkan gambar pada subplot tertentu
        axes[1, i].imshow(normalize_channel(ch), cmap='gray')
        # Set judul untuk subplot
        axes[1, i].set_title(["X", "Y", "Z"][i])
        # Nonaktifkan atau atur axis pada subplot
        axes[1, i].axis('off')

    # LAB channels
    for i, ch in enumerate([lab[..., 0], lab[..., 1], lab[..., 2]]):
        # Tampilkan gambar pada subplot tertentu
        axes[2, i + 1].imshow(ch, cmap='gray')
        # Set judul untuk subplot
        axes[2, i + 1].set_title(["L", "a", "b"][i])
        # Nonaktifkan atau atur axis pada subplot
        axes[2, i + 1].axis('off')

    # Set judul keseluruhan figure
    plt.suptitle("Perbandingan Color Space")
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
