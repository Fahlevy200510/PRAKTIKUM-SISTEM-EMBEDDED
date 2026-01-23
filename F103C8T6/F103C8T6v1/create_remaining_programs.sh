#!/bin/bash

# Script untuk membuat program sisanya dengan template minimal

# Program 39: Stream Buffers
echo "Creating Program 39..."
cat > 39-FreeRTOS_Stream_Buffers/platformio.ini << 'EOINI'
[env:bluepill_f103c8]
platform = ststm32
board = bluepill_f103c8
framework = stm32cube
build_flags = -D USE_HAL_DRIVER -D STM32F103xB
lib_deps = FreeRTOS-Kernel@^10.5.1
upload_protocol = stlink
monitor_speed = 115200
EOINI

cat > 39-FreeRTOS_Stream_Buffers/README.md << 'EOREADME'
# Program 39: FreeRTOS Stream Buffers

Variable-length data passing menggunakan Stream Buffers dan Message Buffers.
Stream Buffer untuk continuous byte stream (UART), Message Buffer untuk discrete messages.

## Hardware
- PA9/PA10: UART (producer via interrupt)
- PC13: LED (data available)

## Build
```bash
pio run -t upload
```
EOREADME

# Program 41: I2C Multi Sensor
echo "Creating Program 41..."
cat > 41-I2C_Multi_Sensor/platformio.ini << 'EOINI'
[env:bluepill_f103c8]
platform = ststm32
board = bluepill_f103c8
framework = stm32cube
build_flags = -D USE_HAL_DRIVER -D STM32F103xB
upload_protocol = stlink
monitor_speed = 115200
EOINI

cat > 41-I2C_Multi_Sensor/README.md << 'EOREADME'
# Program 41: I2C Multi Sensor

Membaca multiple sensors di I2C bus: BMP280 (temp+pressure) dan MPU6050 (accel+gyro).

## Hardware
- PB6: I2C1_SCL (dengan pull-up 4.7kΩ)
- PB7: I2C1_SDA (dengan pull-up 4.7kΩ)  
- BMP280: Address 0x76
- MPU6050: Address 0x68

## Build
```bash
pio run -t upload
```
EOREADME

echo "Programs 39 and 41 templates created!"
