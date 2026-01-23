/**
 * ============================================================================
 * PRAKTIKUM 5: PWM LED DIMMER
 * ============================================================================
 * 
 * Tujuan Pembelajaran:
 * - Memahami konsep Pulse Width Modulation (PWM)
 * - Mengkonfigurasi Timer dalam mode PWM
 * - Mengontrol kecerahan LED menggunakan PWM
 * - Membuat efek breathing LED
 * 
 * Hardware:
 * - STM32F103C8T6 (Blue Pill)
 * - LED eksternal di PA0 (TIM2_CH1) dengan resistor 220-330Ω
 * - LED eksternal di PA1 (TIM2_CH2) - opsional
 * - Potensiometer di PA4 (ADC) - opsional
 * 
 * Wiring:
 *   PA0 ----[330Ω]----[LED]---- GND
 *   PA1 ----[330Ω]----[LED]---- GND (opsional)
 * 
 * PWM Configuration:
 * - Timer Clock: 72MHz
 * - Prescaler: 71 (Clock/72 = 1MHz)
 * - Period: 999 (1MHz/1000 = 1kHz PWM frequency)
 * - Duty Cycle: 0-999 (0-100%)
 * 
 * ============================================================================
 */

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

/* Timer Handle */
TIM_HandleTypeDef htim2;

/* UART Handle */
UART_HandleTypeDef huart1;

/* PWM parameters */
#define PWM_PERIOD      999     // 1kHz PWM frequency
#define PWM_PRESCALER   71      // 72MHz / 72 = 1MHz timer clock

/* Breathing effect parameters */
volatile uint16_t pwm_duty = 0;
volatile int8_t pwm_direction = 1;

/* Private function prototypes */
void SystemClock_Config(void);
void Error_Handler(void);
void GPIO_Init(void);
void TIM2_PWM_Init(void);
void UART1_Init(void);
void UART_SendString(const char* str);
void Breathing_Effect(void);
void Set_PWM_Duty(uint8_t channel, uint16_t duty);

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
    TIM2_PWM_Init();
    
    UART_SendString("\r\n===================================\r\n");
    UART_SendString("STM32F103C8T6 PWM LED Dimmer Demo\r\n");
    UART_SendString("===================================\r\n");
    UART_SendString("PWM Frequency: 1kHz\r\n");
    UART_SendString("Duty Cycle Range: 0-100%\r\n");
    UART_SendString("===================================\r\n\r\n");
    
    /* Start PWM pada kedua channel */
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);  // PA0
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);  // PA1
    
    while (1)
    {
        /* Mode 1: Breathing Effect - LED bernapas */
        Breathing_Effect();
        
        /* 
         * Mode alternatif bisa dicoba:
         * - Manual control via UART
         * - Kontrol via potensiometer (ADC)
         * - Pola khusus (fade in/out cepat)
         */
    }
}

/**
 * @brief  Efek breathing - LED menyala dan redup secara perlahan
 * @retval None
 */
void Breathing_Effect(void)
{
    /* Update duty cycle */
    pwm_duty += pwm_direction * 5;  // Step = 5
    
    /* Reverse direction at limits */
    if (pwm_duty >= PWM_PERIOD)
    {
        pwm_duty = PWM_PERIOD;
        pwm_direction = -1;
    }
    else if (pwm_duty <= 0 || pwm_duty > PWM_PERIOD)  // Handle underflow
    {
        pwm_duty = 0;
        pwm_direction = 1;
    }
    
    /* Set PWM duty cycle untuk kedua channel */
    Set_PWM_Duty(1, pwm_duty);                        // Channel 1: normal
    Set_PWM_Duty(2, PWM_PERIOD - pwm_duty);          // Channel 2: inverted
    
    /* Delay untuk kecepatan breathing */
    HAL_Delay(10);
}

/**
 * @brief  Set duty cycle untuk PWM channel
 * @param  channel: PWM channel (1, 2, 3, atau 4)
 * @param  duty: Duty cycle (0 - PWM_PERIOD)
 * @retval None
 */
void Set_PWM_Duty(uint8_t channel, uint16_t duty)
{
    if (duty > PWM_PERIOD) duty = PWM_PERIOD;
    
    switch (channel)
    {
        case 1:
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, duty);
            break;
        case 2:
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, duty);
            break;
        case 3:
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, duty);
            break;
        case 4:
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, duty);
            break;
    }
}

/**
 * @brief  Inisialisasi TIM2 dalam mode PWM
 * @retval None
 */
void TIM2_PWM_Init(void)
{
    TIM_OC_InitTypeDef sConfigOC = {0};
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Enable clocks */
    __HAL_RCC_TIM2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    /* Konfigurasi PA0 dan PA1 sebagai Alternate Function untuk TIM2 */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* Konfigurasi TIM2 base */
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = PWM_PRESCALER;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = PWM_PERIOD;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    
    if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
    
    /* Konfigurasi PWM channel */
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;  // Initial duty cycle = 0
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    
    /* Configure Channel 1 - PA0 */
    if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }
    
    /* Configure Channel 2 - PA1 */
    if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief  Inisialisasi GPIO untuk LED indikator
 */
void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    /* LED di PC13 untuk status */
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
