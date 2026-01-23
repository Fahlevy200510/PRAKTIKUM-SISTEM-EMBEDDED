# Praktikum 1: Blink LED - Dasar GPIO Output

## Tujuan Pembelajaran
- Memahami konfigurasi GPIO sebagai output pada STM32F103C8T6
- Memahami penggunaan HAL_Delay untuk delay sederhana
- Memahami struktur dasar program STM32 dengan HAL

## Hardware yang Dibutuhkan
- Board STM32F103C8T6 (Blue Pill)
- ST-Link V2 programmer
- Kabel jumper

## Skematik
```
Blue Pill Board:
    PC13 ----[LED]---- GND (LED internal, active LOW)
    
LED Eksternal (opsional):
    PA0 ----[330Ω]----[LED]---- GND
```

## Penjelasan Kode

### 1. Inisialisasi HAL
```c
HAL_Init();
```
Fungsi ini menginisialisasi:
- Flash prefetch buffer
- SysTick timer (untuk HAL_Delay)
- NVIC priority grouping

### 2. Konfigurasi Clock
```c
SystemClock_Config();
```
Mengkonfigurasi clock sistem menggunakan:
- HSE (High Speed External) 8MHz crystal
- PLL untuk menghasilkan 72MHz system clock

### 3. Inisialisasi GPIO
```c
GPIO_Init();
```
- Enable clock untuk GPIO port (wajib sebelum menggunakan GPIO)
- Konfigurasi pin sebagai output push-pull

### 4. Toggle LED
```c
HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
```
Membalik state GPIO (HIGH menjadi LOW atau sebaliknya)

## Cara Compile dan Upload

1. Buka terminal di folder project
2. Jalankan perintah:
```bash
pio run              # Compile saja
pio run -t upload    # Compile dan upload
```

## Latihan

1. **Ubah kecepatan kedip**: Modifikasi nilai HAL_Delay
2. **Buat pola SOS**: Implementasikan pola morse code SOS (... --- ...)
3. **Multiple LED**: Tambahkan LED di pin lain dan buat pola berurutan
4. **Kontrol brightness**: Gunakan PWM (lihat Praktikum 5)

## Troubleshooting

| Masalah | Solusi |
|---------|--------|
| LED tidak menyala | Cek koneksi, pastikan LED active LOW |
| Upload gagal | Pastikan ST-Link terhubung dengan benar |
| Clock error | Pastikan crystal 8MHz terpasang |

## Referensi
- STM32F103C8 Datasheet
- Mastering STM32 - Chapter 6: GPIO Management
