# Program 41: I2C Multi Sensor

Membaca multiple sensor di I2C bus yang sama: BMP280 (0x76) dan MPU6050 (0x68).

## Hardware
- PB6: I2C1_SCL (pull-up 4.7kΩ ke 3.3V)
- PB7: I2C1_SDA (pull-up 4.7kΩ ke 3.3V)
- BMP280: Address 0x76
- MPU6050: Address 0x68
- PA9/PA10: UART1 115200 untuk log

## Build & Run
```bash
pio run -t upload
pio device monitor -b 115200
```

## Output yang diharapkan
```
I2C scan...
- Found device at 0x68
- Found device at 0x76
Scan done.

BMP280 ID: 0x58 (OK)
MPU6050 WHO_AM_I: 0x68 (OK)
Looping read every 2s...
BMP280 ID: 0x58
MPU6050 ID: 0x68
```
