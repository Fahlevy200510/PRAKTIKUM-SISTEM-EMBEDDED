/**
 * Program 28: DAC Waveform Generator dengan Timer + DMA
 * 
 * CATATAN PENTING:
 * STM32F103C8T6 TIDAK MEMILIKI DAC peripheral!
 * Program ini menggunakan teknik PWM + Low Pass Filter sebagai DAC emulation.
 * Untuk DAC hardware, gunakan STM32F103RB/RC atau eksternal DAC (MCP4725).
 * 
 * Deskripsi:
 * Program ini membuat waveform generator menggunakan PWM untuk mensimulasikan 
 * output analog. Cocok untuk testing, audio signal generation, atau control signal.
 * 
 * Fitur:
 * - PWM-based DAC emulation (Resolution: 8-bit, 256 levels)
 * - Multiple waveforms: Sine, Square, Triangle, Sawtooth
 * - Frequency adjustable (10Hz - 10kHz)
 * - DMA untuk automatic waveform update
 * - UART interface untuk kontrol
 * 
 * Waveform Types:
 * 1. Sine Wave - Smooth sinusoidal output
 * 2. Square Wave - Digital on/off signal
 * 3. Triangle Wave - Linear ramp up/down
 * 4. Sawtooth Wave - Linear ramp up, fast drop
 * 
 * Hardware:
 * - STM32F103C8T6 Blue Pill
 * - RC Low Pass Filter (R=1kΩ, C=10µF) pada output PWM
 * - Oscilloscope atau multimeter untuk melihat output
 * - UART USB adapter (PA9/PA10)
 * 
 * Pin Configuration:
 * - PA0: PWM Output (TIM2_CH1) -> RC Filter -> Analog Output
 * - PA9: UART TX
 * - PA10: UART RX
 * - PC13: Status LED
 * 
 * Circuit untuk Low Pass Filter:
 * PA0 ---[ 1kΩ ]---+--- Analog Output
 *                  |
 *                 ---
 *                 --- 10µF
 *                  |
 *                 GND
 * 
 * Cutoff Frequency: fc = 1/(2π*R*C) = 1/(2π*1000*0.00001) ≈ 15.9Hz
 */

#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

/* Timer and DMA handles */
TIM_HandleTypeDef htim2;
DMA_HandleTypeDef hdma_tim2_ch1;

/* UART handle */
UART_HandleTypeDef huart1;

/* Waveform buffer */
#define WAVEFORM_SAMPLES 256
uint16_t waveform_buffer[WAVEFORM_SAMPLES];

/* Waveform types */
typedef enum {
    WAVE_SINE = 0,
    WAVE_SQUARE,
    WAVE_TRIANGLE,
    WAVE_SAWTOOTH
} WaveformType;

/* Current waveform parameters */
WaveformType current_waveform = WAVE_SINE;
uint32_t current_frequency = 100; // Hz

/* Function prototypes */
void SystemClock_Config(void);
void GPIO_Init(void);
void UART_Init(void);
void TIM2_PWM_DMA_Init(uint32_t frequency);
void Generate_Sine_Wave(void);
void Generate_Square_Wave(void);
void Generate_Triangle_Wave(void);
void Generate_Sawtooth_Wave(void);
void Update_Waveform(void);
void Display_Menu(void);
void Process_Command(char cmd);

/* Buffer */
char msg[100];

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
    TIM2_PWM_DMA_Init(current_frequency);

    sprintf(msg, "\r\n╔═══════════════════════════════════════╗\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "║   PWM DAC Waveform Generator         ║\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "║   STM32F103C8T6                      ║\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "╚═══════════════════════════════════════╝\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "\r\nNOTE: Gunakan RC Low Pass Filter\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "      R=1kΩ, C=10µF pada output PA0\r\n\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    /* Generate initial waveform (Sine) */
    Update_Waveform();

    /* Start PWM with DMA */
    HAL_TIM_PWM_Start_DMA(&htim2, TIM_CHANNEL_1, (uint32_t*)waveform_buffer, WAVEFORM_SAMPLES);

    sprintf(msg, "Waveform generator started!\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    Display_Menu();

    uint8_t rx_data;

    /* Main loop */
    while(1)
    {
        /* Wait for command dari UART */
        if(HAL_UART_Receive(&huart1, &rx_data, 1, 100) == HAL_OK)
        {
            Process_Command((char)rx_data);
            Display_Menu();
        }

        /* Status LED heartbeat */
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_Delay(500);
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
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* PC13 - Status LED */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* PA0 - TIM2 CH1 (PWM output) */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/**
 * UART Initialization
 */
void UART_Init(void)
{
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

/**
 * TIM2 PWM with DMA Initialization
 */
void TIM2_PWM_DMA_Init(uint32_t frequency)
{
    __HAL_RCC_TIM2_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* Calculate timer parameters for desired waveform frequency */
    /* TIM2 Clock = 72MHz (APB1 x2) */
    /* PWM Frequency = 72MHz / (PSC+1) / (ARR+1) */
    /* For 8-bit resolution: ARR = 255 */
    /* Update rate = frequency * WAVEFORM_SAMPLES */
    
    uint32_t tim_clock = 72000000;
    uint32_t update_rate = frequency * WAVEFORM_SAMPLES;
    uint32_t prescaler = (tim_clock / (256 * update_rate)) - 1;

    /* DMA configuration */
    hdma_tim2_ch1.Instance = DMA1_Channel5;
    hdma_tim2_ch1.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_tim2_ch1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_tim2_ch1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_tim2_ch1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_tim2_ch1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_tim2_ch1.Init.Mode = DMA_CIRCULAR;
    hdma_tim2_ch1.Init.Priority = DMA_PRIORITY_HIGH;
    HAL_DMA_Init(&hdma_tim2_ch1);

    /* Link DMA to Timer */
    __HAL_LINKDMA(&htim2, hdma[TIM_DMA_ID_CC1], hdma_tim2_ch1);

    /* Timer configuration */
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = prescaler;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 255; // 8-bit resolution
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_PWM_Init(&htim2);

    /* PWM configuration */
    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1);

    /* Enable DMA interrupt */
    HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
}

/**
 * Generate Sine Wave
 */
void Generate_Sine_Wave(void)
{
    for(int i = 0; i < WAVEFORM_SAMPLES; i++)
    {
        float angle = (2.0f * M_PI * i) / WAVEFORM_SAMPLES;
        float value = (sinf(angle) + 1.0f) / 2.0f; // Normalize to 0-1
        waveform_buffer[i] = (uint16_t)(value * 255);
    }
}

/**
 * Generate Square Wave
 */
void Generate_Square_Wave(void)
{
    for(int i = 0; i < WAVEFORM_SAMPLES; i++)
    {
        waveform_buffer[i] = (i < WAVEFORM_SAMPLES/2) ? 255 : 0;
    }
}

/**
 * Generate Triangle Wave
 */
void Generate_Triangle_Wave(void)
{
    for(int i = 0; i < WAVEFORM_SAMPLES; i++)
    {
        if(i < WAVEFORM_SAMPLES/2)
        {
            // Rising edge
            waveform_buffer[i] = (i * 512) / WAVEFORM_SAMPLES;
        }
        else
        {
            // Falling edge
            waveform_buffer[i] = 255 - ((i - WAVEFORM_SAMPLES/2) * 512) / WAVEFORM_SAMPLES;
        }
    }
}

/**
 * Generate Sawtooth Wave
 */
void Generate_Sawtooth_Wave(void)
{
    for(int i = 0; i < WAVEFORM_SAMPLES; i++)
    {
        waveform_buffer[i] = (i * 255) / WAVEFORM_SAMPLES;
    }
}

/**
 * Update Waveform berdasarkan current selection
 */
void Update_Waveform(void)
{
    switch(current_waveform)
    {
        case WAVE_SINE:
            Generate_Sine_Wave();
            sprintf(msg, "Waveform: SINE, Frequency: %lu Hz\r\n", current_frequency);
            break;
        case WAVE_SQUARE:
            Generate_Square_Wave();
            sprintf(msg, "Waveform: SQUARE, Frequency: %lu Hz\r\n", current_frequency);
            break;
        case WAVE_TRIANGLE:
            Generate_Triangle_Wave();
            sprintf(msg, "Waveform: TRIANGLE, Frequency: %lu Hz\r\n", current_frequency);
            break;
        case WAVE_SAWTOOTH:
            Generate_Sawtooth_Wave();
            sprintf(msg, "Waveform: SAWTOOTH, Frequency: %lu Hz\r\n", current_frequency);
            break;
    }
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

/**
 * Display Menu
 */
void Display_Menu(void)
{
    sprintf(msg, "\r\n--- Commands ---\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "1 - Sine Wave\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "2 - Square Wave\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "3 - Triangle Wave\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "4 - Sawtooth Wave\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "+ - Increase Frequency\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "- - Decrease Frequency\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "\r\n> ");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

/**
 * Process Command
 */
void Process_Command(char cmd)
{
    sprintf(msg, "%c\r\n", cmd);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    switch(cmd)
    {
        case '1':
            current_waveform = WAVE_SINE;
            Update_Waveform();
            break;
        case '2':
            current_waveform = WAVE_SQUARE;
            Update_Waveform();
            break;
        case '3':
            current_waveform = WAVE_TRIANGLE;
            Update_Waveform();
            break;
        case '4':
            current_waveform = WAVE_SAWTOOTH;
            Update_Waveform();
            break;
        case '+':
            if(current_frequency < 1000)
            {
                current_frequency += 10;
                HAL_TIM_PWM_Stop_DMA(&htim2, TIM_CHANNEL_1);
                TIM2_PWM_DMA_Init(current_frequency);
                HAL_TIM_PWM_Start_DMA(&htim2, TIM_CHANNEL_1, (uint32_t*)waveform_buffer, WAVEFORM_SAMPLES);
                Update_Waveform();
            }
            break;
        case '-':
            if(current_frequency > 10)
            {
                current_frequency -= 10;
                HAL_TIM_PWM_Stop_DMA(&htim2, TIM_CHANNEL_1);
                TIM2_PWM_DMA_Init(current_frequency);
                HAL_TIM_PWM_Start_DMA(&htim2, TIM_CHANNEL_1, (uint32_t*)waveform_buffer, WAVEFORM_SAMPLES);
                Update_Waveform();
            }
            break;
        default:
            sprintf(msg, "Unknown command\r\n");
            HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
            break;
    }
}

/**
 * SysTick Handler
 */
void SysTick_Handler(void)
{
    HAL_IncTick();
}

/**
 * DMA IRQ Handler
 */
void DMA1_Channel5_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_tim2_ch1);
}
