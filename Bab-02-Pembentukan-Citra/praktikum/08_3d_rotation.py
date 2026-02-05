# ============================================================
# PROGRAM: 08_3d_rotation.py
# PRAKTIKUM COMPUTER VISION - BAB 2: PEMBENTUKAN CITRA
# ============================================================
# Deskripsi: Demonstrasi rotasi 3D dengan axis-angle (Rodrigues)
#
# Tujuan Pembelajaran:
#   1. Memahami representasi rotasi 3D axis-angle
#   2. Menggunakan rumus Rodrigues untuk rotasi 3D
# ============================================================

import os
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

# ============================================================
# PANDUAN FUNGSI (RINGKAS) - ARTI PARAMETER
# ============================================================
# np.radians(deg)    : konversi derajat ke radian
# np.sin/np.cos      : fungsi trigonometri
# np.linalg.norm(v)  : panjang vektor
# matplotlib.pyplot.savefig(path, dpi, bbox_inches)
#   - path : lokasi simpan gambar

# ============================================================
# KONFIGURASI PATH
# ============================================================
DIR_SCRIPT = os.path.dirname(os.path.abspath(__file__))
DIR_OUTPUT = os.path.join(DIR_SCRIPT, "output", "output8")
# Buat folder jika belum ada
os.makedirs(DIR_OUTPUT, exist_ok=True)

# ============================================================
# FUNGSI ROTASI 3D (RODRIGUES)
# ============================================================

# Keterangan: Menghitung matriks rotasi 3D dari axis-angle.
def rodrigues_rotation_matrix(axis, angle_deg):
    """Membuat matriks rotasi 3D dari axis-angle (Rodrigues).

    Parameter:
    - axis (array-like): vektor sumbu rotasi (x, y, z).
    - angle_deg (float): sudut rotasi dalam derajat.

    Return:
    - np.ndarray: matriks rotasi 3x3.
    """
    # Buat array numpy dari data
    axis = np.array(axis, dtype=np.float32)
    # Hitung norma/magnitude vektor
    axis = axis / (np.linalg.norm(axis) + 1e-12)
    # Konversi derajat ke radian
    theta = np.radians(angle_deg)

    kx, ky, kz = axis
    # Buat array numpy dari data
    K = np.array([
        [0, -kz, ky],
        [kz, 0, -kx],
        [-ky, kx, 0]
    ], dtype=np.float32)

    # Buat matriks identitas
    I = np.eye(3, dtype=np.float32)
    R = I + np.sin(theta) * K + (1 - np.cos(theta)) * (K @ K)
    # Kembalikan hasil dari fungsi
    return R


# Keterangan: Merotasi kumpulan titik 3D dengan axis-angle.
def rotasi_axis_angle(points, axis, angle_deg):
    """Rotasi kumpulan titik 3D.

    Parameter:
    - points (np.ndarray): titik 3D berbentuk (N, 3).
    - axis (array-like): vektor sumbu rotasi.
    - angle_deg (float): sudut rotasi dalam derajat.

    Return:
    - np.ndarray: titik hasil rotasi (N, 3).
    """
    R = rodrigues_rotation_matrix(axis, angle_deg)
    # Kembalikan hasil dari fungsi
    return (R @ points.T).T


# Keterangan: Memproyeksikan titik 3D ke bidang 2D secara ortografik.
def project_orthographic(points):
    """Proyeksi ortografik sederhana: (x, y).

    Parameter:
    - points (np.ndarray): titik 3D berbentuk (N, 3).

    Return:
    - tuple[np.ndarray, np.ndarray]: koordinat (x, y).
    """
    # Kembalikan hasil dari fungsi
    return points[:, 0], points[:, 1]


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

# Keterangan: Menjalankan demo rotasi 3D axis-angle.
def main():
    """Fungsi utama program.

    Menjalankan demo rotasi 3D dan menyimpan hasil.
    """
    # Cetak informasi ke console
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("PRAKTIKUM: ROTASI 3D (AXIS-ANGLE)")
    # Cetak informasi ke console
    print("Bab 2 - Pembentukan Citra")
    # Cetak informasi ke console
    print("=" * 60)

    # Kubus 3D
    cube = np.array([
        [-1, -1, -1],
        [ 1, -1, -1],
        [ 1,  1, -1],
        [-1,  1, -1],
        [-1, -1,  1],
        [ 1, -1,  1],
        [ 1,  1,  1],
        [-1,  1,  1],
    ], dtype=np.float32)

    edges = [
        (0, 1), (1, 2), (2, 3), (3, 0),
        (4, 5), (5, 6), (6, 7), (7, 4),
        (0, 4), (1, 5), (2, 6), (3, 7)
    ]

    axis = [1, 1, 0]
    angle_deg = 45

    rotated = rotasi_axis_angle(cube, axis, angle_deg)

    # Siapkan kanvas plot untuk menampilkan hasil
    fig, axes = plt.subplots(1, 2, figsize=(10, 5))
    plot_wireframe(axes[0], project_orthographic(cube), edges, "Original")
    plot_wireframe(axes[1], project_orthographic(rotated), edges,
                   f"Rotasi {angle_deg}° (axis={axis})")

    # Set judul keseluruhan figure
    plt.suptitle("Rotasi 3D dengan Axis-Angle (Rodrigues)")
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
