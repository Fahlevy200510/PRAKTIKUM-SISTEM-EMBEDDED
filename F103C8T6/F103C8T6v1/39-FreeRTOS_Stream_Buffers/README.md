# Program 39: FreeRTOS Stream Buffers

Variable-length data passing menggunakan **Stream Buffer** (continuous byte stream) dan **Message Buffer** (discrete messages).

## Hardware
- PA9/PA10: USART1 115200 (log output)
- PC13: LED (data indicator)

## Fitur
- Producer task kirim payload variabel ke Stream Buffer
- Consumer task baca Stream Buffer dan tampilkan via UART
- Message Buffer untuk pesan diskrit (null-terminated)
- Trigger level 16 byte untuk wake-up consumer lebih efisien

## Build & Run
```bash
pio run -t upload
pio device monitor -b 115200
```

## Ekspektasi Output
```
[STREAM CONS] sensor:23.4
[MSG CONS] MSG #3 @tick 2100
```
