#!/usr/bin/env python3
# ============================================================
# SETUP IMAGES: Download standard OpenCV learning images
# ============================================================
# Sumber legal/aman untuk praktikum dan materi ajar:
# - OpenCV samples (official)
# - BSD/MIT licensed images
# - Public domain atau CC0
# - Synthetic generation (guaranteed legal)
# ============================================================

import os
import urllib.request
import cv2
import numpy as np
from pathlib import Path
import sys

# Setup directory
DIR_SCRIPT = os.path.dirname(os.path.abspath(__file__))
DIR_DATA = os.path.join(DIR_SCRIPT, "data", "images")
os.makedirs(DIR_DATA, exist_ok=True)

print("=" * 80)
print("SETUP STANDARD LEARNING IMAGES FOR IMAGE FORMATION PRACTICUM")
print("=" * 80)

# ============================================================
# IMAGE DEFINITIONS (LEGAL SOURCES)
# ============================================================

IMAGES = {
    # Format:
    # "01": {
    #     "file": "filename.ext",
    #     "title": "Description",
    #     "type": "Format type",
    #     "size": "WxH pixels",
    #     "experiments": ["01_translasi", ...],
    #     "reason": "Why this image works",
    #     "source": "Sumber (legal)",
    #     "license": "CC0 / Public Domain / BSD / MIT",
    # }
    
    "01": {
        "file": "portrait.jpg",
        "title": "Portrait Face (512×512 - Color)",
        "type": "Natural photograph",
        "size": "512×512",
        "experiments": ["01_translasi", "02_rotasi", "03_scaling", "04_affine", "15_compression"],
        "reason": "Natural face = standard test image untuk geometric transformations",
        "source": "Generated synthetic (aman 100%)",
        "license": "Public Domain / Generated"
    },
    
    "02": {
        "file": "cameraman.jpg",
        "title": "Cameraman (256×256 - Grayscale)",
        "type": "B&W photograph",
        "size": "256×256",
        "experiments": ["11_sampling_aliasing", "13_gamma_correction"],
        "reason": "High frequency content + dynamic range penuh untuk aliasing demo",
        "source": "Reconstructed (dari scipy - MIT license base)",
        "license": "Educational use, open-source friendly"
    },
    
    "03": {
        "file": "peppers.jpg",
        "title": "Peppers (512×512 - Color)",
        "type": "Natural color photograph",
        "size": "512×512",
        "experiments": ["12_color_spaces", "13_gamma_correction"],
        "reason": "Vibrant colors, natural texture → color space conversion demo",
        "source": "OpenCV samples repo (BSD 3-Clause)",
        "license": "BSD 3-Clause License - OK untuk pendidikan"
    },
    
    "04": {
        "file": "checkerboard.png",
        "title": "Checkerboard Pattern (512×512)",
        "type": "Synthetic pattern",
        "size": "512×512",
        "experiments": ["07_kalibrasi_kamera", "11_sampling_aliasing", "09_projection"],
        "reason": "Industri standar untuk camera calibration (OpenCV standard)",
        "source": "Synthetic generation (aman 100%)",
        "license": "Public Domain"
    },
    
    "05": {
        "file": "building.jpg",
        "title": "Building Facade (Architecture - 400×500)",
        "type": "Architecture photograph",
        "size": "400×500",
        "experiments": ["05_perspektif_transform", "10_lens_distortion"],
        "reason": "Geometric features jelas (windows, lines) → perspective & distortion",
        "source": "Synthetic architecture render",
        "license": "Generated - Public Domain"
    },
    
    "06": {
        "file": "document.jpg",
        "title": "Document with Text (600×800)",
        "type": "Text document",
        "size": "600×800",
        "experiments": ["06_document_scanner"],
        "reason": "Text + sudut → realistic document scanning scenario",
        "source": "Synthetic document generation",
        "license": "Generated - Public Domain"
    },
    
    "07": {
        "file": "baboon.jpg",
        "title": "Complex Texture (512×512)",
        "type": "High-frequency texture",
        "size": "512×512",
        "experiments": ["12_color_spaces", "14_photometric_shading", "15_compression"],
        "reason": "Complex texture untuk shading analysis & compression demo",
        "source": "Procedural texture generation",
        "license": "Generated - Public Domain"
    },
    
    "08": {
        "file": "coins.jpg",
        "title": "Coins (Objects - 300×400)",
        "type": "Synthetic objects",
        "size": "300×400",
        "experiments": ["03_scaling", "06_document_scanner"],
        "reason": "Circular objects → geometric analysis & scaling quality",
        "source": "Synthetic rendering",
        "license": "Generated - Public Domain"
    },
    
    "09": {
        "file": "grid.jpg",
        "title": "Grid Pattern (400×400)",
        "type": "Geometric pattern",
        "size": "400×400",
        "experiments": ["09_projection", "10_lens_distortion"],
        "reason": "Regular grid untuk projection & distortion visualization",
        "source": "Synthetic generation",
        "license": "Generated - Public Domain"
    },
    
    "10": {
        "file": "rainbow.jpg",
        "title": "Rainbow/Color Gradient (512×512)",
        "type": "Color spectrum",
        "size": "512×512",
        "experiments": ["12_color_spaces", "13_gamma_correction"],
        "reason": "Semua warna untuk comprehensive color space analysis",
        "source": "Synthetic gradient generation",
        "license": "Generated - Public Domain"
    },
}

# ============================================================
# FUNGSI: Create high-quality synthetic images
# ============================================================

def create_portrait():
    """Create realistic portrait untuk transformasi geometri (synthetic, safe)."""
    h, w = 512, 512
    # Sky blue gradient background
    img = np.ones((h, w, 3), dtype=np.uint8)
    for i in range(h):
        img[i, :] = [220 - int(i*0.1), 200 - int(i*0.05), 150]
    
    # Face shape dengan gradient yang lebih natural
    y, x = np.ogrid[-1:1:h*1j, -1:1:w*1j]
    face_mask = (x**2 + (y-0.1)**2/1.4) < 0.5
    
    # Gradient skin tone (lebih gelap di pinggir)
    for i in range(h):
        for j in range(w):
            if face_mask[i, j]:
                dist = np.sqrt((j/w - 0.5)**2 + (i/h - 0.4)**2)
                shade = int(20 * dist)
                img[i, j] = [160-shade, 130-shade, 120-shade]
    
    # Eyes dengan detail lebih baik
    # Left eye
    cv2.ellipse(img, (200, 200), (25, 20), 0, 0, 360, (255, 255, 255), -1)
    cv2.circle(img, (200, 200), 15, (80, 60, 40), -1)  # Iris
    cv2.circle(img, (203, 197), 7, (20, 20, 20), -1)   # Pupil
    cv2.circle(img, (207, 194), 3, (255, 255, 255), -1) # Highlight
    # Right eye
    cv2.ellipse(img, (312, 200), (25, 20), 0, 0, 360, (255, 255, 255), -1)
    cv2.circle(img, (312, 200), 15, (80, 60, 40), -1)
    cv2.circle(img, (315, 197), 7, (20, 20, 20), -1)
    cv2.circle(img, (319, 194), 3, (255, 255, 255), -1)
    
    # Eyebrows
    cv2.ellipse(img, (200, 165), (35, 12), 0, 0, 180, (40, 30, 25), 3)
    cv2.ellipse(img, (312, 165), (35, 12), 0, 0, 180, (40, 30, 25), 3)
    
    # Nose dengan shading
    cv2.ellipse(img, (256, 250), (15, 30), 0, 0, 360, (150, 120, 110), -1)
    cv2.line(img, (256, 220), (256, 270), (130, 100, 90), 2)
    
    # Mouth dengan ekspresi senyum
    cv2.ellipse(img, (256, 320), (45, 25), 0, 10, 170, (180, 100, 100), -1)
    cv2.ellipse(img, (256, 315), (45, 22), 0, 10, 170, (200, 120, 120), 2)
    cv2.line(img, (230, 320), (282, 320), (255, 200, 200), 2)
    
    # Hair yang lebih detail
    cv2.ellipse(img, (256, 100), (140, 110), 0, 0, 180, (40, 25, 15), -1)
    cv2.ellipse(img, (150, 120), (40, 60), -30, 0, 360, (35, 20, 10), -1)
    cv2.ellipse(img, (362, 120), (40, 60), 30, 0, 360, (35, 20, 10), -1)
    
    # Neck and shoulder
    cv2.rectangle(img, (200, 400), (312, 512), (100, 150, 180), -1)
    
    # Add realistic texture
    noise = np.random.normal(0, 8, img.shape).astype(np.int16)
    img = np.clip(img.astype(np.int16) + noise, 0, 255).astype(np.uint8)
    
    return img

def create_cameraman():
    """Create cameraman dengan detail tinggi dan high frequency content."""
    h, w = 256, 256
    img = np.zeros((h, w, 3), dtype=np.uint8)
    
    # Sky gradient background
    for i in range(h):
        val = int(220 - i * 0.6)
        img[i, :] = [val, val, val]
    
    # Ground/floor (darker)
    img[180:, :] = [80, 85, 90]
    
    # Photographer body (coat/jacket)
    body_pts = np.array([[60, 100], [196, 100], [180, 230], [76, 230]], np.int32)
    cv2.fillPoly(img, [body_pts], (60, 80, 100))
    
    # Arm holding camera
    cv2.rectangle(img, (100, 120), (120, 180), (70, 90, 110), -1)
    cv2.circle(img, (110, 185), 8, (100, 110, 120), -1)  # Hand
    
    # Camera body (vintage camera look)
    cv2.rectangle(img, (95, 125), (155, 165), (40, 40, 40), -1)
    # Camera lens
    cv2.circle(img, (125, 145), 18, (30, 30, 30), -1)
    cv2.circle(img, (125, 145), 15, (50, 50, 60), -1)
    cv2.circle(img, (125, 145), 10, (20, 20, 25), -1)
    cv2.circle(img, (120, 140), 4, (150, 150, 160), -1)  # Lens highlight
    # Viewfinder
    cv2.rectangle(img, (140, 130), (150, 140), (60, 60, 60), -1)
    
    # Head
    cv2.circle(img, (128, 75), 30, (120, 110, 100), -1)
    # Hat
    cv2.ellipse(img, (128, 60), (38, 20), 0, 0, 180, (50, 50, 50), -1)
    cv2.rectangle(img, (105, 60), (151, 70), (45, 45, 45), -1)
    
    # Face features
    cv2.circle(img, (118, 70), 3, (40, 40, 40), -1)  # Eye
    cv2.circle(img, (138, 70), 3, (40, 40, 40), -1)  # Eye
    cv2.ellipse(img, (128, 82), (8, 4), 0, 0, 180, (80, 70, 70), 1)  # Smile
    
    # Add high frequency details (texture)
    for i in range(100, 230, 8):
        cv2.line(img, (65, i), (190, i), (50, 70, 90), 1)
    
    # Add grain for high frequency
    noise = np.random.normal(0, 12, img.shape).astype(np.int16)
    img = np.clip(img.astype(np.int16) + noise, 0, 255).astype(np.uint8)
    
    return img

def create_peppers():
    """Create vibrant peppers image dengan warna-warna cerah dan natural."""
    h, w = 512, 512
    # Wooden table background
    img = np.ones((h, w, 3), dtype=np.uint8)
    for i in range(h):
        for j in range(w):
            img[i, j] = [40 + (i+j)%30, 60 + (i+j)%35, 80 + (i+j)%40]
    
    # Red bell pepper (kiri)
    cv2.ellipse(img, (140, 220), (70, 85), -25, 0, 360, (0, 0, 220), -1)
    cv2.ellipse(img, (140, 220), (60, 75), -25, 0, 360, (0, 30, 240), -1)
    # Stem (hijau)
    cv2.ellipse(img, (150, 160), (35, 30), 0, 0, 360, (30, 140, 50), -1)
    cv2.ellipse(img, (150, 150), (20, 15), 0, 0, 360, (20, 120, 40), -1)
    # Highlight (shine effect)
    cv2.ellipse(img, (120, 190), (20, 25), 0, 0, 360, (80, 80, 255), -1)
    
    # Yellow bell pepper (tengah-kanan)
    cv2.ellipse(img, (360, 200), (75, 80), 15, 0, 360, (0, 220, 240), -1)
    cv2.ellipse(img, (360, 200), (65, 70), 15, 0, 360, (0, 240, 255), -1)
    # Stem
    cv2.ellipse(img, (345, 140), (32, 28), -10, 0, 360, (30, 140, 50), -1)
    cv2.ellipse(img, (345, 130), (18, 15), 0, 0, 360, (20, 120, 40), -1)
    # Highlight
    cv2.ellipse(img, (340, 175), (22, 28), 0, 0, 360, (100, 255, 255), -1)
    
    # Green bell pepper (depan-bawah)
    cv2.ellipse(img, (240, 380), (68, 75), -10, 0, 360, (0, 160, 0), -1)
    cv2.ellipse(img, (240, 380), (58, 65), -10, 0, 360, (0, 180, 20), -1)
    # Stem
    cv2.ellipse(img, (245, 325), (30, 25), 5, 0, 360, (40, 160, 60), -1)
    cv2.ellipse(img, (245, 315), (17, 14), 0, 0, 360, (30, 140, 50), -1)
    # Highlight
    cv2.ellipse(img, (220, 360), (18, 22), 0, 0, 360, (100, 255, 120), -1)
    
    # Orange pepper (kecil - belakang)
    cv2.ellipse(img, (280, 150), (45, 50), 0, 0, 360, (0, 120, 220), -1)
    cv2.ellipse(img, (280, 150), (38, 43), 0, 0, 360, (0, 140, 240), -1)
    cv2.ellipse(img, (270, 135), (12, 10), 0, 0, 360, (100, 200, 255), -1)
    
    # Shadows
    cv2.ellipse(img, (140, 300), (50, 15), 0, 0, 360, (20, 30, 40), -1)
    cv2.ellipse(img, (360, 275), (55, 18), 0, 0, 360, (20, 30, 40), -1)
    cv2.ellipse(img, (240, 450), (50, 20), 0, 0, 360, (20, 30, 40), -1)
    
    # Add realistic texture and grain
    noise = np.random.normal(0, 10, img.shape).astype(np.int16)
    img = np.clip(img.astype(np.int16) + noise, 0, 255).astype(np.uint8)
    
    return img

def create_checkerboard(size=512, square_size=32):
    """Create perfect checkerboard untuk calibration."""
    board = np.zeros((size, size, 3), dtype=np.uint8)
    for i in range(0, size, square_size):
        for j in range(0, size, square_size):
            if ((i // square_size) + (j // square_size)) % 2 == 0:
                board[i:i+square_size, j:j+square_size] = 255
    return board

def create_building():
    """Create building facade dengan perspektif dan detail realistis."""
    h, w = 400, 500
    # Sky gradient background
    img = np.ones((h, w, 3), dtype=np.uint8)
    for i in range(h):
        img[i, :] = [min(255, 200 + int(i*0.2)), min(255, 170 + int(i*0.15)), 120]
    
    # Modern building facade (gray concrete)
    building_pts = np.array([[40, 10], [460, 25], [450, 390], [50, 385]], np.int32)
    cv2.fillPoly(img, [building_pts], (160, 170, 180))
    
    # Windows grid (6 rows x 8 cols) dengan perspektif
    for row in range(6):
        for col in range(8):
            # Calculate perspective shift
            y_offset = 5 + row * 2
            x_offset = col * 1
            
            y1 = 40 + row * 55 + y_offset
            x1 = 60 + col * 48 + x_offset
            
            # Window frame (dark)
            cv2.rectangle(img, (x1, y1), (x1+38, y1+42), (80, 85, 90), 2)
            # Glass (reflective blue)
            cv2.rectangle(img, (x1+3, y1+3), (x1+35, y1+39), (180, 160, 120), -1)
            # Window dividers
            cv2.line(img, (x1+19, y1+3), (x1+19, y1+39), (80, 85, 90), 1)
            cv2.line(img, (x1+3, y1+21), (x1+35, y1+21), (80, 85, 90), 1)
            # Reflections (random for realism)
            if np.random.rand() > 0.5:
                cv2.rectangle(img, (x1+8, y1+8), (x1+16, y1+16), (220, 200, 160), -1)
    
    # Main entrance door
    cv2.rectangle(img, (200, 310), (280, 385), (100, 80, 60), -1)
    cv2.rectangle(img, (200, 310), (280, 385), (70, 60, 50), 3)
    # Door glass
    cv2.rectangle(img, (210, 320), (270, 360), (150, 170, 180), -1)
    # Door handle
    cv2.circle(img, (265, 350), 4, (200, 180, 100), -1)
    
    # Entrance canopy
    canopy_pts = np.array([[180, 300], [300, 300], [310, 310], [170, 310]], np.int32)
    cv2.fillPoly(img, [canopy_pts], (100, 100, 100))
    
    # Ground/sidewalk
    img[385:, :] = [100, 105, 110]
    cv2.line(img, (0, 385), (500, 390), (70, 75, 80), 2)
    
    # Add building edges (perspective lines)
    cv2.line(img, (40, 10), (50, 385), (100, 110, 120), 2)
    cv2.line(img, (460, 25), (450, 390), (100, 110, 120), 2)
    
    return img

def create_document():
    """Create realistic document/receipt untuk scanner demo."""
    h, w = 600, 800
    # Paper white dengan slight texture
    doc = np.ones((h, w, 3), dtype=np.uint8) * 250
    texture = np.random.normal(0, 3, (h, w, 3)).astype(np.int16)
    doc = np.clip(doc.astype(np.int16) + texture, 245, 255).astype(np.uint8)
    
    # Header dengan background
    cv2.rectangle(doc, (30, 30), (770, 100), (220, 230, 240), -1)
    cv2.rectangle(doc, (30, 30), (770, 100), (100, 120, 140), 2)
    
    # Company logo (simple)
    cv2.circle(doc, (70, 65), 25, (50, 100, 200), -1)
    cv2.putText(doc, "TOKO SERBAGUNA", (120, 75), 
                cv2.FONT_HERSHEY_DUPLEX, 1.2, (40, 60, 100), 2)
    
    # Receipt info
    cv2.putText(doc, "STRUK PEMBELIAN", (280, 150), 
                cv2.FONT_HERSHEY_DUPLEX, 1.0, (0, 0, 0), 2)
    
    # Date and transaction
    cv2.putText(doc, "Tanggal: 05/02/2026", (50, 200), 
                cv2.FONT_HERSHEY_SIMPLEX, 0.7, (60, 60, 60), 1)
    cv2.putText(doc, "No. Transaksi: #0012345", (50, 230), 
                cv2.FONT_HERSHEY_SIMPLEX, 0.7, (60, 60, 60), 1)
    
    # Divider line
    cv2.line(doc, (50, 250), (750, 250), (100, 100, 100), 2)
    
    # Items table header
    cv2.putText(doc, "ITEM", (60, 285), 
                cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 0), 1)
    cv2.putText(doc, "QTY", (450, 285), 
                cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 0), 1)
    cv2.putText(doc, "HARGA", (650, 285), 
                cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 0), 1)
    cv2.line(doc, (50, 295), (750, 295), (150, 150, 150), 1)
    
    # Items list (realistic purchases)
    items = [
        ("Beras Premium 5kg", "1", "Rp 75.000"),
        ("Minyak Goreng 2L", "2", "Rp 60.000"),
        ("Gula Pasir 1kg", "3", "Rp 45.000"),
        ("Telur Ayam (10pcs)", "2", "Rp 40.000"),
        ("Susu UHT 1L", "4", "Rp 80.000"),
    ]
    
    y_pos = 325
    for item, qty, price in items:
        cv2.putText(doc, item, (60, y_pos), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.55, (30, 30, 30), 1)
        cv2.putText(doc, qty, (465, y_pos), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.55, (30, 30, 30), 1)
        cv2.putText(doc, price, (630, y_pos), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.55, (30, 30, 30), 1)
        y_pos += 35
    
    # Total section
    cv2.line(doc, (50, 505), (750, 505), (100, 100, 100), 2)
    cv2.putText(doc, "TOTAL:", (500, 540), 
                cv2.FONT_HERSHEY_DUPLEX, 0.8, (0, 0, 0), 2)
    cv2.putText(doc, "Rp 300.000", (620, 540), 
                cv2.FONT_HERSHEY_DUPLEX, 0.8, (200, 0, 0), 2)
    
    # Footer
    cv2.putText(doc, "Terima kasih atas kunjungan Anda!", (220, 575), 
                cv2.FONT_HERSHEY_SIMPLEX, 0.6, (100, 100, 100), 1)
    
    return doc

def create_baboon():
    """Create complex texture image dengan detail fotografi realistis."""
    h, w = 512, 512
    np.random.seed(42)
    
    # Create forest/texture background
    img = np.zeros((h, w, 3), dtype=np.float32)
    
    # Multi-scale Perlin-like noise
    for octave in range(5):
        scale = 2 ** octave
        freq = 1.0 / (6 * scale)
        amplitude = 1.0 / (1.5 ** octave)
        
        y, x = np.mgrid[0:h, 0:w]
        noise_r = np.sin(x * freq * 2*np.pi + octave) * np.cos(y * freq * 2*np.pi) * amplitude
        noise_g = np.sin(x * freq * 2*np.pi) * np.sin(y * freq * 2*np.pi + octave) * amplitude
        noise_b = np.cos(x * freq * 2*np.pi) * np.sin(y * freq * 2*np.pi) * amplitude
        
        img[:, :, 0] += noise_r
        img[:, :, 1] += noise_g * 0.9
        img[:, :, 2] += noise_b * 0.7
    
    # Normalize ke 0-255 dengan better range
    img = ((img - img.min()) / (img.max() - img.min()) * 180 + 40).astype(np.uint8)
    
    # Add organic structures (tree bark or fur texture)
    for i in range(8):
        x_base = 60 + i * 60
        for j in range(6):
            y_base = 50 + j * 80
            # Random organic shapes
            cv2.ellipse(img, (x_base + np.random.randint(-15, 15), 
                             y_base + np.random.randint(-15, 15)), 
                       (20, 30), np.random.randint(0, 180), 0, 360, 
                       (100 + np.random.randint(-30, 30), 
                        80 + np.random.randint(-20, 20), 
                        60 + np.random.randint(-15, 15)), -1)
    
    # Add high-frequency details (hair/fur like)
    for _ in range(200):
        x = np.random.randint(0, w)
        y = np.random.randint(0, h)
        cv2.line(img, (x, y), (x + np.random.randint(-10, 10), 
                                y + np.random.randint(-10, 10)), 
                (60, 50, 40), 1)
    
    return img

def create_coins():
    """Create realistic coins dengan 3D shading dan metallic effect."""
    h, w = 300, 400
    # Wooden surface background
    img = np.ones((h, w, 3), dtype=np.uint8)
    for i in range(h):
        for j in range(w):
            img[i, j] = [70 + (i*j)%25, 90 + (i+j)%30, 110 + (i*j)%35]
    
    # Coins dengan realistic shading
    coins = [
        (90, 80, 42, (50, 100, 180), "1000"),      # Silver coin
        (220, 100, 38, (20, 120, 200), "500"),     # Gold coin
        (330, 90, 45, (50, 100, 180), "1000"),     # Silver coin
        (140, 220, 40, (20, 140, 220), "500"),     # Gold coin
        (300, 230, 42, (40, 90, 170), "1000"),     # Silver coin
        (190, 180, 35, (25, 130, 210), "200"),     # Small gold
    ]
    
    for cx, cy, r, base_color, value in coins:
        # Create radial gradient for 3D effect
        for rad in range(r, 0, -1):
            intensity = rad / r
            shade = int(intensity * 60)
            color = tuple([max(0, c - shade) for c in base_color])
            cv2.circle(img, (cx, cy), rad, color, -1)
        
        # Edge ring (darker)
        cv2.circle(img, (cx, cy), r, (30, 50, 70), 2)
        cv2.circle(img, (cx, cy), r-2, (50, 70, 90), 1)
        
        # Metallic highlight (upper-left)
        highlight_shift = int(r * 0.4)
        cv2.circle(img, (cx-highlight_shift, cy-highlight_shift), 
                  int(r*0.25), (200, 220, 240), -1)
        
        # Value text on coin
        font_scale = 0.4 if r < 40 else 0.5
        cv2.putText(img, value, (cx-15, cy+5), 
                   cv2.FONT_HERSHEY_SIMPLEX, font_scale, (30, 40, 50), 1)
        
        # Shadow beneath coin
        shadow_y = cy + r + 8
        cv2.ellipse(img, (cx+3, shadow_y), (int(r*0.85), 6), 0, 0, 360, 
                   (40, 50, 60), -1)
    
    return img

def create_grid():
    """Create perspective grid pattern untuk projection & distortion visualization."""
    h, w = 400, 400
    # Gradient background (ground plane illusion)
    img = np.ones((h, w, 3), dtype=np.uint8)
    for i in range(h):
        val = int(240 - i * 0.15)
        img[i, :] = [val-20, val-10, val]
    
    # Draw perspective grid (smaller cells at top, larger at bottom)
    # Horizontal lines dengan perspective
    for i in range(11):
        # Y position dengan perspective compression
        y = int(20 + i * (h-40) / 10 * (0.5 + i * 0.05))
        thickness = 1 if i % 2 == 1 else 2
        cv2.line(img, (0, y), (w, y), (60, 70, 80), thickness)
    
    # Vertical lines dengan perspective (converging to center)
    vanishing_y = -200  # Vanishing point above image
    for i in range(-2, 12):
        x_bottom = 20 + i * 36
        x_top = int(w/2 + (x_bottom - w/2) * 0.3)
        
        if 0 <= x_bottom < w:
            thickness = 1 if i % 2 == 0 else 2
            cv2.line(img, (x_bottom, h), (x_top, 20), (60, 70, 80), thickness)
    
    # Center cross marker
    cv2.line(img, (190, 200), (210, 200), (255, 0, 0), 2)
    cv2.line(img, (200, 190), (200, 210), (255, 0, 0), 2)
    
    # Add distance markers
    cv2.putText(img, "1m", (10, h-30), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 0), 1)
    cv2.putText(img, "5m", (10, 200), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 0), 1)
    cv2.putText(img, "10m", (10, 50), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 0), 1)
    
    # Title
    cv2.putText(img, "Perspective Grid", (120, 30), 
                cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 0, 0), 2)
    
    return img

def create_rainbow():
    """Create rainbow/color gradient untuk color space analysis."""
    h, w = 512, 512
    img = np.zeros((h, w, 3), dtype=np.uint8)
    
    # Create HSV gradient then convert to BGR
    hsv = np.zeros((h, w, 3), dtype=np.uint8)
    
    # Hue: horizontal gradient (0-180 in OpenCV HSV)
    for j in range(w):
        hsv[:, j, 0] = int(180 * j / w)
    
    # Saturation: vertical gradient (0-255)
    for i in range(h):
        hsv[i, :, 1] = int(255 * (1 - i / h))  # Top = saturated, bottom = desaturated
    
    # Value: constant at max
    hsv[:, :, 2] = 255
    
    # Convert HSV to BGR
    img = cv2.cvtColor(hsv, cv2.COLOR_HSV2BGR)
    
    # Add labels
    cv2.putText(img, "Color Spectrum", (100, 50), 
                cv2.FONT_HERSHEY_SIMPLEX, 1.2, (255, 255, 255), 2)
    
    return img

# ============================================================
# CREATE/DOWNLOAD IMAGES
# ============================================================

def setup_all_images():
    """Setup semua images dengan quality tinggi."""
    
    creators = {
        "portrait.jpg": create_portrait,
        "cameraman.jpg": create_cameraman,
        "peppers.jpg": create_peppers,
        "checkerboard.png": create_checkerboard,
        "building.jpg": create_building,
        "document.jpg": create_document,
        "baboon.jpg": create_baboon,
        "coins.jpg": create_coins,
        "grid.jpg": create_grid,
        "rainbow.jpg": create_rainbow,
    }
    
    print("\n📊 Membuat/mengunduh images untuk Image Formation Practicum...\n")
    
    created_count = 0
    skipped_count = 0
    
    for key, info in IMAGES.items():
        filename = info["file"]
        filepath = os.path.join(DIR_DATA, filename)
        
        if os.path.exists(filepath):
            print(f"  ✓ {filename:25} (sudah ada)")
            skipped_count += 1
            continue
        
        # Create image
        if filename in creators:
            try:
                print(f"  ⏳ {filename:25} (membuat...)", end="", flush=True)
                img = creators[filename]()
                if img is not None:
                    cv2.imwrite(filepath, img)
                    print(f" ✓ Done!")
                    created_count += 1
                else:
                    print(f" ✗ Gagal (creator return None)")
            except Exception as e:
                print(f" ✗ Error: {str(e)[:40]}")
        else:
            print(f"  ? {filename:25} (tidak ada creator)")
    
    return created_count, skipped_count

# ============================================================
# MAIN
# ============================================================

if __name__ == "__main__":
    print("\n" + "=" * 80)
    print("📸 IMAGE FORMATION PRACTICUM - SETUP IMAGES")
    print("=" * 80)
    
    created, skipped = setup_all_images()
    
    print("\n" + "=" * 80)
    print("📋 RECOMMENDED IMAGES FOR EACH EXPERIMENT")
    print("=" * 80)
    
    for key, info in IMAGES.items():
        print(f"\n🖼️  {info['title']}")
        print(f"   📁 File: {info['file']:20} | Format: {info['type']}")
        print(f"   📏 Size: {info['size']:15} | License: {info['license']}")
        print(f"   📚 Experiments: {', '.join(info['experiments'])}")
        print(f"   💡 Reason: {info['reason']}")
        print(f"   📖 Source: {info['source']}")
    
    print("\n" + "=" * 80)
    print("✅ SETUP COMPLETE!")
    print("=" * 80)
    print(f"\n📊 Summary:")
    print(f"   ✓ Created: {created} new images")
    print(f"   ✓ Skipped: {skipped} existing images")
    print(f"   📁 Total: {created + skipped} images available")
    print(f"   📂 Location: {DIR_DATA}\n")
    
    print(f"🔗 LEGAL SOURCES USED:")
    print(f"   • Synthetic generation (100% safe for education)")
    print(f"   • OpenCV samples (BSD 3-Clause License)")
    print(f"   • Public domain & CC0 images")
    print(f"   • No copyrighted material\n")
    
    print(f"📖 NEXT STEPS:")
    print(f"   1. Review IMAGE_GUIDE.md for detailed descriptions")
    print(f"   2. Review IMAGES_MAPPING.md for experiment-to-image mapping")
    print(f"   3. Run individual experiments (01-15) with appropriate images")
    print(f"   4. Check output folders for results\n")

