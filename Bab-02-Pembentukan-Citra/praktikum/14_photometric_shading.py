# ============================================================
# PROGRAM: 14_photometric_shading.py
# PRAKTIKUM COMPUTER VISION - BAB 2: PEMBENTUKAN CITRA
# ============================================================
# Deskripsi: Simulasi shading Lambertian + Phong pada bola
#
# Tujuan Pembelajaran:
#   1. Memahami diffuse (Lambertian) dan specular (Phong)
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
# np.meshgrid(x, y) : membuat grid koordinat
# np.maximum(a, b)  : clamp nilai minimum
# matplotlib.pyplot.savefig(path, dpi, bbox_inches)
#   - path : lokasi simpan gambar

DIR_SCRIPT = os.path.dirname(os.path.abspath(__file__))
DIR_OUTPUT = os.path.join(DIR_SCRIPT, "output", "output14")
# Buat folder jika belum ada
os.makedirs(DIR_OUTPUT, exist_ok=True)


# Keterangan: Menormalkan vektor agar panjangnya 1.
def normalize(v):
    """Normalisasi vektor agar panjangnya 1.

    Parameter:
    - v (np.ndarray): vektor input.

    Return:
    - np.ndarray: vektor ternormalisasi.
    """
    # Kembalikan hasil dari fungsi
    return v / (np.linalg.norm(v) + 1e-12)


# Keterangan: Menjalankan simulasi shading Lambertian + Phong.
def main():
    """Fungsi utama program.

    Menjalankan simulasi shading Lambertian + Phong.
    """
    # Cetak informasi ke console
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("PRAKTIKUM: PHOTOMETRIC SHADING")
    # Cetak informasi ke console
    print("Bab 2 - Pembentukan Citra")
    # Cetak informasi ke console
    print("=" * 60)

    size = 256
    x = np.linspace(-1, 1, size)
    y = np.linspace(-1, 1, size)
    xv, yv = np.meshgrid(x, y)
    mask = xv**2 + yv**2 <= 1
    # Inisialisasi array numpy dengan nilai nol
    zv = np.zeros_like(xv)
    zv[mask] = np.sqrt(1 - xv[mask]**2 - yv[mask]**2)

    normals = np.stack([xv, yv, zv], axis=-1)
    normals = np.where(mask[..., None], normals, 0)

    light_dir = normalize(np.array([0.4, -0.3, 1.0], dtype=np.float32))
    view_dir = normalize(np.array([0.0, 0.0, 1.0], dtype=np.float32))

    # Lambertian
    dot_nl = np.maximum(0, np.sum(normals * light_dir, axis=-1))
    diffuse = dot_nl

    # Phong specular
    reflect = 2 * dot_nl[..., None] * normals - light_dir
    dot_rv = np.maximum(0, np.sum(reflect * view_dir, axis=-1))
    shininess = 30
    specular = dot_rv ** shininess

    ambient = 0.1
    shaded = np.clip(ambient + 0.8 * diffuse + 0.6 * specular, 0, 1)

    # Visualisasi
    fig, axes = plt.subplots(1, 3, figsize=(12, 4))
    # Tampilkan gambar pada subplot tertentu
    axes[0].imshow(diffuse, cmap='gray', vmin=0, vmax=1)
    # Set judul untuk subplot
    axes[0].set_title("Diffuse (Lambertian)")
    # Nonaktifkan atau atur axis pada subplot
    axes[0].axis('off')

    # Tampilkan gambar pada subplot tertentu
    axes[1].imshow(specular, cmap='gray', vmin=0, vmax=1)
    # Set judul untuk subplot
    axes[1].set_title("Specular (Phong)")
    # Nonaktifkan atau atur axis pada subplot
    axes[1].axis('off')

    # Tampilkan gambar pada subplot tertentu
    axes[2].imshow(shaded, cmap='gray', vmin=0, vmax=1)
    # Set judul untuk subplot
    axes[2].set_title("Combined")
    # Nonaktifkan atau atur axis pada subplot
    axes[2].axis('off')

    # Set judul keseluruhan figure
    plt.suptitle("Simulasi Shading")
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
