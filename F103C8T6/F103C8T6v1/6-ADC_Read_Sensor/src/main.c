/**
 * ============================================================================
 * PRAKTIKUM 6: ADC READ SENSOR
 * ============================================================================
 * 
 * Tujuan Pembelajaran:
 * - Memahami konsep Analog to Digital Conversion
 * - Mengkonfigurasi ADC untuk membaca nilai analog
 * - Mengkonversi nilai ADC ke tegangan
 * - Membaca sensor suhu internal dan Vrefint
 * - Menggunakan DMA untuk pembacaan ADC kontinu
 * 
 * Hardware:
 * - STM32F103C8T6 (Blue Pill)
 * - Potensiometer 10kΩ di PA0
 * - LDR (Light Dependent Resistor) di PA1 - opsional
 * - LED di PC13
 * 
 * Wiring Potensiometer:
 *   3.3V ----[POT]---- GND
 *              |
 *             PA0
 * 
 * ADC Configuration:
 * - Resolution: 12-bit (0-4095)
 * - Vref: 3.3V
 * - Voltage = (ADC_Value / 4095) * 3.3V
 * 
 * ============================================================================
 */

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

/* ADC Handle */
ADC_HandleTypeDef hadc1;

/* UART Handle */
UART_HandleTypeDef huart1;

/* ADC Values */
volatile uint16_t adc_value = 0;
volatile uint16_t adc_values[3];  // Multiple channels
volatile uint8_t adc_conversion_complete = 0;

/* Calibration values */
#define VREF            3.3f    // Reference voltage
#define ADC_RESOLUTION  4095.0f // 12-bit ADC

/* Private function prototypes */
void SystemClock_Config(void);
void Error_Handler(void);
void GPIO_Init(void);
void ADC1_Init(void);
void ADC1_MultiChannel_Init(void);
void UART1_Init(void);
void UART_SendString(const char* str);
float ADC_To_Voltage(uint16_t adc_value);
float Read_Temperature(void);

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
    ADC1_Init();
    
    UART_SendString("\r\n===================================\r\n");
    UART_SendString("STM32F103C8T6 ADC Demo\r\n");
    UART_SendString("===================================\r\n");
    UART_SendString("ADC Resolution: 12-bit (0-4095)\r\n");
    UART_SendString("Reference Voltage: 3.3V\r\n");
    UART_SendString("===================================\r\n\r\n");
    
    char msg[100];
    
    while (1)
    {
        /* Start ADC conversion */
        HAL_ADC_Start(&hadc1);
        
        /* Wait for conversion to complete */
        if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
        {
            /* Read ADC value */
            adc_value = HAL_ADC_GetValue(&hadc1);
            
            /* Convert to voltage */
            float voltage = ADC_To_Voltage(adc_value);
            
            /* Calculate percentage (untuk potensiometer) */
            float percentage = (adc_value / ADC_RESOLUTION) * 100.0f;
            
            /* Print hasil */
            sprintf(msg, "ADC Raw: %4d | Voltage: %.2fV | Percentage: %.1f%%\r\n",
                    adc_value, voltage, percentage);
            UART_SendString(msg);
            
            /* LED brightness based on ADC value */
            if (percentage > 50.0f)
            {
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);  // LED ON
            }
            else
            {
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);    // LED OFF
            }
        }
        
        HAL_ADC_Stop(&hadc1);
        
        /* Delay 500ms */
        HAL_Delay(500);
    }
}

/**
 * @brief  Konversi nilai ADC ke tegangan
 * @param  adc_value: Nilai ADC (0-4095)
 * @retval Tegangan dalam volt
 */
float ADC_To_Voltage(uint16_t adc_value)
{
    return (adc_value / ADC_RESOLUTION) * VREF;
}

/**
 * @brief  Inisialisasi ADC1 Single Channel
 * @note   Menggunakan PA0 (ADC1_IN0)
 * @retval None
 */
void ADC1_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Enable clocks */
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    /* Konfigurasi PA0 sebagai Analog Input */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* Konfigurasi ADC1 */
    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;           // Single channel
    hadc1.Init.ContinuousConvMode = DISABLE;              // Single conversion
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;     // Software trigger
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;           // Right alignment
    hadc1.Init.NbrOfConversion = 1;
    
    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }
    
    /* Konfigurasi channel */
    sConfig.Channel = ADC_CHANNEL_0;                      // PA0
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;    // Longest sample time
    
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
    
    /* Kalibrasi ADC (penting untuk akurasi!) */
    HAL_ADCEx_Calibration_Start(&hadc1);
}

/**
 * @brief  Inisialisasi ADC1 Multiple Channels dengan DMA
 * @note   Menggunakan PA0, PA1, dan internal temperature sensor
 * @retval None
 */
void ADC1_MultiChannel_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    /* Konfigurasi PA0 dan PA1 sebagai Analog Input */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* Konfigurasi ADC1 untuk scan mode */
    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;            // Multiple channels
    hadc1.Init.ContinuousConvMode = ENABLE;               // Continuous conversion
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 3;                       // 3 channels
    
    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }
    
    /* Channel 0 - PA0 */
    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    
    /* Channel 1 - PA1 */
    sConfig.Channel = ADC_CHANNEL_1;
    sConfig.Rank = ADC_REGULAR_RANK_2;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    
    /* Internal Temperature Sensor */
    sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
    sConfig.Rank = ADC_REGULAR_RANK_3;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    
    /* Kalibrasi ADC */
    HAL_ADCEx_Calibration_Start(&hadc1);
}

/**
 * @brief  Baca suhu internal MCU
 * @note   Formula: Temp = ((V25 - Vsense) / Avg_Slope) + 25
 *         V25 = 1.43V, Avg_Slope = 4.3mV/°C
 * @retval Suhu dalam derajat Celsius
 */
float Read_Temperature(void)
{
    uint16_t temp_adc;
    float vsense;
    float temperature;
    
    /* Baca nilai ADC dari temperature sensor */
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 100);
    temp_adc = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    
    /* Konversi ke tegangan */
    vsense = ADC_To_Voltage(temp_adc);
    
    /* Hitung suhu */
    /* V25 = 1.43V, Avg_Slope = 4.3mV/°C (dari datasheet) */
    temperature = ((1.43f - vsense) / 0.0043f) + 25.0f;
    
    return temperature;
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
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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
    
    /* ADC Clock = PCLK2 / 6 = 72MHz / 6 = 12MHz (max 14MHz) */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
    
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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
