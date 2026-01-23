/**
 * Program 27: ADC Multi-Channel Scan Mode dengan DMA
 * 
 * Deskripsi:
 * Program ini mendemonstrasikan penggunaan ADC dalam mode scan untuk 
 * membaca multiple channel secara berurutan dengan DMA transfer.
 * Cocok untuk aplikasi multi-sensor monitoring.
 * 
 * Fitur:
 * - ADC1 multi-channel scan mode (8 channels)
 * - DMA circular mode untuk continuous conversion
 * - Sampling rate: 1 kHz per channel
 * - Voltage calculation dan averaging
 * - Temperature sensor internal (CH16)
 * - VREFINT internal (CH17)
 * - UART output untuk monitoring
 * 
 * ADC Channels:
 * - CH0 (PA0): External sensor 1
 * - CH1 (PA1): External sensor 2
 * - CH2 (PA2): External sensor 3
 * - CH3 (PA3): External sensor 4
 * - CH4 (PA4): External sensor 5
 * - CH5 (PA5): External sensor 6
 * - CH16: Temperature sensor (internal)
 * - CH17: VREFINT (internal, 1.2V reference)
 * 
 * Hardware:
 * - STM32F103C8T6 Blue Pill
 * - 6x Potensiometer (10k) untuk input analog
 * - UART USB adapter (PA9/PA10)
 * - LED PC13 (onboard)
 * 
 * Pin Configuration:
 * - PA0-PA5: ADC channels 0-5 (analog input)
 * - PA9: UART TX
 * - PA10: UART RX
 * - PC13: Status LED
 */

#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* ADC and DMA handles */
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

/* UART handle */
UART_HandleTypeDef huart1;

/* ADC buffer (DMA target) */
#define ADC_CHANNELS 8
__IO uint16_t adc_buffer[ADC_CHANNELS];

/* Function prototypes */
void SystemClock_Config(void);
void GPIO_Init(void);
void UART_Init(void);
void ADC_DMA_Init(void);
float ADC_to_Voltage(uint16_t adc_value);
float Calculate_Temperature(uint16_t adc_value, float vrefint_voltage);
float Calculate_VREFINT(uint16_t adc_value);

/* Buffer */
char msg[150];

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
    ADC_DMA_Init();

    sprintf(msg, "\r\n=== ADC Multi-Channel Scan with DMA ===\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "Channels: 8 (6 external + 2 internal)\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "Resolution: 12-bit (0-4095)\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "Reference: 3.3V\r\n\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    /* Start ADC with DMA */
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, ADC_CHANNELS);

    sprintf(msg, "ADC Started! Continuous conversion mode.\r\n\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    /* Main loop */
    while(1)
    {
        /* Wait untuk DMA transfer complete (data siap) */
        HAL_Delay(1000);

        /* Calculate VREFINT untuk kalibrasi */
        float vrefint_voltage = Calculate_VREFINT(adc_buffer[7]);

        sprintf(msg, "╔═══════════════════════════════════════╗\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        sprintf(msg, "║     ADC Multi-Channel Reading        ║\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        sprintf(msg, "╠═══════════════════════════════════════╣\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

        /* Display external channels (CH0-CH5) */
        for(int i = 0; i < 6; i++)
        {
            float voltage = ADC_to_Voltage(adc_buffer[i]);
            sprintf(msg, "║ CH%d (PA%d): %4u | %1.3fV           ║\r\n", 
                    i, i, adc_buffer[i], voltage);
            HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        }

        sprintf(msg, "╠═══════════════════════════════════════╣\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

        /* Display temperature */
        float temperature = Calculate_Temperature(adc_buffer[6], vrefint_voltage);
        sprintf(msg, "║ Temperature:  %2.1f°C               ║\r\n", temperature);
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

        /* Display VREFINT */
        sprintf(msg, "║ VREFINT:      %1.3fV                ║\r\n", vrefint_voltage);
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

        sprintf(msg, "╚═══════════════════════════════════════╝\r\n\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

        /* Toggle status LED */
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    }
}

/**
 * System Clock Configuration
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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

    /* ADC clock = PCLK2/6 = 12 MHz */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
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

    /* PA0-PA5 - ADC channels (analog input) */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | 
                          GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
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
 * ADC and DMA Initialization
 */
void ADC_DMA_Init(void)
{
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* DMA configuration */
    hdma_adc1.Instance = DMA1_Channel1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_HIGH;
    HAL_DMA_Init(&hdma_adc1);

    /* Link DMA to ADC */
    __HAL_LINKDMA(&hadc1, DMA_Handle, hdma_adc1);

    /* ADC configuration */
    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE; // Multi-channel scan
    hadc1.Init.ContinuousConvMode = ENABLE;    // Continuous conversion
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = ADC_CHANNELS;
    HAL_ADC_Init(&hadc1);

    /* Configure ADC channels */
    ADC_ChannelConfTypeDef sConfig = {0};

    /* External channels (CH0-CH5) */
    for(int i = 0; i < 6; i++)
    {
        sConfig.Channel = ADC_CHANNEL_0 + i;
        sConfig.Rank = ADC_REGULAR_RANK_1 + i;
        sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;
        HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    }

    /* Internal temperature sensor (CH16) */
    sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
    sConfig.Rank = ADC_REGULAR_RANK_7;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5; // Slow sampling for temp
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    /* Internal VREFINT (CH17) */
    sConfig.Channel = ADC_CHANNEL_VREFINT;
    sConfig.Rank = ADC_REGULAR_RANK_8;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    /* Enable temperature sensor dan VREFINT */
    ADC1->CR2 |= ADC_CR2_TSVREFE;

    /* ADC calibration */
    HAL_ADCEx_Calibration_Start(&hadc1);
}

/**
 * Convert ADC value ke voltage
 */
float ADC_to_Voltage(uint16_t adc_value)
{
    return (adc_value * 3.3f) / 4095.0f;
}

/**
 * Calculate VREFINT voltage (should be ~1.2V)
 */
float Calculate_VREFINT(uint16_t adc_value)
{
    /* VREFINT typical value = 1.2V at 3.3V VDDA */
    return (1.2f * 4095.0f) / adc_value;
}

/**
 * Calculate temperature dari internal sensor
 * Formula: Temp = (V25 - Vsense) / Avg_Slope + 25°C
 * V25 = 1.43V (typical voltage at 25°C)
 * Avg_Slope = 4.3 mV/°C
 */
float Calculate_Temperature(uint16_t adc_value, float vrefint_voltage)
{
    /* Convert ADC value ke voltage */
    float vsense = (adc_value * vrefint_voltage) / 4095.0f;

    /* STM32F103 temperature sensor constants */
    const float V25 = 1.43f;        // Voltage at 25°C
    const float Avg_Slope = 0.0043f; // 4.3mV/°C

    /* Calculate temperature */
    float temperature = ((V25 - vsense) / Avg_Slope) + 25.0f;

    return temperature;
}

/**
 * DMA Transfer Complete Callback
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    /* DMA transfer completed, data ready in adc_buffer */
    /* This callback is optional, data can be read anytime from buffer */
}

/**
 * SysTick Handler
 */
void SysTick_Handler(void)
{
    HAL_IncTick();
}

/**
 * DMA1 Channel1 IRQ Handler
 */
void DMA1_Channel1_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_adc1);
}
