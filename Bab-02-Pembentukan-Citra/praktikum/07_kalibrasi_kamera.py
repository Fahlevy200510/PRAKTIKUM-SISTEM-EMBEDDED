# ============================================================
# PROGRAM: 07_kalibrasi_kamera.py
# PRAKTIKUM COMPUTER VISION - BAB 2: PEMBENTUKAN CITRA
# ============================================================
# Deskripsi: Program untuk kalibrasi kamera menggunakan pola
#            checkerboard untuk mendapatkan parameter intrinsik
# 
# Tujuan Pembelajaran:
#   1. Memahami parameter intrinsik dan ekstrinsik kamera
#   2. Menggunakan pola kalibrasi checkerboard
#   3. Menghitung distorsi lensa dan undistortion
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
import glob

# ============================================================
# PANDUAN FUNGSI (RINGKAS) - ARTI PARAMETER
# ============================================================
# cv2.findChessboardCorners(gray, pattern_size, flags)
#   - pattern_size: (cols-1, rows-1) jumlah inner corners
#
# cv2.cornerSubPix(gray, corners, winSize, zeroZone, criteria)
#   - winSize : ukuran jendela pencarian sub-pixel
#   - criteria: kriteria terminasi
#
# cv2.calibrateCamera(objPoints, imgPoints, imageSize, cameraMatrix, distCoeffs)
#
# cv2.undistort(img, cameraMatrix, distCoeffs)
#
# cv2.getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, alpha, newImgSize)
#
# cv2.initUndistortRectifyMap(cameraMatrix, distCoeffs, R, newCameraMatrix, size, m1type)
#
# cv2.remap(src, map1, map2, interpolation)
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
DIR_OUTPUT = os.path.join(DIR_SCRIPT, "output", "output7")

# Pastikan folder output ada
os.makedirs(DIR_OUTPUT, exist_ok=True)


# 1. Ukuran papan catur (jumlah inner corners)
# Contoh: checkerboard 9x6 memiliki 8x5 inner corners
CHECKERBOARD_COLS = 9  # jumlah kotak horizontal
CHECKERBOARD_ROWS = 6  # jumlah kotak vertikal

# 2. Ukuran kotak dalam satuan real (mm atau cm)
SQUARE_SIZE = 25  # mm

# 3. Jumlah gambar minimum untuk kalibrasi yang baik
MIN_IMAGES = 10

# 4. Kriteria terminasi untuk corner refinement
CRITERIA = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)

# ============================================================
# FUNGSI HELPER
# ============================================================

# Keterangan: Membuat gambar pola checkerboard untuk kalibrasi.
def buat_checkerboard_pattern(rows=6, cols=9, square_size=50):
    """Membuat gambar pola checkerboard.

    Parameter:
    - rows (int): jumlah baris kotak.
    - cols (int): jumlah kolom kotak.
    - square_size (int): ukuran kotak dalam piksel.

    Return:
    - np.ndarray: gambar checkerboard (grayscale).
    """
    height = rows * square_size
    width = cols * square_size
    
    # Inisialisasi array numpy dengan nilai nol
    pattern = np.zeros((height, width), dtype=np.uint8)
    
    # Iterasi melalui range untuk pemrosesan
    for i in range(rows):
        # Iterasi melalui range
        for j in range(cols):
            if (i + j) % 2 == 0:
                y1 = i * square_size
                y2 = (i + 1) * square_size
                x1 = j * square_size
                x2 = (j + 1) * square_size
                pattern[y1:y2, x1:x2] = 255
    
    # Tambahkan border
    border = 20
    pattern_with_border = np.ones((height + 2*border, width + 2*border), 
                                   dtype=np.uint8) * 200
    pattern_with_border[border:border+height, border:border+width] = pattern
    
    # Kembalikan hasil dari fungsi
    return pattern_with_border


# Keterangan: Membuat simulasi dataset kalibrasi dari berbagai sudut.
def simulasi_gambar_kalibrasi(pattern, num_images=15):
    """Membuat simulasi gambar kalibrasi dengan berbagai sudut pandang.

    Parameter:
    - pattern (np.ndarray): gambar checkerboard (grayscale).
    - num_images (int): jumlah gambar simulasi.

    Return:
    - list[np.ndarray]: daftar gambar simulasi (BGR).
    """
    images = []
    
    h, w = pattern.shape[:2]
    
    # Konversi ke BGR untuk simulasi kamera
    pattern_bgr = cv2.cvtColor(pattern, cv2.COLOR_GRAY2BGR)
    
    # Berbagai transformasi untuk simulasi pengambilan dari sudut berbeda
    transformations = [
        # (scale, angle, tx, ty, perspective_strength)
        (0.8, 0, 50, 50, 0),
        (0.7, 15, 100, 30, 0.0001),
        (0.75, -10, 80, 60, 0.0002),
        (0.85, 5, 40, 80, 0.00015),
        (0.65, 20, 120, 40, 0.0001),
        (0.9, -5, 30, 30, 0.00005),
        (0.7, 10, 90, 70, 0.00018),
        (0.8, -15, 60, 50, 0.00012),
        (0.75, 8, 110, 35, 0.00008),
        (0.85, -8, 45, 65, 0.0001),
        (0.6, 25, 130, 45, 0.0002),
        (0.78, 12, 70, 55, 0.00015),
        (0.82, -12, 55, 75, 0.00013),
        (0.68, 18, 95, 40, 0.00017),
        (0.88, -3, 35, 45, 0.00006),
    ]
    
    output_size = (640, 480)
    
    for i, (scale, angle, tx, ty, persp) in enumerate(transformations[:num_images]):
        # Buat transformation matrix
        center = (w // 2, h // 2)
        
        # Rotation + Scale matrix
        M_rot = cv2.getRotationMatrix2D(center, angle, scale)
        M_rot[0, 2] += tx
        M_rot[1, 2] += ty
        
        # Apply affine transform first
        temp = cv2.warpAffine(pattern_bgr, M_rot, (output_size[0], output_size[1]))
        
        # Apply perspective if specified
        if persp > 0:
            pts1 = np.float32([[0, 0], [output_size[0], 0], 
                              [0, output_size[1]], [output_size[0], output_size[1]]])
            pts2 = np.float32([
                [output_size[0]*persp*100, output_size[1]*persp*50],
                [output_size[0]*(1-persp*100), output_size[1]*persp*80],
                [output_size[0]*persp*50, output_size[1]*(1-persp*50)],
                [output_size[0]*(1-persp*80), output_size[1]*(1-persp*100)]
            ])
            # Hitung matriks transformasi perspektif
            M_persp = cv2.getPerspectiveTransform(pts1, pts2)
            # Terapkan transformasi perspektif pada gambar
            temp = cv2.warpPerspective(temp, M_persp, output_size)
        
        # Tambahkan sedikit noise
        noise = np.random.normal(0, 5, temp.shape).astype(np.int16)
        temp = np.clip(temp.astype(np.int16) + noise, 0, 255).astype(np.uint8)
        
        images.append(temp)
    
    # Kembalikan hasil dari fungsi
    return images


# ============================================================
# FUNGSI KALIBRASI
# ============================================================

# Keterangan: Menjelaskan parameter intrinsik/ekstrinsik kamera.
def penjelasan_parameter_kamera():
    """Menjelaskan parameter intrinsik dan ekstrinsik kamera."""
    # Cetak informasi ke console
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("PARAMETER KAMERA")
    # Cetak informasi ke console
    print("=" * 60)
    
    # Cetak informasi ke console
    print("""
CAMERA MATRIX (Intrinsic Parameters):
┌                         ┐
│  fx    0    cx          │     fx, fy = focal length (pixel)
│   0   fy    cy          │     cx, cy = principal point (optical center)
│   0    0     1          │
└                         ┘

DISTORTION COEFFICIENTS:
    (k1, k2, p1, p2, k3)

    Radial Distortion (k1, k2, k3):
    ├── k1, k2, k3 > 0 : barrel distortion (melengkung keluar)
    └── k1, k2, k3 < 0 : pincushion distortion (melengkung dalam)
    
    x_corrected = x(1 + k1*r² + k2*r⁴ + k3*r⁶)
    y_corrected = y(1 + k1*r² + k2*r⁴ + k3*r⁶)
    
    Tangential Distortion (p1, p2):
    └── Disebabkan oleh lensa tidak sejajar dengan sensor
    
    x_corrected = x + [2*p1*x*y + p2*(r² + 2*x²)]
    y_corrected = y + [p1*(r² + 2*y²) + 2*p2*x*y]

EXTRINSIC PARAMETERS:
    R = Rotation matrix (3x3) - orientasi kamera
    t = Translation vector (3x1) - posisi kamera

    P = K[R|t]  (projection matrix)
    
    Mengubah koordinat dunia (X, Y, Z) ke koordinat gambar (x, y):
    ┌   ┐     ┌   ┐   ┌       ┐   ┌   ┐
    │ x │     │ fx  0  cx │   │ r11 r12 r13 tx │   │ X │
    │ y │ = s │  0 fy  cy │ × │ r21 r22 r23 ty │ × │ Y │
    │ 1 │     │  0  0   1 │   │ r31 r32 r33 tz │   │ Z │
    └   ┘     └   ┘   └       ┘   │ 1 │
                                    └   ┘
    """)


# Keterangan: Mendeteksi sudut checkerboard pada gambar.
def deteksi_checkerboard(gambar, pattern_size):
    """Mendeteksi corner checkerboard dalam gambar.

    Parameter:
    - gambar (np.ndarray): input image (BGR atau grayscale).
    - pattern_size (tuple[int, int]): (cols-1, rows-1) untuk inner corners.

    Return:
    - bool: apakah checkerboard ditemukan.
    - np.ndarray | None: koordinat corners yang ditemukan.
    """
    # Konversi ke grayscale jika perlu
    if len(gambar.shape) == 3:
        # Konversi gambar ke format warna berbeda
        gray = cv2.cvtColor(gambar, cv2.COLOR_BGR2GRAY)
    else:
        # Buat salinan dari array/gambar
        gray = gambar.copy()
    
    # Temukan corners
    found, corners = cv2.findChessboardCorners(
        gray, pattern_size,
        cv2.CALIB_CB_ADAPTIVE_THRESH + 
        cv2.CALIB_CB_FAST_CHECK + 
        cv2.CALIB_CB_NORMALIZE_IMAGE
    )
    
    # Refine corners jika ditemukan
    if found:
        corners = cv2.cornerSubPix(gray, corners, (11, 11), (-1, -1), CRITERIA)
    
    # Kembalikan hasil dari fungsi
    return found, corners


# Keterangan: Menghitung parameter kamera dari kumpulan gambar.
def kalibrasi_kamera(images, pattern_size, square_size):
    """Melakukan kalibrasi kamera dari sekumpulan gambar.

    Parameter:
    - images (list[np.ndarray]): list gambar checkerboard.
    - pattern_size (tuple[int, int]): (cols-1, rows-1).
    - square_size (float): ukuran kotak dalam satuan real.

    Return:
    - float | None: RMS re-projection error.
    - np.ndarray | None: camera matrix intrinsik.
    - np.ndarray | None: koefisien distorsi.
    - list[np.ndarray] | None: rotation vectors.
    - list[np.ndarray] | None: translation vectors.
    """
    # Cetak informasi ke console
    print("\n[INFO] Memulai proses kalibrasi...")
    
    # Siapkan object points (koordinat 3D di world frame)
    objp = np.zeros((pattern_size[0] * pattern_size[1], 3), np.float32)
    objp[:, :2] = np.mgrid[0:pattern_size[0], 0:pattern_size[1]].T.reshape(-1, 2)
    # Kalikan variabel dengan nilai
    objp *= square_size  # Skala ke ukuran real
    
    # Arrays untuk menyimpan points dari semua gambar
    objpoints = []  # 3D points in world
    imgpoints = []  # 2D points in image
    
    # Proses setiap gambar
    successful_images = []
    
    for i, img in enumerate(images):
        found, corners = deteksi_checkerboard(img, pattern_size)
        
        if found:
            objpoints.append(objp)
            imgpoints.append(corners)
            successful_images.append(i)
            # Cetak informasi ke console
            print(f"  [✓] Gambar {i+1}: Checkerboard terdeteksi")
        else:
            # Cetak informasi ke console
            print(f"  [✗] Gambar {i+1}: Checkerboard tidak terdeteksi")
    
    if len(objpoints) < 3:
        # Cetak informasi ke console
        print("[ERROR] Minimal 3 gambar dengan checkerboard terdeteksi diperlukan!")
        # Kembalikan hasil dari fungsi
        return None, None, None, None, None
    
    # Cetak informasi ke console
    print(f"\n[INFO] Menggunakan {len(objpoints)} gambar untuk kalibrasi...")
    
    # Kalibrasi
    h, w = images[0].shape[:2]
    ret, camera_matrix, dist_coeffs, rvecs, tvecs = cv2.calibrateCamera(
        objpoints, imgpoints, (w, h), None, None
    )
    
    # Kembalikan hasil dari fungsi
    return ret, camera_matrix, dist_coeffs, rvecs, tvecs


# Keterangan: Mengoreksi distorsi lensa pada gambar.
def undistort_gambar(gambar, camera_matrix, dist_coeffs):
    """Menghilangkan distorsi dari gambar.

    Parameter:
    - gambar (np.ndarray): input image (BGR).
    - camera_matrix (np.ndarray): matriks intrinsik.
    - dist_coeffs (np.ndarray): koefisien distorsi.

    Return:
    - np.ndarray: hasil undistort (direct).
    - np.ndarray: hasil undistort (remap).
    - np.ndarray: hasil undistort (cropped).
    """
    h, w = gambar.shape[:2]
    
    # Metode 1: Direct undistort
    hasil_direct = cv2.undistort(gambar, camera_matrix, dist_coeffs)
    
    # Metode 2: Dengan remapping (lebih fleksibel)
    new_camera_matrix, roi = cv2.getOptimalNewCameraMatrix(
        camera_matrix, dist_coeffs, (w, h), 1, (w, h)
    )
    
    # Hitung undistortion maps
    mapx, mapy = cv2.initUndistortRectifyMap(
        camera_matrix, dist_coeffs, None, new_camera_matrix, (w, h), cv2.CV_32FC1
    )
    
    # Apply remapping
    hasil_remap = cv2.remap(gambar, mapx, mapy, cv2.INTER_LINEAR)
    
    # Crop ROI
    x, y, w_roi, h_roi = roi
    hasil_cropped = hasil_remap[y:y+h_roi, x:x+w_roi]
    
    # Kembalikan hasil dari fungsi
    return hasil_direct, hasil_remap, hasil_cropped


# Keterangan: Menghitung error reprojection untuk evaluasi kalibrasi.
def hitung_reprojection_error(objpoints, imgpoints, rvecs, tvecs,
                              camera_matrix, dist_coeffs):
    """Menghitung re-projection error untuk menilai kualitas kalibrasi.

    Parameter:
    - objpoints (list[np.ndarray]): titik 3D dunia.
    - imgpoints (list[np.ndarray]): titik 2D gambar.
    - rvecs (list[np.ndarray]): rotation vectors.
    - tvecs (list[np.ndarray]): translation vectors.
    - camera_matrix (np.ndarray): matriks intrinsik.
    - dist_coeffs (np.ndarray): koefisien distorsi.

    Return:
    - float: mean reprojection error.
    """
    total_error = 0
    total_points = 0
    
    # Iterasi melalui range untuk pemrosesan
    for i in range(len(objpoints)):
        imgpoints2, _ = cv2.projectPoints(
            objpoints[i], rvecs[i], tvecs[i], camera_matrix, dist_coeffs
        )
        error = cv2.norm(imgpoints[i], imgpoints2, cv2.NORM_L2) / len(imgpoints2)
        # Tambahkan nilai ke variabel
        total_error += error
        # Tambahkan nilai ke variabel
        total_points += 1
    
    mean_error = total_error / total_points
    # Kembalikan hasil dari fungsi
    return mean_error


# Keterangan: Memvisualisasikan efek distorsi lensa.
def visualisasi_distorsi():
    """Memvisualisasikan berbagai jenis distorsi lensa."""
    # Cetak informasi ke console
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("VISUALISASI DISTORSI LENSA")
    # Cetak informasi ke console
    print("=" * 60)
    
    # Buat grid pattern
    size = 400
    grid = np.ones((size, size, 3), dtype=np.uint8) * 255
    
    # Gambar grid
    spacing = 40
    # Iterasi melalui range untuk pemrosesan
    for i in range(0, size, spacing):
        # Gambar garis pada gambar
        cv2.line(grid, (i, 0), (i, size-1), (0, 0, 0), 1)
        # Gambar garis pada gambar
        cv2.line(grid, (0, i), (size-1, i), (0, 0, 0), 1)
    
    # Berbagai jenis distorsi
    fig, axes = plt.subplots(2, 3, figsize=(15, 10))
    
    distortion_types = [
        ((0, 0, 0, 0, 0), "No Distortion\n(Ideal)"),
        ((0.3, 0, 0, 0, 0), "Barrel Distortion\n(k1 > 0)"),
        ((-0.3, 0, 0, 0, 0), "Pincushion Distortion\n(k1 < 0)"),
        ((0.2, 0.1, 0, 0, 0), "Complex Radial\n(k1, k2 > 0)"),
        ((0, 0, 0.01, 0, 0), "Tangential (p1)"),
        ((0, 0, 0, 0.01, 0), "Tangential (p2)"),
    ]
    
    # Camera matrix sederhana
    K = np.array([
        [size, 0, size/2],
        [0, size, size/2],
        [0, 0, 1]
    ], dtype=np.float32)
    
    axes_flat = axes.flatten()
    
    for ax, (dist, title) in zip(axes_flat, distortion_types):
        # Buat array numpy dari data
        dist_coeffs = np.array(dist, dtype=np.float32)
        
        # Buat distorted image dengan menerapkan inverse undistort
        # (simulasi efek distorsi)
        mapx, mapy = cv2.initUndistortRectifyMap(
            K, -dist_coeffs, None, K, (size, size), cv2.CV_32FC1
        )
        distorted = cv2.remap(grid, mapx, mapy, cv2.INTER_LINEAR)
        
        ax.imshow(cv2.cvtColor(distorted, cv2.COLOR_BGR2RGB))
        ax.set_title(title)
        ax.axis('off')
    
    # Set judul keseluruhan figure
    plt.suptitle("Jenis-jenis Distorsi Lensa", fontsize=14)
    # Atur spacing antar subplot
    plt.tight_layout()
    output_path = os.path.join(DIR_OUTPUT, "output.png")

    # Simpan figure ke file dengan kualitas DPI tertentu
    plt.savefig(output_path, dpi=100, bbox_inches="tight")

    # Cetak informasi ke console
    print(f"[SAVED] {output_path}")

    # Tutup figure untuk menghemat memory
    plt.close()


# Keterangan: Menjalankan pipeline demo kalibrasi kamera.
def demo_calibration_pipeline():
    """Demo pipeline kalibrasi lengkap dengan gambar simulasi.

    Return:
    - np.ndarray | None: camera matrix.
    - np.ndarray | None: dist coeffs.
    """
    # Cetak informasi ke console
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("DEMO PIPELINE KALIBRASI KAMERA")
    # Cetak informasi ke console
    print("=" * 60)
    
    # Buat pattern checkerboard
    pattern = buat_checkerboard_pattern(
        rows=CHECKERBOARD_ROWS, 
        cols=CHECKERBOARD_COLS, 
        square_size=50
    )
    
    # Cetak informasi ke console
    print(f"[INFO] Pola checkerboard: {CHECKERBOARD_COLS}x{CHECKERBOARD_ROWS}")
    # Cetak informasi ke console
    print(f"[INFO] Inner corners: {CHECKERBOARD_COLS-1}x{CHECKERBOARD_ROWS-1}")
    
    # Tampilkan pattern
    plt.figure(figsize=(8, 6))
    # Tampilkan gambar pada plot
    plt.imshow(pattern, cmap='gray')
    plt.title(f"Pola Kalibrasi Checkerboard\n{CHECKERBOARD_COLS}×{CHECKERBOARD_ROWS} kotak")
    plt.axis('off')
    output_path = os.path.join(DIR_OUTPUT, "output.png")

    # Simpan figure ke file dengan kualitas DPI tertentu
    plt.savefig(output_path, dpi=100, bbox_inches="tight")

    # Cetak informasi ke console
    print(f"[SAVED] {output_path}")

    # Tutup figure untuk menghemat memory
    plt.close()
    
    # Buat gambar simulasi kalibrasi
    print("\n[INFO] Membuat gambar simulasi kalibrasi...")
    images = simulasi_gambar_kalibrasi(pattern, num_images=12)
    
    # Tampilkan beberapa gambar simulasi
    fig, axes = plt.subplots(3, 4, figsize=(16, 12))
    pattern_size = (CHECKERBOARD_COLS - 1, CHECKERBOARD_ROWS - 1)
    
    for i, (ax, img) in enumerate(zip(axes.flatten(), images)):
        # Buat salinan dari array/gambar
        img_display = img.copy()
        
        # Deteksi dan gambar corners
        found, corners = deteksi_checkerboard(img, pattern_size)
        if found:
            cv2.drawChessboardCorners(img_display, pattern_size, corners, found)
        
        ax.imshow(cv2.cvtColor(img_display, cv2.COLOR_BGR2RGB))
        status = "✓" if found else "✗"
        ax.set_title(f"Image {i+1} [{status}]")
        ax.axis('off')
    
    # Set judul keseluruhan figure
    plt.suptitle("Gambar Kalibrasi dengan Detected Corners", fontsize=14)
    # Atur spacing antar subplot
    plt.tight_layout()
    output_path = os.path.join(DIR_OUTPUT, "output.png")

    # Simpan figure ke file dengan kualitas DPI tertentu
    plt.savefig(output_path, dpi=100, bbox_inches="tight")

    # Cetak informasi ke console
    print(f"[SAVED] {output_path}")

    # Tutup figure untuk menghemat memory
    plt.close()
    
    # Lakukan kalibrasi
    ret, camera_matrix, dist_coeffs, rvecs, tvecs = kalibrasi_kamera(
        images, pattern_size, SQUARE_SIZE
    )
    
    if ret is not None:
        # Cetak informasi ke console
        print("\n" + "=" * 60)
        # Cetak informasi ke console
        print("HASIL KALIBRASI")
        # Cetak informasi ke console
        print("=" * 60)
        
        # Cetak informasi ke console
        print(f"\nRMS Re-projection Error: {ret:.4f} pixels")
        # Cetak informasi ke console
        print("(Nilai < 1.0 pixel dianggap baik)")
        
        # Cetak informasi ke console
        print("\nCamera Matrix (Intrinsic):")
        # Cetak informasi ke console
        print(camera_matrix)
        
        # Cetak informasi ke console
        print(f"\nFocal Length: fx={camera_matrix[0,0]:.2f}, fy={camera_matrix[1,1]:.2f}")
        # Cetak informasi ke console
        print(f"Principal Point: cx={camera_matrix[0,2]:.2f}, cy={camera_matrix[1,2]:.2f}")
        
        # Cetak informasi ke console
        print("\nDistortion Coefficients (k1, k2, p1, p2, k3):")
        # Cetak informasi ke console
        print(dist_coeffs.ravel())
        
        # Demo undistortion
        print("\n[INFO] Demonstrasi undistortion...")
        test_img = images[0]
        direct, remap, cropped = undistort_gambar(test_img, camera_matrix, dist_coeffs)
        
        # Siapkan kanvas plot untuk menampilkan hasil
        fig, axes = plt.subplots(1, 3, figsize=(15, 5))
        
        # Tampilkan gambar pada subplot tertentu
        axes[0].imshow(cv2.cvtColor(test_img, cv2.COLOR_BGR2RGB))
        # Set judul untuk subplot
        axes[0].set_title("Original (with distortion)")
        # Nonaktifkan atau atur axis pada subplot
        axes[0].axis('off')
        
        # Tampilkan gambar pada subplot tertentu
        axes[1].imshow(cv2.cvtColor(remap, cv2.COLOR_BGR2RGB))
        # Set judul untuk subplot
        axes[1].set_title("Undistorted (full)")
        # Nonaktifkan atau atur axis pada subplot
        axes[1].axis('off')
        
        # Tampilkan gambar pada subplot tertentu
        axes[2].imshow(cv2.cvtColor(cropped, cv2.COLOR_BGR2RGB))
        # Set judul untuk subplot
        axes[2].set_title("Undistorted (cropped)")
        # Nonaktifkan atau atur axis pada subplot
        axes[2].axis('off')
        
        # Set judul keseluruhan figure
        plt.suptitle("Hasil Undistortion", fontsize=14)
        # Atur spacing antar subplot
        plt.tight_layout()
        output_path = os.path.join(DIR_OUTPUT, "output.png")

        # Simpan figure ke file dengan kualitas DPI tertentu
        plt.savefig(output_path, dpi=100, bbox_inches="tight")

        # Cetak informasi ke console
        print(f"[SAVED] {output_path}")

        # Tutup figure untuk menghemat memory
        plt.close()
        
        # Kembalikan hasil dari fungsi
        return camera_matrix, dist_coeffs
    
    # Kembalikan hasil dari fungsi
    return None, None


# Keterangan: Menyimpan parameter kalibrasi ke file.
def simpan_parameter_kalibrasi(camera_matrix, dist_coeffs, filename):
    """Menyimpan parameter kalibrasi ke file.

    Parameter:
    - camera_matrix (np.ndarray): matriks intrinsik.
    - dist_coeffs (np.ndarray): koefisien distorsi.
    - filename (str): path tanpa ekstensi.
    """
    # Metode 1: NumPy
    np.savez(filename + '.npz', 
             camera_matrix=camera_matrix, 
             dist_coeffs=dist_coeffs)
    
    # Metode 2: OpenCV FileStorage (XML/YAML)
    fs = cv2.FileStorage(filename + '.yaml', cv2.FILE_STORAGE_WRITE)
    fs.write('camera_matrix', camera_matrix)
    fs.write('dist_coeffs', dist_coeffs)
    fs.release()
    
    # Cetak informasi ke console
    print(f"[INFO] Parameter disimpan ke {filename}.npz dan {filename}.yaml")


# Keterangan: Memuat parameter kalibrasi dari file.
def muat_parameter_kalibrasi(filename):
    """Memuat parameter kalibrasi dari file.

    Parameter:
    - filename (str): path tanpa ekstensi.

    Return:
    - np.ndarray | None: camera matrix.
    - np.ndarray | None: dist coeffs.
    """
    if os.path.exists(filename + '.npz'):
        data = np.load(filename + '.npz')
        # Kembalikan hasil dari fungsi
        return data['camera_matrix'], data['dist_coeffs']
    elif os.path.exists(filename + '.yaml'):
        fs = cv2.FileStorage(filename + '.yaml', cv2.FILE_STORAGE_READ)
        camera_matrix = fs.getNode('camera_matrix').mat()
        dist_coeffs = fs.getNode('dist_coeffs').mat()
        fs.release()
        # Kembalikan hasil dari fungsi
        return camera_matrix, dist_coeffs
    else:
        # Cetak informasi ke console
        print(f"[ERROR] File {filename} tidak ditemukan!")
        # Kembalikan hasil dari fungsi
        return None, None


# ============================================================
# PROGRAM UTAMA
# ============================================================

# Keterangan: Menjalankan demo kalibrasi kamera dan penyimpanan parameter.
def main():
    """Fungsi utama program.

    Menjalankan demo kalibrasi kamera dan menyimpan parameter.
    """
    
    # Cetak informasi ke console
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("PRAKTIKUM: KALIBRASI KAMERA")
    # Cetak informasi ke console
    print("Bab 2 - Pembentukan Citra")
    # Cetak informasi ke console
    print("=" * 60)
    
    # Cetak informasi ke console
    print("""
KALIBRASI KAMERA bertujuan untuk:
├── Mengetahui parameter intrinsik (focal length, principal point)
├── Menghitung koefisien distorsi lensa
├── Mendapatkan parameter ekstrinsik (posisi & orientasi)
└── Menghasilkan gambar yang sudah dikoreksi distorsinya

MENGAPA KALIBRASI PENTING?
├── 3D reconstruction membutuhkan parameter yang akurat
├── AR/VR memerlukan proyeksi yang tepat
├── Robot vision untuk navigasi dan manipulasi
└── Metrologi untuk pengukuran akurat

LANGKAH-LANGKAH KALIBRASI:
1. Siapkan pola kalibrasi (checkerboard/circles grid)
2. Ambil 10-20 gambar dari berbagai sudut
3. Deteksi corner/feature points
4. Hitung parameter menggunakan cv2.calibrateCamera()
5. Evaluasi dengan re-projection error
6. Simpan parameter untuk penggunaan selanjutnya
    """)
    
    # 1. Penjelasan parameter kamera
    penjelasan_parameter_kamera()
    
    # 2. Visualisasi distorsi
    visualisasi_distorsi()
    
    # 3. Demo pipeline kalibrasi
    camera_matrix, dist_coeffs = demo_calibration_pipeline()
    
    # 4. Simpan parameter jika berhasil
    if camera_matrix is not None:
        output_dir = os.path.dirname(os.path.abspath(__file__))
        simpan_parameter_kalibrasi(
            camera_matrix, dist_coeffs, 
            os.path.join(output_dir, "calibration_params")
        )
    
    # Ringkasan
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("RINGKASAN KALIBRASI KAMERA")
    # Cetak informasi ke console
    print("=" * 60)
    # Cetak informasi ke console
    print("""
FUNGSI UTAMA:
├── cv2.findChessboardCorners(img, pattern_size)
├── cv2.cornerSubPix(img, corners, winSize, zeroZone, criteria)
├── cv2.calibrateCamera(objPoints, imgPoints, imageSize, ...)
├── cv2.undistort(img, cameraMatrix, distCoeffs)
└── cv2.getOptimalNewCameraMatrix(...)

TIPS KALIBRASI YANG BAIK:
├── Gunakan minimal 10-20 gambar
├── Variasikan posisi dan orientasi checkerboard
├── Pastikan checkerboard memenuhi sebagian besar frame
├── Ambil gambar dengan pencahayaan yang baik
├── Hindari blur dan motion artifact
└── RMS error < 1.0 pixel = kalibrasi bagus

PENYIMPANAN PARAMETER:
├── NumPy .npz untuk Python
├── OpenCV FileStorage .yaml/.xml untuk cross-platform
└── JSON untuk web applications

APLIKASI:
├── 3D Reconstruction
├── Augmented Reality
├── Robot Vision
├── Stereo Vision
├── Structure from Motion
└── Autonomous Vehicles
""")


# Jalankan program utama
if __name__ == "__main__":
    main()
