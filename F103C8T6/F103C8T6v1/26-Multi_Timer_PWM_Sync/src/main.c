/**
 * Program 26: Multi-Timer PWM Synchronized
 * 
 * Deskripsi:
 * Program ini mendemonstrasikan sinkronisasi multiple timer untuk 
 * menghasilkan PWM yang tersinkronisasi dengan presisi tinggi.
 * Cocok untuk aplikasi motor control, LED matrix, atau DAC multi-channel.
 * 
 * Fitur:
 * - TIM1 sebagai master timer (trigger source)
 * - TIM2, TIM3, TIM4 sebagai slave timers
 * - Synchronized PWM output dengan phase shift yang teratur
 * - Frequency: 1 kHz, adjustable duty cycle
 * - 4 channel PWM tersinkronisasi
 * 
 * Timer Configuration:
 * - TIM1 (Master): Generate trigger signal (TRGO)
 * - TIM2 (Slave): Synchronized dengan TIM1, phase 0°
 * - TIM3 (Slave): Synchronized dengan TIM1, phase 90°
 * - TIM4 (Slave): Synchronized dengan TIM1, phase 180°
 * 
 * Aplikasi:
 * - 3-phase motor control
 * - RGB LED PWM dimming
 * - Multi-channel audio DAC
 * - Synchronized sensor sampling
 * 
 * Hardware:
 * - STM32F103C8T6 Blue Pill
 * - 4x LED atau oscilloscope untuk melihat sync
 * - Logic analyzer (optional) untuk verifikasi timing
 * 
 * Pin Configuration:
 * - PA8:  TIM1_CH1 (Master PWM)
 * - PA0:  TIM2_CH1 (Slave PWM, phase 0°)
 * - PA6:  TIM3_CH1 (Slave PWM, phase 90°)
 * - PB6:  TIM4_CH1 (Slave PWM, phase 180°)
 * - PC13: Status LED
 * - PA9:  UART TX (debug)
 */

#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* Timer handles */
TIM_HandleTypeDef htim1; // Master
TIM_HandleTypeDef htim2; // Slave 1
TIM_HandleTypeDef htim3; // Slave 2
TIM_HandleTypeDef htim4; // Slave 3

/* UART handle */
UART_HandleTypeDef huart1;

/* Function prototypes */
void SystemClock_Config(void);
void GPIO_Init(void);
void UART_Init(void);
void TIM1_Master_Init(void);
void TIM2_Slave_Init(void);
void TIM3_Slave_Init(void);
void TIM4_Slave_Init(void);
void Update_PWM_DutyCycle(uint32_t duty_percent);

/* Buffer */
char msg[100];

/* PWM parameters */
#define PWM_FREQUENCY   1000    // 1 kHz
#define TIMER_CLOCK     72000000 // 72 MHz
#define PWM_PERIOD      (TIMER_CLOCK / PWM_FREQUENCY) - 1

/**
 * Main function
 */
int main(void)
{
    /* Initialize HAL */
    HAL_Init();

    /* Configure system clock */
    SystemClock_Config();

    /* Initialize peripherals */
    GPIO_Init();
    UART_Init();

    sprintf(msg, "\r\n=== Multi-Timer Synchronized PWM ===\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "PWM Frequency: %d Hz\r\n", PWM_FREQUENCY);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "Channels: 4 (TIM1, TIM2, TIM3, TIM4)\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "\r\nOutput Pins:\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "- PA8:  TIM1_CH1 (Master, phase 0°)\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "- PA0:  TIM2_CH1 (Slave, phase 0°)\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "- PA6:  TIM3_CH1 (Slave, phase 90°)\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "- PB6:  TIM4_CH1 (Slave, phase 180°)\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "\r\nStarting synchronized PWM...\r\n\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    /* Initialize timers */
    TIM1_Master_Init();
    TIM2_Slave_Init();
    TIM3_Slave_Init();
    TIM4_Slave_Init();

    /* Start PWM on all channels */
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);

    /* Initial duty cycle 50% */
    Update_PWM_DutyCycle(50);

    sprintf(msg, "PWM started! Initial duty cycle: 50%%\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    uint32_t duty = 10;
    uint8_t direction = 1;

    /* Main loop - sweep duty cycle */
    while(1)
    {
        /* Update duty cycle */
        Update_PWM_DutyCycle(duty);

        sprintf(msg, "Duty Cycle: %lu%%\r\n", duty);
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

        /* Sweep duty cycle dari 10% ke 90% */
        if(direction)
        {
            duty += 10;
            if(duty >= 90)
                direction = 0;
        }
        else
        {
            duty -= 10;
            if(duty <= 10)
                direction = 1;
        }

        /* Status LED toggle */
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);

        HAL_Delay(2000);
    }
}

/**
 * System Clock Configuration
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
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

/**
 * GPIO Initialization
 */
void GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* PC13 - Status LED */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* TIM1 CH1 - PA8 */
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* TIM2 CH1 - PA0 */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* TIM3 CH1 - PA6 */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* TIM4 CH1 - PB6 */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/**
 * UART Initialization
 */
void UART_Init(void)
{
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart1);
}

/**
 * TIM1 Master Timer Initialization
 */
void TIM1_Master_Init(void)
{
    __HAL_RCC_TIM1_CLK_ENABLE();

    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    /* Timer base configuration */
    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 0;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = PWM_PERIOD;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_PWM_Init(&htim1);

    /* Master mode configuration - TRGO on Update Event */
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig);

    /* PWM channel configuration */
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = PWM_PERIOD / 2; // 50% duty cycle
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);
}

/**
 * TIM2 Slave Timer Initialization (Phase 0°)
 */
void TIM2_Slave_Init(void)
{
    __HAL_RCC_TIM2_CLK_ENABLE();

    TIM_SlaveConfigTypeDef sSlaveConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    /* Timer base configuration */
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 0;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = PWM_PERIOD;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_PWM_Init(&htim2);

    /* Slave mode configuration - triggered by TIM1 */
    sSlaveConfig.SlaveMode = TIM_SLAVEMODE_TRIGGER;
    sSlaveConfig.InputTrigger = TIM_TS_ITR0; // TIM1 TRGO
    HAL_TIM_SlaveConfigSynchro(&htim2, &sSlaveConfig);

    /* PWM channel configuration */
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = PWM_PERIOD / 2; // 50% duty cycle
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1);
}

/**
 * TIM3 Slave Timer Initialization (Phase 90°)
 */
void TIM3_Slave_Init(void)
{
    __HAL_RCC_TIM3_CLK_ENABLE();

    TIM_SlaveConfigTypeDef sSlaveConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    /* Timer base configuration */
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 0;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = PWM_PERIOD;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_PWM_Init(&htim3);

    /* Slave mode configuration - triggered by TIM1 */
    sSlaveConfig.SlaveMode = TIM_SLAVEMODE_TRIGGER;
    sSlaveConfig.InputTrigger = TIM_TS_ITR0; // TIM1 TRGO
    HAL_TIM_SlaveConfigSynchro(&htim3, &sSlaveConfig);

    /* PWM channel configuration with phase shift */
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = (PWM_PERIOD / 2) + (PWM_PERIOD / 4); // 50% duty + 90° phase
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1);
}

/**
 * TIM4 Slave Timer Initialization (Phase 180°)
 */
void TIM4_Slave_Init(void)
{
    __HAL_RCC_TIM4_CLK_ENABLE();

    TIM_SlaveConfigTypeDef sSlaveConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    /* Timer base configuration */
    htim4.Instance = TIM4;
    htim4.Init.Prescaler = 0;
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = PWM_PERIOD;
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_PWM_Init(&htim4);

    /* Slave mode configuration - triggered by TIM1 */
    sSlaveConfig.SlaveMode = TIM_SLAVEMODE_TRIGGER;
    sSlaveConfig.InputTrigger = TIM_TS_ITR2; // TIM3 TRGO (cascaded)
    HAL_TIM_SlaveConfigSynchro(&htim4, &sSlaveConfig);

    /* PWM channel configuration with 180° phase shift */
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0; // Inverted phase
    sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW; // Inverted polarity = 180° shift
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1);
}

/**
 * Update PWM Duty Cycle untuk semua channels
 */
void Update_PWM_DutyCycle(uint32_t duty_percent)
{
    uint32_t pulse = (PWM_PERIOD * duty_percent) / 100;

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pulse);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pulse);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulse + (PWM_PERIOD / 4));
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pulse);
}

/**
 * SysTick Handler
 */
void SysTick_Handler(void)
{
    HAL_IncTick();
}
