# ============================================================
# PROGRAM: 05_perspektif_transform_real.py
# PRAKTIKUM COMPUTER VISION - BAB 2: PEMBENTUKAN CITRA
# ============================================================
# Deskripsi: Program ini mendemonstrasikan transformasi perspektif
#            dengan FOTO NYATA dan interaktif click 4 titik
# 
# Cara Penggunaan:
#   1. Program akan menampilkan foto di window
#   2. KLIK 4 TITIK (TL → TR → BR → BL) di gambar
#   3. Program otomatis apply perspective transform
#   4. Hasil disimpan di output/
# ============================================================

import cv2
import numpy as np
import os

# ============================================================
# KONFIGURASI
# ============================================================

DIR_SCRIPT = os.path.dirname(os.path.abspath(__file__))
DIR_OUTPUT = os.path.join(DIR_SCRIPT, "output", "output5_real")

os.makedirs(DIR_OUTPUT, exist_ok=True)

# Nama file gambar (pilih salah satu yang ada)
NAMA_FILE_GAMBAR = "portrait.jpg"

# Ukuran output untuk bird's eye view
OUTPUT_WIDTH = 400
OUTPUT_HEIGHT = 500


# ============================================================
# FUNGSI: Mencari path gambar
# ============================================================

def dapatkan_path_gambar(nama_file):
    """Mendapatkan path lengkap file gambar."""
    # Coba di folder data/images
    path_data = os.path.join(DIR_SCRIPT, "data", "images", nama_file)
    if os.path.exists(path_data):
        return path_data
    
    # Coba di folder Bab-01
    path_data = os.path.join(DIR_SCRIPT, "..", "..", 
                             "Bab-01-Pendahuluan", "data", "images", nama_file)
    if os.path.exists(path_data):
        return path_data
    
    # Coba langsung di folder praktikum
    path_data = os.path.join(DIR_SCRIPT, nama_file)
    if os.path.exists(path_data):
        return path_data
    
    # Tidak ditemukan
    return None


# ============================================================
# FUNGSI: Interactive Click untuk pilih 4 titik
# ============================================================

class PointSelector:
    """Kelas untuk select 4 titik dengan mouse click."""
    
    def __init__(self, gambar):
        """Inisialisasi dengan gambar input."""
        # Simpan gambar asli untuk ditampilkan
        self.gambar_asli = gambar.copy()
        # Buat copy untuk drawing (menampilkan titik)
        self.gambar_draw = gambar.copy()
        # List untuk menyimpan 4 titik yang diklik
        self.points = []
        # Label untuk setiap titik
        self.labels = ['TL (Top-Left)', 'TR (Top-Right)', 
                       'BR (Bottom-Right)', 'BL (Bottom-Left)']
        # Warna untuk setiap titik
        self.colors = [(0, 255, 0), (0, 255, 255), 
                       (0, 0, 255), (255, 0, 255)]
    
    def mouse_callback(self, event, x, y, flags, param):
        """Callback ketika mouse diklik."""
        # Jika mouse button kiri diklik
        if event == cv2.EVENT_LBUTTONDOWN:
            # Tampilkan info ke console
            print(f"✓ Titik {len(self.points)+1}/{4}: {self.labels[len(self.points)]} = ({x}, {y})")
            
            # Tambahkan titik ke list
            self.points.append([x, y])
            
            # Gambar lingkaran pada gambar
            cv2.circle(self.gambar_draw, (x, y), 10, 
                      self.colors[len(self.points)-1], -1)
            # Tambahkan teks pada gambar
            cv2.putText(self.gambar_draw, self.labels[len(self.points)-1], 
                       (x+15, y-15), cv2.FONT_HERSHEY_SIMPLEX, 
                       0.6, self.colors[len(self.points)-1], 2)
            
            # Tampilkan gambar yang sudah digambar
            cv2.imshow('SELECT 4 TITIK', self.gambar_draw)
            
            # Jika sudah 4 titik, selesai
            if len(self.points) == 4:
                # Cetak informasi ke console
                print("\n✓ Semua 4 titik sudah dipilih!")
    
    def select(self):
        """Tampilkan window dan tunggu user pilih 4 titik."""
        # Cetak informasi ke console
        print("=" * 60)
        # Cetak informasi ke console
        print("INSTRUKSI: KLIK 4 TITIK SUDUT DOKUMEN (Urut: TL→TR→BR→BL)")
        # Cetak informasi ke console
        print("=" * 60)
        # Tampilkan gambar pada window
        cv2.imshow('SELECT 4 TITIK', self.gambar_draw)
        # Set callback untuk mouse click
        cv2.setMouseCallback('SELECT 4 TITIK', self.mouse_callback)
        
        # Tunggu sampai 4 titik dipilih atau ESC ditekan
        while len(self.points) < 4:
            # Tunggu key press (1ms timeout)
            key = cv2.waitKey(1) & 0xFF
            # Jika ESC ditekan, keluar
            if key == 27:
                # Cetak informasi ke console
                print("Pembatalan oleh user.")
                # Tutup window
                cv2.destroyAllWindows()
                return None
        
        # Tutup window
        cv2.destroyAllWindows()
        # Kembalikan hasil dari fungsi
        return np.array(self.points, dtype=np.float32)


# ============================================================
# FUNGSI: Urutkan 4 titik (TL, TR, BR, BL)
# ============================================================

def order_points(pts):
    """Urutkan 4 titik: top-left, top-right, bottom-right, bottom-left."""
    # Buat array numpy dari data
    pts = np.array(pts, dtype=np.float32)
    
    # Urutkan berdasarkan koordinat Y
    sorted_by_y = pts[np.argsort(pts[:, 1])]
    
    # 2 titik teratas dan 2 titik terbawah
    top_points = sorted_by_y[:2]
    bottom_points = sorted_by_y[2:]
    
    # Urutkan berdasarkan X
    top_left, top_right = top_points[np.argsort(top_points[:, 0])]
    bottom_left, bottom_right = bottom_points[np.argsort(bottom_points[:, 0])]
    
    # Kembalikan hasil dari fungsi
    return np.array([top_left, top_right, bottom_right, bottom_left], 
                    dtype=np.float32)


# ============================================================
# FUNGSI: Apply Perspective Transform
# ============================================================

def apply_perspective_transform(gambar, pts_src):
    """Apply perspective transform untuk bird's eye view."""
    # Urutkan titik ke urutan baku
    pts_src = order_points(pts_src)
    
    # Titik tujuan (persegi panjang sempurna)
    pts_dst = np.float32([
        [0, 0],
        [OUTPUT_WIDTH - 1, 0],
        [OUTPUT_WIDTH - 1, OUTPUT_HEIGHT - 1],
        [0, OUTPUT_HEIGHT - 1]
    ])
    
    # Hitung matriks transformasi perspektif
    M = cv2.getPerspectiveTransform(pts_src, pts_dst)
    
    # Terapkan transformasi perspektif pada gambar
    hasil = cv2.warpPerspective(gambar, M, (OUTPUT_WIDTH, OUTPUT_HEIGHT))
    
    # Kembalikan hasil dari fungsi
    return hasil, M, pts_src, pts_dst


# ============================================================
# FUNGSI: Tampilkan Perbandingan Original vs Hasil
# ============================================================

def tampilkan_perbandingan(gambar_asli, pts_src, hasil_transform):
    """Tampilkan 2 gambar side-by-side dan simpan."""
    # Cetak informasi ke console
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("HASIL TRANSFORMASI PERSPEKTIF")
    # Cetak informasi ke console
    print("=" * 60)
    
    # Buat copy untuk marking titik
    gambar_marked = gambar_asli.copy()
    pts_ordered = order_points(pts_src)
    labels = ['TL', 'TR', 'BR', 'BL']
    colors = [(0, 255, 0), (0, 255, 255), (0, 0, 255), (255, 0, 255)]
    
    # Gambar lingkaran dan label untuk setiap titik
    for i, (pt, label, color) in enumerate(zip(pts_ordered, labels, colors)):
        # Gambar lingkaran pada gambar
        cv2.circle(gambar_marked, tuple(pt.astype(int)), 8, color, -1)
        # Tambahkan teks pada gambar
        cv2.putText(gambar_marked, label, (int(pt[0])+10, int(pt[1])-10),
                   cv2.FONT_HERSHEY_SIMPLEX, 0.8, color, 2)
    
    # Buat figure untuk plot
    fig, axes = plt.subplots(1, 2, figsize=(14, 6))
    
    # Tampilkan gambar pada subplot tertentu
    axes[0].imshow(cv2.cvtColor(gambar_marked, cv2.COLOR_BGR2RGB))
    # Set judul untuk subplot
    axes[0].set_title("Original dengan 4 Titik Sudut", fontsize=12)
    # Nonaktifkan atau atur axis pada subplot
    axes[0].axis('off')
    
    # Tampilkan gambar pada subplot tertentu
    axes[1].imshow(cv2.cvtColor(hasil_transform, cv2.COLOR_BGR2RGB))
    # Set judul untuk subplot
    axes[1].set_title("Bird's Eye View (Perspective Corrected)", fontsize=12)
    # Nonaktifkan atau atur axis pada subplot
    axes[1].axis('off')
    
    # Set judul keseluruhan figure
    plt.suptitle("Perspective Transform - Foto Nyata", fontsize=14, fontweight='bold')
    # Atur spacing antar subplot
    plt.tight_layout()
    
    # Simpan figure ke file dengan kualitas DPI tertentu
    output_path = os.path.join(DIR_OUTPUT, "hasil_transform.png")
    # Simpan figure ke file dengan kualitas DPI tertentu
    plt.savefig(output_path, dpi=150, bbox_inches="tight")
    # Cetak informasi ke console
    print(f"\n[SAVED] {output_path}")
    # Tutup figure untuk menghemat memory
    plt.close()


# ============================================================
# FUNGSI: Tampilkan Info Matriks Homography
# ============================================================

def tampilkan_info_homography(M, pts_src, pts_dst):
    """Tampilkan informasi matriks homography."""
    # Cetak informasi ke console
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("INFORMASI MATRIKS HOMOGRAPHY (3×3)")
    # Cetak informasi ke console
    print("=" * 60)
    # Cetak informasi ke console
    print("\nMatriks M (Perspective Transform Matrix):")
    # Cetak matriks M ke console
    print(M)
    
    # Cetak informasi ke console
    print("\nTitik Sumber (Asli - Perspektif):")
    # Cetak informasi ke console
    print(f"  TL: {pts_src[0]}")
    # Cetak informasi ke console
    print(f"  TR: {pts_src[1]}")
    # Cetak informasi ke console
    print(f"  BR: {pts_src[2]}")
    # Cetak informasi ke console
    print(f"  BL: {pts_src[3]}")
    
    # Cetak informasi ke console
    print("\nTitik Tujuan (Bird's Eye View):")
    # Cetak informasi ke console
    print(f"  TL: {pts_dst[0]}")
    # Cetak informasi ke console
    print(f"  TR: {pts_dst[1]}")
    # Cetak informasi ke console
    print(f"  BR: {pts_dst[2]}")
    # Cetak informasi ke console
    print(f"  BL: {pts_dst[3]}")
    
    # Hitung determinan matriks
    det = np.linalg.det(M)
    # Cetak informasi ke console
    print(f"\nDeterminan: {det:.4f}")
    # Hitung inverse matriks
    M_inv = np.linalg.inv(M)
    # Cetak informasi ke console
    print(f"\nInverse Matrix (untuk transform balik):")
    # Cetak matriks inverse ke console
    print(M_inv)


# ============================================================
# FUNGSI UTAMA
# ============================================================

def main():
    """Fungsi utama program."""
    # Cetak informasi ke console
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("PERSPEKTIF TRANSFORM - FOTO NYATA")
    # Cetak informasi ke console
    print("=" * 60)
    
    # Cari dan load gambar
    path_gambar = dapatkan_path_gambar(NAMA_FILE_GAMBAR)
    
    # Jika file tidak ditemukan
    if path_gambar is None or not os.path.exists(path_gambar):
        # Cetak informasi ke console
        print(f"\n❌ Error: File '{NAMA_FILE_GAMBAR}' tidak ditemukan!")
        # Cetak informasi ke console
        print(f"Lokasi pencarian:")
        # Cetak informasi ke console
        print(f"  1. {os.path.join(DIR_SCRIPT, 'data', 'images')}")
        # Cetak informasi ke console
        print(f"  2. Bab-01-Pendahuluan/data/images")
        # Cetak informasi ke console
        print(f"  3. Folder praktikum langsung")
        return
    
    # Baca gambar
    gambar = cv2.imread(path_gambar)
    
    # Cek kondisi logis
    if gambar is None:
        # Cetak informasi ke console
        print(f"\n❌ Error: Gagal membaca gambar dari {path_gambar}")
        return
    
    # Cetak informasi ke console
    print(f"\n✓ Gambar berhasil dimuat: {path_gambar}")
    # Cetak informasi ke console
    print(f"  Ukuran: {gambar.shape[1]} x {gambar.shape[0]}")
    
    # Tampilkan ukuran output
    # Cetak informasi ke console
    print(f"\nUkuran output: {OUTPUT_WIDTH} x {OUTPUT_HEIGHT}")
    
    # Buat selector untuk pilih 4 titik
    selector = PointSelector(gambar)
    
    # Biarkan user klik 4 titik
    pts_src = selector.select()
    
    # Cek kondisi logis
    if pts_src is None:
        return
    
    # Apply perspective transform
    # Cetak informasi ke console
    print("\nMenghitung matriks transformasi perspektif...")
    hasil, M, pts_src_ordered, pts_dst = apply_perspective_transform(gambar, pts_src)
    
    # Tampilkan info matriks
    tampilkan_info_homography(M, pts_src_ordered, pts_dst)
    
    # Tampilkan perbandingan dan simpan
    tampilkan_perbandingan(gambar, pts_src, hasil)
    
    # Simpan gambar hasil transform
    output_image_path = os.path.join(DIR_OUTPUT, "transform_hasil.jpg")
    # Simpan gambar dengan format JPG
    cv2.imwrite(output_image_path, hasil)
    # Cetak informasi ke console
    print(f"[SAVED] {output_image_path}")
    
    # Ringkasan
    # Cetak informasi ke console
    print("\n" + "=" * 60)
    # Cetak informasi ke console
    print("RINGKASAN")
    # Cetak informasi ke console
    print("=" * 60)
    # Cetak informasi ke console
    print(f"""
✓ Program selesai!

PENJELASAN:
- cv2.getPerspectiveTransform() menghitung matriks 3×3 dari 4 titik
- Matriks ini menyimpan "peta jalan" transformasi
- cv2.warpPerspective() menggerakkan setiap pixel sesuai peta jalan
- Hasilnya: Dokumen terlihat dari atas (bird's eye view)

OUTPUT:
- {os.path.join(DIR_OUTPUT, 'hasil_transform.png')} → Perbandingan before-after
- {os.path.join(DIR_OUTPUT, 'transform_hasil.jpg')} → Gambar hasil transform

NEXT STEP:
- Coba dengan 4 titik berbeda untuk hasil berbeda
- Bandingkan dengan affine transform (3 titik) untuk lihat perbedaannya
    """)


# Jalankan program utama
if __name__ == "__main__":
    main()
