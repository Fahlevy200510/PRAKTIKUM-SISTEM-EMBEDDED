# ============================================================
# PROGRAM: 05_perspektif_transform_auto.py (AUTOMATED VERSION)
# PRAKTIKUM COMPUTER VISION - BAB 2: PEMBENTUKAN CITRA
# ============================================================
# Deskripsi: Program ini mendemonstrasikan transformasi perspektif
#            dengan koordinat otomatis (tanpa interaksi manual)
# 
# Penerapan Nyata: Koreksi perspektif foto dokumen, KTP, atau kartu
#                  yang difoto dari sudut miring
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
DIR_OUTPUT = os.path.join(DIR_SCRIPT, "output", "output5")
os.makedirs(DIR_OUTPUT, exist_ok=True)

NAMA_FILE_GAMBAR = "document.jpg"
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
    """Urutkan 4 titik: top-left, top-right, bottom-right, bottom-left."""
    pts = np.array(pts, dtype=np.float32)
    sorted_by_y = pts[np.argsort(pts[:, 1])]
    
    top_points = sorted_by_y[:2]
    bottom_points = sorted_by_y[2:]
    
    top_left, top_right = top_points[np.argsort(top_points[:, 0])]
    bottom_left, bottom_right = bottom_points[np.argsort(bottom_points[:, 0])]
    
    return np.array([top_left, top_right, bottom_right, bottom_left], 
                    dtype=np.float32)

def main():
    """Fungsi utama program."""
    print("\n" + "=" * 60)
    print("PERSPEKTIF TRANSFORM - AUTOMATED DEMO")
    print("=" * 60)
    
    # Load gambar
    path_gambar = dapatkan_path_gambar(NAMA_FILE_GAMBAR)
    
    if path_gambar is None or not os.path.exists(path_gambar):
        print(f"\n❌ Error: File '{NAMA_FILE_GAMBAR}' tidak ditemukan!")
        return
    
    gambar = cv2.imread(path_gambar)
    
    if gambar is None:
        print(f"\n❌ Error: Gagal membaca gambar dari {path_gambar}")
        return
    
    h, w = gambar.shape[:2]
    print(f"\n✓ Gambar berhasil dimuat: {path_gambar}")
    print(f"  Ukuran: {w} x {h}")
    
    # KOORDINAT OTOMATIS - Simulasi dokumen yang difoto dari sudut
    # Mengambil area tengah dengan perspektif sedikit miring
    margin = 80
    pts_src = np.float32([
        [margin, margin],              # Top-left (sedikit ke dalam)
        [w - margin, margin + 40],     # Top-right (sedikit miring)
        [w - margin - 20, h - margin], # Bottom-right
        [margin + 20, h - margin - 40] # Bottom-left (perspektif)
    ])
    
    # Titik tujuan (persegi panjang sempurna)
    pts_dst = np.float32([
        [0, 0],
        [OUTPUT_WIDTH - 1, 0],
        [OUTPUT_WIDTH - 1, OUTPUT_HEIGHT - 1],
        [0, OUTPUT_HEIGHT - 1]
    ])
    
    # Hitung matriks transformasi perspektif
    print("\nMenghitung matriks transformasi perspektif...")
    M = cv2.getPerspectiveTransform(pts_src, pts_dst)
    
    # Terapkan transformasi
    hasil = cv2.warpPerspective(gambar, M, (OUTPUT_WIDTH, OUTPUT_HEIGHT))
    
    # Visualisasi
    gambar_marked = gambar.copy()
    pts_ordered = order_points(pts_src)
    labels = ['TL', 'TR', 'BR', 'BL']
    colors = [(0, 255, 0), (0, 255, 255), (0, 0, 255), (255, 0, 255)]
    
    for pt, label, color in zip(pts_ordered, labels, colors):
        cv2.circle(gambar_marked, tuple(pt.astype(int)), 10, color, -1)
        cv2.putText(gambar_marked, label, (int(pt[0])+15, int(pt[1])-15),
                   cv2.FONT_HERSHEY_SIMPLEX, 1.0, color, 2)
    
    # Gambar garis penghubung
    for i in range(4):
        pt1 = pts_ordered[i].astype(int)
        pt2 = pts_ordered[(i+1)%4].astype(int)
        cv2.line(gambar_marked, tuple(pt1), tuple(pt2), (255, 255, 0), 2)
    
    # Plot perbandingan
    fig, axes = plt.subplots(1, 2, figsize=(14, 7))
    
    axes[0].imshow(cv2.cvtColor(gambar_marked, cv2.COLOR_BGR2RGB))
    axes[0].set_title("Original dengan 4 Titik Sudut\n(Perspektif Miring)", fontsize=13)
    axes[0].axis('off')
    
    axes[1].imshow(cv2.cvtColor(hasil, cv2.COLOR_BGR2RGB))
    axes[1].set_title("Hasil Koreksi Perspektif\n(Bird's Eye View)", fontsize=13)
    axes[1].axis('off')
    
    plt.suptitle("Perspective Transform - Koreksi Dokumen Miring", 
                 fontsize=15, fontweight='bold', y=0.98)
    plt.tight_layout()
    
    # Simpan hasil
    output_path = os.path.join(DIR_OUTPUT, "hasil_transform.png")
    plt.savefig(output_path, dpi=150, bbox_inches="tight")
    print(f"\n[SAVED] {output_path}")
    plt.close()
    
    # Simpan gambar hasil transform
    output_image_path = os.path.join(DIR_OUTPUT, "transform_hasil.jpg")
    cv2.imwrite(output_image_path, hasil)
    print(f"[SAVED] {output_image_path}")
    
    # Info matriks
    print("\n" + "=" * 60)
    print("INFORMASI MATRIKS HOMOGRAPHY")
    print("=" * 60)
    print(f"\nMatriks Transformasi (3×3):")
    print(M)
    print(f"\nDeterminan: {np.linalg.det(M):.4f}")
    
    print("\n" + "=" * 60)
    print("PENJELASAN PENERAPAN NYATA")
    print("=" * 60)
    print("""
CONTOH PENGGUNAAN:
1. Scan Dokumen/KTP dengan Smartphone
   - Foto KTP dari sudut miring → koreksi jadi lurus
   
2. Whiteboard Photo Correction
   - Foto papan tulis dari sudut → jadi tampak lurus
   
3. Document Digitization
   - Foto buku/majalah → ekstrak halaman jadi flat
   
4. Parking Lot/Traffic Camera
   - Koreksi perspektif untuk deteksi plat nomor

KEUNTUNGAN:
✓ Tidak perlu scanner fisik
✓ Bisa dilakukan dengan kamera smartphone
✓ Hasil lebih mudah dibaca dan diproses OCR
✓ Otomatis menghilangkan distorsi perspektif
    """)
    
    print("\n✓ Program selesai!")

if __name__ == "__main__":
    main()
