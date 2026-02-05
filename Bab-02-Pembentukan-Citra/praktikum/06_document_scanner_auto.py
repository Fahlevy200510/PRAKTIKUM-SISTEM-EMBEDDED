# ============================================================
# PROGRAM: 06_document_scanner_auto.py (AUTOMATED VERSION)
# PRAKTIKUM COMPUTER VISION - BAB 2: PEMBENTUKAN CITRA
# ============================================================
# Deskripsi: Program document scanner otomatis dengan deteksi
#            sudut dokumen menggunakan edge detection & contour
# 
# Penerapan Nyata: Aplikasi mobile scanner (CamScanner, Adobe Scan)
#                  yang otomatis mendeteksi tepi dokumen
# ============================================================

import cv2
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import os

# ============================================================
# KONFIGURASI
# ============================================================

DIR_SCRIPT = os.path.dirname(os.path.abspath(__file__))
DIR_OUTPUT = os.path.join(DIR_SCRIPT, "output", "output6")
os.makedirs(DIR_OUTPUT, exist_ok=True)

NAMA_FILE_GAMBAR = "document.jpg"
RESIZE_WIDTH = 500
OUTPUT_WIDTH = 600
OUTPUT_HEIGHT = 800

def dapatkan_path_gambar(nama_file):
    """Mendapatkan path lengkap file gambar."""
    path_data = os.path.join(DIR_SCRIPT, "data", "images", nama_file)
    if os.path.exists(path_data):
        return path_data
    
    path_data = os.path.join(DIR_SCRIPT, "..", "..", 
                             "Bab-01-Pendahuluan", "data", "images", nama_file)
    if os.path.exists(path_data):
        return path_data
    
    path_data = os.path.join(DIR_SCRIPT, nama_file)
    if os.path.exists(path_data):
        return path_data
    
    return None

def order_points(pts):
    """Urutkan 4 titik: TL, TR, BR, BL."""
    rect = np.zeros((4, 2), dtype=np.float32)
    
    s = pts.sum(axis=1)
    rect[0] = pts[np.argmin(s)]  # Top-left (sum terkecil)
    rect[2] = pts[np.argmax(s)]  # Bottom-right (sum terbesar)
    
    diff = np.diff(pts, axis=1)
    rect[1] = pts[np.argmin(diff)]  # Top-right (diff terkecil)
    rect[3] = pts[np.argmax(diff)]  # Bottom-left (diff terbesar)
    
    return rect

def find_document_contour(image):
    """Deteksi kontur dokumen secara otomatis."""
    # Resize untuk processing lebih cepat
    h, w = image.shape[:2]
    ratio = h / RESIZE_WIDTH
    resized = cv2.resize(image, (RESIZE_WIDTH, int(w / ratio)))
    
    # Konversi ke grayscale
    gray = cv2.cvtColor(resized, cv2.COLOR_BGR2GRAY)
    
    # Blur untuk mengurangi noise
    blurred = cv2.GaussianBlur(gray, (5, 5), 0)
    
    # Edge detection
    edged = cv2.Canny(blurred, 50, 200)
    
    # Dilasi untuk menutup gap
    kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (5, 5))
    dilated = cv2.dilate(edged, kernel, iterations=1)
    
    # Find contours
    contours, _ = cv2.findContours(dilated, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    
    # Sort by area, ambil yang terbesar
    contours = sorted(contours, key=cv2.contourArea, reverse=True)[:5]
    
    doc_contour = None
    
    for contour in contours:
        # Approximate polygon
        peri = cv2.arcLength(contour, True)
        approx = cv2.approxPolyDP(contour, 0.02 * peri, True)
        
        # Jika 4 sisi, kemungkinan dokumen
        if len(approx) == 4:
            doc_contour = approx
            break
    
    # Scale back to original size
    if doc_contour is not None:
        doc_contour = doc_contour.reshape(4, 2) * ratio
    
    return doc_contour, edged, dilated

def apply_perspective_transform(image, pts):
    """Apply perspective transform."""
    rect = order_points(pts)
    
    # Hitung dimensi output berdasarkan jarak titik
    (tl, tr, br, bl) = rect
    
    widthA = np.sqrt(((br[0] - bl[0]) ** 2) + ((br[1] - bl[1]) ** 2))
    widthB = np.sqrt(((tr[0] - tl[0]) ** 2) + ((tr[1] - tl[1]) ** 2))
    maxWidth = max(int(widthA), int(widthB))
    
    heightA = np.sqrt(((tr[0] - br[0]) ** 2) + ((tr[1] - br[1]) ** 2))
    heightB = np.sqrt(((tl[0] - bl[0]) ** 2) + ((tl[1] - bl[1]) ** 2))
    maxHeight = max(int(heightA), int(heightB))
    
    # Gunakan ukuran default jika terlalu kecil
    if maxWidth < 200 or maxHeight < 200:
        maxWidth = OUTPUT_WIDTH
        maxHeight = OUTPUT_HEIGHT
    
    # Titik tujuan
    dst = np.array([
        [0, 0],
        [maxWidth - 1, 0],
        [maxWidth - 1, maxHeight - 1],
        [0, maxHeight - 1]
    ], dtype=np.float32)
    
    # Hitung matrix dan warp
    M = cv2.getPerspectiveTransform(rect, dst)
    warped = cv2.warpPerspective(image, M, (maxWidth, maxHeight))
    
    return warped, M, rect

def main():
    """Fungsi utama program."""
    print("\n" + "=" * 60)
    print("DOCUMENT SCANNER - AUTOMATED DETECTION")
    print("=" * 60)
    
    # Load gambar
    path_gambar = dapatkan_path_gambar(NAMA_FILE_GAMBAR)
    
    if path_gambar is None or not os.path.exists(path_gambar):
        print(f"\n❌ Error: File '{NAMA_FILE_GAMBAR}' tidak ditemukan!")
        return
    
    image = cv2.imread(path_gambar)
    
    if image is None:
        print(f"\n❌ Error: Gagal membaca gambar dari {path_gambar}")
        return
    
    h, w = image.shape[:2]
    print(f"\n✓ Gambar berhasil dimuat: {path_gambar}")
    print(f"  Ukuran: {w} x {h}")
    
    # Deteksi kontur dokumen
    print("\n🔍 Mendeteksi tepi dokumen...")
    doc_contour, edged, dilated = find_document_contour(image)
    
    if doc_contour is None:
        print("⚠ Tidak dapat mendeteksi dokumen, menggunakan koordinat default...")
        # Gunakan koordinat default (hampir seluruh gambar dengan margin)
        margin = 30
        doc_contour = np.array([
            [margin, margin],
            [w - margin, margin],
            [w - margin, h - margin],
            [margin, h - margin]
        ], dtype=np.float32)
    else:
        print("✓ Dokumen terdeteksi!")
    
    # Apply perspective transform
    print("\n📐 Menerapkan transformasi perspektif...")
    scanned, M, rect = apply_perspective_transform(image, doc_contour)
    
    # Post-processing: binarization untuk dokumen yang lebih tajam
    print("\n🎨 Menerapkan post-processing...")
    scanned_gray = cv2.cvtColor(scanned, cv2.COLOR_BGR2GRAY)
    scanned_thresh = cv2.adaptiveThreshold(
        scanned_gray, 255, 
        cv2.ADAPTIVE_THRESH_GAUSSIAN_C, 
        cv2.THRESH_BINARY, 11, 2
    )
    
    # Visualisasi proses
    image_contour = image.copy()
    cv2.drawContours(image_contour, [doc_contour.astype(int)], -1, (0, 255, 0), 3)
    
    # Mark corners
    for i, pt in enumerate(rect):
        cv2.circle(image_contour, tuple(pt.astype(int)), 10, (0, 0, 255), -1)
        cv2.putText(image_contour, str(i+1), 
                   (int(pt[0])+15, int(pt[1])-15),
                   cv2.FONT_HERSHEY_SIMPLEX, 1.0, (0, 0, 255), 2)
    
    # Plot hasil
    fig, axes = plt.subplots(2, 3, figsize=(15, 10))
    
    # Row 1: Processing steps
    axes[0, 0].imshow(cv2.cvtColor(image, cv2.COLOR_BGR2RGB))
    axes[0, 0].set_title("1. Original Image", fontsize=11)
    axes[0, 0].axis('off')
    
    axes[0, 1].imshow(edged, cmap='gray')
    axes[0, 1].set_title("2. Edge Detection", fontsize=11)
    axes[0, 1].axis('off')
    
    axes[0, 2].imshow(cv2.cvtColor(image_contour, cv2.COLOR_BGR2RGB))
    axes[0, 2].set_title("3. Document Detected", fontsize=11)
    axes[0, 2].axis('off')
    
    # Row 2: Results
    axes[1, 0].imshow(cv2.cvtColor(scanned, cv2.COLOR_BGR2RGB))
    axes[1, 0].set_title("4. Scanned (Color)", fontsize=11)
    axes[1, 0].axis('off')
    
    axes[1, 1].imshow(scanned_gray, cmap='gray')
    axes[1, 1].set_title("5. Grayscale", fontsize=11)
    axes[1, 1].axis('off')
    
    axes[1, 2].imshow(scanned_thresh, cmap='gray')
    axes[1, 2].set_title("6. Thresholded (B&W)", fontsize=11)
    axes[1, 2].axis('off')
    
    plt.suptitle("Document Scanner Pipeline - Automated Detection", 
                 fontsize=14, fontweight='bold', y=0.98)
    plt.tight_layout()
    
    # Simpan hasil
    output_path = os.path.join(DIR_OUTPUT, "scanner_pipeline.png")
    plt.savefig(output_path, dpi=150, bbox_inches="tight")
    print(f"\n[SAVED] {output_path}")
    plt.close()
    
    # Simpan hasil individual
    cv2.imwrite(os.path.join(DIR_OUTPUT, "scanned_color.jpg"), scanned)
    cv2.imwrite(os.path.join(DIR_OUTPUT, "scanned_gray.jpg"), scanned_gray)
    cv2.imwrite(os.path.join(DIR_OUTPUT, "scanned_bw.jpg"), scanned_thresh)
    print(f"[SAVED] Output individual images")
    
    print("\n" + "=" * 60)
    print("PENJELASAN PENERAPAN NYATA")
    print("=" * 60)
    print("""
APLIKASI MOBILE SCANNER MODERN:
✓ CamScanner, Adobe Scan, Microsoft Lens
✓ Google Drive Scanner, Notes Scanner

PIPELINE PROCESSING:
1. Edge Detection (Canny)
   → Deteksi tepi gambar untuk temukan outline dokumen
   
2. Contour Detection
   → Cari kontur terbesar dengan 4 sisi (dokumen)
   
3. Perspective Transform
   → Koreksi perspektif jadi tampak dari atas
   
4. Post-Processing
   → Adaptive thresholding untuk hasil B&W yang tajam

KEUNTUNGAN:
✓ Otomatis deteksi dokumen (tanpa manual crop)
✓ Koreksi perspektif otomatis
✓ Hasil lebih profesional & mudah dibaca
✓ Siap untuk OCR (text recognition)
✓ Ukuran file lebih kecil (mode B&W)

INDUSTRI USAGE:
• Banking: Scan KTP, kartu identitas
• Office: Digitalisasi dokumen fisik
• Education: Scan catatan, tugas
• Legal: Arsip dokumen penting
    """)
    
    print("\n✓ Program selesai!")

if __name__ == "__main__":
    main()
