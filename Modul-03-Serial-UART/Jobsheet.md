# JOBSHEET BAB 03: Serial UART Communication

## 📋 Informasi Praktikum

| Item | Keterangan |
|------|------------|
| **Topik** | Serial UART Communication |
| **Platform** | STM32F103C8T6 (Blue Pill), ESP32 DevKitC |
| **Framework** | Arduino (PlatformIO) |
| **Jumlah Program STM32** | 12 |
| **Jumlah Program ESP32** | 12 |
| **Durasi** | 3 x 50 menit |

---

## 🎯 Tujuan Praktikum

Setelah menyelesaikan praktikum ini, mahasiswa mampu:

1. Memahami struktur frame UART dan parameter konfigurasi
2. Mengkonfigurasi UART pada STM32 dan ESP32
3. Mengirim dan menerima data melalui Serial Monitor
4. Membangun komunikasi MCU-to-MCU via UART
5. Mengimplementasikan protokol komunikasi sederhana
6. Menerapkan teknik buffer dan error handling

---

## 🔧 Alat dan Bahan

### Hardware

| No | Komponen | Jumlah | Keterangan |
|----|----------|--------|------------|
| 1 | STM32F103C8T6 (Blue Pill) | 1 | ARM Cortex-M3, 72MHz |
| 2 | ESP32 DevKitC | 1 | Dual-core, 240MHz |
| 3 | ST-Link V2 | 1 | Programmer STM32 |
| 4 | USB Cable Micro | 2 | Power & programming |
| 5 | USB-TTL Converter | 1 | CH340/CP2102/FT232 |
| 6 | LED 5mm | 3 | Status indicator |
| 7 | Resistor 330Ω | 3 | Current limiting LED |
| 8 | Push Button | 2 | User input |
| 9 | Breadboard | 1 | 830 tie-points |
| 10 | Kabel Jumper | 20 | Male-Male & Male-Female |
| 11 | Logic Analyzer | 1 | Opsional, untuk debugging |

### Software

| No | Software | Versi | Keterangan |
|----|----------|-------|------------|
| 1 | VS Code | Latest | IDE utama |
| 2 | PlatformIO | Latest | Build system |
| 3 | STM32 Platform | ststm32 | Platform STM32 |
| 4 | ESP32 Platform | espressif32 | Platform ESP32 |
| 5 | Serial Monitor | Built-in | VS Code terminal |
| 6 | HTerm / RealTerm | Latest | Advanced serial terminal |
| 7 | Saleae Logic | Latest | Opsional, logic analysis |

---

## 📐 Konfigurasi Pin

### STM32F103C8T6 UART Pins

```
┌─────────────────────────────────────────────────────────────┐
│                  STM32F103C8T6 UART Pinout                   │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│    USART1:                    USART2:                       │
│    PA9  ─── TX1               PA2  ─── TX2                  │
│    PA10 ─── RX1               PA3  ─── RX2                  │
│                                                             │
│    USART3:                                                  │
│    PB10 ─── TX3               LED Status:                   │
│    PB11 ─── RX3               PC13 ─── Built-in LED         │
│                               PB3  ─── Status LED 1         │
│                               PB4  ─── Status LED 2         │
│                                                             │
│    Button Input:                                            │
│    PA0  ─── Button 1                                        │
│    PA1  ─── Button 2                                        │
│                                                             │
│    ⚠️ USART1 shares pins with ST-Link (PA9/PA10)            │
│       Use USART2/USART3 when ST-Link is connected           │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### ESP32 DevKitC UART Pins

```
┌─────────────────────────────────────────────────────────────┐
│                    ESP32 UART Pinout                         │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│    UART0 (USB Serial):        UART2 (User):                 │
│    GPIO1  ─── TX0             GPIO17 ─── TX2                │
│    GPIO3  ─── RX0             GPIO16 ─── RX2                │
│                                                             │
│    UART1 (User):              LED Status:                   │
│    GPIO10 ─── TX1*            GPIO2  ─── Built-in LED       │
│    GPIO9  ─── RX1*            GPIO4  ─── Status LED 1       │
│                               GPIO5  ─── Status LED 2       │
│    * UART1 default pins are used by flash                   │
│      Remap to other GPIO if using UART1                     │
│                                                             │
│    Button Input:                                            │
│    GPIO0  ─── BOOT Button                                   │
│    GPIO13 ─── User Button                                   │
│                                                             │
│    ⚠️ UART0 digunakan untuk USB Serial Monitor              │
│       Gunakan UART2 untuk komunikasi dengan device lain     │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

## 🔌 Skema Rangkaian

### Rangkaian 1: STM32 ke PC via USB-TTL

```
    STM32F103C8T6              USB-TTL Converter           PC
   ┌─────────────┐            ┌─────────────┐        ┌─────────┐
   │             │            │             │        │         │
   │  PA2 (TX2) ─┼────────────┼─► RX        │        │ Serial  │
   │  PA3 (RX2) ◄┼────────────┼── TX        │◄══USB══│ Monitor │
   │  GND ───────┼────────────┼── GND       │        │         │
   │             │            │             │        │         │
   └─────────────┘            └─────────────┘        └─────────┘
   
   Konfigurasi:
   • Baud Rate: 115200
   • Data Bits: 8
   • Parity: None
   • Stop Bits: 1
```

### Rangkaian 2: ESP32 ke PC (Built-in USB)

```
    ESP32 DevKitC                                        PC
   ┌─────────────┐                                  ┌─────────┐
   │             │                                  │         │
   │  GPIO1 (TX) │◄══════════════════════════USB═══│ Serial  │
   │  GPIO3 (RX) │                                  │ Monitor │
   │             │                                  │         │
   └─────────────┘                                  └─────────┘
   
   Konfigurasi:
   • Baud Rate: 115200
   • USB CDC built-in
   • No external converter needed
```

### Rangkaian 3: STM32 ↔ ESP32 Communication

```
    STM32F103C8T6                               ESP32 DevKitC
   ┌─────────────┐                             ┌─────────────┐
   │             │                             │             │
   │  PA2 (TX2) ─┼─────────────────────────────┼─► GPIO16(RX)│
   │  PA3 (RX2) ◄┼─────────────────────────────┼── GPIO17(TX)│
   │  GND ───────┼─────────────────────────────┼── GND       │
   │             │                             │             │
   │  PB3 (LED) ─┼──[330Ω]──►|── GND          │  GPIO4(LED)─┼──[330Ω]──►|── GND
   │             │                             │             │
   └─────────────┘                             └─────────────┘
   
   ⚠️ PENTING:
   1. TX ke RX (crossed connection)
   2. GND HARUS terhubung (common ground)
   3. Kedua device menggunakan 3.3V logic (compatible)
   4. Baud rate HARUS sama di kedua sisi
```

---

## 📝 Daftar Program Praktikum

### Program ESP32

| No | Nama Program | Topik | Tingkat |
|----|--------------|-------|---------|
| 1 | Modul-01 | Serial Print Basic | Dasar |
| 2 | Modul-02 | Serial Read Character | Dasar |
| 3 | Modul-03 | Serial Command Parser | Menengah |
| 4 | Modul-04 | Serial with Interrupt | Menengah |
| 5 | Modul-05 | Multi-UART Configuration | Menengah |
| 6 | Modul-06 | Serial Data Logger | Menengah |
| 7 | Modul-07 | Serial Protocol Implementation | Lanjut |
| 8 | Modul-08 | Circular Buffer RX | Lanjut |
| 9 | Modul-09 | Checksum Validation | Menengah |
| 10 | Modul-10 | MCU Communication (Receiver) | Lanjut |
| 11 | Modul-11 | Serial JSON Parser | Lanjut |
| 12 | Modul-12 | Complete Serial Application | Lanjut |

### Program STM32

| No | Nama Program | Topik | Tingkat |
|----|--------------|-------|---------|
| 1 | STM32_01 | USART Basic Print | Dasar |
| 2 | STM32_02 | USART Read Character | Dasar |
| 3 | STM32_03 | USART Command Handler | Menengah |
| 4 | STM32_04 | USART Interrupt Mode | Menengah |
| 5 | STM32_05 | Multi-USART Setup | Menengah |
| 6 | STM32_06 | USART with DMA | Lanjut |
| 7 | STM32_07 | Protocol Implementation | Lanjut |
| 8 | STM32_08 | Ring Buffer Implementation | Lanjut |
| 9 | STM32_09 | Error Detection (Checksum) | Menengah |
| 10 | STM32_10 | MCU Communication (Sender) | Lanjut |
| 11 | STM32_11 | Binary Protocol | Lanjut |
| 12 | STM32_12 | Complete Serial System | Lanjut |

---

## 📚 Tugas Praktikum

### Tugas 1: Serial Communication Basic (30 menit)

**Tujuan:** Memahami konfigurasi UART dan komunikasi basic

**Langkah Kerja ESP32:**

1. **Setup Hardware (5 menit)**
   - Hubungkan ESP32 ke PC via USB
   - Buka VS Code dengan PlatformIO
   - Create new project atau buka Modul-01

2. **Serial Print Test (10 menit)**
   ```cpp
   void setup() {
       Serial.begin(115200);
       delay(1000);  // Wait for serial
       Serial.println("ESP32 UART Test Started!");
   }
   
   void loop() {
       Serial.println("Hello from ESP32!");
       Serial.printf("Millis: %lu\n", millis());
       delay(1000);
   }
   ```
   - Upload program
   - Buka Serial Monitor (115200 baud)
   - Verifikasi output

3. **Serial Read Test (15 menit)**
   ```cpp
   void setup() {
       Serial.begin(115200);
       Serial.println("Type something:");
   }
   
   void loop() {
       if (Serial.available()) {
           String input = Serial.readStringUntil('\n');
           Serial.print("You typed: ");
           Serial.println(input);
       }
   }
   ```
   - Ketik text di Serial Monitor
   - Verifikasi echo response

**Langkah Kerja STM32:**

1. **Setup Hardware (5 menit)**
   - Hubungkan USB-TTL ke PA2(TX), PA3(RX), GND
   - Connect USB-TTL ke PC
   - Hubungkan ST-Link untuk programming

2. **Serial Print Test (10 menit)**
   - Buka program STM32_01
   - Upload via ST-Link
   - Buka serial terminal (HTerm/RealTerm)
   - Pilih COM port USB-TTL, 115200 baud
   - Verifikasi output

**Pertanyaan Analisis:**
1. Apa yang terjadi jika baud rate tidak cocok?
2. Mengapa ESP32 menggunakan Serial sedangkan STM32 menggunakan Serial2?
3. Jelaskan fungsi `delay(1000)` setelah `Serial.begin()`!

---

### Tugas 2: Command Parser (30 menit)

**Tujuan:** Implementasi command parser via serial

**Langkah Kerja:**

1. **Buka program Modul-03 (ESP32) atau STM32_03**

2. **Implementasi Command Parser:**
   ```cpp
   // Commands: LED ON, LED OFF, STATUS, HELP
   
   void processCommand(String cmd) {
       cmd.trim();  // Remove whitespace
       cmd.toUpperCase();  // Case insensitive
       
       if (cmd == "LED ON") {
           digitalWrite(LED_PIN, HIGH);
           Serial.println("OK: LED is ON");
       }
       else if (cmd == "LED OFF") {
           digitalWrite(LED_PIN, LOW);
           Serial.println("OK: LED is OFF");
       }
       else if (cmd == "STATUS") {
           Serial.printf("LED: %s\n", digitalRead(LED_PIN) ? "ON" : "OFF");
           Serial.printf("Uptime: %lu ms\n", millis());
       }
       else if (cmd == "HELP") {
           Serial.println("Commands: LED ON, LED OFF, STATUS, HELP");
       }
       else {
           Serial.println("ERROR: Unknown command");
       }
   }
   ```

3. **Test Commands:**
   - Kirim "LED ON" → LED menyala
   - Kirim "LED OFF" → LED mati
   - Kirim "STATUS" → Tampilkan status
   - Kirim "HELP" → Tampilkan daftar command

**Tugas Modifikasi:**
Tambahkan command baru:
- `BLINK <delay>` → LED blink dengan delay tertentu
- `ADC <pin>` → Baca nilai ADC dari pin tertentu
- `INFO` → Tampilkan info chip (ESP32: chip model, revision)

---

### Tugas 3: MCU-to-MCU Communication (40 menit)

**Tujuan:** Komunikasi serial antara STM32 dan ESP32

**Langkah Kerja:**

1. **Persiapan Hardware (10 menit)**
   ```
   Wiring:
   STM32 PA2 (TX) ──────────► ESP32 GPIO16 (RX)
   STM32 PA3 (RX) ◄────────── ESP32 GPIO17 (TX)
   STM32 GND ───────────────── ESP32 GND
   ```

2. **Program STM32 sebagai Sender (15 menit)**
   - Buka STM32_10
   - Program mengirim data sensor simulasi
   ```cpp
   void loop() {
       // Format: $DATA,temperature,humidity*checksum\n
       float temp = 25.0 + random(-50, 50) / 10.0;
       float hum = 60.0 + random(-100, 100) / 10.0;
       
       char buffer[64];
       sprintf(buffer, "$DATA,%.1f,%.1f", temp, hum);
       
       // Calculate checksum
       uint8_t checksum = 0;
       for (int i = 1; buffer[i]; i++) {
           checksum ^= buffer[i];
       }
       
       Serial2.printf("%s*%02X\n", buffer, checksum);
       delay(1000);
   }
   ```

3. **Program ESP32 sebagai Receiver (15 menit)**
   - Buka Modul-10
   - Program menerima dan mem-parse data
   ```cpp
   void loop() {
       if (Serial2.available()) {
           String msg = Serial2.readStringUntil('\n');
           
           if (validateMessage(msg)) {
               parseData(msg);
               displayData();
           } else {
               Serial.println("Invalid message!");
           }
       }
   }
   ```

4. **Testing dan Verifikasi**
   - Upload kedua program
   - Monitor output di ESP32 Serial Monitor
   - Verifikasi data yang diterima

**Dokumentasi:**
| No | Data Dikirim | Data Diterima | Checksum Valid |
|----|--------------|---------------|----------------|
| 1 | | | |
| 2 | | | |
| 3 | | | |

---

### Tugas 4: Protocol Implementation (30 menit)

**Tujuan:** Implementasi protokol komunikasi dengan ACK/NACK

**Spesifikasi Protokol:**
```
Request:  $CMD,<command>,<params>*<checksum>\r\n
Response: $ACK,<cmd_id>*<checksum>\r\n  (sukses)
          $NAK,<cmd_id>,<error_code>*<checksum>\r\n (gagal)

Commands:
- LED,ON,<pin>   → Nyalakan LED
- LED,OFF,<pin>  → Matikan LED
- READ,ADC,<pin> → Baca ADC
- READ,GPIO,<pin>→ Baca GPIO
```

**Implementasi:**
1. STM32 mengirim command
2. ESP32 menerima, validasi, execute
3. ESP32 mengirim ACK/NAK
4. STM32 tunggu response (timeout 1 detik)

**Error Codes:**
- 01: Unknown command
- 02: Invalid parameters
- 03: Checksum error
- 04: Execution failed

---

## 📊 Rubrik Penilaian Praktikum

| Komponen | Bobot | Kriteria |
|----------|-------|----------|
| **Implementasi** | 40% | Semua tugas berjalan dengan benar |
| | | - Excellent (36-40): Semua + modifikasi |
| | | - Good (28-35): Semua tugas dasar |
| | | - Fair (20-27): Sebagian tugas |
| | | - Poor (<20): Sedikit berhasil |
| **Analisis** | 30% | Pemahaman konsep dan debugging |
| | | - Excellent (27-30): Analysis mendalam |
| | | - Good (21-26): Analysis memadai |
| | | - Fair (15-20): Analysis minimal |
| | | - Poor (<15): Tidak ada analysis |
| **Laporan** | 20% | Dokumentasi lengkap |
| | | - Excellent (18-20): Lengkap + insights |
| | | - Good (14-17): Lengkap |
| | | - Fair (10-13): Cukup lengkap |
| | | - Poor (<10): Tidak lengkap |
| **Keaktifan** | 10% | Partisipasi dan inisiatif |

---

## 📝 Format Laporan Praktikum

### Struktur Laporan

1. **Cover** (1 halaman)
   - Judul: Praktikum 03 - Serial UART Communication
   - Nama, NIM, Tanggal

2. **Tujuan** (0.5 halaman)
   - List tujuan praktikum

3. **Dasar Teori** (1-2 halaman)
   - UART frame structure
   - Baud rate calculation
   - STM32 vs ESP32 UART

4. **Metodologi** (1 halaman)
   - Alat dan bahan
   - Diagram wiring
   - Prosedur kerja

5. **Hasil dan Analisis** (3-4 halaman)
   - Screenshot setiap tugas
   - Tabel pengukuran
   - Analisis perbandingan
   - Jawaban pertanyaan

6. **Kesimpulan** (0.5 halaman)
   - Ringkasan pembelajaran
   - Challenges dan solutions

7. **Lampiran**
   - Source code modifikasi
   - Screenshot tambahan

---

## ⚠️ Troubleshooting

### Masalah Umum UART

| Masalah | Penyebab | Solusi |
|---------|----------|--------|
| Tidak ada output | TX/RX terbalik | Swap wires |
| Karakter aneh (garbage) | Baud rate mismatch | Samakan baud rate |
| Partial data | Buffer overflow | Increase buffer size |
| No response | GND not connected | Connect GND |
| ESP32 crash saat Serial2 | Wrong pins | Use correct UART2 pins |
| STM32 not printing | Wrong USART selected | Check USART number |

### Debug Tips

1. **Loopback Test:**
   - Connect TX to RX on same device
   - Send data, should receive back
   - Verifies hardware works

2. **Logic Analyzer:**
   - Capture actual signal
   - Verify timing
   - Check data bits

3. **LED Indicator:**
   - Toggle LED on TX/RX
   - Visual feedback of activity

---

## 📚 Referensi Tambahan

1. **STM32F103 Reference Manual** - Chapter 27: USART
2. **ESP32 Technical Reference** - Chapter 12: UART Controller
3. **RS-232 Standard** - EIA/TIA-232-F
4. [Arduino Serial Reference](https://www.arduino.cc/reference/en/language/functions/communication/serial/)
5. [ESP-IDF UART Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/uart.html)

