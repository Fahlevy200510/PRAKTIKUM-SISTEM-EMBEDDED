/**
 * ============================================================================
 * PRAKTIKUM 2: BUTTON INPUT - GPIO Input dengan Polling
 * ============================================================================
 * 
 * Tujuan Pembelajaran:
 * - Memahami konfigurasi GPIO sebagai input
 * - Memahami penggunaan pull-up dan pull-down resistor
 * - Memahami teknik debouncing sederhana
 * - Mengontrol output berdasarkan input
 * 
 * Hardware:
 * - STM32F103C8T6 (Blue Pill)
 * - Push button di PA0 dengan pull-down resistor
 * - LED di PC13 (built-in)
 * 
 * Wiring:
 *   PA0 ----[Button]---- 3.3V
 *        |
 *       [10K]
 *        |
 *       GND (Pull-down)
 * 
 * ============================================================================
 */

#include "stm32f1xx_hal.h"

/* Define untuk pin */
#define BUTTON_PIN          GPIO_PIN_0
#define BUTTON_PORT         GPIOA
#define LED_PIN             GPIO_PIN_13
#define LED_PORT            GPIOC

/* Konstanta debounce */
#define DEBOUNCE_DELAY_MS   50

/* Private function prototypes */
void SystemClock_Config(void);
void Error_Handler(void);
void GPIO_Init(void);
uint8_t Button_Read_Debounced(void);

/* Variable global */
volatile uint8_t led_state = 0;

/**
 * @brief  Main program
 * @retval int
 */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    
    uint8_t last_button_state = 0;
    uint8_t current_button_state = 0;
    
    while (1)
    {
        /* Baca status tombol dengan debouncing */
        current_button_state = Button_Read_Debounced();
        
        /* Deteksi rising edge (tombol ditekan) */
        if (current_button_state == 1 && last_button_state == 0)
        {
            /* Toggle LED setiap kali tombol ditekan */
            led_state = !led_state;
            
            /* Update LED - PC13 active LOW */
            HAL_GPIO_WritePin(LED_PORT, LED_PIN, 
                             led_state ? GPIO_PIN_RESET : GPIO_PIN_SET);
        }
        
        last_button_state = current_button_state;
        
        /* Delay kecil untuk mengurangi CPU load */
        HAL_Delay(10);
    }
}

/**
 * @brief  Baca tombol dengan debouncing
 * @retval 1 jika tombol ditekan, 0 jika tidak
 */
uint8_t Button_Read_Debounced(void)
{
    static uint32_t last_debounce_time = 0;
    static uint8_t last_reading = 0;
    static uint8_t button_state = 0;
    
    uint8_t reading = HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN);
    
    /* Jika pembacaan berubah, reset timer debounce */
    if (reading != last_reading)
    {
        last_debounce_time = HAL_GetTick();
    }
    
    /* Jika pembacaan stabil selama DEBOUNCE_DELAY_MS */
    if ((HAL_GetTick() - last_debounce_time) > DEBOUNCE_DELAY_MS)
    {
        button_state = reading;
    }
    
    last_reading = reading;
    return button_state;
}

/**
 * @brief  Inisialisasi GPIO
 * @retval None
 */
void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Enable clock untuk GPIOA dan GPIOC */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    /* Konfigurasi PA0 sebagai input dengan internal pull-down */
    GPIO_InitStruct.Pin = BUTTON_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;  // Internal pull-down
    HAL_GPIO_Init(BUTTON_PORT, &GPIO_InitStruct);
    
    /* Konfigurasi PC13 sebagai output untuk LED */
    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);
    
    /* LED mati saat awal (HIGH karena active low) */
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
}

/**
 * @brief  System Clock Configuration - 72MHz
 * @retval None
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
        HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
        HAL_Delay(100);
    }
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}
