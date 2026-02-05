# Referensi Modul 12: FreeRTOS Memory Management & Advanced Features

## 📚 Dokumentasi Resmi

### FreeRTOS Official
1. **FreeRTOS Memory Management**
   - https://freertos.org/a00111.html
   - Penjelasan lengkap heap_1 hingga heap_5

2. **FreeRTOS Event Groups**
   - https://freertos.org/FreeRTOS-Event-Groups.html
   - API reference dan contoh penggunaan

3. **FreeRTOS Stream and Message Buffers**
   - https://freertos.org/RTOS-stream-message-buffers.html
   - Perbedaan dan use cases

4. **FreeRTOS Stack Overflow Detection**
   - https://freertos.org/Stacks-and-stack-overflow-checking.html
   - Konfigurasi dan hook functions

5. **FreeRTOS Static Allocation**
   - https://freertos.org/Static_Vs_Dynamic_Memory_Allocation.html
   - xTaskCreateStatic() dan resource statis lainnya

6. **FreeRTOS Low Power Support**
   - https://freertos.org/low-power-tickless-rtos.html
   - Tickless idle mode implementation

### STM32 Official
7. **STM32F103 Reference Manual (RM0008)**
   - https://www.st.com/resource/en/reference_manual/rm0008.pdf
   - Memory organization dan low power modes

8. **STM32 Low Power Modes Application Note (AN4621)**
   - https://www.st.com/resource/en/application_note/an4621.pdf
   - Sleep, Stop, Standby mode details

9. **STM32CubeF1 HAL Library**
   - https://www.st.com/en/embedded-software/stm32cubef1.html
   - Power management APIs

### ESP32 Official
10. **ESP-IDF Programming Guide - Heap Memory Allocation**
    - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/mem_alloc.html
    - Multi-heap architecture

11. **ESP-IDF FreeRTOS Additions**
    - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos_additions.html
    - ESP32-specific extensions

12. **ESP32 Power Management**
    - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/power_management.html
    - Light sleep, deep sleep

13. **ESP32 Sleep Modes**
    - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/sleep_modes.html
    - RTC memory, wakeup sources

---

## 📖 Buku Referensi

### FreeRTOS Specific
14. **"Mastering the FreeRTOS Real Time Kernel" by Richard Barry**
    - https://freertos.org/Documentation/RTOS_book.html
    - FREE PDF - Chapter 4: Memory Management
    - **Wajib baca untuk memahami heap schemes**

15. **"FreeRTOS Reference Manual"**
    - https://freertos.org/Documentation/FreeRTOS_Reference_Manual_V10.0.0.pdf
    - API reference lengkap

### Embedded Systems General
16. **"Making Embedded Systems" by Elecia White**
    - O'Reilly, 2011
    - Chapter on Memory Management
    - ISBN: 978-1449302146

17. **"Real-Time Concepts for Embedded Systems" by Qing Li**
    - CRC Press, 2003
    - Memory Management in RTOS
    - ISBN: 978-1578201242

18. **"Embedded Systems Architecture" by Daniele Lacamera**
    - Packt, 2018
    - Modern embedded memory management
    - ISBN: 978-1788832502

---

## 🎬 Video Tutorial

### YouTube Channels
19. **Shawn Hymel - FreeRTOS with ESP32**
    - "Introduction to FreeRTOS" series
    - https://www.youtube.com/watch?v=F321087yYy4
    - Episode on memory management

20. **DigiKey - FreeRTOS Tutorial**
    - "FreeRTOS with STM32" series
    - https://www.youtube.com/watch?v=WQGAs9MwXno
    - Memory and stack considerations

21. **Controllers Tech**
    - STM32 FreeRTOS tutorials
    - https://www.youtube.com/@ControllersTech
    - Low power mode implementation

22. **ESP32 Official Channel**
    - Power management tutorials
    - https://www.youtube.com/@EspressifSystems

---

## 📄 Application Notes

### Memory Management
23. **FreeRTOS Memory Management Application Note**
    - https://freertos.org/a00111.html
    - Choosing the right heap scheme

24. **ARM Cortex-M Memory Protection Unit**
    - https://developer.arm.com/documentation/dui0553/a/cortex-m4-peripherals/optional-memory-protection-unit
    - MPU untuk memory protection

### Low Power
25. **STM32 Ultra-Low-Power Features (AN4365)**
    - https://www.st.com/resource/en/application_note/an4365.pdf
    - Comprehensive low power guide

26. **ESP32 Power Consumption Optimization**
    - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/low-power-mode.html
    - Power optimization techniques

---

## 🔧 Tools dan Software

### Development
27. **PlatformIO IDE**
    - https://platformio.org/
    - Cross-platform development

28. **STM32CubeIDE**
    - https://www.st.com/en/development-tools/stm32cubeide.html
    - ST official IDE

29. **ESP-IDF**
    - https://github.com/espressif/esp-idf
    - ESP32 development framework

### Debugging
30. **Segger SystemView**
    - https://www.segger.com/products/development-tools/systemview/
    - RTOS visualization dan analysis

31. **Percepio Tracealyzer**
    - https://percepio.com/tracealyzer/
    - FreeRTOS trace analysis

32. **FreeRTOS+Trace**
    - https://freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_Trace/
    - Built-in tracing support

### Power Measurement
33. **Nordic Power Profiler Kit**
    - https://www.nordicsemi.com/Products/Development-hardware/Power-Profiler-Kit-2
    - Accurate power measurement

34. **Otii Arc**
    - https://www.qoitech.com/otii/
    - Power profiler for embedded

---

## 📝 Technical Articles

### Memory Management
35. **"Memory Management for Embedded Systems" - Embedded.com**
    - https://www.embedded.com/memory-management-for-embedded-systems/
    - Best practices

36. **"Stack Overflow Protection in Embedded Systems"**
    - https://www.embedded.com/stack-overflow-protection/
    - Detection techniques

37. **"Heap Fragmentation in Embedded Systems"**
    - https://www.embedded.com/heap-fragmentation/
    - Prevention strategies

### Low Power
38. **"Low Power Design for IoT Devices"**
    - https://www.embedded.com/low-power-design-for-iot/
    - Power optimization strategies

39. **"Tickless Mode Explained"**
    - https://mcuoneclipse.com/2016/12/06/tickless-mode-with-freertos/
    - Implementation details

---

## 🎓 Online Courses

40. **Udemy - "Mastering RTOS: Hands on FreeRTOS and STM32Fx with Debugging"**
    - Memory management sections
    - https://www.udemy.com/course/mastering-rtos-hands-on-with-freertos-arduino-and-stm32fx/

41. **Coursera - "Introduction to Embedded Systems Software and Development Environments"**
    - Memory concepts
    - https://www.coursera.org/learn/introduction-embedded-systems

42. **edX - "Embedded Systems - Shape The World"**
    - University of Texas
    - https://www.edx.org/course/embedded-systems-shape-the-world

---

## 🔬 Research Papers

43. **"Memory Management in Real-Time Operating Systems"**
    - IEEE conference papers
    - Search on IEEE Xplore

44. **"Energy-Efficient Memory Management for Embedded Systems"**
    - ACM Digital Library
    - Power-aware allocation strategies

45. **"Stack Size Estimation for Real-Time Embedded Systems"**
    - Analysis techniques
    - IEEE Embedded Systems Letters

---

## 🌐 Community Resources

### Forums
46. **FreeRTOS Community Forums**
    - https://forums.freertos.org/
    - Official support forum

47. **STM32 Community**
    - https://community.st.com/
    - ST official forum

48. **ESP32 Forum**
    - https://esp32.com/
    - Community discussions

### GitHub Repositories
49. **FreeRTOS Kernel**
    - https://github.com/FreeRTOS/FreeRTOS-Kernel
    - Source code reference

50. **ESP-IDF FreeRTOS**
    - https://github.com/espressif/esp-idf/tree/master/components/freertos
    - ESP32 port

---

## 📋 Quick Reference Cards

### Heap Scheme Selection
```
┌─────────────────────────────────────────────────────┐
│ HEAP SCHEME SELECTION GUIDE                         │
├─────────────────────────────────────────────────────┤
│ No free needed?        → heap_1                     │
│ Same-size alloc/free?  → heap_2                     │
│ Using standard malloc? → heap_3                     │
│ General purpose?       → heap_4 (RECOMMENDED)       │
│ Multiple RAM regions?  → heap_5                     │
└─────────────────────────────────────────────────────┘
```

### Stack Size Guidelines
```
┌─────────────────────────────────────────────────────┐
│ STACK SIZE GUIDELINES (32-bit MCU, in words)        │
├─────────────────────────────────────────────────────┤
│ Minimal (no printf):        128 - 192 words         │
│ Basic task:                 256 - 384 words         │
│ With printf:                512 - 768 words         │
│ Complex operations:         1024+ words             │
│                                                     │
│ Rule: Monitor high water mark, keep 20-30% margin   │
└─────────────────────────────────────────────────────┘
```

### Memory Debugging APIs
```
┌─────────────────────────────────────────────────────┐
│ MEMORY DEBUGGING QUICK REFERENCE                    │
├─────────────────────────────────────────────────────┤
│ xPortGetFreeHeapSize()                              │
│   → Current free heap bytes                         │
│                                                     │
│ xPortGetMinimumEverFreeHeapSize()                   │
│   → Lowest ever free heap (high water mark)         │
│                                                     │
│ uxTaskGetStackHighWaterMark(TaskHandle)             │
│   → Minimum free stack words                        │
│                                                     │
│ vPortGetHeapStats(&HeapStats_t)                     │
│   → Detailed heap statistics                        │
│                                                     │
│ vTaskList(buffer)                                   │
│   → All task info with stack                        │
│                                                     │
│ vTaskGetRunTimeStats(buffer)                        │
│   → CPU usage per task                              │
└─────────────────────────────────────────────────────┘
```
