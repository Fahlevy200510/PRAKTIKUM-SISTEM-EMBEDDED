/**
 * ============================================================================
 * PRAKTIKUM 7: EXTERNAL INTERRUPT (EXTI)
 * ============================================================================
 * 
 * Tujuan Pembelajaran:
 * - Memahami konsep External Interrupt
 * - Mengkonfigurasi GPIO sebagai interrupt source
 * - Memahami NVIC dan priority
 * - Mengimplementasikan interrupt-driven button handling
 * 
 * Hardware:
 * - STM32F103C8T6 (Blue Pill)
 * - Push button di PA0 (EXTI0)
 * - Push button di PA1 (EXTI1) - opsional
 * - LED di PC13
 * - LED di PA5 - opsional
 * 
 * Wiring:
 *   PA0 ----[Button]---- GND (Internal pull-up enabled)
 *   PA1 ----[Button]---- GND (Internal pull-up enabled)
 * 
 * Interrupt Modes:
 * - Rising Edge: Trigger saat transisi LOW ke HIGH
 * - Falling Edge: Trigger saat transisi HIGH ke LOW
 * - Rising and Falling: Trigger pada kedua transisi
 * 
 * ============================================================================
 */

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

/* UART Handle */
UART_HandleTypeDef huart1;

/* Counters */
volatile uint32_t button1_count = 0;
volatile uint32_t button2_count = 0;
volatile uint32_t last_interrupt_time = 0;

/* Debounce time in ms */
#define DEBOUNCE_TIME_MS    200

/* Private function prototypes */
void SystemClock_Config(void);
void Error_Handler(void);
void GPIO_Init(void);
void EXTI_Init(void);
void UART1_Init(void);
void UART_SendString(const char* str);

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
    EXTI_Init();
    
    UART_SendString("\r\n===================================\r\n");
    UART_SendString("STM32F103C8T6 External Interrupt Demo\r\n");
    UART_SendString("===================================\r\n");
    UART_SendString("Button 1 (PA0): Toggle LED + increment counter\r\n");
    UART_SendString("Button 2 (PA1): Reset counter\r\n");
    UART_SendString("===================================\r\n\r\n");
    
    char msg[100];
    uint32_t last_print = 0;
    uint32_t prev_count = 0;
    
    while (1)
    {
        /* Tampilkan counter jika berubah */
        if (button1_count != prev_count)
        {
            prev_count = button1_count;
            sprintf(msg, "Button pressed! Count: %lu\r\n", button1_count);
            UART_SendString(msg);
        }
        
        /* Periodic status setiap 5 detik */
        if (HAL_GetTick() - last_print >= 5000)
        {
            last_print = HAL_GetTick();
            sprintf(msg, "[Status] Button1: %lu, Button2: %lu\r\n", 
                    button1_count, button2_count);
            UART_SendString(msg);
        }
        
        /* Main loop dapat melakukan task lain */
        /* Interrupt akan menangani button press */
    }
}

/**
 * @brief  EXTI Line 0 Interrupt Handler (PA0)
 * @retval None
 */
void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

/**
 * @brief  EXTI Line 1 Interrupt Handler (PA1)
 * @retval None
 */
void EXTI1_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}

/**
 * @brief  GPIO EXTI Callback
 * @param  GPIO_Pin: Pin yang memicu interrupt
 * @retval None
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    uint32_t current_time = HAL_GetTick();
    
    /* Debouncing: Abaikan interrupt yang terlalu cepat */
    if ((current_time - last_interrupt_time) < DEBOUNCE_TIME_MS)
    {
        return;
    }
    last_interrupt_time = current_time;
    
    if (GPIO_Pin == GPIO_PIN_0)
    {
        /* Button 1: Toggle LED dan increment counter */
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        button1_count++;
    }
    else if (GPIO_Pin == GPIO_PIN_1)
    {
        /* Button 2: Reset counters */
        button1_count = 0;
        button2_count++;
        
        /* Indicate reset dengan kedip cepat */
        for (int i = 0; i < 6; i++)
        {
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
            HAL_Delay(50);
        }
    }
}

/**
 * @brief  Inisialisasi External Interrupt
 * @retval None
 */
void EXTI_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Enable clock untuk GPIOA dan AFIO (untuk remap) */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();
    
    /* Konfigurasi PA0 sebagai External Interrupt - Falling Edge */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;    // Trigger saat button ditekan (LOW)
    GPIO_InitStruct.Pull = GPIO_PULLUP;             // Internal pull-up
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* Konfigurasi PA1 sebagai External Interrupt - Falling Edge */
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* Enable EXTI0 interrupt dengan priority tinggi */
    HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
    
    /* Enable EXTI1 interrupt dengan priority lebih rendah */
    HAL_NVIC_SetPriority(EXTI1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);
}

/**
 * @brief  Inisialisasi GPIO untuk LED
 */
void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    /* LED di PC13 */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    /* LED mati saat awal */
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
