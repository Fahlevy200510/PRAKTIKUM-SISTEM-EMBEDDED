/**
 * ============================================================================
 * PRAKTIKUM 1: BLINK LED - Dasar GPIO Output
 * ============================================================================
 * 
 * Tujuan Pembelajaran:
 * - Memahami konfigurasi GPIO sebagai output
 * - Memahami penggunaan HAL_Delay untuk delay sederhana
 * - Memahami struktur dasar program STM32
 * 
 * Hardware:
 * - STM32F103C8T6 (Blue Pill)
 * - LED internal di PC13 (active low)
 * - atau LED eksternal di PA0 dengan resistor 220-330 ohm
 * 
 * Catatan:
 * - LED internal Blue Pill terhubung ke PC13 (active LOW)
 * - Untuk menyalakan LED: GPIO_PIN_RESET (0)
 * - Untuk mematikan LED: GPIO_PIN_SET (1)
 * 
 * ============================================================================
 */

#include "stm32f1xx_hal.h"

/* Private function prototypes */
void SystemClock_Config(void);
void Error_Handler(void);
void GPIO_Init(void);

/**
 * @brief  Main program entry point
 * @retval int
 */
int main(void)
{
    /* Reset semua peripheral, Inisialisasi Flash interface dan Systick */
    HAL_Init();
    
    /* Konfigurasi system clock ke 72 MHz menggunakan HSE + PLL */
    SystemClock_Config();
    
    /* Inisialisasi GPIO untuk LED */
    GPIO_Init();
    
    /* Infinite loop - LED berkedip */
    while (1)
    {
        /* Toggle LED di PC13 */
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        
        /* Delay 500ms */
        HAL_Delay(500);
        
        /* 
         * Variasi bisa dicoba:
         * - Ubah delay menjadi 100ms untuk kedipan cepat
         * - Ubah delay menjadi 1000ms untuk kedipan lambat
         * - Buat pola SOS morse code
         */
    }
}

/**
 * @brief  Inisialisasi GPIO
 * @note   PC13 dikonfigurasi sebagai output push-pull untuk LED
 * @retval None
 */
void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Enable clock untuk GPIOC */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    /* Konfigurasi PC13 sebagai output */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;    // Push-Pull output
    GPIO_InitStruct.Pull = GPIO_NOPULL;            // Tanpa pull-up/pull-down
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;   // Low speed cukup untuk LED
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    /* Optional: Tambahkan LED eksternal di PA0 */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* Set initial state - LED mati (HIGH untuk PC13 karena active low) */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}

/**
 * @brief  System Clock Configuration
 * @note   Menggunakan HSE 8MHz dengan PLL untuk menghasilkan 72MHz
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* Konfigurasi HSE Oscillator dan PLL */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;  // 8MHz * 9 = 72MHz
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /* Konfigurasi clock untuk AHB, APB1, dan APB2 */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;   // 72MHz
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;    // 36MHz (max 36MHz)
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;    // 72MHz

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief  Error Handler
 * @retval None
 */
void Error_Handler(void)
{
    /* LED berkedip cepat untuk menandakan error */
    while (1)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_Delay(100);
    }
}

/**
 * @brief  SysTick Handler - diperlukan untuk HAL_Delay
 * @retval None
 */
void SysTick_Handler(void)
{
    HAL_IncTick();
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    /* User dapat menambahkan implementasi untuk melaporkan error */
}
#endif
