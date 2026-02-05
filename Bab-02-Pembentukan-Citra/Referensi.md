# Referensi dan Sumber Belajar
## Bab 2: Pembentukan Citra (Image Formation)

---

## 📚 Buku Referensi Utama

### 1. Computer Vision: Algorithms and Applications, 2nd Edition
- **Penulis**: Richard Szeliski
- **Bab Terkait**: Chapter 2 - Image Formation
- **Link**: [szeliski.org/Book](https://szeliski.org/Book/)
- **Topik yang Dibahas**:
  - Geometric primitives and transformations
  - 2D transformations (translation, rotation, scaling)
  - 3D transformations and projections
  - Camera models (pinhole, lens)
  - Lens distortion models

### 2. Multiple View Geometry in Computer Vision, 2nd Edition
- **Penulis**: Richard Hartley & Andrew Zisserman
- **Publisher**: Cambridge University Press, 2004
- **ISBN**: 978-0521540513
- **Topik Relevan**:
  - Projective geometry fundamentals
  - Camera models and calibration
  - Homogeneous coordinates
  - The fundamental and essential matrices

### 3. Learning OpenCV 4
- **Penulis**: Adrian Kaehler & Gary Bradski
- **Publisher**: O'Reilly Media, 2023
- **ISBN**: 978-1492055990
- **Bab Relevan**: Chapter 11 - General Image Transforms

---

## 📖 Dokumentasi Resmi

### OpenCV Documentation
1. **Geometric Transformations**
   - [cv2.warpAffine](https://docs.opencv.org/4.x/da/d54/group__imgproc__transform.html#ga0203d9ee5fcd28d40dbc4a1ea4451983)
   - [cv2.warpPerspective](https://docs.opencv.org/4.x/da/d54/group__imgproc__transform.html#gaf73673a7e8e18ec6963e3774e6a94b87)
   - [cv2.resize](https://docs.opencv.org/4.x/da/d54/group__imgproc__transform.html#ga47a974309e9102f5f08231edc7e7529d)

2. **Camera Calibration**
   - [Camera Calibration Tutorial](https://docs.opencv.org/4.x/dc/dbb/tutorial_py_calibration.html)
   - [cv2.calibrateCamera](https://docs.opencv.org/4.x/d9/d0c/group__calib3d.html#ga687a1ab946686f0d85ae0363b5af1d7b)
   - [cv2.undistort](https://docs.opencv.org/4.x/d9/d0c/group__calib3d.html#ga69f2545a8b62a6b0fc2ee060dc30559d)

3. **Homography & Perspective**
   - [cv2.getPerspectiveTransform](https://docs.opencv.org/4.x/da/d54/group__imgproc__transform.html#ga20f62aa3235d869c9956436c870893ae)
   - [cv2.findHomography](https://docs.opencv.org/4.x/d9/d0c/group__calib3d.html#ga4abc2ece9fab9398f2e560d53c8c9780)

4. **Color Spaces & Gamma**
   - [Color Space Conversions](https://docs.opencv.org/4.x/d8/d01/group__imgproc__color__conversions.html)
   - [Gamma Correction (OpenCV FAQ)](https://docs.opencv.org/4.x/d4/d13/tutorial_py_filtering.html)

5. **Image Compression (JPEG)**
   - [Image Codecs and IO](https://docs.opencv.org/4.x/d4/da8/group__imgcodecs.html)

### NumPy Documentation
- [Linear Algebra](https://numpy.org/doc/stable/reference/routines.linalg.html)
- [Matrix Operations](https://numpy.org/doc/stable/reference/generated/numpy.matmul.html)
- [Random Sampling](https://numpy.org/doc/stable/reference/random/index.html)

---

## 🎓 Tutorial dan Course Online

### Video Tutorials

1. **First Principles of Computer Vision (Columbia University)**
   - YouTube Channel: [First Principles of Computer Vision](https://www.youtube.com/c/FirstPrinciplesofComputerVision)
   - Playlist Relevan: "Camera and Imaging"
   - Topik: Pinhole camera, lens distortion, camera calibration

2. **OpenCV Python Tutorial (freeCodeCamp)**
   - [OpenCV Course - Full Tutorial with Python](https://www.youtube.com/watch?v=oXlwWbU8l2o)
   - Durasi: 4 jam
   - Topik: Transformations, perspective correction

3. **PyImageSearch**
   - [4 Point OpenCV getPerspective Transform](https://pyimagesearch.com/2014/08/25/4-point-opencv-getperspective-transform-example/)
   - [OpenCV Document Scanner](https://pyimagesearch.com/2014/09/01/build-kick-ass-mobile-document-scanner-just-5-minutes/)
   - Topik: Practical document scanner implementation

4. **Sampling & Aliasing (Signal Processing)**
   - [Sampling Theorem (MIT OCW)](https://ocw.mit.edu/courses/6-003-signals-and-systems-fall-2011/)

### Online Courses

1. **Coursera - Robotics: Perception (UPenn)**
   - [Course Link](https://www.coursera.org/learn/robotics-perception)
   - Topik: Camera models, calibration, 3D vision

2. **Udacity - Introduction to Computer Vision**
   - [Course Link](https://www.udacity.com/course/introduction-to-computer-vision--ud810)
   - Topik: Image formation, camera projections

3. **Stanford - Digital Photography (Levoy)**
   - [Course Link](https://graphics.stanford.edu/courses/cs178/)
   - Topik: Sampling, color, gamma, compression

---

## 📝 Artikel dan Paper Ilmiah

### Fundamental Papers

1. **Camera Calibration**
   - Zhang, Z. (2000). "A Flexible New Technique for Camera Calibration"
   - IEEE Transactions on Pattern Analysis and Machine Intelligence
   - [Link PDF](https://www.microsoft.com/en-us/research/wp-content/uploads/2016/02/tr98-71.pdf)

2. **Homography Estimation**
   - Hartley, R. (1997). "In Defense of the Eight-Point Algorithm"
   - IEEE Transactions on PAMI

3. **RANSAC Algorithm**
   - Fischler, M.A. & Bolles, R.C. (1981). "Random Sample Consensus"
   - Communications of the ACM

### Review Articles

1. **A Survey on Image Transformation Techniques**
   - Topik: Comprehensive review of transformation techniques
   - Keywords: affine, projective, non-rigid transformations

---

## 🔧 Kode dan Repository

### GitHub Repositories

1. **OpenCV Samples**
   - [OpenCV Python Samples](https://github.com/opencv/opencv/tree/master/samples/python)
   - File relevan: `calibrate.py`, `plane_ar.py`

2. **LearnOpenCV**
   - [LearnOpenCV Repository](https://github.com/spmallick/learnopencv)
   - Contoh: Camera Calibration, Homography

3. **PyImageSearch Code**
   - [imutils library](https://github.com/PyImageSearch/imutils)
   - Fungsi berguna: `perspective.four_point_transform()`

### Colab Notebooks

1. **Camera Calibration Tutorial**
   - [Google Colab Link](https://colab.research.google.com/github/tensorflow/docs/blob/master/site/en/tutorials/images/camera_calibration.ipynb)

---

## 🛠️ Tools dan Software

### Calibration Tools

1. **GML Camera Calibration Toolbox**
   - GUI-based camera calibration
   - [Website](http://graphics.cs.msu.ru/en/node/909)

2. **Kalibr**
   - Camera and IMU calibration
   - [GitHub](https://github.com/ethz-asl/kalibr)

3. **Camera Calibration Toolbox for MATLAB**
   - Jean-Yves Bouguet's toolbox
   - [Website](http://www.vision.caltech.edu/bouguetj/calib_doc/)

### Online Tools

1. **Camera Calibration Pattern Generator**
   - [calib.io Pattern Generator](https://calib.io/pages/camera-calibration-pattern-generator)
   - Generate checkerboard, circles grid patterns

2. **OpenCV Camera Calibration Web App**
   - Browser-based calibration tool

---

## 🌐 Komunitas dan Forum

### Stack Overflow
- Tag: `opencv`, `camera-calibration`, `perspective-transform`
- [OpenCV Questions](https://stackoverflow.com/questions/tagged/opencv)

### Reddit
- r/computervision
- r/opencv

### OpenCV Forum
- [OpenCV Answers](https://answers.opencv.org/)
- Kategori: Camera Calibration, Image Transform

---

## 📌 Cheat Sheet: Transformation Matrix

### 2D Transformations

```
Translation:      Rotation:           Scaling:
| 1  0  tx |      | cos(θ) -sin(θ) |  | sx  0  |
| 0  1  ty |      | sin(θ)  cos(θ) |  | 0   sy |

Affine (6 DOF):                    Perspective (8 DOF):
| a  b  tx |                       | h00  h01  h02 |
| c  d  ty |                       | h10  h11  h12 |
                                   | h20  h21  h22 |
```

### Quick Reference Functions

| Transformasi | OpenCV Function | Points Required |
|-------------|-----------------|-----------------|
| Translation | warpAffine | - |
| Rotation | getRotationMatrix2D → warpAffine | 1 (center) |
| Scaling | resize | - |
| Affine | getAffineTransform → warpAffine | 3 |
| Perspective | getPerspectiveTransform → warpPerspective | 4 |
| Homography | findHomography → warpPerspective | ≥4 |

---

## 📅 Update History

| Tanggal | Versi | Perubahan |
|---------|-------|-----------|
| 2024-01 | 1.0 | Dokumen awal |
| 2024-01 | 1.1 | Penambahan referensi calibration |

---

## 📧 Kontributor

Jika menemukan referensi tambahan yang berguna, silakan berkontribusi melalui:
- Pull request ke repository praktikum
- Diskusi di forum mata kuliah
- Email ke dosen pengampu

---

*Dokumen ini adalah bagian dari Praktikum Computer Vision - Universitas XYZ*
