/**
 * ============================================================================
 * PRAKTIKUM 8: I2C COMMUNICATION
 * ============================================================================
 * 
 * Tujuan Pembelajaran:
 * - Memahami protokol I2C (Inter-Integrated Circuit)
 * - Mengkonfigurasi I2C pada STM32
 * - Berkomunikasi dengan device I2C (OLED SSD1306, EEPROM, sensor)
 * - Memahami addressing dan data transfer
 * 
 * Hardware:
 * - STM32F103C8T6 (Blue Pill)
 * - OLED SSD1306 128x64 I2C (opsional)
 * - EEPROM AT24C02 (opsional)
 * - DS3231 RTC Module (opsional)
 * 
 * I2C Pins pada STM32F103:
 *   I2C1: SCL = PB6, SDA = PB7
 *   I2C2: SCL = PB10, SDA = PB11
 * 
 * Wiring (I2C1):
 *   PB6 (SCL) ----[4.7kΩ]---- 3.3V
 *   PB7 (SDA) ----[4.7kΩ]---- 3.3V
 *   PB6 ----> SCL device
 *   PB7 ----> SDA device
 *   GND ----> GND device
 *   3.3V ---> VCC device
 * 
 * Common I2C Addresses:
 * - OLED SSD1306: 0x3C atau 0x3D
 * - EEPROM AT24C02: 0x50
 * - DS3231 RTC: 0x68
 * - BMP280/BME280: 0x76 atau 0x77
 * 
 * ============================================================================
 */

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

/* I2C Handle */
I2C_HandleTypeDef hi2c1;

/* UART Handle */
UART_HandleTypeDef huart1;

/* Common I2C addresses */
#define SSD1306_I2C_ADDR        0x3C << 1   // 7-bit address shifted
#define AT24C02_I2C_ADDR        0x50 << 1
#define DS3231_I2C_ADDR         0x68 << 1

/* SSD1306 Commands */
#define SSD1306_CMD             0x00
#define SSD1306_DATA            0x40

/* Private function prototypes */
void SystemClock_Config(void);
void Error_Handler(void);
void GPIO_Init(void);
void I2C1_Init(void);
void UART1_Init(void);
void UART_SendString(const char* str);
uint8_t I2C_Scan(void);
HAL_StatusTypeDef SSD1306_Init(void);
void SSD1306_Clear(void);
void SSD1306_WriteString(uint8_t x, uint8_t y, const char* str);
HAL_StatusTypeDef EEPROM_Write(uint8_t addr, uint8_t data);
HAL_StatusTypeDef EEPROM_Read(uint8_t addr, uint8_t* data);

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
    I2C1_Init();
    
    UART_SendString("\r\n===================================\r\n");
    UART_SendString("STM32F103C8T6 I2C Communication Demo\r\n");
    UART_SendString("===================================\r\n");
    UART_SendString("I2C1: SCL=PB6, SDA=PB7\r\n");
    UART_SendString("Speed: 100kHz (Standard Mode)\r\n");
    UART_SendString("===================================\r\n\r\n");
    
    /* Scan I2C bus untuk mencari device */
    UART_SendString("Scanning I2C bus...\r\n");
    uint8_t device_count = I2C_Scan();
    
    char msg[100];
    sprintf(msg, "Found %d device(s)\r\n\r\n", device_count);
    UART_SendString(msg);
    
    /* Demo: Test komunikasi dengan EEPROM jika ada */
    UART_SendString("Testing EEPROM communication...\r\n");
    
    /* Write test data */
    uint8_t test_data = 0x42;
    if (EEPROM_Write(0x00, test_data) == HAL_OK)
    {
        UART_SendString("EEPROM Write: 0x42 to address 0x00\r\n");
        
        /* Read back */
        uint8_t read_data = 0;
        HAL_Delay(10);  // EEPROM write cycle time
        
        if (EEPROM_Read(0x00, &read_data) == HAL_OK)
        {
            sprintf(msg, "EEPROM Read: 0x%02X from address 0x00\r\n", read_data);
            UART_SendString(msg);
            
            if (read_data == test_data)
            {
                UART_SendString("EEPROM test PASSED!\r\n");
            }
            else
            {
                UART_SendString("EEPROM test FAILED - data mismatch\r\n");
            }
        }
    }
    else
    {
        UART_SendString("EEPROM not found or write failed\r\n");
    }
    
    UART_SendString("\r\n");
    
    /* Main loop */
    uint32_t counter = 0;
    
    while (1)
    {
        /* Toggle LED untuk indikasi program berjalan */
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        
        /* Print status setiap 5 detik */
        if (counter % 5 == 0)
        {
            sprintf(msg, "[%lu s] I2C demo running...\r\n", counter);
            UART_SendString(msg);
        }
        
        counter++;
        HAL_Delay(1000);
    }
}

/**
 * @brief  Scan I2C bus untuk mencari device
 * @retval Jumlah device yang ditemukan
 */
uint8_t I2C_Scan(void)
{
    uint8_t count = 0;
    char msg[50];
    
    UART_SendString("     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\r\n");
    
    for (uint8_t row = 0; row < 8; row++)
    {
        sprintf(msg, "%02X: ", row * 16);
        UART_SendString(msg);
        
        for (uint8_t col = 0; col < 16; col++)
        {
            uint8_t addr = row * 16 + col;
            
            /* Skip reserved addresses */
            if (addr < 0x03 || addr > 0x77)
            {
                UART_SendString("   ");
                continue;
            }
            
            /* Try to communicate with device */
            if (HAL_I2C_IsDeviceReady(&hi2c1, addr << 1, 1, 10) == HAL_OK)
            {
                sprintf(msg, "%02X ", addr);
                UART_SendString(msg);
                count++;
            }
            else
            {
                UART_SendString("-- ");
            }
        }
        UART_SendString("\r\n");
    }
    
    return count;
}

/**
 * @brief  Write byte ke EEPROM AT24C02
 * @param  addr: Memory address (0-255)
 * @param  data: Data to write
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef EEPROM_Write(uint8_t addr, uint8_t data)
{
    uint8_t buffer[2] = {addr, data};
    return HAL_I2C_Master_Transmit(&hi2c1, AT24C02_I2C_ADDR, buffer, 2, HAL_MAX_DELAY);
}

/**
 * @brief  Read byte dari EEPROM AT24C02
 * @param  addr: Memory address (0-255)
 * @param  data: Pointer to store read data
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef EEPROM_Read(uint8_t addr, uint8_t* data)
{
    /* First, set the address pointer */
    HAL_StatusTypeDef status;
    status = HAL_I2C_Master_Transmit(&hi2c1, AT24C02_I2C_ADDR, &addr, 1, HAL_MAX_DELAY);
    if (status != HAL_OK) return status;
    
    /* Then read the data */
    return HAL_I2C_Master_Receive(&hi2c1, AT24C02_I2C_ADDR, data, 1, HAL_MAX_DELAY);
}

/**
 * @brief  Inisialisasi OLED SSD1306
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef SSD1306_Init(void)
{
    /* SSD1306 initialization sequence */
    uint8_t init_cmds[] = {
        0xAE,       // Display OFF
        0xD5, 0x80, // Set clock divide ratio
        0xA8, 0x3F, // Set multiplex ratio (1-64)
        0xD3, 0x00, // Set display offset
        0x40,       // Set start line address
        0x8D, 0x14, // Enable charge pump
        0x20, 0x00, // Set memory addressing mode (horizontal)
        0xA1,       // Set segment re-map
        0xC8,       // Set COM output scan direction
        0xDA, 0x12, // Set COM pins hardware configuration
        0x81, 0xCF, // Set contrast control
        0xD9, 0xF1, // Set pre-charge period
        0xDB, 0x40, // Set VCOMH deselect level
        0xA4,       // Entire display ON
        0xA6,       // Set normal display (not inverted)
        0xAF        // Display ON
    };
    
    for (uint8_t i = 0; i < sizeof(init_cmds); i++)
    {
        uint8_t data[2] = {SSD1306_CMD, init_cmds[i]};
        if (HAL_I2C_Master_Transmit(&hi2c1, SSD1306_I2C_ADDR, data, 2, HAL_MAX_DELAY) != HAL_OK)
        {
            return HAL_ERROR;
        }
    }
    
    return HAL_OK;
}

/**
 * @brief  Inisialisasi I2C1
 * @retval None
 */
void I2C1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Enable clocks */
    __HAL_RCC_I2C1_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    /* Konfigurasi PB6 (SCL) dan PB7 (SDA) */
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;         // Open-drain untuk I2C
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /* Konfigurasi I2C1 */
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;                 // 100kHz Standard Mode
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    
    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
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
    
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
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
