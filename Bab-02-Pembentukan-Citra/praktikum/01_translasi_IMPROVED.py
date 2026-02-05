# ============================================================
# PROGRAM: 01_translasi_IMPROVED.py
# PRAKTIKUM COMPUTER VISION - BAB 2: PEMBENTUKAN CITRA
# ============================================================
# Deskripsi: Program ini mendemonstrasikan translasi (pergeseran)
#            gambar menggunakan transformation matrix
# 
# Tujuan Pembelajaran:
#   1. Memahami konsep translasi dalam transformasi geometri
#   2. Memahami penggunaan cv2.warpAffine()
#   3. Memahami struktur translation matrix
# ============================================================

# ====================
# IMPORT LIBRARY
# ====================
# cv2: OpenCV untuk computer vision
import cv2
# numpy: untuk operasi array dan matrix
import numpy as np
# matplotlib: untuk plotting dan visualisasi
import matplotlib
# Gunakan backend Agg (non-GUI) agar tidak buka window
matplotlib.use('Agg')
import matplotlib.pyplot as plt
# os: untuk operasi file system
import os
# REFERENSI: Lihat CV2_FUNCTIONS_REFERENCE.py untuk dokumentasi lengkap cv2 functions


# ============================================================
# PANDUAN FUNGSI OPENCV - PENJELASAN DETAIL
# ============================================================
# cv2.imread(path)
#   path: string path lokasi file gambar
#   Return: numpy array BGR (Blue-Green-Red)
#
# cv2.resize(img, dsize, fx, fy, interpolation)
#   img: gambar input (numpy array)
#   dsize: (width, height) ukuran baru atau None
#   fx: faktor skala horizontal (misal 0.5 = 50%)
#   fy: faktor skala vertikal (misal 2.0 = 200%)
#   interpolation: metode interpolasi
#   Return: gambar yang sudah di-resize
#
# cv2.warpAffine(src, M, dsize, dst, flags, borderMode, borderValue)
#   src: gambar sumber (input)
#   M: matriks transformasi affine 2x3
#   dsize: (width, height) ukuran output
#   dst: gambar tujuan (opsional, output buffer)
#   flags: kombinasi flag interpolasi
#   borderMode: mode pengisian area kosong di tepi
#   borderValue: warna untuk area kosong (jika BORDER_CONSTANT)
#   Return: gambar hasil transformasi
#
# cv2.rectangle(img, pt1, pt2, color, thickness)
#   img: gambar yang akan digambar
#   pt1: (x, y) titik sudut kiri-atas
#   pt2: (x, y) titik sudut kanan-bawah
#   color: warna BGR misal (255, 0, 0) = biru
#   thickness: ketebalan garis, -1 untuk isi penuh
#   Return: None (modifikasi langsung pada img)
#
# cv2.circle(img, center, radius, color, thickness)
#   img: gambar yang akan digambar
#   center: (x, y) koordinat pusat lingkaran
#   radius: jari-jari dalam piksel
#   color: warna BGR
#   thickness: ketebalan garis, -1 untuk isi penuh
#   Return: None (modifikasi langsung)
#
# cv2.putText(img, text, org, fontFace, fontScale, color, thickness, lineType)
#   img: gambar yang akan ditambahkan teks
#   text: string teks yang akan ditampilkan
#   org: (x, y) koordinat bottom-left dari teks
#   fontFace: jenis font, misal cv2.FONT_HERSHEY_SIMPLEX
#   fontScale: ukuran font (float), misal 1.0
#   color: warna BGR
#   thickness: ketebalan huruf (int)
#   lineType: tipe garis (opsional)
#   Return: None (modifikasi langsung)
#
# cv2.line(img, pt1, pt2, color, thickness)
#   img: gambar yang akan digambar garis
#   pt1: (x, y) titik awal garis
#   pt2: (x, y) titik akhir garis
#   color: warna BGR
#   thickness: ketebalan garis dalam piksel
#   Return: None (modifikasi langsung)
#
# matplotlib.pyplot.savefig(fname, dpi, bbox_inches)
#   fname: nama file output
#   dpi: dots per inch (resolusi), misal 150
#   bbox_inches: 'tight' untuk crop otomatis
#   Return: None (simpan file ke disk)

# ============================================================
# VARIABEL YANG BISA DIUBAH-UBAH (EKSPERIMEN)
# ============================================================

# 1. File gambar yang akan diproses
NAMA_FILE_GAMBAR = "portrait.jpg"

# 2. Nilai translasi dalam piksel
#    tx: pergeseran horizontal (positif = ke kanan, negatif = ke kiri)
#    ty: pergeseran vertikal (positif = ke bawah, negatif = ke atas)
TX = 100  # Coba ubah: -100, 0, 50, 150, 200
TY = 50   # Coba ubah: -50, 0, 30, 100, 150

# 3. Mode border (apa yang mengisi area kosong setelah pergeseran)
#    cv2.BORDER_CONSTANT: Warna solid (gunakan WARNA_BORDER di bawah)
#    cv2.BORDER_REPLICATE: Duplikasi piksel tepi terdekat
#    cv2.BORDER_REFLECT: Pantulan cermin dari tepi
#    cv2.BORDER_WRAP: Wrap around (tiling)
MODE_BORDER = cv2.BORDER_CONSTANT

# 4. Warna border jika MODE_BORDER = BORDER_CONSTANT (format BGR)
#    (B, G, R) dimana setiap nilai 0-255
#    Contoh: (0, 0, 0) = hitam, (255, 255, 255) = putih, (50, 50, 50) = abu-abu gelap
WARNA_BORDER = (50, 50, 50)  # Abu-abu gelap

# ============================================================
# KONFIGURASI PATH
# ============================================================

# Dapatkan direktori tempat script ini berada
DIR_SCRIPT = os.path.dirname(os.path.abspath(__file__))
# Buat path ke folder data/images (tempat gambar input)
DIR_DATA = os.path.join(DIR_SCRIPT, "data", "images")
# Buat path ke folder output/output1 (tempat hasil disimpan)
DIR_OUTPUT = os.path.join(DIR_SCRIPT, "output", "output1")

# Pastikan folder output ada, jika belum ada maka buat
os.makedirs(DIR_OUTPUT, exist_ok=True)

# ============================================================
# FUNGSI HELPER
# ============================================================

# Keterangan: Fungsi ini mencari file gambar di beberapa lokasi
# Berguna karena file bisa ada di folder praktikum atau folder lain
def dapatkan_path_gambar(nama_file):
    """Mendapatkan path lengkap file gambar.

    Parameter:
    - nama_file (str): nama file gambar yang dicari.

    Return:
    - str: path absolut/relatif yang ditemukan.
    """
    # Dapatkan direktori script saat ini
    direktori_script = os.path.dirname(os.path.abspath(__file__))
    
    # Coba lokasi 1: folder data/images di direktori praktikum
    path_data = os.path.join(direktori_script, "data", "images", nama_file)
    
    # Jika file tidak ada di lokasi 1, coba lokasi 2
    if not os.path.exists(path_data):
        # Coba folder Bab-01-Pendahuluan/data/images (shared folder)
        path_data = os.path.join(direktori_script, "..", "..", 
                                  "Bab-01-Pendahuluan", "data", "images", nama_file)
    
    # Jika masih tidak ada, coba lokasi 3: langsung di folder praktikum
    if not os.path.exists(path_data):
        path_data = os.path.join(direktori_script, nama_file)
    
    # Kembalikan path yang ditemukan
    return path_data


# Keterangan: Membuat gambar sintetis untuk demo jika file tidak ada
# Gambar ini berisi shapes geometris yang mudah dilihat pergeserannya
def buat_gambar_sample():
    """Membuat gambar sample dengan shapes untuk demonstrasi.

    Return:
    - np.ndarray: gambar buatan berformat BGR.
    """
    # Inisialisasi kanvas gambar kosong (hitam) dengan ukuran 400 tinggi x 600 lebar, 3 channel BGR
    gambar = np.zeros((400, 600, 3), dtype=np.uint8)
    
    # Buat gradient warna vertikal pada background untuk efek visual menarik
    # Loop untuk setiap baris dari atas (0) sampai bawah (400)
    for i in range(400):
        # Hitung warna gradient: atas lebih gelap, bawah lebih terang
        # Channel B (Blue): 0 sampai 200
        # Channel G (Green): 100 sampai 200  
        # Channel R (Red): 200 sampai 0 (merah berkurang ke bawah)
        gambar[i, :] = [int(i/2), int(100 + i/4), int(200 - i/4)]
    
    # Gambar persegi putih sebagai elemen referensi posisi
    # pt1=(50,50): sudut kiri-atas, pt2=(150,150): sudut kanan-bawah
    # color=(255,255,255): putih, thickness=-1: isi penuh
    cv2.rectangle(gambar, (50, 50), (150, 150), (255, 255, 255), -1)
    
    # Gambar lingkaran cyan (biru cerah) sebagai elemen referensi kedua
    # center=(300,200): pusat di x=300, y=200
    # radius=60: jari-jari 60 piksel
    # color=(0,255,255): cyan (B=0, G=255, R=255)
    # thickness=-1: isi penuh
    cv2.circle(gambar, (300, 200), 60, (0, 255, 255), -1)
    
    # Tambahkan teks "OpenCV" untuk memudahkan melihat pergeseran
    # text="OpenCV": string yang ditampilkan
    # org=(200,350): posisi bottom-left teks
    # fontFace=FONT_HERSHEY_SIMPLEX: jenis font
    # fontScale=1.5: ukuran font
    # color=(255,255,255): putih
    # thickness=2: ketebalan huruf
    cv2.putText(gambar, "OpenCV", (200, 350), 
                cv2.FONT_HERSHEY_SIMPLEX, 1.5, (255, 255, 255), 2)
    
    # Gambar garis horizontal abu-abu sebagai referensi sumbu-x (y=200)
    # pt1=(0,200): mulai dari kiri, pt2=(600,200): sampai kanan
    # color=(128,128,128): abu-abu sedang
    # thickness=1: garis tipis
    cv2.line(gambar, (0, 200), (600, 200), (128, 128, 128), 1)
    
    # Gambar garis vertikal abu-abu sebagai referensi sumbu-y (x=300)
    # pt1=(300,0): mulai dari atas, pt2=(300,400): sampai bawah
    cv2.line(gambar, (300, 0), (300, 400), (128, 128, 128), 1)
    
    # Kembalikan gambar yang telah dibuat
    return gambar


# ============================================================
# FUNGSI TRANSLASI
# ============================================================

# Keterangan: Fungsi utama untuk melakukan translasi (pergeseran) gambar
# Menggunakan cv2.warpAffine() dengan translation matrix
def translasi_gambar(gambar, tx, ty, border_mode=cv2.BORDER_CONSTANT,
                     border_value=(0, 0, 0)):
    """Melakukan translasi (pergeseran) gambar.

    Parameter:
    - gambar (np.ndarray): gambar input format BGR.
    - tx (int/float): pergeseran horizontal dalam piksel (+ ke kanan, - ke kiri).
    - ty (int/float): pergeseran vertikal dalam piksel (+ ke bawah, - ke atas).
    - border_mode (int): mode pengisian area kosong (cv2.BORDER_*).
    - border_value (tuple[int, int, int]): warna border BGR jika BORDER_CONSTANT.

    Return:
    - tuple: (gambar_hasil, matriks_translasi)
        - gambar_hasil: gambar yang sudah ditranslasi
        - matriks_translasi: matriks 2x3 yang digunakan
    """
    # Ambil dimensi gambar: tinggi (height) dan lebar (width)
    # shape mengembalikan (height, width, channels)
    tinggi, lebar = gambar.shape[:2]
    
    # PENJELASAN TRANSLATION MATRIX:
    # Matriks transformasi translasi berukuran 2x3:
    # | 1  0  tx |
    # | 0  1  ty |
    #
    # Persamaan transformasi untuk setiap piksel:
    # x_baru = 1*x_lama + 0*y_lama + tx = x_lama + tx
    # y_baru = 0*x_lama + 1*y_lama + ty = y_lama + ty
    #
    # Artinya:
    # - Setiap piksel bergeser tx piksel ke kanan (jika tx positif)
    # - Setiap piksel bergeser ty piksel ke bawah (jika ty positif)
    
    # Buat matriks translasi 2x3 dengan tipe float32 (required oleh OpenCV)
    # Baris 1: [1, 0, tx] untuk transformasi koordinat x
    # Baris 2: [0, 1, ty] untuk transformasi koordinat y
    translation_matrix = np.float32([
        [1, 0, tx],  # Baris transformasi x: x' = x + tx
        [0, 1, ty]   # Baris transformasi y: y' = y + ty
    ])
    
    # Terapkan transformasi affine menggunakan matriks translasi
    # cv2.warpAffine akan:
    # 1. Membaca setiap piksel dari gambar input
    # 2. Menghitung posisi baru menggunakan translation_matrix
    # 3. Menempatkan piksel di posisi baru pada gambar output
    # 4. Area kosong diisi sesuai borderMode dan borderValue
    hasil = cv2.warpAffine(
        gambar,                # Gambar sumber (input) yang akan ditranslasi
        translation_matrix,    # Matriks transformasi 2x3 yang kita buat
        (lebar, tinggi),       # Ukuran output (width, height) - sama dengan input
        borderMode=border_mode,   # Mode pengisian area kosong (CONSTANT/REPLICATE/REFLECT)
        borderValue=border_value  # Warna untuk area kosong jika borderMode=CONSTANT
    )
    
    # Kembalikan gambar hasil transformasi dan matriks yang digunakan
    # Matriks dikembalikan untuk keperluan debugging/analisis
    return hasil, translation_matrix


# Keterangan: Fungsi untuk mendemonstrasikan translasi ke berbagai arah
# Berguna untuk memahami efek parameter tx dan ty
def demo_translasi_berbagai_arah(gambar):
    """Mendemonstrasikan translasi ke berbagai arah.

    Parameter:
    - gambar (np.ndarray): gambar input (BGR).
    """
    # Tampilkan header informasi demo ke console
    print("\n" + "=" * 60)
    print("DEMO TRANSLASI KE BERBAGAI ARAH")
    print("=" * 60)
    
    # Definisikan daftar kombinasi translasi yang akan dicoba
    # Format: (tx, ty, "Label deskriptif")
    # tx: pergeseran horizontal, ty: pergeseran vertikal
    translasi_list = [
        (0, 0, "Original (Tanpa Translasi)"),        # Tidak bergeser
        (100, 0, "Kanan 100px (+tx)"),               # Geser kanan saja
        (-100, 0, "Kiri 100px (-tx)"),               # Geser kiri saja
        (0, 80, "Bawah 80px (+ty)"),                 # Geser bawah saja
        (0, -80, "Atas 80px (-ty)"),                 # Geser atas saja
        (100, 80, "Kanan-Bawah (diagonal)"),         # Geser diagonal kanan-bawah
        (-100, -80, "Kiri-Atas (diagonal)"),         # Geser diagonal kiri-atas
    ]
    
    # Siapkan kanvas plot dengan grid 2 baris x 4 kolom = 8 subplots
    # figsize=(16, 8): ukuran figure dalam inch (lebar, tinggi)
    fig, axes = plt.subplots(2, 4, figsize=(16, 8))
    
    # Flatten axes 2D menjadi 1D array untuk kemudahan iterasi
    # Sebelum: axes[row][col], Sesudah: axes[i]
    axes = axes.flatten()
    
    # Iterasi melalui setiap kombinasi translasi dalam list
    # enumerate memberikan index (i) dan value (tx, ty, label)
    for i, (tx, ty, label) in enumerate(translasi_list):
        # Terapkan translasi pada gambar dengan parameter tx, ty saat ini
        # border_mode=BORDER_CONSTANT: area kosong diisi warna solid
        # border_value=WARNA_BORDER: gunakan warna yang didefinisikan di atas
        hasil, _ = translasi_gambar(gambar, tx, ty, 
                                    border_mode=cv2.BORDER_CONSTANT,
                                    border_value=WARNA_BORDER)
        
        # Konversi gambar dari BGR (OpenCV) ke RGB (matplotlib)
        # Matplotlib mengharapkan RGB, sedangkan OpenCV menggunakan BGR
        hasil_rgb = cv2.cvtColor(hasil, cv2.COLOR_BGR2RGB)
        
        # Tampilkan gambar hasil pada subplot ke-i
        # axes[i]: subplot saat ini
        # imshow: tampilkan gambar sebagai image
        axes[i].imshow(hasil_rgb)
        
        # Set judul subplot dengan label dan nilai tx, ty
        # \n: newline untuk membuat 2 baris
        axes[i].set_title(f"{label}\ntx={tx}, ty={ty}")
        
        # Nonaktifkan sumbu x dan y untuk tampilan lebih rapi (tidak ada angka/tick)
        axes[i].axis('off')
        
        # Cetak informasi translasi ke console untuk tracking
        print(f"[{i+1}] {label}: tx={tx}, ty={ty}")
    
    # Kosongkan subplot terakhir (index 7) karena hanya ada 7 item
    # 8 subplot disediakan tapi hanya 7 yang digunakan
    axes[7].axis('off')
    
    # Set judul keseluruhan untuk figure
    # fontsize=14: ukuran font judul
    # fontweight='bold': tebal
    # y=1.00: posisi vertikal judul (1.00 = di atas figure)
    plt.suptitle("Translasi ke Berbagai Arah - Perbandingan Visual", 
                 fontsize=14, fontweight='bold', y=1.00)
    
    # Atur spacing antar subplot agar tidak overlap
    # pad=3.0: padding di sekitar figure
    plt.tight_layout(pad=3.0)
    
    # Simpan figure ke file PNG dengan resolusi tinggi
    # dpi=150: dots per inch (resolusi)
    # bbox_inches='tight': crop otomatis ruang kosong di pinggir
    output_path = os.path.join(DIR_OUTPUT, "translasi_berbagai_arah.png")
    plt.savefig(output_path, dpi=150, bbox_inches='tight')
    
    # Cetak informasi file yang tersimpan
    print(f"\n✓ Tersimpan: {output_path}")
    
    # Tutup figure untuk menghemat memori
    # Penting saat membuat banyak plot
    plt.close()


# ============================================================
# FUNGSI UTAMA (MAIN)
# ============================================================

# Keterangan: Fungsi main adalah entry point program
# Fungsi ini akan dijalankan ketika script dieksekusi
def main():
    """Fungsi utama program."""
    # Cetak header program ke console
    print("\n" + "=" * 60)
    print("PROGRAM: TRANSLASI (PERGESERAN) GAMBAR")
    print("=" * 60)
    
    # Cari path lengkap file gambar menggunakan fungsi helper
    # Akan mencari di beberapa lokasi yang mungkin
    path_gambar = dapatkan_path_gambar(NAMA_FILE_GAMBAR)
    
    # Inisialisasi variabel gambar
    gambar = None
    
    # Coba baca gambar dari path yang ditemukan
    if path_gambar and os.path.exists(path_gambar):
        # imread: baca file gambar ke numpy array BGR
        # Jika berhasil: gambar berisi numpy array
        # Jika gagal: gambar = None
        gambar = cv2.imread(path_gambar)
        
        # Jika berhasil dibaca, cetak info
        if gambar is not None:
            print(f"\n✓ Gambar berhasil dimuat: {path_gambar}")
            # shape: (height, width, channels)
            h, w = gambar.shape[:2]
            print(f"  Ukuran: {w} x {h} pixels")
    
    # Jika gambar tidak berhasil dimuat, buat gambar sample
    if gambar is None:
        print(f"\n⚠ Gambar tidak ditemukan, membuat gambar sample...")
        # Panggil fungsi untuk membuat gambar sintetis
        gambar = buat_gambar_sample()
        print(f"✓ Gambar sample berhasil dibuat (600x400)")
    
    # Jalankan demo translasi ke berbagai arah
    print(f"\n🔄 Menjalankan demo translasi...")
    demo_translasi_berbagai_arah(gambar)
    
    # Cetak ringkasan hasil
    print("\n" + "=" * 60)
    print("RINGKASAN")
    print("=" * 60)
    print(f"""
✓ Program selesai dijalankan!

PENJELASAN TRANSLASI:
- Translasi adalah pergeseran gambar tanpa rotasi/scaling
- Menggunakan matriks 2x3: [[1, 0, tx], [0, 1, ty]]
- tx > 0: geser ke KANAN
- tx < 0: geser ke KIRI
- ty > 0: geser ke BAWAH
- ty < 0: geser ke ATAS

OUTPUT YANG DIHASILKAN:
- File: {os.path.join(DIR_OUTPUT, 'translasi_berbagai_arah.png')}
- Berisi: 7 gambar dengan translasi berbeda

NEXT STEP:
1. Buka file output untuk melihat hasil visual
2. Coba ubah nilai TX dan TY di bagian atas program
3. Coba ubah MODE_BORDER dan WARNA_BORDER
4. Jalankan ulang program untuk melihat perbedaannya
    """)


# Jalankan fungsi main jika script ini dieksekusi langsung
# Tidak akan dijalankan jika di-import sebagai module
if __name__ == "__main__":
    main()
