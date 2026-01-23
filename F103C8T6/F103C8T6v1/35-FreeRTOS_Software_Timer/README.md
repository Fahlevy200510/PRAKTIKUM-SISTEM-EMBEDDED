# Program 35: FreeRTOS Software Timer

Software timers untuk periodic & one-shot operations tanpa blocking tasks.

## Hardware
- PC13: LED Periodic (1s toggle)
- PC14: LED One-shot (5s pulse)
- PC15: LED Heartbeat (200ms toggle)
- PA9/PA10: UART

## Features
- Periodic timer: Auto-reload every 1 second
- One-shot timer: Fires once after 5 seconds (then restarts)
- Heartbeat: Fast blink untuk show system alive

## Build
```bash
pio run -t upload
pio device monitor
```
