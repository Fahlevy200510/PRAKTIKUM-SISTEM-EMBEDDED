# Program 39: W5500 DHCP Initialization

## Deskripsi
Initialize W5500 Ethernet controller dan setup network parameters.

## Hardware
- W5500 Ethernet Module
- Ethernet cable
- STM32F103C8T6

## Wiring
```
W5500 → STM32
MOSI → PA7
MISO → PA6
SCK  → PA5
CS   → PA4
RST  → PA3
VCC  → 3.3V
GND  → GND
```

## Build
```bash
pio run -t upload
pio device monitor
```

## Key Concepts
- W5500 SPI communication
- Network initialization
- MAC address configuration
