# ============================================================
# PROGRAM: 09_projection_perspective.py
# PRAKTIKUM COMPUTER VISION - BAB 2: PEMBENTUKAN CITRA
# ============================================================
# Deskripsi: Demonstrasi proyeksi 3D ke 2D (ortografik vs perspektif)
#
# Tujuan Pembelajaran:
#   1. Memahami perbedaan proyeksi ortografik dan perspektif
#   2. Menggunakan camera intrinsics sederhana
# ============================================================

import os
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

# ============================================================
# PANDUAN FUNGSI (RINGKAS) - ARTI PARAMETER
# ============================================================
# Proyeksi perspektif: x' = (fx * X/Z) + cx, y' = (fy * Y/Z) + cy
# np.clip(z, min, max) : mencegah pembagian nol
# matplotlib.pyplot.savefig(path, dpi, bbox_inches)
#   - path : lokasi simpan gambar

# ============================================================
# KONFIGURASI PATH
# ============================================================
DIR_SCRIPT = os.path.dirname(os.path.abspath(__file__))
DIR_OUTPUT = os.path.join(DIR_SCRIPT, "output", "output9")
# Buat folder jika belum ada
os.makedirs(DIR_OUTPUT, exist_ok=True)

# ============================================================
# FUNGSI PROYEKSI
# ============================================================

# Keterangan: Proyeksi ortografik sederhana (x, y).
def project_orthographic(points):
    """Proyeksi ortografik sederhana: (x, y).

    Parameter:
    - points (np.ndarray): titik 3D berbentuk (N, 3).

    Return:
    - tuple[np.ndarray, np.ndarray]: koordinat (x, y).
    """
    # Kembalikan hasil dari fungsi
    return points[:, 0], points[:, 1]


# Keterangan: Proyeksi ortografik dengan faktor skala.
def project_scaled_orthographic(points, scale=200):
    """Proyeksi ortografik dengan skala.

    Parameter:
    - points (np.ndarray): titik 3D (N, 3).
    - scale (float): faktor skala pada sumbu x/y.

    Return:
    - tuple[np.ndarray, np.ndarray]: koordinat (x, y) terskala.
    """
    x, y = project_orthographic(points)
    # Kembalikan hasil dari fungsi
    return scale * x, scale * y


# Keterangan: Proyeksi perspektif menggunakan matriks intrisik K.
def project_perspective(points, K):
    """Proyeksi perspektif menggunakan camera intrinsics.

    Parameter:
    - points (np.ndarray): titik 3D (N, 3).
    - K (np.ndarray): matriks intrinsik 3x3.

    Return:
    - tuple[np.ndarray, np.ndarray]: koordinat (x, y) terproyeksi.
    """
    # Buat salinan dari array/gambar
    pts = points.copy()
    z = pts[:, 2].reshape(-1, 1)
    z = np.clip(z, 1e-6, None)
    pts_norm = pts / z
    pts_h = pts_norm.T
    proj = (K @ pts_h).T
    # Kembalikan hasil dari fungsi
    return proj[:, 0], proj[:, 1]


# Keterangan: Menggambar wireframe 2D dari titik dan edge.
def plot_wireframe(ax, pts2d, edges, title):
    """Plot wireframe 2D.

    Parameter:
    - ax (matplotlib.axes.Axes): axes untuk plotting.
    - pts2d (tuple[np.ndarray, np.ndarray]): koordinat (x, y).
    - edges (list[tuple[int, int]]): daftar pasangan indeks titik.
    - title (str): judul subplot.
    """
    x, y = pts2d
    for i, j in edges:
        ax.plot([x[i], x[j]], [y[i], y[j]], "-", linewidth=2)
    ax.scatter(x, y, s=40)
    ax.set_aspect("equal")
    ax.set_title(title)
    ax.axis("off")


# ============================================================
# PROGRAM UTAMA
# ============================================================

# Keterangan: Menjalankan demo perbandingan proyeksi 3D ke 2D.
def main():
    """Fungsi utama program.

    Menjalankan demo proyeksi 3D ke 2D.
    """
    # Cetak informasi ke console
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("PRAKTIKUM: PROYEKSI 3D KE 2D")
    # Cetak informasi ke console
    print("Bab 2 - Pembentukan Citra")
    # Cetak informasi ke console
    print("=" * 60)

    # Kubus 3D (geser ke depan kamera)
    cube = np.array([
        [-1, -1,  2],
        [ 1, -1,  2],
        [ 1,  1,  2],
        [-1,  1,  2],
        [-1, -1,  4],
        [ 1, -1,  4],
        [ 1,  1,  4],
        [-1,  1,  4],
    ], dtype=np.float32)

    edges = [
        (0, 1), (1, 2), (2, 3), (3, 0),
        (4, 5), (5, 6), (6, 7), (7, 4),
        (0, 4), (1, 5), (2, 6), (3, 7)
    ]

    # Camera intrinsics sederhana
    fx = fy = 300
    cx = cy = 0
    # Buat array numpy dari data
    K = np.array([
        [fx, 0, cx],
        [0, fy, cy],
        [0,  0,  1]
    ], dtype=np.float32)

    # Siapkan kanvas plot untuk menampilkan hasil
    fig, axes = plt.subplots(1, 3, figsize=(12, 4))

    plot_wireframe(axes[0], project_orthographic(cube), edges, "Ortografik")
    plot_wireframe(axes[1], project_scaled_orthographic(cube, scale=200), edges,
                   "Scaled Orthografik")
    plot_wireframe(axes[2], project_perspective(cube, K), edges, "Perspektif")

    # Set judul keseluruhan figure
    plt.suptitle("Perbandingan Proyeksi 3D ke 2D")
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
