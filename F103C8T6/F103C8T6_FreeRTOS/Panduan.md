#### **BAGIAN 1: Advanced Task Management (1-5)**

*Fokus: Mengendalikan bagaimana CPU berpindah antar pekerjaan.*

1. **Dynamic Task Injection:** Membuat dan menghapus task secara *runtime* menggunakan `xTaskCreate` dan `vTaskDelete` berdasarkan input tombol, memantau penggunaan heap RAM.
2. **Rate Monotonic Scheduling (RMS):** Demo *Preemptive Multitasking* dengan 3 task beda prioritas. Melihat bagaimana Task High Priority "mencuri" CPU dari Low Priority.
3. **Absolute Timing Control:** Menggunakan `vTaskDelayUntil` untuk memastikan frekuensi sampling sensor stabil (jitter-free), berbeda dengan `vTaskDelay` biasa yang relatif.
4. **Idle Task Hook Utilization:** Memanfaatkan waktu CPU saat "menganggur" (tidak ada task jalan) menggunakan `vApplicationIdleHook` untuk masuk ke mode hemat daya ringan.
5. **Task Suspend & Resume:** Menghentikan sementara task pemroses data saat error terdeteksi tanpa menghapusnya dari memori.

#### **BAGIAN 2: Queue & Data Flow Patterns (6-10)**

*Fokus: Mengirim data antar task tanpa variabel global (Thread-safe).*

6. **Struct Message Passing:** Mengirim paket data kompleks (ID Sensor, Timestamp, Value) antar task menggunakan  *Queue of Structs* .
7. **Mailbox Pattern:** Menggunakan Queue ukuran 1 dengan mode *Overwrite* (`xQueueOverwrite`) untuk update data display LCD (data lama ditimpa data baru).
8. **Flow Control (Back-pressure):** Sinkronisasi kecepatan Task Produsen (cepat) dan Task Konsumen (lambat) agar buffer tidak *overflow* (Task Produsen diblokir saat Queue penuh).
9. **Queue Peek Operations:** Task monitor "mengintip" data antrian (`xQueuePeek`) untuk debugging tanpa menghapus data tersebut dari antrian proses utama.
10. **Queue Sets Implementation:** Satu "Super Task" mendengarkan data dari berbagai sumber (Queue A, Queue B, Semaphore) sekaligus menggunakan `xQueueCreateSet`.

#### **BAGIAN 3: Synchronization & Resource Guarding (11-15)**

*Fokus: Mencegah tabrakan data (Race Condition) pada hardware bersama.*

11. **Gatekeeper Task Pattern:** Pola desain untuk mengamankan UART. Semua task kirim data ke Queue, hanya Gatekeeper yang boleh akses hardware UART (Solusi paling aman).
12. **Recursive Mutex Locking:** Mengamankan fungsi yang memanggil dirinya sendiri (rekursif) atau fungsi bertingkat yang mengakses resource sama agar tidak  *deadlock* .
13. **Priority Inversion Simulation & Fix:** Mendemokan masalah "Task High Priority terblokir oleh Low Priority" dan solusinya menggunakan  *Mutex with Priority Inheritance* .
14. **Counting Semaphore for Event Buffering:** Menangkap burst signal dari tombol (misal ditekan 10x cepat) dan memastikan task memproses tepat 10x (buffered events).
15. **Barrier Synchronization:** Menahan 3 task inisialisasi hardware. Sistem utama baru jalan setelah ketiganya mencapai titik sinkronisasi (menggunakan Event Groups).

#### **BAGIAN 4: Lightweight Task Notifications (16-19)**

*Fokus: Alternatif Queue/Semaphore yang 45% lebih cepat dan hemat RAM.*

16. **Direct Task Notification (Binary):** Pengganti Binary Semaphore tercepat untuk sinkronisasi ISR ke Task.
17. **Notification as Value:** Mengirim nilai data 32-bit langsung ke register task tujuan tanpa membuat objek Queue (Zero-copy).
18. **Notification as Event Bits:** Menggunakan notifikasi task sebagai *flags* (misal: bit 1 = start, bit 2 = stop) untuk kontrol state mesin.
19. **Pulse Notification:** Task menunggu notifikasi dengan  *timeout* . Jika tidak ada sinyal dalam 100ms, jalankan prosedur error handling.

#### **BAGIAN 5: Software Timers (20-23)**

*Fokus: Manajemen waktu tanpa menggunakan Hardware Timer yang terbatas.*

20. **Software Watchdog:** Timer *One-shot* yang harus di-reset berkala oleh task utama. Jika gagal reset, callback timer akan mereset sistem.
21. **Button Debounce with Timer:** Mengganti delay blocking `HAL_Delay()` dengan timer software untuk membaca tombol tanpa menghentikan CPU.
22. **Timer ID Utilization:** Menggunakan 5 timer software berbeda namun diarahkan ke **satu fungsi callback** yang sama, dibedakan via ID Timer.
23. **Backlight Timeout Agent:** Timer yang otomatis mematikan lampu LCD jika tidak ada aktivitas user selama 30 detik.

#### **BAGIAN 6: Advanced Interrupt Handling (24-28)**

*Fokus: "Deferring Work" - Menjaga ISR tetap pendek.*

24. **Deferred Interrupt Processing:** ISR Eksternal hanya memberikan  *Semaphore* . Pemrosesan data berat dilakukan di Task prioritas tinggi (bukan di dalam ISR).
25. **UART Ring Buffer with ISR:** ISR UART menerima karakter dan memasukkannya ke buffer. Task baru bangun jika karakter 'Enter' diterima.
26. **High Frequency ADC Handling:** Menangani interrupt ADC frekuensi tinggi menggunakan *Direct Task Notification* agar CPU tidak hang.
27. **Interrupt Nesting Safety:** Mengonfigurasi prioritas NVIC agar interrupt kernel RTOS (SysTick/PendSV) tidak bertabrakan dengan interrupt hardware krusial.
28. **Queue Send From ISR:** Mengirim data hasil pembacaan sensor dari dalam fungsi interrupt ke Task pemroses.

#### **BAGIAN 7: Stream & Message Buffers (Fitur FreeRTOS v10+) (29-33)**

*Fokus: Primitif baru yang dioptimalkan untuk aliran data (Byte Streams).*

29. **UART Stream Buffer:** Pengganti Queue untuk data serial. Mengizinkan pembacaan byte dalam jumlah banyak sekaligus (Batch retrieval).
30. **Message Buffer for Logging:** Mengirim string log error dengan panjang bervariasi (variable length) antar task.
31. **Blocking Stream Read:** Task parser menunggu sampai minimal 10 byte tersedia di buffer sebelum bangun (menghemat cycle CPU).
32. **DMA to Stream Buffer:** Mentransfer data dari DMA langsung ke Stream Buffer (Advanced optimization).
33. **Multi-Sender Message Buffer:** Banyak task sensor mengirim log ke satu buffer penampung.

#### **BAGIAN 8: Memory Management & Safety (34-38)**

*Fokus: Stabilitas sistem jangka panjang.*

34. **Static Memory Allocation:** Membuat Task dan Queue tanpa Heap (`xTaskCreateStatic`) untuk sistem *mission-critical* yang anti-gagal alokasi.
35. **Stack Overflow Detection:** Memicu dan menangani error `vApplicationStackOverflowHook` untuk mendeteksi task yang memakan memori berlebih.
36. **Malloc Failed Hook:** Menangani situasi saat Heap RAM habis dan sistem gagal membuat objek baru.
37. **Heap Fragmentation Monitor:** Menampilkan sisa memori (`xPortGetFreeHeapSize`) dan watermark terendah secara real-time.
38. **Dynamic Buffer Management:** Pola *Allocate -> Use -> Free* yang aman untuk mencegah kebocoran memori (Memory Leak).

#### **BAGIAN 9: Power & System State (39-42)**

*Fokus: Efisiensi energi dan monitoring.*

39. **Tickless Idle Implementation:** Mengaktifkan mode *Tickless* agar MCU tidur total (Stop Mode) saat tidak ada task yang perlu jalan, bangun otomatis saat timer habis.
40. **Run-Time Statistics:** Menggunakan Timer Hardware kecepatan tinggi untuk mengukur % penggunaan CPU per task (profiling).
41. **System State Machine:** Mengelola status global (Init, Ready, Error, Maintenance) menggunakan  *Event Groups* .
42. **Backup Register Preservation:** Menyimpan status task terakhir ke *Backup Registers* (RTC Domain) sebelum reset.

#### **BAGIAN 10: Industrial Design Patterns (43-50)**

*Fokus: Menggabungkan semua konsep menjadi aplikasi nyata.*

43. **CLI (Command Line Interface) Shell:** Sistem operasi mini. User mengetik perintah di Serial ("status", "led on", "reset"), diproses oleh Task Parser.
44. **Sensor Fusion System:** Mengambil data dari 3 sensor berbeda (ADC, I2C, SPI), disinkronisasi dengan  *Queue Sets* , lalu dirata-rata.
45. **SD Card Data Logger (Non-Blocking):** Task Sensor mengisi buffer, Task SD Card menulis ke kartu memori hanya saat buffer setengah penuh.
46. **Modbus RTU Slave Emulator:** Implementasi protokol industri dengan timeout ketat menggunakan Software Timer.
47. **IoT MQTT Simulation:** Simulasi Task Network (Kirim Data), Task Keep-Alive (Ping), dan Task Sensor yang berjalan asinkron.
48. **Double Buffering Audio/Signal:** Pola Ping-Pong Buffer. Satu buffer diisi ADC, buffer lainnya diproses DSP. Tukar saat penuh.
49. **Emergency Stop Handler:** Task prioritas tertinggi yang dipicu interrupt tombol darurat. Langsung *Suspend* semua task lain dan matikan aktuator.
50. **System Health Monitor:** "Task Dokter" yang memantau detak jantung (flags) task lain. Jika ada task macet/hang, sistem melakukan  *Soft Reset* .

---

### 📝 Catatan Penting untuk Memulai

1. **Jangan Pakai `HAL_Delay`:** Dalam semua program di atas, `HAL_Delay` dilarang digunakan di dalam Task karena memblokir CPU (Busy Wait). Gunakan `vTaskDelay` atau `osDelay`.
2. **Prioritas Task:** Kunci keberhasilan program 1-50 adalah pengaturan prioritas (`osPriorityLow`, `osPriorityNormal`, `osPriorityHigh`).
3. **Config FreeRTOS:** Pastikan di `FreeRTOSConfig.h`, fitur seperti `configUSE_IDLE_HOOK`, `configUSE_TICK_HOOK`, dan `configCHECK_FOR_STACK_OVERFLOW` diaktifkan sesuai kebutuhan program.
