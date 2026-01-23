/**
 ******************************************************************************
 * @file    main.c
 * @brief   I2C Multi Sensor: BMP280 + MPU6050 ID read
 * @date    2026-01-22
 ******************************************************************************
 * Program 41: Multiple I2C devices on same bus
 * - Scan bus
 * - Read chip ID BMP280 (0x58 expected)
 * - Read WHO_AM_I MPU6050 (0x68 expected)
 *
 * Hardware:
 * - PB6: I2C1_SCL (with 4.7k pull-up to 3.3V)
 * - PB7: I2C1_SDA (with 4.7k pull-up to 3.3V)
 * - BMP280 addr 0x76, MPU6050 addr 0x68
 * - UART1 PA9/PA10 for logging
 ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include <stdio.h>

UART_HandleTypeDef huart1;
I2C_HandleTypeDef hi2c1;

#define BMP280_ADDR  (0x76 << 1)
#define MPU6050_ADDR (0x68 << 1)

void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART_Init(void);
static void I2C1_Init(void);
static void scan_bus(void);
static HAL_StatusTypeDef read_reg(uint16_t dev, uint8_t reg, uint8_t *val);

int _write(int file, char *ptr, int len) {
    HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    UART_Init();
    I2C1_Init();

    printf("\r\n=== I2C Multi Sensor (BMP280 + MPU6050) ===\r\n\r\n");
    scan_bus();

    uint8_t id_bmp = 0, id_mpu = 0;
    if (read_reg(BMP280_ADDR, 0xD0, &id_bmp) == HAL_OK) {
        printf("BMP280 ID: 0x%02X %s\r\n", id_bmp, id_bmp == 0x58 ? "(OK)" : "(unexpected)");
    } else {
        printf("BMP280 not responding\r\n");
    }

    if (read_reg(MPU6050_ADDR, 0x75, &id_mpu) == HAL_OK) {
        printf("MPU6050 WHO_AM_I: 0x%02X %s\r\n", id_mpu, id_mpu == 0x68 ? "(OK)" : "(unexpected)");
    } else {
        printf("MPU6050 not responding\r\n");
    }

    printf("\r\nLooping read every 2s...\r\n");

    while (1) {
        if (read_reg(BMP280_ADDR, 0xD0, &id_bmp) == HAL_OK) {
            printf("BMP280 ID: 0x%02X\r\n", id_bmp);
        }
        if (read_reg(MPU6050_ADDR, 0x75, &id_mpu) == HAL_OK) {
            printf("MPU6050 ID: 0x%02X\r\n", id_mpu);
        }
        HAL_Delay(2000);
    }
}

static void scan_bus(void) {
    printf("I2C scan...\r\n");
    for (uint8_t addr = 1; addr < 127; addr++) {
        uint8_t byte = 0;
        if (HAL_I2C_Mem_Read(&hi2c1, addr << 1, 0x00, I2C_MEMADD_SIZE_8BIT, &byte, 1, 10) == HAL_OK) {
            printf("- Found device at 0x%02X\r\n", addr);
        }
    }
    printf("Scan done.\r\n\r\n");
}

static HAL_StatusTypeDef read_reg(uint16_t dev, uint8_t reg, uint8_t *val) {
    return HAL_I2C_Mem_Read(&hi2c1, dev, reg, I2C_MEMADD_SIZE_8BIT, val, 1, 50);
}

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
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

static void GPIO_Init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
}

static void UART_Init(void) {
    __HAL_RCC_USART1_CLK_ENABLE();
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

static void I2C1_Init(void) {
    __HAL_RCC_I2C1_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    HAL_I2C_Init(&hi2c1);
}

void Error_Handler(void) {
    __disable_irq();
    while (1) {}
}
