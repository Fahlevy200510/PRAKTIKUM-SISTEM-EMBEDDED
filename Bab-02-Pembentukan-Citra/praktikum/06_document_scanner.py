# ============================================================
# PROGRAM: 06_document_scanner.py
# PRAKTIKUM COMPUTER VISION - BAB 2: PEMBENTUKAN CITRA
# ============================================================
# Deskripsi: Program document scanner lengkap dengan deteksi
#            otomatis sudut dokumen menggunakan edge detection
# 
# Tujuan Pembelajaran:
#   1. Mengintegrasikan berbagai teknik CV untuk aplikasi nyata
#   2. Deteksi kontur dan seleksi kontur 4 sisi
#   3. Koreksi perspektif otomatis
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

# ============================================================
# PANDUAN FUNGSI (RINGKAS) - ARTI PARAMETER
# ============================================================
# cv2.cvtColor(img, code)
#   - code : konversi warna (BGR↔GRAY/RGB)
#
# cv2.GaussianBlur(img, ksize, sigmaX)
#   - ksize: ukuran kernel blur (harus ganjil)
#
# cv2.Canny(img, t1, t2)
#   - t1, t2: threshold low/high untuk edge
#
# cv2.findContours(img, mode, method)
#   - mode  : cv2.RETR_* (cara ambil kontur)
#   - method: cv2.CHAIN_* (cara simpan kontur)
#
# cv2.approxPolyDP(contour, epsilon, closed)
#   - epsilon: toleransi aproksimasi
#
# cv2.getPerspectiveTransform(pts_src, pts_dst)
#   - pts_src, pts_dst: 4 titik korespondensi
#
# cv2.warpPerspective(src, M, dsize)
#   - M     : matriks homography 3x3
#   - dsize : (lebar, tinggi) output
#
# cv2.adaptiveThreshold(src, maxValue, adaptiveMethod, thresholdType, blockSize, C)
#   - blockSize: ukuran blok (ganjil)
#   - C       : konstanta pengurang
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
DIR_OUTPUT = os.path.join(DIR_SCRIPT, "output", "output6")

# Pastikan folder output ada
os.makedirs(DIR_OUTPUT, exist_ok=True)


# 1. File gambar yang akan diproses
NAMA_FILE_GAMBAR = "portrait.jpg"

# 2. Ukuran resize untuk pemrosesan (lebih kecil = lebih cepat)
RESIZE_WIDTH = 500

# 3. Parameter Canny edge detection
CANNY_THRESHOLD1 = 50
CANNY_THRESHOLD2 = 200

# 4. Parameter Gaussian blur
BLUR_KERNEL_SIZE = 5

# 5. Ukuran output dokumen (inch × DPI)
# Contoh: A4 = 8.27" x 11.69" pada 100 DPI
OUTPUT_WIDTH = 827
OUTPUT_HEIGHT = 1169

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


# Keterangan: Membuat gambar simulasi dokumen untuk demo.
def buat_gambar_dokumen():
    """Membuat gambar simulasi dokumen untuk demonstrasi.

    Return:
    - np.ndarray: gambar buatan berformat BGR.
    - np.ndarray: 4 titik sudut dokumen.
    """
    # Background meja kayu
    gambar = np.zeros((700, 900, 3), dtype=np.uint8)
    
    # Tekstur meja
    for i in range(700):
        # Iterasi melalui range
        for j in range(900):
            noise = np.random.randint(-10, 10)
            gambar[i, j] = [80 + noise, 60 + noise, 45 + noise]
    
    # Blur untuk tekstur natural
    gambar = cv2.GaussianBlur(gambar, (5, 5), 0)
    
    # Dokumen putih dengan perspektif
    # Titik-titik sudut dokumen (dalam perspektif)
    pts_dokumen = np.array([
        [180, 80],    # top-left
        [700, 120],   # top-right
        [750, 580],   # bottom-right
        [130, 620]    # bottom-left
    ], np.int32)
    
    # Isi dokumen dengan warna putih
    cv2.fillPoly(gambar, [pts_dokumen], (250, 250, 250))
    
    # Bayangan dokumen
    shadow_pts = pts_dokumen + np.array([[10, 10]])
    # Inisialisasi array numpy dengan nilai nol
    mask = np.zeros_like(gambar)
    cv2.fillPoly(mask, [shadow_pts.astype(np.int32)], (50, 50, 50))
    gambar = cv2.addWeighted(gambar, 1, mask, 0.3, 0)
    
    # Konten dalam dokumen (simulasi teks)
    # Kita perlu menghitung posisi teks yang mengikuti perspektif
    
    # Header
    header_y = 150
    # Tambahkan teks pada gambar
    cv2.putText(gambar, "SURAT KETERANGAN", (280, header_y), 
                cv2.FONT_HERSHEY_SIMPLEX, 1.0, (30, 30, 30), 2)
    
    # Garis teks
    for i in range(8):
        y_pos = 220 + i * 45
        x_start = 200 + int((y_pos - 80) * 0.05)  # Menyesuaikan perspektif
        x_end = 680 + int((y_pos - 80) * 0.1)
        line_length = np.random.randint(int((x_end - x_start) * 0.6), 
                                        int((x_end - x_start) * 0.95))
        # Gambar garis pada gambar
        cv2.line(gambar, (x_start, y_pos), (x_start + line_length, y_pos), 
                (100, 100, 100), 2)
    
    # Tanda tangan area
    cv2.rectangle(gambar, (500, 500), (700, 570), (150, 150, 150), 2)
    # Tambahkan teks pada gambar
    cv2.putText(gambar, "TTD", (570, 545), 
                cv2.FONT_HERSHEY_SIMPLEX, 0.6, (100, 100, 100), 1)
    
    # Kembalikan hasil dari fungsi
    return gambar, pts_dokumen.astype(np.float32)


# ============================================================
# FUNGSI DOCUMENT SCANNER
# ============================================================

# Keterangan: Mengurutkan 4 titik sudut ke urutan baku.
def order_points(pts):
    """Mengurutkan 4 titik: top-left, top-right, bottom-right, bottom-left.

    Parameter:
    - pts (array-like): 4 titik input (x, y).

    Return:
    - np.ndarray: titik yang sudah terurut.
    """
    # Inisialisasi array numpy dengan nilai nol
    rect = np.zeros((4, 2), dtype=np.float32)
    
    # Top-left: koordinat dengan sum minimum
    # Bottom-right: koordinat dengan sum maksimum
    s = pts.sum(axis=1)
    rect[0] = pts[np.argmin(s)]
    rect[2] = pts[np.argmax(s)]
    
    # Top-right: koordinat dengan diff minimum (y - x kecil)
    # Bottom-left: koordinat dengan diff maksimum (y - x besar)
    diff = np.diff(pts, axis=1)
    rect[1] = pts[np.argmin(diff)]
    rect[3] = pts[np.argmax(diff)]
    
    # Kembalikan hasil dari fungsi
    return rect


# Keterangan: Mendeteksi kontur dokumen dan sudutnya.
def deteksi_dokumen(gambar, show_steps=True):
    """Mendeteksi sudut dokumen dalam gambar.

    Parameter:
    - gambar (np.ndarray): input image (BGR).
    - show_steps (bool): apakah menampilkan langkah-langkah.

    Return:
    - np.ndarray | None: 4 titik sudut dokumen atau None jika gagal.
    """
    # Simpan original untuk skala
    tinggi_orig, lebar_orig = gambar.shape[:2]
    
    # Resize untuk pemrosesan lebih cepat
    rasio = RESIZE_WIDTH / lebar_orig
    # Resize gambar ke ukuran baru
    gambar_kecil = cv2.resize(gambar, (RESIZE_WIDTH, int(tinggi_orig * rasio)))
    
    # Step 1: Konversi ke grayscale
    gray = cv2.cvtColor(gambar_kecil, cv2.COLOR_BGR2GRAY)
    
    # Step 2: Blur untuk mengurangi noise
    blurred = cv2.GaussianBlur(gray, (BLUR_KERNEL_SIZE, BLUR_KERNEL_SIZE), 0)
    
    # Step 3: Edge detection
    edges = cv2.Canny(blurred, CANNY_THRESHOLD1, CANNY_THRESHOLD2)
    
    # Step 4: Dilasi untuk menghubungkan edge yang terputus
    kernel = np.ones((3, 3), np.uint8)
    edges_dilated = cv2.dilate(edges, kernel, iterations=1)
    
    # Step 5: Temukan kontur
    contours, _ = cv2.findContours(edges_dilated, cv2.RETR_LIST, 
                                   cv2.CHAIN_APPROX_SIMPLE)
    
    # Step 6: Urutkan kontur berdasarkan area (terbesar dulu)
    contours = sorted(contours, key=cv2.contourArea, reverse=True)[:5]
    
    # Step 7: Cari kontur dengan 4 titik (dokumen)
    doc_contour = None
    for contour in contours:
        # Aproksimasi kontur
        peri = cv2.arcLength(contour, True)
        approx = cv2.approxPolyDP(contour, 0.02 * peri, True)
        
        # Jika kontur memiliki 4 titik, kemungkinan itu dokumen
        if len(approx) == 4:
            doc_contour = approx
            break
    
    # Tampilkan langkah-langkah jika diminta
    if show_steps:
        # Siapkan kanvas plot untuk menampilkan hasil
        fig, axes = plt.subplots(2, 3, figsize=(15, 10))
        
        # Tampilkan gambar pada subplot tertentu
        axes[0, 0].imshow(cv2.cvtColor(gambar_kecil, cv2.COLOR_BGR2RGB))
        # Set judul untuk subplot
        axes[0, 0].set_title("1. Original (Resized)")
        # Nonaktifkan atau atur axis pada subplot
        axes[0, 0].axis('off')
        
        # Tampilkan gambar pada subplot tertentu
        axes[0, 1].imshow(gray, cmap='gray')
        # Set judul untuk subplot
        axes[0, 1].set_title("2. Grayscale")
        # Nonaktifkan atau atur axis pada subplot
        axes[0, 1].axis('off')
        
        # Tampilkan gambar pada subplot tertentu
        axes[0, 2].imshow(blurred, cmap='gray')
        # Set judul untuk subplot
        axes[0, 2].set_title("3. Gaussian Blur")
        # Nonaktifkan atau atur axis pada subplot
        axes[0, 2].axis('off')
        
        # Tampilkan gambar pada subplot tertentu
        axes[1, 0].imshow(edges, cmap='gray')
        # Set judul untuk subplot
        axes[1, 0].set_title("4. Canny Edges")
        # Nonaktifkan atau atur axis pada subplot
        axes[1, 0].axis('off')
        
        # Tampilkan gambar pada subplot tertentu
        axes[1, 1].imshow(edges_dilated, cmap='gray')
        # Set judul untuk subplot
        axes[1, 1].set_title("5. Dilated Edges")
        # Nonaktifkan atau atur axis pada subplot
        axes[1, 1].axis('off')
        
        # Tampilkan kontur yang ditemukan
        gambar_kontur = gambar_kecil.copy()
        if doc_contour is not None:
            # Gambar kontur pada gambar
            cv2.drawContours(gambar_kontur, [doc_contour], -1, (0, 255, 0), 3)
            for pt in doc_contour.reshape(-1, 2):
                # Gambar lingkaran pada gambar
                cv2.circle(gambar_kontur, tuple(pt), 8, (0, 0, 255), -1)
        
        # Tampilkan gambar pada subplot tertentu
        axes[1, 2].imshow(cv2.cvtColor(gambar_kontur, cv2.COLOR_BGR2RGB))
        # Set judul untuk subplot
        axes[1, 2].set_title("6. Detected Document")
        # Nonaktifkan atau atur axis pada subplot
        axes[1, 2].axis('off')
        
        # Set judul keseluruhan figure
        plt.suptitle("Langkah-langkah Deteksi Dokumen", fontsize=14)
        # Atur spacing antar subplot
        plt.tight_layout()
        output_path = os.path.join(DIR_OUTPUT, "output.png")

        # Simpan figure ke file dengan kualitas DPI tertentu
        plt.savefig(output_path, dpi=100, bbox_inches="tight")

        # Cetak informasi ke console
        print(f"[SAVED] {output_path}")

        # Tutup figure untuk menghemat memory
        plt.close()
    
    # Return corners scaled back ke ukuran original
    if doc_contour is not None:
        corners = doc_contour.reshape(4, 2) / rasio
        # Kembalikan hasil dari fungsi
        return corners.astype(np.float32)
    
    # Kembalikan hasil dari fungsi
    return None


# Keterangan: Meluruskan dokumen berdasarkan 4 sudut terdeteksi.
def koreksi_perspektif(gambar, corners, output_size=None):
    """Melakukan koreksi perspektif pada dokumen.

    Parameter:
    - gambar (np.ndarray): input image (BGR).
    - corners (array-like): 4 titik sudut dokumen.
    - output_size (tuple[int, int] | None): (width, height) atau None.

    Return:
    - np.ndarray: gambar hasil koreksi.
    """
    # Urutkan titik
    rect = order_points(corners)
    (tl, tr, br, bl) = rect
    
    # Hitung dimensi output
    if output_size is None:
        # Gunakan jarak terbesar sebagai dimensi
        lebar_top = np.linalg.norm(tr - tl)
        # Hitung norma/magnitude vektor
        lebar_bottom = np.linalg.norm(br - bl)
        lebar = int(max(lebar_top, lebar_bottom))
        
        # Hitung norma/magnitude vektor
        tinggi_left = np.linalg.norm(bl - tl)
        # Hitung norma/magnitude vektor
        tinggi_right = np.linalg.norm(br - tr)
        tinggi = int(max(tinggi_left, tinggi_right))
    else:
        lebar, tinggi = output_size
    
    # Titik tujuan
    dst = np.array([
        [0, 0],
        [lebar - 1, 0],
        [lebar - 1, tinggi - 1],
        [0, tinggi - 1]
    ], dtype=np.float32)
    
    # Hitung transformation matrix
    M = cv2.getPerspectiveTransform(rect, dst)
    
    # Terapkan transformasi
    hasil = cv2.warpPerspective(gambar, M, (lebar, tinggi))
    
    # Kembalikan hasil dari fungsi
    return hasil


# Keterangan: Meningkatkan kualitas hasil scan dokumen.
def enhance_dokumen(gambar):
    """Meningkatkan kualitas hasil scan.

    Parameter:
    - gambar (np.ndarray): hasil scan (BGR).

    Return:
    - tuple[np.ndarray, np.ndarray, np.ndarray, np.ndarray]:
        (gray, adaptive_threshold, clahe, sharpened).
    """
    # Konversi ke grayscale
    gray = cv2.cvtColor(gambar, cv2.COLOR_BGR2GRAY)
    
    # Adaptive thresholding untuk hasil seperti scan
    # Metode 1: Adaptive threshold
    thresh_adaptive = cv2.adaptiveThreshold(
        gray, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C, cv2.THRESH_BINARY, 11, 2
    )
    
    # Metode 2: CLAHE (Contrast Limited Adaptive Histogram Equalization)
    clahe = cv2.createCLAHE(clipLimit=2.0, tileGridSize=(8, 8))
    enhanced_clahe = clahe.apply(gray)
    
    # Metode 3: Sharpening
    kernel_sharp = np.array([[-1, -1, -1],
                             [-1,  9, -1],
                             [-1, -1, -1]])
    # Terapkan filter konvolusi pada gambar
    sharpened = cv2.filter2D(gray, -1, kernel_sharp)
    
    # Kembalikan hasil dari fungsi
    return gray, thresh_adaptive, enhanced_clahe, sharpened


# Keterangan: Pipeline lengkap deteksi + koreksi + enhancement.
def scan_dokumen_lengkap(gambar, output_size=None):
    """Pipeline lengkap document scanner.

    Parameter:
    - gambar (np.ndarray): input image (BGR).
    - output_size (tuple[int, int] | None): ukuran output (width, height).

    Return:
    - np.ndarray | None: hasil scan.
    - np.ndarray | None: corners yang terdeteksi.
    """
    # Cetak informasi ke console
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("DOCUMENT SCANNER - PIPELINE LENGKAP")
    # Cetak informasi ke console
    print("=" * 60)
    
    # Step 1: Deteksi dokumen
    print("[Step 1] Mendeteksi sudut dokumen...")
    corners = deteksi_dokumen(gambar, show_steps=True)
    
    # Cek apakah variabel kosong/None
    if corners is None:
        # Cetak informasi ke console
        print("[ERROR] Tidak dapat mendeteksi dokumen!")
        # Kembalikan hasil dari fungsi
        return None, None
    
    # Cetak informasi ke console
    print(f"[INFO] Sudut terdeteksi: {corners.tolist()}")
    
    # Step 2: Koreksi perspektif
    print("\n[Step 2] Melakukan koreksi perspektif...")
    # Cek apakah variabel kosong/None
    if output_size is None:
        output_size = (OUTPUT_WIDTH, OUTPUT_HEIGHT)
    scanned = koreksi_perspektif(gambar, corners, output_size)
    # Cetak informasi ke console
    print(f"[INFO] Ukuran output: {scanned.shape}")
    
    # Step 3: Enhancement
    print("\n[Step 3] Meningkatkan kualitas hasil scan...")
    gray, thresh, clahe, sharp = enhance_dokumen(scanned)
    
    # Tampilkan hasil enhancement
    fig, axes = plt.subplots(2, 3, figsize=(15, 10))
    
    # Tampilkan gambar pada subplot tertentu
    axes[0, 0].imshow(cv2.cvtColor(gambar, cv2.COLOR_BGR2RGB))
    # Set judul untuk subplot
    axes[0, 0].set_title("Original")
    # Nonaktifkan atau atur axis pada subplot
    axes[0, 0].axis('off')
    
    # Tampilkan gambar pada subplot tertentu
    axes[0, 1].imshow(cv2.cvtColor(scanned, cv2.COLOR_BGR2RGB))
    # Set judul untuk subplot
    axes[0, 1].set_title("Perspective Corrected")
    # Nonaktifkan atau atur axis pada subplot
    axes[0, 1].axis('off')
    
    # Tampilkan gambar pada subplot tertentu
    axes[0, 2].imshow(gray, cmap='gray')
    # Set judul untuk subplot
    axes[0, 2].set_title("Grayscale")
    # Nonaktifkan atau atur axis pada subplot
    axes[0, 2].axis('off')
    
    # Tampilkan gambar pada subplot tertentu
    axes[1, 0].imshow(thresh, cmap='gray')
    axes[1, 0].set_title("Adaptive Threshold\n(seperti hasil scan)")
    # Nonaktifkan atau atur axis pada subplot
    axes[1, 0].axis('off')
    
    # Tampilkan gambar pada subplot tertentu
    axes[1, 1].imshow(clahe, cmap='gray')
    # Set judul untuk subplot
    axes[1, 1].set_title("CLAHE Enhanced")
    # Nonaktifkan atau atur axis pada subplot
    axes[1, 1].axis('off')
    
    # Tampilkan gambar pada subplot tertentu
    axes[1, 2].imshow(sharp, cmap='gray')
    # Set judul untuk subplot
    axes[1, 2].set_title("Sharpened")
    # Nonaktifkan atau atur axis pada subplot
    axes[1, 2].axis('off')
    
    # Set judul keseluruhan figure
    plt.suptitle("Document Scanner - Hasil Enhancement", fontsize=14)
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
    return scanned, corners


# Keterangan: Menjelaskan cara pemilihan titik manual (GUI).
def demo_manual_selection():
    """Menjelaskan pemilihan titik manual dengan GUI interaktif."""
    # Cetak informasi ke console
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("DEMO PEMILIHAN TITIK MANUAL")
    # Cetak informasi ke console
    print("=" * 60)
    
    # Cetak informasi ke console
    print("""
Dalam aplikasi nyata, sering diperlukan pemilihan titik manual
ketika deteksi otomatis gagal.

Berikut contoh implementasi dengan mouse callback:
    """)
    
    code_example = '''
# Contoh implementasi pemilihan titik dengan mouse
points = []

def mouse_callback(event, x, y, flags, param):
    global points
    if event == cv2.EVENT_LBUTTONDOWN:
        if len(points) < 4:
            points.append((x, y))
            # Cetak informasi ke console
            print(f"Point {len(points)}: ({x}, {y})")

# Setup window
cv2.namedWindow("Select 4 Corners")
cv2.setMouseCallback("Select 4 Corners", mouse_callback)

# Loop while untuk proses berulang
while len(points) < 4:
    # Buat salinan dari array/gambar
    temp_img = gambar.copy()
    for pt in points:
        # Gambar lingkaran pada gambar
        cv2.circle(temp_img, pt, 5, (0, 255, 0), -1)
    cv2.imshow("Select 4 Corners", temp_img)
    if cv2.waitKey(1) == 27:  # ESC
        break

cv2.destroyAllWindows()
'''
    # Cetak informasi ke console
    print(code_example)


# Keterangan: Menjelaskan pemrosesan batch dokumen.
def demo_batch_processing():
    """Menjelaskan pemrosesan batch dokumen."""
    # Cetak informasi ke console
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("DEMO BATCH PROCESSING")
    # Cetak informasi ke console
    print("=" * 60)
    
    # Cetak informasi ke console
    print("""
Untuk memproses banyak dokumen sekaligus:

def process_batch(input_folder, output_folder):
    import glob
    
    images = glob.glob(os.path.join(input_folder, "*.jpg"))
    
    for i, img_path in enumerate(images):
        # Cetak informasi ke console
        print(f"Processing {i+1}/{len(images)}: {img_path}")
        
        # Baca gambar
        img = cv2.imread(img_path)
        
        # Deteksi dan koreksi
        corners = deteksi_dokumen(img, show_steps=False)
        if corners is not None:
            scanned = koreksi_perspektif(img, corners)
            
            # Simpan hasil
            filename = os.path.basename(img_path)
            output_path = os.path.join(output_folder, f"scanned_{filename}")
            cv2.imwrite(output_path, scanned)
            # Cetak informasi ke console
            print(f"  Saved: {output_path}")
        else:
            # Cetak informasi ke console
            print(f"  Warning: Could not detect document")
    """)


# ============================================================
# PROGRAM UTAMA
# ============================================================

# Keterangan: Menjalankan demo document scanner lengkap.
def main():
    """Fungsi utama program.

    Menjalankan pipeline document scanner dan menyimpan hasil.
    """
    
    # Cetak informasi ke console
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("PRAKTIKUM: DOCUMENT SCANNER")
    # Cetak informasi ke console
    print("Bab 2 - Pembentukan Citra")
    # Cetak informasi ke console
    print("=" * 60)
    
    # Cetak informasi ke console
    print("""
DOCUMENT SCANNER menggabungkan beberapa teknik:
├── Edge Detection (Canny)
├── Contour Detection
├── Polygon Approximation
├── Perspective Transform
└── Image Enhancement

Pipeline:
  Input → Grayscale → Blur → Edge → Contours → 
  4-Point Detection → Perspective Correction → Enhancement → Output
    """)
    
    # Buat atau muat gambar
    path_gambar = dapatkan_path_gambar(NAMA_FILE_GAMBAR)
    
    if os.path.exists(path_gambar):
        # Cetak informasi ke console
        print(f"[INFO] Memuat gambar: {path_gambar}")
        # Baca gambar dari file
        gambar = cv2.imread(path_gambar)
    else:
        # Cetak informasi ke console
        print("[INFO] Membuat gambar simulasi dokumen...")
        gambar, pts_true = buat_gambar_dokumen()
        # Cetak informasi ke console
        print(f"[INFO] Ground truth corners: {pts_true.tolist()}")
    
    # Cetak informasi ke console
    print(f"[INFO] Ukuran gambar: {gambar.shape}")
    
    # Tampilkan gambar asli
    plt.figure(figsize=(10, 8))
    # Tampilkan gambar pada plot
    plt.imshow(cv2.cvtColor(gambar, cv2.COLOR_BGR2RGB))
    # Set judul pada subplot
    plt.title("Input Image")
    plt.axis('off')
    output_path = os.path.join(DIR_OUTPUT, "output.png")

    # Simpan figure ke file dengan kualitas DPI tertentu
    plt.savefig(output_path, dpi=100, bbox_inches="tight")

    # Cetak informasi ke console
    print(f"[SAVED] {output_path}")

    # Tutup figure untuk menghemat memory
    plt.close()
    
    # Jalankan document scanner
    hasil, corners = scan_dokumen_lengkap(gambar)
    
    if hasil is not None:
        # Demo pemilihan manual
        demo_manual_selection()
        
        # Demo batch processing
        demo_batch_processing()
        
        # Simpan hasil
        output_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 
                                   "hasil_scan.jpg")
        cv2.imwrite(output_path, hasil)
        # Cetak informasi ke console
        print(f"\n[INFO] Hasil disimpan: {output_path}")
    
    # Ringkasan
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("RINGKASAN DOCUMENT SCANNER")
    # Cetak informasi ke console
    print("=" * 60)
    # Cetak informasi ke console
    print("""
FUNGSI-FUNGSI YANG DIGUNAKAN:
├── cv2.Canny(img, t1, t2)           : Edge detection
├── cv2.findContours(...)            : Menemukan kontur
├── cv2.approxPolyDP(contour, eps)   : Aproksimasi polygon
├── cv2.getPerspectiveTransform(...) : Hitung matrix
├── cv2.warpPerspective(...)         : Terapkan transformasi
├── cv2.adaptiveThreshold(...)       : Enhancement
└── cv2.createCLAHE(...)             : Contrast enhancement

PARAMETER PENTING:
├── Canny thresholds: mengontrol sensitivitas edge
├── approxPolyDP epsilon: toleransi aproksimasi (0.02 * perimeter)
├── Gaussian blur kernel: mengurangi noise

TIPS DETEKSI YANG BAIK:
├── Background kontras dengan dokumen
├── Pencahayaan merata tanpa bayangan
├── Dokumen tidak terlalu miring
└── Edge dokumen terlihat jelas

APLIKASI NYATA:
├── CamScanner, Adobe Scan
├── Sistem OCR preprocessing
├── Digitalisasi arsip
└── KTP/SIM scanner
""")


# Jalankan program utama
if __name__ == "__main__":
    main()
