# Program 34: FreeRTOS Mutex - Mutual Exclusion & Priority Inheritance

## Deskripsi
Demonstrasi **FreeRTOS Mutex** untuk melindungi shared resource (UART) dengan priority inheritance untuk menghindari priority inversion problem.

## Hardware Requirements
- STM32F103C8T6 Blue Pill
- 2x LED di PC13 & PC14
- USB-UART adapter di PA9/PA10

## Konsep Utama

### Priority Inheritance
Ketika low priority task memegang mutex dan high priority task menunggu, low priority task **temporarily inherits** priority dari high priority task untuk menyelesaikan critical section lebih cepat.

### Without Priority Inheritance (Problem):
```
HIGH task → waiting for mutex
MEDIUM task → running (preempts LOW)
LOW task → holds mutex (blocked by MEDIUM)
Result: HIGH task starves!
```

### With Priority Inheritance (Solution):
```
HIGH task → waiting for mutex
LOW task → priority boosted to HIGH (finishes fast)
LOW task → releases mutex
HIGH task → gets mutex immediately
```

## Expected Output
```
=== FreeRTOS Mutex Demo ===

[HIGH-1] Tick:2000 | H:1 M:2 L:4 | Timeout:0
[LOW-5] Background log | Tick:2500
[HIGH-2] Tick:4000 | H:2 M:4 L:8 | Timeout:0
```

## License
Educational use - Based on "Mastering STM32"
