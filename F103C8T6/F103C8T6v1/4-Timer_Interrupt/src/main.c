/**
 * ============================================================================
 * PRAKTIKUM 4: TIMER INTERRUPT
 * ============================================================================
 * 
 * Tujuan Pembelajaran:
 * - Memahami konfigurasi Timer sebagai time base generator
 * - Memahami konsep prescaler dan auto-reload
 * - Mengimplementasikan interrupt timer
 * - Membuat aksi periodik tanpa blocking (non-HAL_Delay)
 * 
 * Hardware:
 * - STM32F103C8T6 (Blue Pill)
 * - LED di PC13 (built-in)
 * - LED tambahan di PA0, PA1 (opsional)
 * 
 * Timer Configuration:
 * - TIM2: 500ms interrupt untuk LED utama
 * - TIM3: 100ms interrupt untuk counter
 * 
 * Formula Timer:
 *   Freq_interrupt = Timer_clock / ((Prescaler + 1) * (Period + 1))
 *   
 *   Untuk 72MHz clock dan interval 500ms:
 *   72000000 / ((7199 + 1) * (4999 + 1)) = 2 Hz = 500ms
 * 
 * ============================================================================
 */

#include "stm32f1xx_hal.h"
#include <stdio.h>

/* Timer Handles */
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

/* UART Handle untuk debug */
UART_HandleTypeDef huart1;

/* Variabel counter */
volatile uint32_t timer2_count = 0;
volatile uint32_t timer3_count = 0;
volatile uint32_t seconds_counter = 0;

/* Private function prototypes */
void SystemClock_Config(void);
void Error_Handler(void);
void GPIO_Init(void);
void TIM2_Init(void);
void TIM3_Init(void);
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
    TIM2_Init();
    TIM3_Init();
    
    UART_SendString("\r\n===================================\r\n");
    UART_SendString("STM32F103C8T6 Timer Interrupt Demo\r\n");
    UART_SendString("===================================\r\n");
    UART_SendString("TIM2: LED toggle setiap 500ms\r\n");
    UART_SendString("TIM3: Counter setiap 100ms\r\n");
    UART_SendString("===================================\r\n\r\n");
    
    /* Start timers dengan interrupt */
    HAL_TIM_Base_Start_IT(&htim2);
    HAL_TIM_Base_Start_IT(&htim3);
    
    uint32_t last_print = 0;
    
    while (1)
    {
        /* Tampilkan status setiap 1 detik */
        if (HAL_GetTick() - last_print >= 1000)
        {
            last_print = HAL_GetTick();
            seconds_counter++;
            
            char msg[100];
            sprintf(msg, "[%lu s] TIM2 count: %lu, TIM3 count: %lu\r\n",
                    seconds_counter, timer2_count, timer3_count);
            UART_SendString(msg);
        }
        
        /* Main loop bisa mengerjakan task lain */
        /* Timer interrupt akan tetap berjalan di background */
    }
}

/**
 * @brief  Timer Period Elapsed Callback
 * @param  htim: Timer handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        /* Toggle LED setiap 500ms */
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        timer2_count++;
    }
    else if (htim->Instance == TIM3)
    {
        /* Increment counter setiap 100ms */
        timer3_count++;
        
        /* Toggle LED tambahan di PA0 setiap 100ms */
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
    }
}

/**
 * @brief  Inisialisasi TIM2 - 500ms interval
 * @note   Timer Clock = 72MHz (APB1 timer clock)
 *         Prescaler = 7199, Period = 4999
 *         72MHz / (7200 * 5000) = 2Hz = 500ms
 * @retval None
 */
void TIM2_Init(void)
{
    __HAL_RCC_TIM2_CLK_ENABLE();
    
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 7199;           // Divide by 7200
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 4999;              // Count to 5000
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
    
    /* Enable TIM2 interrupt */
    HAL_NVIC_SetPriority(TIM2_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

/**
 * @brief  Inisialisasi TIM3 - 100ms interval
 * @note   72MHz / (7200 * 1000) = 10Hz = 100ms
 * @retval None
 */
void TIM3_Init(void)
{
    __HAL_RCC_TIM3_CLK_ENABLE();
    
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 7199;           // Divide by 7200
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 999;               // Count to 1000
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    
    if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
    {
        Error_Handler();
    }
    
    /* Enable TIM3 interrupt */
    HAL_NVIC_SetPriority(TIM3_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
}

/**
 * @brief  TIM2 Interrupt Handler
 */
void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim2);
}

/**
 * @brief  TIM3 Interrupt Handler
 */
void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim3);
}

/**
 * @brief  Inisialisasi GPIO
 * @retval None
 */
void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    /* LED di PC13 */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    /* LED tambahan di PA0 */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* Initial state */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
}

/**
 * @brief  Inisialisasi UART1
 */
void UART1_Init(void)
{
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* PA9 = TX */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* PA10 = RX */
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
