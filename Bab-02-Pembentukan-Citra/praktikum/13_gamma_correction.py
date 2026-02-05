# ============================================================
# PROGRAM: 13_gamma_correction.py
# PRAKTIKUM COMPUTER VISION - BAB 2: PEMBENTUKAN CITRA
# ============================================================
# Deskripsi: Demonstrasi gamma correction (encode & decode)
#
# Tujuan Pembelajaran:
#   1. Memahami efek gamma terhadap intensitas
#   2. Melihat perbedaan linear vs gamma-compressed
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
# np.power(x, p) : pangkat (digunakan untuk gamma encode/decode)
# matplotlib.pyplot.savefig(path, dpi, bbox_inches)
#   - path : lokasi simpan gambar

DIR_SCRIPT = os.path.dirname(os.path.abspath(__file__))
DIR_OUTPUT = os.path.join(DIR_SCRIPT, "output", "output13")
# Buat folder jika belum ada
os.makedirs(DIR_OUTPUT, exist_ok=True)


# Keterangan: Mengubah citra linear menjadi gamma-compressed.
def gamma_encode(img, gamma=2.2):
    """Melakukan gamma encode pada citra linear.

    Parameter:
    - img (np.ndarray): citra input float32 rentang 0..1.
    - gamma (float): nilai gamma.

    Return:
    - np.ndarray: citra hasil gamma encode.
    """
    # Kembalikan hasil dari fungsi
    return np.power(img, 1.0 / gamma)


# Keterangan: Mengembalikan citra gamma-compressed ke domain linear.
def gamma_decode(img, gamma=2.2):
    """Melakukan gamma decode ke domain linear.

    Parameter:
    - img (np.ndarray): citra input float32 rentang 0..1.
    - gamma (float): nilai gamma.

    Return:
    - np.ndarray: citra hasil gamma decode.
    """
    # Kembalikan hasil dari fungsi
    return np.power(img, gamma)


# Keterangan: Menjalankan demo gamma correction.
def main():
    """Fungsi utama program.

    Menjalankan demo gamma correction.
    """
    # Cetak informasi ke console
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("PRAKTIKUM: GAMMA CORRECTION")
    # Cetak informasi ke console
    print("Bab 2 - Pembentukan Citra")
    # Cetak informasi ke console
    print("=" * 60)

    # Ramp linear 0..1
    ramp = np.tile(np.linspace(0, 1, 512, dtype=np.float32), (128, 1))

    encoded = gamma_encode(ramp, gamma=2.2)
    decoded = gamma_decode(encoded, gamma=2.2)

    # Siapkan kanvas plot untuk menampilkan hasil
    fig, axes = plt.subplots(1, 3, figsize=(12, 3))

    # Tampilkan gambar pada subplot tertentu
    axes[0].imshow(ramp, cmap='gray', vmin=0, vmax=1)
    # Set judul untuk subplot
    axes[0].set_title("Linear")
    # Nonaktifkan atau atur axis pada subplot
    axes[0].axis('off')

    # Tampilkan gambar pada subplot tertentu
    axes[1].imshow(encoded, cmap='gray', vmin=0, vmax=1)
    # Set judul untuk subplot
    axes[1].set_title("Gamma Encode (1/2.2)")
    # Nonaktifkan atau atur axis pada subplot
    axes[1].axis('off')

    # Tampilkan gambar pada subplot tertentu
    axes[2].imshow(decoded, cmap='gray', vmin=0, vmax=1)
    # Set judul untuk subplot
    axes[2].set_title("Gamma Decode (2.2)")
    # Nonaktifkan atau atur axis pada subplot
    axes[2].axis('off')

    # Set judul keseluruhan figure
    plt.suptitle("Gamma Correction")
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
