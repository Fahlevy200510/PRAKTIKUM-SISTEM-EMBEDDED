# ============================================================
# PROGRAM: 15_compression_artifacts.py
# PRAKTIKUM COMPUTER VISION - BAB 2: PEMBENTUKAN CITRA
# ============================================================
# Deskripsi: Demonstrasi artefak kompresi JPEG
#
# Tujuan Pembelajaran:
#   1. Memahami efek kualitas JPEG terhadap detail dan artefak
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
# cv2.imencode(ext, img, params) : kompresi gambar ke buffer
# cv2.imdecode(buf, flags)       : dekompresi dari buffer
# cv2.putText(img, text, org, fontFace, fontScale, color, thickness)
# matplotlib.pyplot.savefig(path, dpi, bbox_inches)
#   - path : lokasi simpan gambar

DIR_SCRIPT = os.path.dirname(os.path.abspath(__file__))
DIR_OUTPUT = os.path.join(DIR_SCRIPT, "output", "output15")
# Buat folder jika belum ada
os.makedirs(DIR_OUTPUT, exist_ok=True)

# Keterangan: Membuat gambar uji untuk melihat artefak JPEG.
def buat_test_image(w=512, h=384):
    """Membuat gambar uji untuk melihat artefak kompresi.

    Parameter:
    - w (int): lebar gambar.
    - h (int): tinggi gambar.

    Return:
    - np.ndarray: gambar uji (BGR).
    """
    # Inisialisasi kanvas gambar (hitam) ukuran h x w
    img = np.zeros((h, w, 3), dtype=np.uint8)

    # Membuat gradient vertikal agar efek kompresi terlihat jelas
    for y in range(h):
        # Hitung nilai intensitas untuk baris ke-y
        val = int(255 * y / (h - 1))
        # Isi seluruh baris dengan intensitas yang sama
        img[y, :, :] = (val, val, val)

    # Menambah lingkaran berwarna untuk melihat distorsi warna/tepi
    cv2.circle(img, (120, 120), 60, (0, 128, 255), -1)
    # Menambah persegi panjang sebagai area tepi tajam
    cv2.rectangle(img, (250, 60), (450, 180), (255, 0, 0), -1)
    # Menambah teks agar artefak blockiness mudah terlihat
    cv2.putText(img, "JPEG", (180, 300), cv2.FONT_HERSHEY_SIMPLEX, 2, (0, 0, 0), 3)
    # Mengembalikan gambar uji
    return img


# Keterangan: Mengompresi lalu mendekompresi gambar JPEG.
def jpeg_compress(img, quality):
    """Melakukan kompresi JPEG melalui buffer memori.

    Parameter:
    - img (np.ndarray): gambar input (BGR).
    - quality (int): kualitas JPEG (0-100).

    Return:
    - np.ndarray: hasil dekompresi JPEG.
    """
    # Menentukan parameter kualitas JPEG
    encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), quality]
    # Encode gambar ke buffer JPEG
    result, encimg = cv2.imencode('.jpg', img, encode_param)
    # Cek kondisi logis
    if not result:
        raise RuntimeError("Gagal encode JPEG")
    # Decode kembali buffer JPEG ke citra BGR
    decimg = cv2.imdecode(encimg, cv2.IMREAD_COLOR)
    # Mengembalikan hasil dekompresi
    return decimg


# Keterangan: Menjalankan demo artefak kompresi JPEG.
def main():
    """Fungsi utama program.

    Menjalankan demo artefak kompresi JPEG.
    """
    # Cetak informasi ke console
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("PRAKTIKUM: ARTEFAK KOMPRESI JPEG")
    # Cetak informasi ke console
    print("Bab 2 - Pembentukan Citra")
    # Cetak informasi ke console
    print("=" * 60)

    # Membuat gambar uji
    img = buat_test_image()
    # Kompres pada beberapa level kualitas
    q95 = jpeg_compress(img, 95)
    q70 = jpeg_compress(img, 70)
    q40 = jpeg_compress(img, 40)

    # Siapkan kanvas plot perbandingan
    fig, axes = plt.subplots(1, 4, figsize=(14, 4))
    # Tampilkan gambar asli
    axes[0].imshow(cv2.cvtColor(img, cv2.COLOR_BGR2RGB))
    # Set judul untuk subplot
    axes[0].set_title("Original")
    # Nonaktifkan atau atur axis pada subplot
    axes[0].axis('off')

    # Tampilkan hasil JPEG kualitas tinggi
    axes[1].imshow(cv2.cvtColor(q95, cv2.COLOR_BGR2RGB))
    # Set judul untuk subplot
    axes[1].set_title("JPEG Q=95")
    # Nonaktifkan atau atur axis pada subplot
    axes[1].axis('off')

    # Tampilkan hasil JPEG kualitas menengah
    axes[2].imshow(cv2.cvtColor(q70, cv2.COLOR_BGR2RGB))
    # Set judul untuk subplot
    axes[2].set_title("JPEG Q=70")
    # Nonaktifkan atau atur axis pada subplot
    axes[2].axis('off')

    # Tampilkan hasil JPEG kualitas rendah
    axes[3].imshow(cv2.cvtColor(q40, cv2.COLOR_BGR2RGB))
    # Set judul untuk subplot
    axes[3].set_title("JPEG Q=40")
    # Nonaktifkan atau atur axis pada subplot
    axes[3].axis('off')

    # Judul dan layout
    plt.suptitle("Perbandingan Artefak Kompresi JPEG")
    # Atur spacing antar subplot
    plt.tight_layout()
    # Simpan hasil ke file
    output_path = os.path.join(DIR_OUTPUT, "output.png")
    # Simpan figure ke file dengan kualitas DPI tertentu
    plt.savefig(output_path, dpi=120, bbox_inches="tight")
    # Tutup figure untuk menghemat memory
    plt.close()

    # Info output
    print(f"[SAVED] {output_path}")


if __name__ == "__main__":
    main()
