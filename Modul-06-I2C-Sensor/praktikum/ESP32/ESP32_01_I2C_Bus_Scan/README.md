# 01_I2C_Bus_Scan

## Description
I2C bus scanner for ESP32

## Key Features
- Bus scan
- Device enumeration
- GPIO21/22

## Hardware Requirements
- ESP32 DevKitC
- USB-TTL or USB directly
- Required sensors/components

## Wiring Diagram
```
(Define wiring here)
```

## Expected Output
```
(Define expected serial output)
```

## Build and Run
```bash
cd 01_I2C_Bus_Scan
platformio run -t upload
platformio device monitor
```

## Notes
- ESP-IDF framework
- FreeRTOS based
- GPIO and module specific implementation
