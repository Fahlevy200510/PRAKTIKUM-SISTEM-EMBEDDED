# Prompt Notebook LLM – Bab 2 (Slide 1–15)

> **Tujuan**: Membuat 15 slide pembelajaran Bab 2 (Image Formation). Dokumen ini berisi prompt detail untuk **Slide 1–15**.
> - **Format**: Setiap prompt meminta slide penuh grafis + rumus + penerapan coding.
> - **Sumber**: Materi Bab 2 (Szeliski, 2021) + praktikum 01–15.
> - **Output visual**: Gunakan diagram, tabel, dan contoh output dari `praktikum/output/outputN`.

---

## Slide 1 — Percobaan 8: Rotasi 3D (08_3d_rotation.py)
**Prompt**:
Jelaskan rotasi 3D dengan axis-angle dan Rodrigues:
$R = I + \sin\theta [\hat{n}]_\times + (1-\cos\theta)[\hat{n}]_\times^2$.
Tunjukkan visual kubus sebelum–sesudah rotasi. Jelaskan normalisasi axis. Tampilkan output `output/output8`.

**Grafis**: Kubus wireframe + vektor axis.

---

## Slide 2 — 3D Rotations: Axis/Angle vs Quaternion
**Prompt**:
Buat slide perbandingan ringkas axis-angle vs quaternion. Sertakan formula quaternion:
$q=(\sin(\theta/2)\hat{n}, \cos(\theta/2))$ dan catatan $\|q\|=1$.
Jelaskan kelebihan: quaternion stabil untuk interpolasi. Tambahkan diagram sphere unit quaternion.

**Grafis**: Sphere unit + panah rotasi.

---

## Slide 3 — 3D→2D Projection (Teori)
**Prompt**:
Bandingkan ortografik vs perspektif. Sertakan rumus:
- Orthographic: $x = X, y = Y$
- Perspective: $x=fX/Z, y=fY/Z$
Tunjukkan bagaimana objek jauh tampak lebih kecil pada perspektif.

**Grafis**: Diagram kamera + dua hasil proyeksi.

---

## Slide 4 — Percobaan 9: Proyeksi 3D (09_projection_perspective.py)
**Prompt**:
Tampilkan hasil proyeksi ortografik, scaled-ortho, dan perspektif. Jelaskan peran `fx, fy, cx, cy` pada matrix K. Tampilkan output `output/output9`.

**Grafis**: 3 panel hasil proyeksi.

---

## Slide 5 — Percobaan 10: Distorsi Lensa (10_lens_distortion.py)
**Prompt**:
Tampilkan simulasi distorsi grid dan hasil koreksi dengan `cv2.undistort`. Jelaskan tanda k1 dan efek barrel/pincushion. Tampilkan output `output/output10`.

**Grafis**: Grid original, distorted, corrected.

---

## Slide 6 — Photometric Image Formation (BRDF)
**Prompt**:
Jelaskan konsep BRDF: $f_r(\theta_i, \phi_i, \theta_r, \phi_r;\lambda)$. Tulis persamaan radiance:
$L_r(\hat{v}_r)=\int L_i(\hat{v}_i) f_r(\hat{v}_i,\hat{v}_r,\hat{n}) \cos^+\theta_i \, d\hat{v}_i$.
Sertakan diagram incoming light, normal, outgoing light.

**Grafis**: Diagram BRDF 3D.

---

## Slide 7 — Lambertian & Phong Shading
**Prompt**:
Jelaskan komponen Phong:
$L = k_a L_a + k_d \sum L_i[\hat{v}_i\cdot\hat{n}]^+ + k_s \sum L_i(\hat{v}_r\cdot\hat{s}_i)^{k_e}$.
Tampilkan ilustrasi diffuse vs specular.

**Grafis**: Bola diffuse vs specular.

---

## Slide 8 — Percobaan 14: Photometric Shading (14_photometric_shading.py)
**Prompt**:
Tampilkan hasil eksperimen diffuse, specular, combined. Jelaskan efek `shininess` terhadap ukuran highlight. Tampilkan output `output/output14`.

**Grafis**: 3 panel hasil.

---

## Slide 9 — Digital Camera Pipeline
**Prompt**:
Buat diagram pipeline sensor digital: light → lens → sensor (CCD/CMOS) → ADC → ISP → JPEG. Jelaskan ringkas peran exposure, gain (ISO), dan noise.

**Grafis**: Flowchart pipeline kamera.

---

## Slide 10 — Sampling & Aliasing (Teori)
**Prompt**:
Jelaskan Nyquist: $f_s \ge 2 f_{max}$. Beri contoh aliasing pada pattern garis. Sertakan diagram spektrum aliasing sederhana.

**Grafis**: Sine wave sampling & alias.

---

## Slide 11 — Percobaan 11: Sampling & Aliasing (11_sampling_aliasing.py)
**Prompt**:
Tampilkan perbandingan downsampling: NEAREST vs INTER_AREA vs Blur+Nearest. Jelaskan mengapa blur mengurangi aliasing. Tampilkan output `output/output11`.

**Grafis**: 4 panel hasil.

---

## Slide 12 — Color Spaces (Teori)
**Prompt**:
Jelaskan RGB, HSV, LAB, YCrCb, XYZ. Sertakan matriks transform sRGB→XYZ. Jelaskan kapan memakai HSV/LAB (segmentasi warna, perceptual).

**Grafis**: Diagram kubus RGB + roda HSV.

---

## Slide 13 — Percobaan 12: Color Spaces (12_color_spaces.py)
**Prompt**:
Tampilkan visual channel RGB/HSV/LAB/YCrCb/XYZ. Jelaskan apa arti channel H,S,V dan L,a,b. Tampilkan output `output/output12`.

**Grafis**: Grid channel.

---

## Slide 14 — Gamma Correction (Teori & Praktik)
**Prompt**:
Jelaskan gamma: $Y' = Y^{1/\gamma}$ dan decode $Y = (Y')^{\gamma}$. Jelaskan kenapa gamma penting untuk display dan kompresi. Tampilkan output `output/output13`.

**Grafis**: Kurva gamma + ramp.

---

## Slide 15 — Compression & Artefak JPEG (15_compression_artifacts.py)
**Prompt**:
Jelaskan kompresi JPEG (DCT + quantization) dan artefak blockiness. Tampilkan perbandingan kualitas Q=95/70/40. Tampilkan output `output/output15`. Tutup dengan ringkasan: “kualitas turun → detail hilang”.

**Grafis**: 4 panel hasil JPEG.
