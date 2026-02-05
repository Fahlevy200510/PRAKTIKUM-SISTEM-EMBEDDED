/**
 * =============================================================================
 * PROGRAM 46: MODBUS RTU SLAVE - Industrial Protocol dengan FreeRTOS
 * =============================================================================
 * 
 * Demo Modbus RTU slave yang mengimplementasikan function codes dasar:
 * - 0x03: Read Holding Registers
 * - 0x04: Read Input Registers
 * - 0x06: Write Single Register
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include <stdio.h>
#include <string.h>

/* ========================== HARDWARE CONFIGURATION ======================== */
#define LED_PIN         GPIO_PIN_13
#define LED_PORT        GPIOC

/* ========================== DATA STRUCTURES =============================== */

/* Modbus frame buffer */
typedef struct {
    uint8_t  ucBuffer[MODBUS_FRAME_MAX_SIZE];
    uint16_t usLength;
} ModbusFrame_t;

/* ========================== GLOBALS ======================================= */

UART_HandleTypeDef huart1;

/* Modbus frame handling */
static volatile ModbusFrame_t xRxFrame;
static volatile uint8_t ucRxInProgress = 0;
static volatile TickType_t xLastRxTime = 0;

/* Modbus register storage */
static uint8_t  ucCoils[MODBUS_NUM_COILS];
static uint8_t  ucDiscreteInputs[MODBUS_NUM_DISCRETE_INPUTS];
static uint16_t usHoldingRegs[MODBUS_NUM_HOLDING_REGS];
static uint16_t usInputRegs[MODBUS_NUM_INPUT_REGS];

/* Register access mutex */
static SemaphoreHandle_t xRegisterMutex;

/* Task handles */
static TaskHandle_t xModbusTaskHandle;
static TaskHandle_t xSimulationTaskHandle;

/* Frame ready semaphore */
static SemaphoreHandle_t xFrameReadySem;

/* Statistics */
static volatile uint32_t ulFramesReceived = 0;
static volatile uint32_t ulFramesSent = 0;
static volatile uint32_t ulCrcErrors = 0;
static volatile uint32_t ulExceptions = 0;

/* ========================== FUNCTION PROTOTYPES =========================== */

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART1_Init(void);
static void UART_SendString(const char *str);
static void UART_SendBytes(const uint8_t *pucData, uint16_t usLen);

static void vModbusTask(void *pvParameters);
static void vSimulationTask(void *pvParameters);

static uint16_t usCalculateCRC16(const uint8_t *pucData, uint16_t usLen);
static void vProcessModbusFrame(const uint8_t *pucFrame, uint16_t usLen);
static void vSendModbusResponse(const uint8_t *pucData, uint16_t usLen);
static void vSendModbusException(uint8_t ucFuncCode, uint8_t ucExceptionCode);

/* ========================== CRC-16 CALCULATION ============================ */

/**
 * Calculate CRC-16-IBM (Modbus CRC)
 */
static uint16_t usCalculateCRC16(const uint8_t *pucData, uint16_t usLen)
{
    uint16_t usCRC = 0xFFFF;
    uint16_t i, j;
    
    for (i = 0; i < usLen; i++)
    {
        usCRC ^= pucData[i];
        
        for (j = 0; j < 8; j++)
        {
            if (usCRC & 0x0001)
            {
                usCRC = (usCRC >> 1) ^ 0xA001;
            }
            else
            {
                usCRC = usCRC >> 1;
            }
        }
    }
    
    return usCRC;
}

/* ========================== MODBUS PROCESSING ============================= */

/**
 * Send Modbus exception response
 */
static void vSendModbusException(uint8_t ucFuncCode, uint8_t ucExceptionCode)
{
    uint8_t ucResponse[5];
    uint16_t usCRC;
    
    ucResponse[0] = MODBUS_SLAVE_ADDRESS;
    ucResponse[1] = ucFuncCode | 0x80;  /* Set error bit */
    ucResponse[2] = ucExceptionCode;
    
    usCRC = usCalculateCRC16(ucResponse, 3);
    ucResponse[3] = usCRC & 0xFF;         /* CRC low */
    ucResponse[4] = (usCRC >> 8) & 0xFF;  /* CRC high */
    
    UART_SendBytes(ucResponse, 5);
    ulExceptions++;
}

/**
 * Send Modbus response with CRC
 */
static void vSendModbusResponse(const uint8_t *pucData, uint16_t usLen)
{
    uint8_t ucBuffer[MODBUS_FRAME_MAX_SIZE];
    uint16_t usCRC;
    
    if (usLen > MODBUS_FRAME_MAX_SIZE - 2)
    {
        return;
    }
    
    memcpy(ucBuffer, pucData, usLen);
    
    usCRC = usCalculateCRC16(ucBuffer, usLen);
    ucBuffer[usLen] = usCRC & 0xFF;           /* CRC low */
    ucBuffer[usLen + 1] = (usCRC >> 8) & 0xFF; /* CRC high */
    
    UART_SendBytes(ucBuffer, usLen + 2);
    ulFramesSent++;
}

/**
 * Process Read Holding Registers (0x03)
 */
static void vProcessReadHoldingRegs(const uint8_t *pucFrame)
{
    uint16_t usStartAddr = ((uint16_t)pucFrame[2] << 8) | pucFrame[3];
    uint16_t usQuantity = ((uint16_t)pucFrame[4] << 8) | pucFrame[5];
    uint8_t ucResponse[5 + MODBUS_NUM_HOLDING_REGS * 2];
    uint16_t i;
    char cBuffer[80];
    
    snprintf(cBuffer, sizeof(cBuffer), 
             "[MODBUS] FC03: Read %d regs from addr %d\r\n", 
             usQuantity, usStartAddr);
    UART_SendString(cBuffer);
    
    /* Validate address range */
    if ((usStartAddr + usQuantity) > MODBUS_NUM_HOLDING_REGS)
    {
        vSendModbusException(MODBUS_FC_READ_HOLDING_REGS, MODBUS_EX_ILLEGAL_DATA_ADDRESS);
        return;
    }
    
    /* Build response */
    ucResponse[0] = MODBUS_SLAVE_ADDRESS;
    ucResponse[1] = MODBUS_FC_READ_HOLDING_REGS;
    ucResponse[2] = usQuantity * 2;  /* Byte count */
    
    /* Copy register values */
    if (xSemaphoreTake(xRegisterMutex, pdMS_TO_TICKS(50)) == pdTRUE)
    {
        for (i = 0; i < usQuantity; i++)
        {
            ucResponse[3 + i * 2] = (usHoldingRegs[usStartAddr + i] >> 8) & 0xFF;
            ucResponse[4 + i * 2] = usHoldingRegs[usStartAddr + i] & 0xFF;
        }
        xSemaphoreGive(xRegisterMutex);
    }
    
    vSendModbusResponse(ucResponse, 3 + usQuantity * 2);
}

/**
 * Process Read Input Registers (0x04)
 */
static void vProcessReadInputRegs(const uint8_t *pucFrame)
{
    uint16_t usStartAddr = ((uint16_t)pucFrame[2] << 8) | pucFrame[3];
    uint16_t usQuantity = ((uint16_t)pucFrame[4] << 8) | pucFrame[5];
    uint8_t ucResponse[5 + MODBUS_NUM_INPUT_REGS * 2];
    uint16_t i;
    char cBuffer[80];
    
    snprintf(cBuffer, sizeof(cBuffer), 
             "[MODBUS] FC04: Read %d input regs from addr %d\r\n", 
             usQuantity, usStartAddr);
    UART_SendString(cBuffer);
    
    /* Validate address range */
    if ((usStartAddr + usQuantity) > MODBUS_NUM_INPUT_REGS)
    {
        vSendModbusException(MODBUS_FC_READ_INPUT_REGS, MODBUS_EX_ILLEGAL_DATA_ADDRESS);
        return;
    }
    
    /* Build response */
    ucResponse[0] = MODBUS_SLAVE_ADDRESS;
    ucResponse[1] = MODBUS_FC_READ_INPUT_REGS;
    ucResponse[2] = usQuantity * 2;  /* Byte count */
    
    /* Copy register values */
    if (xSemaphoreTake(xRegisterMutex, pdMS_TO_TICKS(50)) == pdTRUE)
    {
        for (i = 0; i < usQuantity; i++)
        {
            ucResponse[3 + i * 2] = (usInputRegs[usStartAddr + i] >> 8) & 0xFF;
            ucResponse[4 + i * 2] = usInputRegs[usStartAddr + i] & 0xFF;
        }
        xSemaphoreGive(xRegisterMutex);
    }
    
    vSendModbusResponse(ucResponse, 3 + usQuantity * 2);
}

/**
 * Process Write Single Register (0x06)
 */
static void vProcessWriteSingleReg(const uint8_t *pucFrame)
{
    uint16_t usRegAddr = ((uint16_t)pucFrame[2] << 8) | pucFrame[3];
    uint16_t usValue = ((uint16_t)pucFrame[4] << 8) | pucFrame[5];
    uint8_t ucResponse[6];
    char cBuffer[80];
    
    snprintf(cBuffer, sizeof(cBuffer), 
             "[MODBUS] FC06: Write reg %d = %d\r\n", usRegAddr, usValue);
    UART_SendString(cBuffer);
    
    /* Validate address */
    if (usRegAddr >= MODBUS_NUM_HOLDING_REGS)
    {
        vSendModbusException(MODBUS_FC_WRITE_SINGLE_REG, MODBUS_EX_ILLEGAL_DATA_ADDRESS);
        return;
    }
    
    /* Write value */
    if (xSemaphoreTake(xRegisterMutex, pdMS_TO_TICKS(50)) == pdTRUE)
    {
        usHoldingRegs[usRegAddr] = usValue;
        xSemaphoreGive(xRegisterMutex);
    }
    
    /* LED control if reg 0 */
    if (usRegAddr == 0)
    {
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, usValue ? GPIO_PIN_RESET : GPIO_PIN_SET);
    }
    
    /* Response is echo of request (without CRC) */
    memcpy(ucResponse, pucFrame, 6);
    vSendModbusResponse(ucResponse, 6);
}

/**
 * Process received Modbus frame
 */
static void vProcessModbusFrame(const uint8_t *pucFrame, uint16_t usLen)
{
    uint16_t usRecvCRC, usCalcCRC;
    uint8_t ucSlaveAddr, ucFuncCode;
    char cBuffer[80];
    
    /* Minimum frame: addr(1) + func(1) + data(min 1) + crc(2) = 5 bytes */
    if (usLen < 5)
    {
        UART_SendString("[MODBUS] Frame too short\r\n");
        return;
    }
    
    /* Verify CRC */
    usRecvCRC = ((uint16_t)pucFrame[usLen - 1] << 8) | pucFrame[usLen - 2];
    usCalcCRC = usCalculateCRC16(pucFrame, usLen - 2);
    
    if (usRecvCRC != usCalcCRC)
    {
        snprintf(cBuffer, sizeof(cBuffer), 
                 "[MODBUS] CRC error: recv=0x%04X, calc=0x%04X\r\n",
                 usRecvCRC, usCalcCRC);
        UART_SendString(cBuffer);
        ulCrcErrors++;
        return;
    }
    
    ucSlaveAddr = pucFrame[0];
    ucFuncCode = pucFrame[1];
    
    /* Check slave address (0 = broadcast, accept but no response) */
    if (ucSlaveAddr != MODBUS_SLAVE_ADDRESS && ucSlaveAddr != 0)
    {
        return;  /* Not for us, ignore */
    }
    
    snprintf(cBuffer, sizeof(cBuffer), 
             "[MODBUS] Addr: %d, Func: 0x%02X\r\n", ucSlaveAddr, ucFuncCode);
    UART_SendString(cBuffer);
    
    /* Process function code */
    switch (ucFuncCode)
    {
        case MODBUS_FC_READ_HOLDING_REGS:
            vProcessReadHoldingRegs(pucFrame);
            break;
            
        case MODBUS_FC_READ_INPUT_REGS:
            vProcessReadInputRegs(pucFrame);
            break;
            
        case MODBUS_FC_WRITE_SINGLE_REG:
            vProcessWriteSingleReg(pucFrame);
            break;
            
        default:
            snprintf(cBuffer, sizeof(cBuffer), 
                     "[MODBUS] Unknown function: 0x%02X\r\n", ucFuncCode);
            UART_SendString(cBuffer);
            vSendModbusException(ucFuncCode, MODBUS_EX_ILLEGAL_FUNCTION);
            break;
    }
}

/* ========================== TASKS ========================================= */

/**
 * Modbus Task - Handles frame reception and processing
 */
static void vModbusTask(void *pvParameters)
{
    (void)pvParameters;
    
    char cBuffer[80];
    uint8_t ucRxByte;
    TickType_t xCurrentTime;
    ModbusFrame_t xFrame;
    uint16_t i;
    
    UART_SendString("[MODBUS] Task started\r\n");
    UART_SendString("[MODBUS] Waiting for master...\r\n\r\n");
    
    xRxFrame.usLength = 0;
    
    for (;;)
    {
        /* Check for received byte */
        if (HAL_UART_Receive(&huart1, &ucRxByte, 1, 1) == HAL_OK)
        {
            xCurrentTime = xTaskGetTickCount();
            
            /* Check for inter-frame delay (new frame) */
            if ((xCurrentTime - xLastRxTime) >= pdMS_TO_TICKS(MODBUS_INTERFRAME_DELAY_MS))
            {
                if (xRxFrame.usLength > 0)
                {
                    /* Process previous frame */
                    memcpy(&xFrame, (void *)&xRxFrame, sizeof(ModbusFrame_t));
                    xRxFrame.usLength = 0;
                    
                    ulFramesReceived++;
                    
                    /* Print received frame */
                    snprintf(cBuffer, sizeof(cBuffer), 
                             "\r\n[RX] Frame: %d bytes: ", xFrame.usLength);
                    UART_SendString(cBuffer);
                    
                    for (i = 0; i < xFrame.usLength && i < 16; i++)
                    {
                        snprintf(cBuffer, sizeof(cBuffer), "%02X ", xFrame.ucBuffer[i]);
                        UART_SendString(cBuffer);
                    }
                    if (xFrame.usLength > 16)
                    {
                        UART_SendString("...");
                    }
                    UART_SendString("\r\n");
                    
                    vProcessModbusFrame(xFrame.ucBuffer, xFrame.usLength);
                }
            }
            
            /* Store byte */
            if (xRxFrame.usLength < MODBUS_FRAME_MAX_SIZE)
            {
                xRxFrame.ucBuffer[xRxFrame.usLength++] = ucRxByte;
            }
            
            xLastRxTime = xCurrentTime;
        }
        else
        {
            /* Timeout - check if we have a complete frame */
            if (xRxFrame.usLength > 0)
            {
                xCurrentTime = xTaskGetTickCount();
                
                if ((xCurrentTime - xLastRxTime) >= pdMS_TO_TICKS(MODBUS_INTERFRAME_DELAY_MS))
                {
                    /* Frame complete */
                    memcpy(&xFrame, (void *)&xRxFrame, sizeof(ModbusFrame_t));
                    xRxFrame.usLength = 0;
                    
                    ulFramesReceived++;
                    
                    /* Print received frame */
                    snprintf(cBuffer, sizeof(cBuffer), 
                             "\r\n[RX] Frame: %d bytes: ", xFrame.usLength);
                    UART_SendString(cBuffer);
                    
                    for (i = 0; i < xFrame.usLength && i < 16; i++)
                    {
                        snprintf(cBuffer, sizeof(cBuffer), "%02X ", xFrame.ucBuffer[i]);
                        UART_SendString(cBuffer);
                    }
                    UART_SendString("\r\n");
                    
                    vProcessModbusFrame(xFrame.ucBuffer, xFrame.usLength);
                }
            }
        }
    }
}

/**
 * Simulation Task - Updates simulated sensor values
 */
static void vSimulationTask(void *pvParameters)
{
    (void)pvParameters;
    
    TickType_t xLastWakeTime;
    uint16_t usTemperature = 250;  /* 25.0°C × 10 */
    uint16_t usHumidity = 500;     /* 50.0% × 10 */
    uint16_t usPressure = 1013;    /* hPa */
    uint32_t ulUptime = 0;
    char cBuffer[120];
    
    UART_SendString("[SIM] Simulation task started\r\n");
    
    /* Initialize holding registers with defaults */
    if (xSemaphoreTake(xRegisterMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        usHoldingRegs[0] = 500;   /* Setpoint 1 */
        usHoldingRegs[1] = 100;   /* Setpoint 2 */
        usHoldingRegs[2] = 1;     /* Control Mode */
        usHoldingRegs[3] = 300;   /* Alarm Threshold */
        xSemaphoreGive(xRegisterMutex);
    }
    
    xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        /* Update simulated sensor values */
        if (xSemaphoreTake(xRegisterMutex, pdMS_TO_TICKS(50)) == pdTRUE)
        {
            /* Simulate temperature variation (24.0-26.0°C) */
            usTemperature = 240 + (xTaskGetTickCount() % 20);
            usInputRegs[0] = usTemperature;
            
            /* Simulate humidity (45-55%) */
            usHumidity = 450 + ((xTaskGetTickCount() / 100) % 100);
            usInputRegs[1] = usHumidity;
            
            /* Pressure relatively stable */
            usPressure = 1010 + (xTaskGetTickCount() % 10);
            usInputRegs[2] = usPressure;
            
            /* ADC simulation */
            usInputRegs[3] = 2048 + ((xTaskGetTickCount() / 10) % 100);
            
            /* Uptime in seconds */
            ulUptime = xTaskGetTickCount() / 1000;
            usInputRegs[4] = ulUptime & 0xFFFF;
            
            /* Status register */
            usInputRegs[5] = 0x0001;  /* Online */
            
            xSemaphoreGive(xRegisterMutex);
        }
        
        /* Print status every 10 seconds */
        if ((ulUptime % 10) == 0 && ulUptime > 0)
        {
            snprintf(cBuffer, sizeof(cBuffer), 
                     "\r\n=== MODBUS SLAVE STATUS ===\r\n"
                     "Frames RX: %lu, TX: %lu\r\n"
                     "CRC Errors: %lu, Exceptions: %lu\r\n"
                     "Input Regs: Temp=%.1f°C, Hum=%.1f%%, Press=%dhPa\r\n",
                     ulFramesReceived, ulFramesSent,
                     ulCrcErrors, ulExceptions,
                     usTemperature / 10.0f, usHumidity / 10.0f, usPressure);
            UART_SendString(cBuffer);
            
            vTaskDelay(pdMS_TO_TICKS(100)); /* Prevent repeated print */
        }
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
    }
}

/* ========================== UART FUNCTIONS ================================ */

static void UART_SendString(const char *str)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}

static void UART_SendBytes(const uint8_t *pucData, uint16_t usLen)
{
    char cBuffer[4];
    uint16_t i;
    
    /* Send raw bytes */
    HAL_UART_Transmit(&huart1, (uint8_t *)pucData, usLen, HAL_MAX_DELAY);
    
    /* Debug print */
    UART_SendString("[TX] ");
    for (i = 0; i < usLen && i < 16; i++)
    {
        snprintf(cBuffer, sizeof(cBuffer), "%02X ", pucData[i]);
        UART_SendString(cBuffer);
    }
    if (usLen > 16)
    {
        UART_SendString("...");
    }
    UART_SendString("\r\n");
}

/* ========================== HOOK FUNCTIONS ================================ */

void vApplicationMallocFailedHook(void)
{
    UART_SendString("MALLOC FAILED!\r\n");
    for (;;);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    UART_SendString("STACK OVERFLOW: ");
    UART_SendString(pcTaskName);
    UART_SendString("\r\n");
    for (;;);
}

/* ========================== HARDWARE INIT ================================= */

static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

static void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* LED PC13 */
    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);
    
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET); /* LED OFF */
}

static void UART1_Init(void)
{
    __HAL_RCC_USART1_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* PA9 = TX, PA10 = RX */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;  /* Using 115200 for demo (easier debug) */
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart1);
}

/* ========================== MAIN ========================================== */

int main(void)
{
    char cBuffer[80];
    
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    UART1_Init();

    UART_SendString("\r\n");
    UART_SendString("============================================\r\n");
    UART_SendString("    PROGRAM 46: MODBUS RTU SLAVE DEMO\r\n");
    UART_SendString("============================================\r\n");
    snprintf(cBuffer, sizeof(cBuffer), 
             "Slave Address: %d\r\n", MODBUS_SLAVE_ADDRESS);
    UART_SendString(cBuffer);
    UART_SendString("Baud Rate: 115200 (for demo)\r\n\r\n");
    
    UART_SendString("Supported Functions:\r\n");
    UART_SendString("  0x03 - Read Holding Registers\r\n");
    UART_SendString("  0x04 - Read Input Registers\r\n");
    UART_SendString("  0x06 - Write Single Register\r\n\r\n");

    /* Create register mutex */
    xRegisterMutex = xSemaphoreCreateMutex();
    if (xRegisterMutex == NULL)
    {
        UART_SendString("ERROR: Failed to create register mutex!\r\n");
        for (;;);
    }
    UART_SendString("Register mutex created\r\n");

    /* Create frame ready semaphore */
    xFrameReadySem = xSemaphoreCreateBinary();
    if (xFrameReadySem == NULL)
    {
        UART_SendString("ERROR: Failed to create frame semaphore!\r\n");
        for (;;);
    }
    UART_SendString("Frame semaphore created\r\n");

    /* Create Modbus task */
    if (xTaskCreate(vModbusTask, "Modbus", configMINIMAL_STACK_SIZE + 80, 
                    NULL, 3, &xModbusTaskHandle) != pdPASS)
    {
        UART_SendString("ERROR: Failed to create Modbus task!\r\n");
        for (;;);
    }
    UART_SendString("Modbus task created\r\n");

    /* Create simulation task */
    if (xTaskCreate(vSimulationTask, "Sim", configMINIMAL_STACK_SIZE + 50, 
                    NULL, 1, &xSimulationTaskHandle) != pdPASS)
    {
        UART_SendString("ERROR: Failed to create simulation task!\r\n");
        for (;;);
    }
    UART_SendString("Simulation task created\r\n");

    UART_SendString("\r\nStarting scheduler...\r\n");
    UART_SendString("----------------------------------------\r\n\r\n");

    vTaskStartScheduler();

    /* Should never reach here */
    for (;;);
}
