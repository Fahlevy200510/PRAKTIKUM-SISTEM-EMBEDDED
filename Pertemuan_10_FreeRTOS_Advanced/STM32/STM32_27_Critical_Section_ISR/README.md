# Program 27: Critical Section ISR

## Deskripsi
Program ini mendemonstrasikan penggunaan critical section untuk melindungi shared data yang diakses oleh task dan ISR (Interrupt Service Routine) secara bersamaan.

## Konsep Utama

### 1. Critical Section
- **Tujuan**: Melindungi shared resource dari race condition
- **Mekanisme**: Disable interrupt sementara
- **Trade-off**: Menunda response time interrupt lain

### 2. API yang Digunakan

**Untuk Task Context:**
```c
taskENTER_CRITICAL();
// Protected code
taskEXIT_CRITICAL();
```

**Untuk ISR Context:**
```c
UBaseType_t uxSavedInterruptStatus;
uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
// Protected code
taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
```

### 3. Kapan Menggunakan Critical Section
- ✅ Shared data antara task dan ISR
- ✅ Multi-step operation yang harus atomic
- ✅ Update register hardware yang sensitive
- ❌ TIDAK untuk operasi lama (> 100us)
- ❌ TIDAK jika bisa pakai queue/semaphore

## Wiring

```
STM32F103C8T6 (Blue Pill)
├── PC13 → LED onboard (indicator)
├── PA9  → TX UART1 (ke USB-TTL RX)
└── PA10 → RX UART1 (ke USB-TTL TX)

USB-TTL
├── TX → PA10 (RX STM32)
├── RX → PA9  (TX STM32)
└── GND → GND STM32
```

## Expected Output

```
=== Critical Section Demo ===
Protecting shared data between Task and ISR

[1.000s] Stats: counter=50, task=30, isr=20
         Integrity: OK (50 = 30 + 20)

[2.000s] Stats: counter=100, task=60, isr=40
         Integrity: OK (100 = 60 + 40)

[3.000s] Stats: counter=150, task=90, isr=60
         Integrity: OK (150 = 90 + 60)
```

## Build & Upload

```bash
cd Program_27_Critical_Section_ISR
pio run -t upload
pio device monitor -b 115200
```

## Key Learning Points

1. **Data Integrity**: Critical section menjamin operasi atomic
2. **ISR vs Task**: API berbeda untuk context berbeda
3. **Performance**: Critical section delay scheduling, gunakan seperlunya
4. **Debugging**: Monitor integrity counter untuk deteksi race condition

## Troubleshooting

- **Counter mismatch**: Critical section tidak dipakai dengan benar
- **System hang**: Critical section terlalu lama
- **LED tidak toggle**: Timer interrupt tidak aktif

## References
- Mastering STM32 Chapter 9: Interrupts
- FreeRTOS Manual: Critical Sections
