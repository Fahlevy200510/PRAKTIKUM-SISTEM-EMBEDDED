/*
 * =============================================================================
 * PROGRAM 46: MODBUS RTU SLAVE
 * =============================================================================
 * 
 * KONSEP DASAR:
 * =============
 * Modbus adalah protokol komunikasi serial yang digunakan secara luas
 * di dunia industri untuk komunikasi antar perangkat. Modbus RTU (Remote
 * Terminal Unit) menggunakan format biner yang efisien dengan CRC-16
 * untuk deteksi error.
 * 
 * Dalam FreeRTOS, Modbus slave diimplementasikan dengan:
 * - Task untuk menerima dan memproses frame Modbus
 * - Queue untuk buffer komunikasi
 * - Timer untuk timeout deteksi
 * 
 * STRUKTUR FRAME MODBUS RTU:
 * ==========================
 * 
 *   ┌─────────────────────────────────────────────────────────────────────┐
 *   │                      MODBUS RTU FRAME FORMAT                        │
 *   └─────────────────────────────────────────────────────────────────────┘
 *   
 *   ┌─────────┬──────────────┬────────────────────┬───────────┐
 *   │ ADDR    │ FUNCTION     │ DATA               │ CRC-16    │
 *   │ 1 byte  │ 1 byte       │ N bytes            │ 2 bytes   │
 *   └─────────┴──────────────┴────────────────────┴───────────┘
 *   
 *   ADDR: Slave address (1-247, 0 = broadcast)
 *   FUNCTION: Function code (Read/Write coils, registers, etc.)
 *   DATA: Request/Response data (variable length)
 *   CRC-16: Error checking (low byte first, then high byte)
 * 
 * 
 * MODBUS FUNCTION CODES (Yang Diimplementasikan):
 * ================================================
 * 
 *   ┌──────────┬─────────────────────────────────────────────────┐
 *   │ Code     │ Function                                        │
 *   ├──────────┼─────────────────────────────────────────────────┤
 *   │ 0x01     │ Read Coils (digital outputs)                    │
 *   │ 0x02     │ Read Discrete Inputs (digital inputs)           │
 *   │ 0x03     │ Read Holding Registers (read/write registers)   │
 *   │ 0x04     │ Read Input Registers (read-only registers)      │
 *   │ 0x05     │ Write Single Coil                               │
 *   │ 0x06     │ Write Single Register                           │
 *   └──────────┴─────────────────────────────────────────────────┘
 * 
 * 
 * REGISTER MAP:
 * =============
 * 
 *   ┌──────────────────────────────────────────────────────────────────────┐
 *   │                        MODBUS REGISTER MAP                           │
 *   ├──────────────────────────────────────────────────────────────────────┤
 *   │ COILS (0x01/0x05) - Digital Outputs                                  │
 *   ├─────────┬────────────────────────────────────────────────────────────┤
 *   │ Addr    │ Description                                                │
 *   │ 0x0000  │ LED 1 (PC13)                                               │
 *   │ 0x0001  │ Relay 1 (simulated)                                        │
 *   │ 0x0002  │ Relay 2 (simulated)                                        │
 *   │ 0x0003  │ Buzzer (simulated)                                         │
 *   ├──────────────────────────────────────────────────────────────────────┤
 *   │ DISCRETE INPUTS (0x02) - Digital Inputs                              │
 *   ├─────────┬────────────────────────────────────────────────────────────┤
 *   │ 0x0000  │ Button 1 (simulated)                                       │
 *   │ 0x0001  │ Sensor 1 (simulated)                                       │
 *   │ 0x0002  │ Alarm flag                                                 │
 *   ├──────────────────────────────────────────────────────────────────────┤
 *   │ HOLDING REGISTERS (0x03/0x06) - Read/Write 16-bit                    │
 *   ├─────────┬────────────────────────────────────────────────────────────┤
 *   │ 0x0000  │ Setpoint 1                                                 │
 *   │ 0x0001  │ Setpoint 2                                                 │
 *   │ 0x0002  │ Control Mode                                               │
 *   │ 0x0003  │ Alarm Threshold                                            │
 *   ├──────────────────────────────────────────────────────────────────────┤
 *   │ INPUT REGISTERS (0x04) - Read-only 16-bit                            │
 *   ├─────────┬────────────────────────────────────────────────────────────┤
 *   │ 0x0000  │ Temperature (°C × 10)                                      │
 *   │ 0x0001  │ Humidity (% × 10)                                          │
 *   │ 0x0002  │ Pressure (hPa)                                             │
 *   │ 0x0003  │ ADC Value                                                  │
 *   │ 0x0004  │ System Uptime (seconds)                                    │
 *   │ 0x0005  │ Device Status                                              │
 *   └─────────┴────────────────────────────────────────────────────────────┘
 * 
 * 
 * ARSITEKTUR SISTEM:
 * ==================
 * 
 *   ┌─────────────────────────────────────────────────────────────────────┐
 *   │                      MODBUS RTU SLAVE SYSTEM                        │
 *   └─────────────────────────────────────────────────────────────────────┘
 *   
 *   MODBUS MASTER                                STM32 SLAVE
 *   ─────────────                                ───────────
 *        │                                            │
 *        │  TX: [01][03][00][00][00][02][CRC16]       │
 *        │  (Read 2 holding regs from addr 0)        │
 *        ├───────────────────────────────────────────►│
 *        │                                   ┌────────┴────────┐
 *        │                                   │   UART RX ISR   │
 *        │                                   │                 │
 *        │                                   │  Frame detect   │
 *        │                                   │  using 3.5      │
 *        │                                   │  char timeout   │
 *        │                                   └────────┬────────┘
 *        │                                            │
 *        │                                   ┌────────▼────────┐
 *        │                                   │  MODBUS TASK    │
 *        │                                   │                 │
 *        │                                   │ 1. Validate CRC │
 *        │                                   │ 2. Check addr   │
 *        │                                   │ 3. Process func │
 *        │                                   │ 4. Build resp   │
 *        │                                   └────────┬────────┘
 *        │                                            │
 *        │  RX: [01][03][04][HILO][HILO][CRC16]       │
 *        │  (4 bytes data, 2 registers)              │
 *        │◄───────────────────────────────────────────┤
 *        │                                            │
 * 
 * 
 * TIMING DIAGRAM (Inter-frame delay):
 * ====================================
 * 
 *   Baud 9600: 1 char ≈ 1ms, 3.5 char ≈ 3.5ms
 *   
 *   ┌────────────────────────────────────────────────────────────────┐
 *   │                                                                │
 *   │  ╔═══════════════════════╗     ╔═══════════════════════╗      │
 *   │  ║    FRAME 1            ║     ║    FRAME 2            ║      │
 *   │  ║ [01][03][00][00]...   ║     ║ [01][06][00][01]...   ║      │
 *   │  ╚═══════════════════════╝     ╚═══════════════════════╝      │
 *   │           │                              │                     │
 *   │           │◄────── ≥3.5 char ──────────►│                     │
 *   │           │       (Inter-frame gap)      │                     │
 *   │                                                                │
 *   └────────────────────────────────────────────────────────────────┘
 * 
 * 
 * CRC-16 CALCULATION:
 * ===================
 * 
 *   Algorithm: CRC-16-IBM (polynomial 0x8005, reflected)
 *   
 *   Initial value: 0xFFFF
 *   For each byte in frame:
 *       CRC = CRC XOR byte
 *       For 8 bits:
 *           If LSB = 1: CRC = (CRC >> 1) XOR 0xA001
 *           Else: CRC = CRC >> 1
 *   
 *   Result: Low byte first in frame, then high byte
 * 
 * 
 * ERROR RESPONSES:
 * ================
 * 
 *   ┌──────────┬────────────────────────────────────────────────┐
 *   │ Code     │ Exception                                      │
 *   ├──────────┼────────────────────────────────────────────────┤
 *   │ 0x01     │ Illegal Function (not supported)               │
 *   │ 0x02     │ Illegal Data Address (out of range)            │
 *   │ 0x03     │ Illegal Data Value (invalid value)             │
 *   │ 0x04     │ Slave Device Failure (internal error)          │
 *   └──────────┴────────────────────────────────────────────────┘
 *   
 *   Error response format:
 *   [ADDR][FUNC+0x80][EXCEPTION_CODE][CRC16]
 * 
 * 
 * EXPECTED SERIAL OUTPUT (Debug UART):
 * ====================================
 * 
 *   === MODBUS RTU SLAVE DEMO ===
 *   Slave Address: 1
 *   Baud Rate: 9600
 *   
 *   [MODBUS] Initialized
 *   [MODBUS] Waiting for master...
 *   
 *   [RX] Frame received: 8 bytes
 *   [RX] 01 03 00 00 00 02 C4 0B
 *   [MODBUS] Addr: 1, Func: 03 (Read Holding Registers)
 *   [MODBUS] Start: 0, Count: 2
 *   [TX] Response: 9 bytes
 *   [TX] 01 03 04 01 F4 00 64 XX XX
 *   
 *   [RX] Frame received: 8 bytes
 *   [RX] 01 06 00 00 01 00 C8 05
 *   [MODBUS] Addr: 1, Func: 06 (Write Single Register)
 *   [MODBUS] Reg: 0, Value: 256
 *   [TX] Response: 8 bytes (echo request)
 * 
 * 
 * PENGGUNAAN PRAKTIS:
 * ===================
 * 1. Industrial PLC communication
 * 2. SCADA systems
 * 3. Building automation (BMS)
 * 4. Energy meters
 * 5. Remote sensor monitoring
 * 
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* STM32F103 @ 72MHz */
#define configCPU_CLOCK_HZ                       72000000UL
#define configTICK_RATE_HZ                       1000
#define configSYSTICK_CLOCK_HZ                   configCPU_CLOCK_HZ

/* Scheduler */
#define configUSE_PREEMPTION                     1
#define configUSE_TIME_SLICING                   1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION  0

/* Memory */
#define configMINIMAL_STACK_SIZE                 128
#define configTOTAL_HEAP_SIZE                    10240
#define configMAX_TASK_NAME_LEN                  12

/* Task configuration */
#define configMAX_PRIORITIES                     5
#define configIDLE_SHOULD_YIELD                  1

/* Feature flags */
#define configUSE_MUTEXES                        1
#define configUSE_COUNTING_SEMAPHORES            1
#define configUSE_RECURSIVE_MUTEXES              0
#define configUSE_QUEUE_SETS                     0
#define configUSE_TASK_NOTIFICATIONS             1
#define configUSE_TIMERS                         1
#define configUSE_16_BIT_TICKS                   0

/* Timer configuration */
#define configTIMER_TASK_PRIORITY                3
#define configTIMER_QUEUE_LENGTH                 5
#define configTIMER_TASK_STACK_DEPTH             128

/* Hook functions */
#define configUSE_IDLE_HOOK                      0
#define configUSE_TICK_HOOK                      0
#define configUSE_MALLOC_FAILED_HOOK             1
#define configCHECK_FOR_STACK_OVERFLOW           2

/* Debug/Stats */
#define configUSE_TRACE_FACILITY                 1
#define configGENERATE_RUN_TIME_STATS            0

/* Co-routines disabled */
#define configUSE_CO_ROUTINES                    0

/* API functions */
#define INCLUDE_vTaskPrioritySet                 1
#define INCLUDE_uxTaskPriorityGet                1
#define INCLUDE_vTaskDelete                      1
#define INCLUDE_vTaskSuspend                     1
#define INCLUDE_vTaskDelayUntil                  1
#define INCLUDE_vTaskDelay                       1
#define INCLUDE_xTaskGetSchedulerState           1
#define INCLUDE_xTaskGetCurrentTaskHandle        1
#define INCLUDE_uxTaskGetStackHighWaterMark      1

/* ARM Cortex-M3 specific */
#ifdef __NVIC_PRIO_BITS
    #define configPRIO_BITS __NVIC_PRIO_BITS
#else
    #define configPRIO_BITS 4
#endif

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY      15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5
#define configKERNEL_INTERRUPT_PRIORITY         (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/* Interrupt handlers */
#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

/* Assertion */
#define configASSERT(x) if((x) == 0) { taskDISABLE_INTERRUPTS(); for(;;); }

/* ==========================================
 * MODBUS RTU CONFIGURATION
 * ==========================================*/
#define MODBUS_SLAVE_ADDRESS                     1
#define MODBUS_BAUD_RATE                         9600
#define MODBUS_FRAME_MAX_SIZE                    256
#define MODBUS_INTERFRAME_DELAY_MS               5   /* 3.5 char at 9600 ≈ 4ms */

/* Register counts */
#define MODBUS_NUM_COILS                         8
#define MODBUS_NUM_DISCRETE_INPUTS               8
#define MODBUS_NUM_HOLDING_REGS                  8
#define MODBUS_NUM_INPUT_REGS                    8

/* Function codes */
#define MODBUS_FC_READ_COILS                     0x01
#define MODBUS_FC_READ_DISCRETE_INPUTS           0x02
#define MODBUS_FC_READ_HOLDING_REGS              0x03
#define MODBUS_FC_READ_INPUT_REGS                0x04
#define MODBUS_FC_WRITE_SINGLE_COIL              0x05
#define MODBUS_FC_WRITE_SINGLE_REG               0x06

/* Exception codes */
#define MODBUS_EX_ILLEGAL_FUNCTION               0x01
#define MODBUS_EX_ILLEGAL_DATA_ADDRESS           0x02
#define MODBUS_EX_ILLEGAL_DATA_VALUE             0x03
#define MODBUS_EX_SLAVE_DEVICE_FAILURE           0x04

#endif /* FREERTOS_CONFIG_H */
