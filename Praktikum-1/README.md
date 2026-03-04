# Praktikum 1 - Button Debounce State Machine (STM32)

Project ini menggunakan tombol pada PB0 (pull-down) untuk toggle LED pada PB1 dengan metode debounce berbasis state machine.

## Struktur Folder

- `Praktikum-1/main.cpp`
- `Praktikum-1/platformio.ini`

## Wiring

- PB0 -> Button -> 3V3 (gunakan `INPUT_PULLDOWN` internal)
- PB1 -> Resistor 220R -> LED -> GND

## Cara Build dan Upload (PlatformIO)

1. Buka folder `Praktikum-1` sebagai project PlatformIO.
2. Build project.
3. Upload ke board STM32.
4. Buka Serial Monitor pada 115200 baud untuk melihat log tombol.

## Perilaku Program

- Saat tombol ditekan valid (setelah debounce), LED akan toggle ON/OFF.
- Program mencetak jumlah tekan (`press count`) dan durasi tekan tombol di Serial Monitor.