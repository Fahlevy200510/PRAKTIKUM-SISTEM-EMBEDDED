# BAB 2: Pembentukan Citra (Image Formation)

## 🎯 Capaian Pembelajaran

Setelah menyelesaikan bab ini, mahasiswa diharapkan mampu:

1. Memahami prinsip dasar pembentukan citra digital dan geometric primitives
2. Menjelaskan model kamera pinhole dan proyeksi perspektif dengan parameter intrinsik-ekstrinsik
3. Menerapkan transformasi geometri 2D dan 3D dengan berbagai tingkat kompleksitas
4. Memahami konsep kalibrasi kamera dan koreksi distorsi lensa
5. Melakukan operasi transformasi perspektif untuk aplikasi nyata seperti document scanner
6. Memahami photometric image formation dan proses sampling dalam sensor digital
7. Menerapkan seluruh konsep melalui program praktikum yang menghasilkan output visual

---

## 1️⃣ Definisi dan Konsep Dasar

### Apa itu Image Formation?

**Image Formation** (Pembentukan Citra) adalah proses di mana cahaya dari dunia nyata (3D) ditangkap dan dikonversi menjadi representasi 2D digital dalam bentuk gambar. Proses ini melibatkan beberapa tahap:

#### Pipeline Pembentukan Citra:

```
Scene → Lighting → Reflectance → Optics → Sensor → Digital Processing → Final Image
  ↓         ↓           ↓          ↓        ↓           ↓                    ↓
3D World  Photon    BRDF/Shading  Lens   CCD/CMOS   ADC/Gamma/Color    RGB Values
```

### Komponen Utama Pembentukan Citra

1. **Geometric Image Formation**:

   - Proyeksi 3D ke 2D menggunakan model kamera
   - Transformasi geometri dan distorsi lensa
2. **Photometric Image Formation**:

   - Interaksi cahaya dengan permukaan (BRDF)
   - Shading dan pencahayaan global
3. **Digital Camera Pipeline**:

   - Sampling dan aliasing
   - Color filter arrays (Bayer pattern)
   - Gamma correction dan compression

---

## 2️⃣ Geometric Primitives dan Transformasi

### A. Geometric Primitives

#### 2D Points dan Lines

```
2D Point: x = (x, y) ∈ ℝ²
Homogeneous: x̃ = (x̃, ỹ, w̃) ∈ ℙ² 

2D Line: ãx + b̃y + c̃ = 0
Normal form: n̂·x + d = 0  where ||n̂|| = 1
```

#### 3D Points dan Planes

```
3D Point: x = (x, y, z) ∈ ℝ³
Homogeneous: x̃ = (x̃, ỹ, z̃, w̃) ∈ ℙ³

3D Plane: ãx + b̃y + c̃z + d̃ = 0  
Normal form: n̂·x + d = 0
```

### B. Hierarchy of 2D Transformations

| Transformasi          | Matrix         | DOF | Preserves                                 | Aplikasi                 |
| --------------------- | -------------- | --- | ----------------------------------------- | ------------------------ |
| **Translation** | `[I t]`      | 2   | Orientation, lengths, angles, parallelism | Simple shifts            |
| **Euclidean**   | `[R t]`      | 3   | Lengths, angles, parallelism              | Rigid body motion        |
| **Similarity**  | `[sR t]`     | 4   | Angles, parallelism                       | Scaled rotation          |
| **Affine**      | `[A t]`      | 6   | Parallelism, ratios                       | General linear transform |
| **Projective**  | `H̃ (3×3)` | 8   | Straight lines                            | Perspective effects      |

#### Transformasi Matrix Forms:

**Translation:**

```
M = | 1  0  tx |
    | 0  1  ty |
```

**Rotation:**

```
M = | cos θ  -sin θ  0 |
    | sin θ   cos θ  0 |
```

**Scaling:**

```
M = | sx  0   0 |
    | 0   sy  0 |
```

**Affine Transform:**

```
M = | a  b  tx |     | x' |   | ax + by + tx |
    | c  d  ty |  ×  | y' | = | cx + dy + ty |
                     | 1  |   |      1       |
```

**Perspective Transform (Homography):**

```
H = | h00  h01  h02 |     | x' |   | h00x + h01y + h02 |
    | h10  h11  h12 |  ×  | y' | = | h10x + h11y + h12 |
    | h20  h21  h22 |     | w' |   | h20x + h21y + h22 |

Final coordinates: x_final = x'/w', y_final = y'/w'
```

### C. 3D Transformations dan Rotations

#### 3D Rotation Representations:

**1. Rotation Matrix (3×3):**

```
R = | r11  r12  r13 |    Properties: RRᵀ = I, det(R) = 1
    | r21  r22  r23 |
    | r31  r32  r33 |
```

**2. Axis-Angle Representation:**

```
ω = θn̂  (3D vector)
R = I + sin θ [n̂]× + (1 - cos θ) [n̂]×²

[n̂]× = |  0   -nz   ny |  (skew-symmetric matrix)
        |  nz   0   -nx |
        | -ny   nx   0  |
```

**3. Unit Quaternions:**

```
q = (x, y, z, w) = (sin(θ/2)n̂, cos(θ/2))  where ||q|| = 1

Rotation matrix from quaternion:
R = | 1-2(y²+z²)    2(xy-zw)     2(xz+yw) |
    |  2(xy+zw)   1-2(x²+z²)    2(yz-xw) |
    |  2(xz-yw)    2(yz+xw)   1-2(x²+y²) |
```

---

## 3️⃣ Model Kamera dan Proyeksi

### A. Pinhole Camera Model

**Proyeksi Perspektif:**

```
Dunia 3D (X, Y, Z) → Gambar 2D (x, y)

x = f × (X/Z)
y = f × (Y/Z)

Dalam homogeneous coordinates:
| x |     | f  0  0  0 | | X |
| y |  =  | 0  f  0  0 | | Y |
| z |     | 0  0  1  0 | | Z |
                         | 1 |
```

### B. Camera Parameters

#### Intrinsic Parameters (Internal):

```
Camera Matrix K:
K = | fx   s   cx |
    |  0  fy   cy |
    |  0   0    1 |

fx, fy : focal length dalam pixel
cx, cy : principal point (image center)
s      : skew parameter (biasanya 0)
```

#### Extrinsic Parameters (External):

```
Rotasi:     R (3×3 matrix)
Translasi:  t (3×1 vector)

Combined: P = K[R|t]  (3×4 projection matrix)
```

### C. Lens Distortion

#### Radial Distortion:

```
Barrel distortion:  garis lurus melengkung keluar
Pincushion distortion: garis lurus melengkung dalam

Model koreksi:
x_corrected = x(1 + k1r² + k2r⁴ + k3r⁶)
y_corrected = y(1 + k1r² + k2r⁴ + k3r⁶)

where r² = x² + y²
```

#### Tangential Distortion:

```
x_corrected = x + [2p1xy + p2(r² + 2x²)]
y_corrected = y + [p1(r² + 2y²) + 2p2xy]
```

### D. Different Projection Models

#### 1. Orthographic Projection:

```
x = X (drop Z coordinate)
y = Y
Used for: telecentris lenses, distant objects
```

#### 2. Scaled Orthography:

```
x = s × X
y = s × Y
More practical for real applications
```

#### 3. Para-perspective:

```
Affine approximation to perspective:
x = a₀ + a₁X + a₂Y + a₆XY
y = a₃ + a₄X + a₅Y + a₇XY
```

#### 4. Full Perspective:

```
x = (h₀₀X + h₀₁Y + h₀₂Z + h₀₃)/(h₂₀X + h₂₁Y + h₂₂Z + h₂₃)
y = (h₁₀X + h₁₁Y + h₁₂Z + h₁₃)/(h₂₀X + h₂₁Y + h₂₂Z + h₂₃)
```

---

## 4️⃣ Photometric Image Formation

### A. Lighting dan Reflectance

#### BRDF (Bidirectional Reflectance Distribution Function):

```
BRDF: fr(θi, φi, θr, φr; λ)
- θi, φi: incident light direction
- θr, φr: reflected light direction  
- λ: wavelength

Radiance equation:
Lr(v̂r; λ) = ∫ Li(v̂i; λ) fr(v̂i, v̂r, n̂; λ) cos+ θi dv̂i
```

#### Phong Shading Model:

```
Lr(v̂r; λ) = ka(λ)La(λ) + kd(λ)∑Li(λ)[v̂i·n̂]+ + ks(λ)∑Li(λ)(v̂r·ŝi)^ke

Components:
- Ambient:  ka(λ)La(λ)
- Diffuse:  kd(λ)∑Li(λ)[v̂i·n̂]+  
- Specular: ks(λ)∑Li(λ)(v̂r·ŝi)^ke
```

### B. Optics dan Lens Effects

#### Thin Lens Model:

```
1/zo + 1/zi = 1/f

zo: object distance
zi: image distance  
f:  focal length
```

#### Depth of Field:

```
Circle of confusion: c = (d × Δzi)/zi
f-number: N = f/d

Depth of field ∝ N × (distance)²/f²
```

#### Vignetting (Natural):

```
Light fall-off: E = L × (π/4) × (d/f)² × cos⁴α

where α is off-axis angle
```

---

## 5️⃣ Digital Camera dan Sensor

### A. Image Sensor Pipeline

```
Scene → Optics → Sensor → ADC → ISP → Output
  ↓       ↓        ↓       ↓     ↓      ↓
3D     Lens    CCD/CMOS  Digital Raw  JPEG
Light  Focus   Array     Values Processing
```

### B. Color Filter Arrays (CFA)

#### Bayer Pattern:

```
R G R G R G    →  Demosaicing  →  Full RGB
G B G B G B                       Image  
R G R G R G     
G B G B G B     

50% Green, 25% Red, 25% Blue
(Green dominant for luminance sensitivity)
```

### C. Sampling dan Aliasing

#### Shannon Sampling Theorem:

```
Nyquist frequency: fN = fs/2
Anti-aliasing condition: fmax < fN

Sampling rate: fs ≥ 2 × fmax (minimum)
```

#### Point Spread Function (PSF):

```
PSF = Optical blur ⊗ Sensor area
MTF = |ℱ{PSF}|  (Modulation Transfer Function)

Aliasing occurs when high frequencies fold back
```

### D. Noise Model

#### Noise Sources:

```
- Shot noise: ∼ Poisson(I)
- Read noise: ∼ Gaussian(0, σr²)
- Dark current: ∼ Poisson(Id·t)
- Quantization: uniform over ±0.5 LSB

Total noise: σ²total = I + σ²r + Id·t + σ²q
```

---

## 6️⃣ Color dan Gamma

### A. Color Spaces

#### CIE XYZ Standard Observer:

```
| X |     | 0.4125  0.3576  0.1804 | | R |
| Y |  =  | 0.2127  0.7152  0.0721 | | G |  (sRGB → XYZ)
| Z |     | 0.0193  0.1192  0.9505 | | B |

Chromaticity: x = X/(X+Y+Z), y = Y/(X+Y+Z)
```

#### L*a*b* Perceptual Color:

```
L* = 116 f(Y/Yn) - 16
a* = 500 [f(X/Xn) - f(Y/Yn)]  
b* = 200 [f(Y/Yn) - f(Z/Zn)]

f(t) = t^(1/3)        if t > δ³
f(t) = t/(3δ²) + 4/29  otherwise
```

---

## 7️⃣ Pemetaan Materi ke Program Praktikum

Semua program menyimpan hasil ke `praktikum/output/outputN` sesuai nomor percobaan.

| Materi | Program | Output Folder | Hasil yang Diharapkan |
|---|---|---|---|
| Translasi 2D | `01_translasi.py` | output/output1 | Gambar bergeser + perbandingan border |
| Rotasi 2D | `02_rotasi.py` | output/output2 | Rotasi berbagai sudut & pusat |
| Scaling & Interpolasi | `03_scaling.py` | output/output3 | Perbandingan metode resize |
| Transformasi Affine | `04_affine_transform.py` | output/output4 | Shear/rotasi/kombinasi |
| Perspektif (Homography) | `05_perspektif_transform.py` | output/output5 | Bird’s eye view |
| Document Scanner | `06_document_scanner.py` | output/output6 | Deteksi + koreksi dokumen |
| Kalibrasi Kamera | `07_kalibrasi_kamera.py` | output/output7 | Intrinsik + undistort |
| Rotasi 3D (Axis-Angle) | `08_3d_rotation.py` | output/output8 | Kubus berotasi 3D |
| Proyeksi 3D ke 2D | `09_projection_perspective.py` | output/output9 | Ortografik vs perspektif |
| Distorsi Lensa | `10_lens_distortion.py` | output/output10 | Barrel/pincushion + koreksi |
| Sampling & Aliasing | `11_sampling_aliasing.py` | output/output11 | Downsample + anti-aliasing |
| Color Spaces | `12_color_spaces.py` | output/output12 | Channel RGB/HSV/LAB/YCrCb/XYZ |
| Gamma Correction | `13_gamma_correction.py` | output/output13 | Perbandingan linear vs gamma |
| Photometric Shading | `14_photometric_shading.py` | output/output14 | Diffuse vs specular |
| Artefak Kompresi | `15_compression_artifacts.py` | output/output15 | JPEG quality comparison |

---

## 8️⃣ Catatan Implementasi Nyata

1. **Transformasi 2D/3D** digunakan pada stabilisasi, AR, dan normalisasi data.
2. **Homography & document scanner** adalah contoh langsung aplikasi industri.
3. **Kalibrasi kamera & distorsi** penting untuk akurasi pengukuran 3D.
4. **Sampling, gamma, color space** memengaruhi kualitas input sebelum pemrosesan lanjutan.
5. **Kompresi** berdampak pada artefak yang sering muncul di dataset dunia nyata.

#### YUV/YCbCr Video:

```
Y'  = 0.299R' + 0.587G' + 0.114B'  (BT.601)
Y'  = 0.2126R' + 0.7152G' + 0.0722B' (BT.709)

Cb = (B' - Y')/1.772
Cr = (R' - Y')/1.402
```

### B. Gamma Correction

#### Gamma Encoding:

```
Display gamma: B = V^γ  (γ ≈ 2.2)
Camera gamma:  V' = V^(1/γ)  (γ ≈ 0.45)

sRGB standard:
V' = 12.92V           if V ≤ 0.0031308
V' = 1.055V^(1/2.4) - 0.055  otherwise
```

---

## 7️⃣ Kalibrasi Kamera

### A. Camera Calibration Process

#### Parameter Estimation:

```
Calibration target: checkerboard, circles, etc.
Multiple views: 10-20 images from different angles

Objective: minimize reprojection error
∑∑ ||xi,j - P(Xi,j))||²

P(X) = K[R|t]X  (projection function)
```

#### Zhang's Method:

```
1. Estimate homographies Hi from each view
2. Solve for intrinsic parameters K from Hi
3. Estimate extrinsic parameters [R|t] for each view  
4. Refine all parameters using bundle adjustment
```

### B. Distortion Correction

#### Brown-Conrady Model:

```
Radial distortion:
x' = x(1 + k1r² + k2r⁴ + k3r⁶)
y' = y(1 + k1r² + k2r⁴ + k3r⁶)

Tangential distortion:  
x' = x + [2p1xy + p2(r² + 2x²)]
y' = y + [p1(r² + 2y²) + 2p2xy]

where r² = x² + y²
```

---

## 8️⃣ Aplikasi Praktis

│                    PIPELINE PEMBENTUKAN CITRA                    │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│   ┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐ │
│   │  DUNIA   │    │  SISTEM  │    │  SENSOR  │    │  GAMBAR  │ │
│   │   3D     │───▶│  OPTIK   │───▶│  KAMERA  │───▶│   2D     │ │
│   └──────────┘    └──────────┘    └──────────┘    └──────────┘ │
│        │               │               │               │        │
│        │               │               │               │        │
│   ┌────▼────┐    ┌────▼────┐    ┌────▼────┐    ┌────▼────┐   │
│   │Geometri │    │Proyeksi │    │Sampling │    │Digital  │   │
│   │Objek    │    │Perspektif│    │Kuantisasi│    │Image   │   │
│   └─────────┘    └─────────┘    └─────────┘    └─────────┘   │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘

```

### Jenis-jenis Transformasi Geometri

```

┌────────────────┐   ┌────────────────┐   ┌────────────────┐
│   TRANSLASI    │   │    ROTASI      │   │    SCALING     │
│                │   │                │   │                │
│    □ → □       │   │    □           │   │    □    →  □   │
│        ↘       │   │     ↺         │   │              ↗  │
│         □      │   │       ◇       │   │               □ │
│                │   │                │   │               □ │
└────────────────┘   └────────────────┘   └────────────────┘

┌────────────────┐   ┌────────────────┐   ┌────────────────┐
│    SHEARING    │   │    AFFINE      │   │  PERSPECTIVE   │
│                │   │                │   │                │
│    □           │   │    □           │   │    □           │
│    ▱           │   │      ◇        │   │      ⬡        │
│                │   │                │   │                │
└────────────────┘   └────────────────┘   └────────────────┘

```

---

## 4️⃣ Contoh Kasus di Industri

### 1. 🚗 Autonomous Vehicle - Tesla Autopilot

**Masalah**: Kamera kendaraan otonom perlu memahami jarak dan posisi objek di jalan.

**Solusi Image Formation**:
- Menggunakan multiple kamera dengan kalibrasi presisi
- Menggabungkan data dari berbagai sudut pandang
- Transformasi perspektif untuk bird's-eye view
- Estimasi kedalaman dari stereo vision

**Hasil**: Sistem dapat mendeteksi pedestrian, kendaraan, dan marka jalan dengan akurat.

### 2. 📱 Google Maps - Street View

**Masalah**: Membangun panorama 360° dari multiple kamera.

**Solusi Image Formation**:
- Kalibrasi presisi untuk array 15+ kamera
- Koreksi distorsi lensa fisheye
- Image stitching dengan homography
- Color correction dan blending

**Hasil**: Panorama seamless yang dapat dijelajahi secara virtual.

### 3. 🏭 Automated Optical Inspection (AOI)

**Masalah**: Inspeksi PCB untuk mendeteksi defect pada produksi elektronik.

**Solusi Image Formation**:
- Telecentric lens untuk menghilangkan distorsi perspektif
- Precise calibration untuk pengukuran akurat
- Multi-angle imaging untuk mendeteksi 3D defects
- Sub-pixel measurement accuracy

**Hasil**: Deteksi defect hingga 10 mikrometer.

### 4. 📸 Adobe Photoshop - Perspective Correction

**Masalah**: Foto bangunan seringkali memiliki garis yang tidak lurus (converging verticals).

**Solusi Image Formation**:
- Automatic perspective detection
- Homography estimation dari vanishing points
- Perspective transform untuk koreksi

**Hasil**: Foto arsitektur dengan garis vertikal yang lurus.

### 5. 🎮 Augmented Reality - Pokemon GO

**Masalah**: Menempatkan objek virtual pada dunia nyata.

**Solusi Image Formation**:
- Real-time camera pose estimation
- Plane detection untuk penempatan objek
- Perspective-correct rendering

**Hasil**: Pokemon yang tampak "ada" di dunia nyata melalui kamera smartphone.

---

## 5️⃣ Ringkasan

### Poin Penting yang Harus Diingat:

| Konsep | Deskripsi | Fungsi OpenCV |
|--------|-----------|---------------|
| Kamera Pinhole | Model dasar proyeksi 3D→2D | - |
| Intrinsic Matrix | Parameter internal kamera | `cv2.calibrateCamera()` |
| Extrinsic Matrix | Posisi & orientasi kamera | `cv2.solvePnP()` |
| Transformasi Affine | Kombinasi transformasi linear | `cv2.warpAffine()` |
| Transformasi Perspektif | Proyeksi antar bidang | `cv2.warpPerspective()` |
| Koreksi Distorsi | Menghilangkan distorsi lensa | `cv2.undistort()` |
| Homography | Mapping antar bidang | `cv2.findHomography()` |

### Flowchart Penggunaan:

```

┌─────────────────────────────────────────────────────────────┐
│              KAPAN MENGGUNAKAN TRANSFORMASI?                 │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  Rotasi gambar? ────────────────▶ cv2.getRotationMatrix2D() │
│                                   cv2.warpAffine()           │
│                                                              │
│  Resize gambar? ────────────────▶ cv2.resize()              │
│                                                              │
│  Crop perspektif? ──────────────▶ cv2.getPerspectiveTransform()
│  (misal: dokumen)                 cv2.warpPerspective()     │
│                                                              │
│  Kalibrasi kamera? ─────────────▶ cv2.calibrateCamera()     │
│                                   cv2.undistort()            │
│                                                              │
│  Menggabungkan gambar? ─────────▶ cv2.findHomography()      │
│  (panorama)                       cv2.warpPerspective()     │
│                                                              │
└─────────────────────────────────────────────────────────────┘

```

---

## 6️⃣ Interpolasi dan Resampling dalam Resize

### Metode-metode Interpolasi:

| Metode | Kompleksitas | Kecepatan | Kualitas | Kasus Penggunaan |
|--------|-------------|----------|----------|-----------------|
| **INTER_NEAREST** | Rendah | ⚡⚡⚡ | ⭐⭐ | Pixel art, mask, real-time |
| **INTER_LINEAR** | Sedang | ⚡⚡ | ⭐⭐⭐ | General purpose, balanced |
| **INTER_CUBIC** | Tinggi | ⚡ | ⭐⭐⭐⭐ | Photo upscaling |
| **INTER_LANCZOS4** | Sangat Tinggi | 🐢 | ⭐⭐⭐⭐⭐ | Highest quality upscaling |
| **INTER_AREA** | Sedang | ⚡⚡ | ⭐⭐⭐⭐ | Downscaling (resampling) |

### Visualisasi Perbedaan Interpolasi:

```
UPSCALING (2×):
Original (4 pixel)         Hasil (16 pixel)
┌─┬─┐                    ┌───┬───┬───┬───┐
│A│B│  NEAREST           │A A│B B│   │   │
├─┼─┤        ──────►     ├───┼───┼───┼───┤
│C│D│                    │A A│B B│   │   │
└─┴─┘                    ├───┼───┼───┼───┤
                         │C C│D D│   │   │
                         ├───┼───┼───┼───┤
                         │C C│D D│   │   │
                         └───┴───┴───┴───┘
                         (Pixelated, blocky)

┌─┬─┐                    ┌───┬───┬───┬───┐
│A│B│  LINEAR            │A..|.B|   │   │
├─┼─┤        ──────►     ├───┼───┼───┼───┤
│C│D│                    │.AC|.D|   │   │
└─┴─┘                    ├───┼───┼───┼───┤
                         │...|...|   │   │
                         ├───┼───┼───┼───┤
                         │...|...|   │   │
                         └───┴───┴───┴───┘
                         (Smooth, interpolated)

DOWNSCALING (0.5×):
Original 4×4              Hasil 2×2
┌─┬─┬─┬─┐              ┌───────┐
│A│A│B│B│    AREA      │A+A+C+C│
├─┼─┼─┼─┤    ──────►   ├───────┤
│A│A│B│B│    (avg)     │B+B+D+D│
├─┼─┼─┼─┤              └───────┘
│C│C│D│D│              (Best for downscaling)
├─┼─┼─┼─┤
│C│C│D│D│
└─┴─┴─┴─┘
```

---

## 7️⃣ Deteksi Kontur dan Contour Properties

### Fungsi Deteksi Kontur:

```python
# Menemukan kontur dalam gambar biner
contours, hierarchy = cv2.findContours(
    binary_image,
    cv2.RETR_TREE,           # Retrieval mode
    cv2.CHAIN_APPROX_SIMPLE  # Approximation method
)

# Retrieval modes:
# - RETR_EXTERNAL: hanya kontur eksternal
# - RETR_LIST: semua kontur tanpa hierarki
# - RETR_TREE: semua kontur dengan hierarki
# - RETR_CCOMP: 2-level hierarki

# Approximation methods:
# - CHAIN_APPROX_NONE: semua titik kontur
# - CHAIN_APPROX_SIMPLE: kompresi titik kolinear
```

### Properties Kontur:

```python
# Luas
area = cv2.contourArea(contour)

# Perimeter
perimeter = cv2.arcLength(contour, closed=True)

# Bounding rectangle
x, y, w, h = cv2.boundingRect(contour)

# Minimum enclosing circle
(cx, cy), radius = cv2.minEnclosingCircle(contour)

# Fitting polygon (Douglas-Peucker)
epsilon = 0.02 * cv2.arcLength(contour, True)
approx = cv2.approxPolyDP(contour, epsilon, True)

# Convex hull
hull = cv2.convexHull(contour)

# Moments (pusat massa)
M = cv2.moments(contour)
cx = int(M['m10'] / M['m00'])
cy = int(M['m01'] / M['m00'])
```

---

## 8️⃣ Tugas Video

### 📹 Deskripsi Tugas Video

**Judul**: "Transformasi Geometri dan Kalibrasi Kamera dalam Computer Vision"

**Durasi**: 5-7 menit

**Format**: Tutorial/Demonstrasi dengan penjelasan

### Checklist Konten Video:

#### A. Pembukaan (30-60 detik)
- [ ] Perkenalan diri
- [ ] Menjelaskan topik yang akan dibahas
- [ ] Menyebutkan tujuan pembelajaran

#### B. Penjelasan Teori (1-2 menit)
- [ ] Menjelaskan apa itu Image Formation
- [ ] Membedakan transformasi Affine vs Perspektif
- [ ] Menjelaskan kapan menggunakan masing-masing

#### C. Demonstrasi Praktis (2-3 menit)
- [ ] Demo rotasi gambar dengan berbagai sudut
- [ ] Demo resize dengan interpolasi berbeda
- [ ] Demo perspective transform (contoh: document scanner)
- [ ] Menunjukkan kode dan hasilnya

#### D. Contoh Aplikasi Nyata (1 menit)
- [ ] Menjelaskan salah satu use case industri
- [ ] Menghubungkan dengan program yang dibuat

#### E. Penutup (30-60 detik)
- [ ] Ringkasan poin penting
- [ ] Saran untuk eksplorasi lebih lanjut
- [ ] Call-to-action (coba sendiri!)

### Poin Penilaian Video:

| Aspek | Bobot | Deskripsi |
|-------|-------|-----------|
| Kejelasan Penjelasan | 25% | Mudah dipahami, tidak membingungkan |
| Kebenaran Materi | 25% | Informasi akurat dan relevan |
| Kualitas Demonstrasi | 25% | Kode berjalan, hasil ditunjukkan |
| Kualitas Produksi | 15% | Audio jelas, video tidak blur |
| Kreativitas | 10% | Contoh menarik, penyampaian engaging |

### Tips Membuat Video:

1. **Persiapan**: Test semua kode sebelum merekam
2. **Rekam layar**: Gunakan OBS Studio atau Loom
3. **Narasi**: Jelaskan apa yang sedang dilakukan
4. **Edit**: Potong bagian yang tidak perlu
5. **Review**: Tonton ulang sebelum submit

---

## 📚 Referensi

1. Szeliski, R. (2022). Computer Vision: Algorithms and Applications, 2nd Edition. Chapter 2.
2. Hartley, R. & Zisserman, A. (2003). Multiple View Geometry in Computer Vision.
3. OpenCV Documentation: Camera Calibration and 3D Reconstruction
4. Zhang, Z. (2000). A Flexible New Technique for Camera Calibration.

---

*Bab ini adalah bagian dari Praktikum Computer Vision berdasarkan buku "Computer Vision: Algorithms and Applications" oleh Richard Szeliski.*
```
