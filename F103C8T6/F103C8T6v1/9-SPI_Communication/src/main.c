/**
 * ============================================================================
 * PRAKTIKUM 9: SPI COMMUNICATION
 * ============================================================================
 * 
 * Tujuan Pembelajaran:
 * - Memahami protokol SPI (Serial Peripheral Interface)
 * - Mengkonfigurasi SPI Master pada STM32
 * - Berkomunikasi dengan device SPI (SD Card, Flash, Display, Sensor)
 * - Memahami clock polarity, phase, dan data transfer
 * 
 * Hardware:
 * - STM32F103C8T6 (Blue Pill)
 * - W25Q Flash Module (opsional)
 * - SD Card Module (opsional)
 * - MAX7219 LED Matrix (opsional)
 * 
 * SPI Pins pada STM32F103:
 *   SPI1: SCK = PA5, MISO = PA6, MOSI = PA7, NSS = PA4
 *   SPI2: SCK = PB13, MISO = PB14, MOSI = PB15, NSS = PB12
 * 
 * Wiring SPI1:
 *   PA5 (SCK)  ----> SCK device
 *   PA6 (MISO) <---- MISO device
 *   PA7 (MOSI) ----> MOSI device
 *   PA4 (CS)   ----> CS/SS device
 *   GND        ----> GND device
 *   3.3V       ----> VCC device
 * 
 * W25Q Flash Memory:
 * - Capacity: 64Mbit (8MB) atau lainnya
 * - Commands: Read ID, Read Data, Write Data, Erase
 * - Manufacturer ID: 0xEF (Winbond)
 * 
 * ============================================================================
 */

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

/* SPI Handle */
SPI_HandleTypeDef hspi1;

/* UART Handle */
UART_HandleTypeDef huart1;

/* W25Q Flash Commands */
#define W25Q_CMD_WRITE_ENABLE       0x06
#define W25Q_CMD_WRITE_DISABLE      0x04
#define W25Q_CMD_READ_STATUS_REG1   0x05
#define W25Q_CMD_READ_DATA          0x03
#define W25Q_CMD_PAGE_PROGRAM       0x02
#define W25Q_CMD_SECTOR_ERASE       0x20
#define W25Q_CMD_CHIP_ERASE         0xC7
#define W25Q_CMD_JEDEC_ID           0x9F
#define W25Q_CMD_MANUFACTURER_ID    0x90

/* Chip Select Pin */
#define SPI_CS_GPIO_PORT            GPIOA
#define SPI_CS_PIN                  GPIO_PIN_4

/* CS Control Macros */
#define CS_LOW()    HAL_GPIO_WritePin(SPI_CS_GPIO_PORT, SPI_CS_PIN, GPIO_PIN_RESET)
#define CS_HIGH()   HAL_GPIO_WritePin(SPI_CS_GPIO_PORT, SPI_CS_PIN, GPIO_PIN_SET)

/* Private function prototypes */
void SystemClock_Config(void);
void Error_Handler(void);
void GPIO_Init(void);
void SPI1_Init(void);
void UART1_Init(void);
void UART_SendString(const char* str);

/* SPI Helper functions */
uint8_t SPI_TransferByte(uint8_t data);
void SPI_TransferBuffer(uint8_t* tx_buf, uint8_t* rx_buf, uint16_t size);

/* W25Q Flash functions */
void W25Q_ReadJEDEC_ID(uint8_t* id);
void W25Q_ReadManufacturerID(uint8_t* mid, uint8_t* did);
uint8_t W25Q_ReadStatusReg1(void);
void W25Q_WriteEnable(void);
void W25Q_WaitForReady(void);
void W25Q_SectorErase(uint32_t address);
void W25Q_PageProgram(uint32_t address, uint8_t* data, uint16_t size);
void W25Q_ReadData(uint32_t address, uint8_t* buffer, uint16_t size);

/**
 * @brief  Main program
 * @retval int
 */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    UART1_Init();
    SPI1_Init();
    
    UART_SendString("\r\n===================================\r\n");
    UART_SendString("STM32F103C8T6 SPI Communication Demo\r\n");
    UART_SendString("===================================\r\n");
    UART_SendString("SPI1: SCK=PA5, MISO=PA6, MOSI=PA7, CS=PA4\r\n");
    UART_SendString("Speed: ~1MHz (APB2/64)\r\n");
    UART_SendString("Mode: CPOL=0, CPHA=0 (Mode 0)\r\n");
    UART_SendString("===================================\r\n\r\n");
    
    char msg[100];
    
    /* Test 1: Read JEDEC ID dari W25Q Flash */
    UART_SendString("Testing W25Q Flash Memory...\r\n");
    
    uint8_t jedec_id[3] = {0};
    W25Q_ReadJEDEC_ID(jedec_id);
    
    sprintf(msg, "JEDEC ID: 0x%02X 0x%02X 0x%02X\r\n", 
            jedec_id[0], jedec_id[1], jedec_id[2]);
    UART_SendString(msg);
    
    /* Decode Manufacturer ID */
    if (jedec_id[0] == 0xEF)
    {
        UART_SendString("Manufacturer: Winbond\r\n");
        
        switch (jedec_id[2])
        {
            case 0x14: UART_SendString("Chip: W25Q80 (8Mbit)\r\n"); break;
            case 0x15: UART_SendString("Chip: W25Q16 (16Mbit)\r\n"); break;
            case 0x16: UART_SendString("Chip: W25Q32 (32Mbit)\r\n"); break;
            case 0x17: UART_SendString("Chip: W25Q64 (64Mbit)\r\n"); break;
            case 0x18: UART_SendString("Chip: W25Q128 (128Mbit)\r\n"); break;
            default: UART_SendString("Chip: Unknown\r\n"); break;
        }
    }
    else if (jedec_id[0] == 0xFF || jedec_id[0] == 0x00)
    {
        UART_SendString("No SPI Flash detected!\r\n");
        UART_SendString("Connect W25Q or similar SPI Flash to test\r\n");
    }
    else
    {
        sprintf(msg, "Unknown Manufacturer ID: 0x%02X\r\n", jedec_id[0]);
        UART_SendString(msg);
    }
    
    UART_SendString("\r\n");
    
    /* Test 2: Demo Loopback (MOSI -> MISO) */
    UART_SendString("SPI Loopback Test (connect MOSI to MISO):\r\n");
    
    uint8_t test_values[] = {0x55, 0xAA, 0x12, 0x34, 0x56, 0x78};
    uint8_t passed = 1;
    
    for (int i = 0; i < sizeof(test_values); i++)
    {
        CS_LOW();
        uint8_t result = SPI_TransferByte(test_values[i]);
        CS_HIGH();
        
        sprintf(msg, "  TX: 0x%02X, RX: 0x%02X ", test_values[i], result);
        UART_SendString(msg);
        
        if (result == test_values[i])
        {
            UART_SendString("[PASS]\r\n");
        }
        else
        {
            UART_SendString("[FAIL]\r\n");
            passed = 0;
        }
    }
    
    if (passed)
    {
        UART_SendString("Loopback test PASSED!\r\n");
    }
    else
    {
        UART_SendString("Loopback test FAILED (connect MOSI to MISO)\r\n");
    }
    
    UART_SendString("\r\n");
    
    /* Test 3: Demo Flash R/W jika terdeteksi */
    if (jedec_id[0] == 0xEF)
    {
        UART_SendString("Flash Read/Write Test:\r\n");
        
        uint32_t test_addr = 0x000000;  // Sector 0
        uint8_t write_data[16] = "Hello STM32!";
        uint8_t read_data[16] = {0};
        
        /* Erase sector */
        UART_SendString("  Erasing sector 0...\r\n");
        W25Q_SectorErase(test_addr);
        W25Q_WaitForReady();
        
        /* Write data */
        UART_SendString("  Writing: 'Hello STM32!'\r\n");
        W25Q_PageProgram(test_addr, write_data, 16);
        W25Q_WaitForReady();
        
        /* Read back */
        W25Q_ReadData(test_addr, read_data, 16);
        sprintf(msg, "  Read: '%s'\r\n", read_data);
        UART_SendString(msg);
        
        if (memcmp(write_data, read_data, 16) == 0)
        {
            UART_SendString("  Flash R/W test PASSED!\r\n");
        }
        else
        {
            UART_SendString("  Flash R/W test FAILED!\r\n");
        }
    }
    
    UART_SendString("\r\nEntering main loop...\r\n");
    
    /* Main loop */
    uint32_t counter = 0;
    
    while (1)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        
        if (counter % 5 == 0)
        {
            sprintf(msg, "[%lu s] SPI demo running...\r\n", counter);
            UART_SendString(msg);
        }
        
        counter++;
        HAL_Delay(1000);
    }
}

/**
 * @brief  Transfer single byte via SPI (full-duplex)
 */
uint8_t SPI_TransferByte(uint8_t data)
{
    uint8_t rx_data = 0;
    HAL_SPI_TransmitReceive(&hspi1, &data, &rx_data, 1, HAL_MAX_DELAY);
    return rx_data;
}

/**
 * @brief  Transfer buffer via SPI
 */
void SPI_TransferBuffer(uint8_t* tx_buf, uint8_t* rx_buf, uint16_t size)
{
    HAL_SPI_TransmitReceive(&hspi1, tx_buf, rx_buf, size, HAL_MAX_DELAY);
}

/**
 * @brief  Read JEDEC ID dari W25Q Flash
 */
void W25Q_ReadJEDEC_ID(uint8_t* id)
{
    CS_LOW();
    SPI_TransferByte(W25Q_CMD_JEDEC_ID);
    id[0] = SPI_TransferByte(0xFF);  // Manufacturer ID
    id[1] = SPI_TransferByte(0xFF);  // Memory Type
    id[2] = SPI_TransferByte(0xFF);  // Capacity
    CS_HIGH();
}

/**
 * @brief  Read Status Register 1
 */
uint8_t W25Q_ReadStatusReg1(void)
{
    CS_LOW();
    SPI_TransferByte(W25Q_CMD_READ_STATUS_REG1);
    uint8_t status = SPI_TransferByte(0xFF);
    CS_HIGH();
    return status;
}

/**
 * @brief  Enable Write operations
 */
void W25Q_WriteEnable(void)
{
    CS_LOW();
    SPI_TransferByte(W25Q_CMD_WRITE_ENABLE);
    CS_HIGH();
}

/**
 * @brief  Wait until Flash is ready
 */
void W25Q_WaitForReady(void)
{
    while (W25Q_ReadStatusReg1() & 0x01)
    {
        HAL_Delay(1);
    }
}

/**
 * @brief  Erase 4KB sector
 */
void W25Q_SectorErase(uint32_t address)
{
    W25Q_WriteEnable();
    
    CS_LOW();
    SPI_TransferByte(W25Q_CMD_SECTOR_ERASE);
    SPI_TransferByte((address >> 16) & 0xFF);
    SPI_TransferByte((address >> 8) & 0xFF);
    SPI_TransferByte(address & 0xFF);
    CS_HIGH();
}

/**
 * @brief  Program page (max 256 bytes)
 */
void W25Q_PageProgram(uint32_t address, uint8_t* data, uint16_t size)
{
    if (size > 256) size = 256;
    
    W25Q_WriteEnable();
    
    CS_LOW();
    SPI_TransferByte(W25Q_CMD_PAGE_PROGRAM);
    SPI_TransferByte((address >> 16) & 0xFF);
    SPI_TransferByte((address >> 8) & 0xFF);
    SPI_TransferByte(address & 0xFF);
    
    for (uint16_t i = 0; i < size; i++)
    {
        SPI_TransferByte(data[i]);
    }
    CS_HIGH();
}

/**
 * @brief  Read data from Flash
 */
void W25Q_ReadData(uint32_t address, uint8_t* buffer, uint16_t size)
{
    CS_LOW();
    SPI_TransferByte(W25Q_CMD_READ_DATA);
    SPI_TransferByte((address >> 16) & 0xFF);
    SPI_TransferByte((address >> 8) & 0xFF);
    SPI_TransferByte(address & 0xFF);
    
    for (uint16_t i = 0; i < size; i++)
    {
        buffer[i] = SPI_TransferByte(0xFF);
    }
    CS_HIGH();
}

/**
 * @brief  Inisialisasi SPI1
 */
void SPI1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Enable clocks */
    __HAL_RCC_SPI1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    /* Konfigurasi SCK (PA5), MOSI (PA7) sebagai AF Push-Pull */
    GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* Konfigurasi MISO (PA6) sebagai Input */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* Konfigurasi CS (PA4) sebagai GPIO Output */
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* CS idle high */
    CS_HIGH();
    
    /* Konfigurasi SPI1 */
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;      // CPOL = 0
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;          // CPHA = 0
    hspi1.Init.NSS = SPI_NSS_SOFT;                  // Software CS control
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;  // ~1.125 MHz
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 10;
    
    if (HAL_SPI_Init(&hspi1) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief  Inisialisasi GPIO
 */
void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    /* LED PC13 */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}

/**
 * @brief  Inisialisasi UART1
 */
void UART1_Init(void)
{
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* TX (PA9) */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* RX (PA10) */
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    
    HAL_UART_Init(&huart1);
}

void UART_SendString(const char* str)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
}

/**
 * @brief  System Clock Configuration - 72MHz
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

void Error_Handler(void)
{
    while (1)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_Delay(100);
    }
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}
