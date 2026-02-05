# Prompt Notebook LLM – Bab 2 (Slide 01–15)

> **Tujuan**: Membuat 30 slide pembelajaran Bab 2 (Image Formation). Dokumen ini berisi prompt detail untuk **Slide 01–15**.
>
> - **Format**: Setiap prompt meminta slide penuh grafis + rumus + penerapan coding.
> - **Sumber**: Materi Bab 2 (Szeliski, 2021) + praktikum 01–15.
> - **Output visual**: Gunakan diagram, tabel, dan contoh output dari `praktikum/output/outputN`.

---

## Slide 01 — Cover & Peta Bab 2

**Prompt**:
Buat slide cover yang menampilkan judul besar **“Bab 2: Pembentukan Citra (Image Formation)”**. Tambahkan subjudul: “Geometri, Proyeksi, Fotometri, Sampling, Warna, Kompresi”. Sertakan ilustrasi pipeline besar dari 3D Scene → Light → Lens → Sensor → Digital Image. Pakai ikon kamera dan diagram sederhana. Di bawah judul tulis: “Praktikum Computer Vision”.

**Grafis**: Diagram pipeline 5–6 tahap (ikon cahaya, lensa, sensor, matriks).

---

## Slide 02 — Ringkasan Struktur Bab 2 (30.000 ft view)

**Prompt**:
Buat slide ringkasan struktur bab: **(1) Geometric primitives & transformations**, **(2) 3D rotations & projection**, **(3) Lens distortion**, **(4) Photometric image formation**, **(5) Digital camera pipeline**, **(6) Sampling & aliasing**, **(7) Color & gamma**, **(8) Compression**. Sertakan flowchart horizontal 8 blok. Tambahkan satu kalimat manfaat praktis setiap blok (mis. “Homography → document scanner”).

**Grafis**: Flowchart 8 blok, ikon per blok.

---

## Slide 03 — Geometric Primitives (2D & 3D)

**Prompt**:
Jelaskan representasi titik 2D/3D dalam koordinat homogen.

- Rumus: 2D point $\tilde{x}=(\tilde{x},\tilde{y},\tilde{w})$, 3D point $\tilde{x}=(\tilde{x},\tilde{y},\tilde{z},\tilde{w})$.
- Garis 2D: $\tilde{l}=(a,b,c)$, persamaan $ax+by+c=0$.
- Plane 3D: $\tilde{m}=(a,b,c,d)$, persamaan $ax+by+cz+d=0$.
  Sertakan diagram 2D garis dengan normal, dan 3D bidang dengan normal.

**Grafis**: Skema titik–garis–bidang + normal vector.

---

## Slide 04 — Hierarki Transformasi 2D

**Prompt**:
Buat slide tabel hierarki transformasi 2D: Translation (2 DOF), Euclidean (3 DOF), Similarity (4 DOF), Affine (6 DOF), Projective (8 DOF). Tambahkan properti yang dipertahankan (panjang, sudut, paralel). Sertakan rumus matrix umum:

- Translasi $\begin{bmatrix}1&0&tx\\0&1&ty\end{bmatrix}$
- Affine $\begin{bmatrix}a&b&tx\\c&d&ty\end{bmatrix}$
- Homography $H$ 3×3.

**Grafis**: Tabel + ikon transformasi.

---

## Slide 05 — Percobaan 1: Translasi (01_translasi.py)

**Prompt**:
Buat slide praktikum translasi. Sertakan:

- Rumus translasi 2D (matrix 2×3).
- Penjelasan parameter `tx, ty`.
- Potongan pseudocode: `M = [[1,0,tx],[0,1,ty]]` → `cv2.warpAffine()`.
- Contoh hasil (ambil gambar dari `praktikum/output/output1`).
  Tambahkan tips border mode: CONSTANT, REPLICATE, REFLECT.

**Grafis**: Gambar sebelum–sesudah translasi + tabel border mode.

---

## Slide 06 — Percobaan 2: Rotasi (02_rotasi.py)

**Prompt**:
Jelaskan rotasi 2D dan masalah “terpotong”. Sertakan rumus rotasi 2D dan formula bounding box baru:
$\text{newW}=H|\sin\theta|+W|\cos\theta|$, $\text{newH}=H|\cos\theta|+W|\sin\theta|$.
Tampilkan output `output/output2`. Jelaskan `cv2.getRotationMatrix2D(center, angle, scale)`.

**Grafis**: Diagram rotasi + contoh output.

---

## Slide 07 — Percobaan 3: Scaling & Interpolasi (03_scaling.py)

**Prompt**:
Buat slide perbandingan interpolasi (NEAREST, LINEAR, CUBIC, LANCZOS4, AREA). Sertakan tabel kualitas vs kecepatan. Tampilkan contoh output `output/output3`. Tambahkan rumus ide umum interpolasi (tetangga terdekat vs rata-rata tetangga). Jelaskan kapan gunakan AREA (downsampling).

**Grafis**: 5 panel hasil resize.

---

## Slide 08 — Transformasi Affine (Teori)

**Prompt**:
Jelaskan affine transform: 6 DOF, butuh 3 pasang titik. Rumus matrix 2×3 dan bentuk persamaan $x' = ax + by + tx$, $y' = cx + dy + ty$. Jelaskan properti: garis lurus & paralel tetap paralel. Sertakan diagram 3 titik sumber → 3 titik tujuan.

**Grafis**: Diagram 3 titik + matrix.

---

## Slide 09 — Percobaan 4: Affine Transform (04_affine_transform.py)

**Prompt**:
Tampilkan penerapan affine: shear horizontal/vertical, kombinasi transformasi. Sertakan snippet fungsi `cv2.getAffineTransform(pts_src, pts_dst)` dan `cv2.warpAffine`. Masukkan output `output/output4`. Tambahkan ringkas: “Affine dipakai untuk alignment, koreksi skew, augmentasi data”.

**Grafis**: Kolase hasil affine.

---

## Slide 10 — Transformasi Perspektif/Homography (Teori)

**Prompt**:
Jelaskan homography 3×3 dengan 8 DOF. Rumus:
$\tilde{x}' = H\tilde{x}$, lalu $x' = x/w$, $y' = y/w$.
Jelaskan bahwa butuh 4 titik korespondensi. Tampilkan perbandingan affine vs perspective (paralel bisa konvergen). Sertakan diagram trapezoid → rectangle.

**Grafis**: Diagram perspektif + matrix.

---

## Slide 11 — Percobaan 5: Perspektif (05_perspektif_transform.py)

**Prompt**:
Tampilkan aplikasi koreksi perspektif. Jelaskan `cv2.getPerspectiveTransform()` dan `cv2.warpPerspective()`. Tampilkan output `output/output5`. Tambahkan catatan ordering titik: TL, TR, BR, BL.

**Grafis**: Before–after bird’s‑eye view.

---

## Slide 12 — Percobaan 6: Document Scanner (06_document_scanner.py)

**Prompt**:
Buat slide pipeline scanner dokumen: Grayscale → Blur → Canny → Contours → 4-point → Warp → Enhancement. Jelaskan fungsi kunci: `cv2.Canny`, `cv2.findContours`, `cv2.approxPolyDP`, `cv2.warpPerspective`, `cv2.adaptiveThreshold`. Tampilkan output `output/output6`.

**Grafis**: Pipeline step-by-step.

---

## Slide 13 — Model Kamera Pinhole & Intrinsic

**Prompt**:
Jelaskan model pinhole: $x = fX/Z$, $y = fY/Z$. Tunjukkan matrix intrinsik $K$:
$\begin{bmatrix}fx & s & cx\\0 & fy & cy\\0&0&1\end{bmatrix}$.
Tambahkan pengertian extrinsic $[R|t]$, dan camera matrix $P = K[R|t]$.

**Grafis**: Diagram kamera pinhole, sinar, dan sensor.

---

## Slide 14 — Percobaan 7: Kalibrasi Kamera (07_kalibrasi_kamera.py)

**Prompt**:
Tampilkan proses kalibrasi checkerboard: deteksi corner → `calibrateCamera` → distorsi → `undistort`. Sertakan contoh parameter intrinsik dan koefisien distorsi. Tampilkan output `output/output7`. Tambahkan “RMS reprojection error < 1 px → baik”.

**Grafis**: Gambar checkerboard + hasil undistort.

---

## Slide 15 — Distorsi Lensa (Teori Ringkas)

**Prompt**:
Jelaskan distorsi radial dengan rumus:
$x_c = x(1 + k_1 r^2 + k_2 r^4 + k_3 r^6)$,
$y_c = y(1 + k_1 r^2 + k_2 r^4 + k_3 r^6)$.
Sertakan gambar barrel vs pincushion. Hubungkan ke koreksi pada kalibrasi kamera.

**Grafis**: Grid lurus → barrel → pincushion.
