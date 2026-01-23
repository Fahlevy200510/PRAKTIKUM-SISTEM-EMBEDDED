#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_mem;
CRC_HandleTypeDef hcrc;

uint32_t src_data[64];
uint32_t dst_data[64];

void SystemClock_Config(void);
void GPIO_Init(void);
void UART1_Init(void);
void DMA_Init(void);
void CRC_Init(void);
void UART_SendString(const char *s);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    UART1_Init();
    DMA_Init();
    CRC_Init();

    UART_SendString("\r\nDMA Mem2Mem + CRC Demo\r\n");

    for (uint32_t i = 0; i < 64; i++)
        src_data[i] = 0x11110000 + i;

    HAL_DMA_Start(&hdma_mem, (uint32_t)src_data, (uint32_t)dst_data, 64);
    HAL_DMA_PollForTransfer(&hdma_mem, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);

    uint32_t crc_src = HAL_CRC_Calculate(&hcrc, src_data, 64);
    uint32_t crc_dst = HAL_CRC_Calculate(&hcrc, dst_data, 64);

    char msg[80];
    snprintf(msg, sizeof(msg), "CRC src=0x%08lX dst=0x%08lX\r\n", crc_src, crc_dst);
    UART_SendString(msg);

    if (memcmp(src_data, dst_data, sizeof(src_data)) == 0)
        UART_SendString("DMA copy OK\r\n");
    else
        UART_SendString("DMA copy FAIL\r\n");

    while (1)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_Delay(500);
    }
}

void DMA1_Channel1_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_mem);
}

void DMA_Init(void)
{
    __HAL_RCC_DMA1_CLK_ENABLE();

    hdma_mem.Instance = DMA1_Channel1;
    hdma_mem.Init.Direction = DMA_MEMORY_TO_MEMORY;
    hdma_mem.Init.PeriphInc = DMA_PINC_ENABLE;
    hdma_mem.Init.MemInc = DMA_MINC_ENABLE;
    hdma_mem.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_mem.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_mem.Init.Mode = DMA_NORMAL;
    hdma_mem.Init.Priority = DMA_PRIORITY_HIGH;
    HAL_DMA_Init(&hdma_mem);

    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

void CRC_Init(void)
{
    __HAL_RCC_CRC_CLK_ENABLE();
    hcrc.Instance = CRC;
    HAL_CRC_Init(&hcrc);
}

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

void UART_SendString(const char *s)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)s, strlen(s), HAL_MAX_DELAY);
}

void GPIO_Init(void)
{
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}

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
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
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
