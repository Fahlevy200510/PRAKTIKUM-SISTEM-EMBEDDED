/**
 * Program 29: WWDG (Window Watchdog) Implementation
 * 
 * Deskripsi:
 * Program ini mendemonstrasikan penggunaan Window Watchdog (WWDG) untuk 
 * mendeteksi software fault. Berbeda dengan IWDG, WWDG memiliki "window"
 * dimana refresh harus dilakukan dalam range tertentu (tidak terlalu cepat,
 * tidak terlalu lambat).
 * 
 * Perbedaan WWDG vs IWDG:
 * - WWDG: Window-based, refresh harus dalam range waktu tertentu
 * - IWDG: Simple timeout, refresh sebelum timeout
 * - WWDG: Dapat generate interrupt sebelum reset
 * - IWDG: Langsung reset, tanpa interrupt
 * 
 * Fitur:
 * - WWDG configuration dengan window counter
 * - Early Wakeup Interrupt (EWI) untuk warning
 * - Auto-refresh dalam window yang benar
 * - Demonstrasi refresh too early (fault)
 * - Demonstrasi refresh too late (fault)
 * - UART monitoring
 * 
 * WWDG Parameters:
 * - Counter: 0x7F (127) down to 0x40 (64) = reset
 * - Window: 0x50 (80)
 * - Refresh allowed: counter < window (antara 80-64)
 * - Refresh too early: counter >= window = reset
 * - Timeout: ~6.5ms @ 72MHz PCLK1
 * 
 * Hardware:
 * - STM32F103C8T6 Blue Pill
 * - UART USB adapter (PA9/PA10)
 * - 3x Button (PA0, PA1, PA2) untuk test scenarios
 * - LED PC13 (onboard)
 * 
 * Pin Configuration:
 * - PA0: Button 1 (Normal refresh)
 * - PA1: Button 2 (Too early refresh - test)
 * - PA2: Button 3 (Too late refresh - test)
 * - PA9: UART TX
 * - PA10: UART RX
 * - PC13: Status LED
 */

#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* WWDG handle */
WWDG_HandleTypeDef hwwdg;

/* UART handle */
UART_HandleTypeDef huart1;

/* Test mode flags */
typedef enum {
    MODE_NORMAL = 0,
    MODE_TOO_EARLY,
    MODE_TOO_LATE,
    MODE_DISABLED
} TestMode;

TestMode current_mode = MODE_NORMAL;
volatile uint32_t early_wakeup_count = 0;
volatile uint8_t watchdog_refreshed = 0;

/* Function prototypes */
void SystemClock_Config(void);
void GPIO_Init(void);
void UART_Init(void);
void WWDG_Init(void);
void Check_Reset_Source(void);
void Display_Menu(void);
void Process_Button(void);

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

    /* LED blink untuk indikasi boot */
    for(int i = 0; i < 3; i++)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_Delay(100);
    }

    sprintf(msg, "\r\n\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "╔═══════════════════════════════════════╗\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "║   WWDG Window Watchdog Demo          ║\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "║   STM32F103C8T6                      ║\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "╚═══════════════════════════════════════╝\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    /* Check reset source */
    Check_Reset_Source();

    sprintf(msg, "\r\nWWDG Configuration:\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "- Counter Start: 127 (0x7F)\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "- Window: 80 (0x50)\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "- Refresh Window: 80 - 64\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "- Timeout: ~6.5ms\r\n\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    Display_Menu();

    /* Initialize WWDG */
    WWDG_Init();

    sprintf(msg, "\r\nWWDG started! Auto-refresh enabled.\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    uint32_t last_refresh = HAL_GetTick();

    /* Main loop */
    while(1)
    {
        /* Check buttons untuk mode selection */
        Process_Button();

        /* WWDG refresh logic berdasarkan mode */
        if(current_mode != MODE_DISABLED)
        {
            uint32_t now = HAL_GetTick();
            
            switch(current_mode)
            {
                case MODE_NORMAL:
                    /* Normal refresh - dalam window yang benar */
                    if((now - last_refresh) >= 4) // 4ms (dalam window 6.5ms)
                    {
                        HAL_WWDG_Refresh(&hwwdg);
                        last_refresh = now;
                        watchdog_refreshed = 1;
                        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
                    }
                    break;

                case MODE_TOO_EARLY:
                    /* Too early refresh - langsung refresh (akan trigger reset) */
                    HAL_WWDG_Refresh(&hwwdg);
                    sprintf(msg, "Refresh TOO EARLY! System will reset...\r\n");
                    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
                    HAL_Delay(100);
                    /* System akan reset karena refresh outside window */
                    break;

                case MODE_TOO_LATE:
                    /* Too late refresh - delay sampai timeout */
                    sprintf(msg, "Delaying refresh... Timeout akan terjadi!\r\n");
                    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
                    HAL_Delay(20); // Delay lebih dari timeout
                    /* System akan reset karena counter reach 0x3F */
                    break;

                default:
                    break;
            }
        }

        /* Display status setiap 500ms */
        static uint32_t last_display = 0;
        if((HAL_GetTick() - last_display) >= 500)
        {
            last_display = HAL_GetTick();
            
            if(watchdog_refreshed)
            {
                sprintf(msg, "[OK] WWDG refreshed, EWI count: %lu\r\n", 
                        early_wakeup_count);
                HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);
                watchdog_refreshed = 0;
            }
        }
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

    /* PA0, PA1, PA2 - Buttons */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
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
 * WWDG Initialization
 */
void WWDG_Init(void)
{
    __HAL_RCC_WWDG_CLK_ENABLE();

    /* WWDG Configuration:
     * - Prescaler: WWDG_PRESCALER_8
     * - Window: 80 (0x50)
     * - Counter: 127 (0x7F)
     * 
     * WWDG clock = PCLK1 / 4096 / 8 = 36MHz / 4096 / 8 ≈ 1098 Hz
     * Timeout = (127-64) / 1098 Hz ≈ 57.4ms
     * Window period = (127-80) / 1098 Hz ≈ 42.8ms
     */
    
    hwwdg.Instance = WWDG;
    hwwdg.Init.Prescaler = WWDG_PRESCALER_8;
    hwwdg.Init.Window = 80;    // 0x50
    hwwdg.Init.Counter = 127;  // 0x7F
    hwwdg.Init.EWIMode = WWDG_EWI_ENABLE; // Enable Early Wakeup Interrupt
    
    if(HAL_WWDG_Init(&hwwdg) != HAL_OK)
    {
        sprintf(msg, "ERROR: WWDG Init Failed!\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        Error_Handler();
    }

    /* Enable WWDG interrupt */
    HAL_NVIC_SetPriority(WWDG_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(WWDG_IRQn);
}

/**
 * Check Reset Source
 */
void Check_Reset_Source(void)
{
    sprintf(msg, "\r\nReset Source:\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    if(__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST))
    {
        sprintf(msg, ">>> WWDG Reset Detected! <<<\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        __HAL_RCC_CLEAR_RESET_FLAGS();
    }
    else if(__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST))
    {
        sprintf(msg, "Power-On Reset\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        __HAL_RCC_CLEAR_RESET_FLAGS();
    }
    else if(__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST))
    {
        sprintf(msg, "External Reset Pin\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        __HAL_RCC_CLEAR_RESET_FLAGS();
    }
    else
    {
        sprintf(msg, "Unknown Reset\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        __HAL_RCC_CLEAR_RESET_FLAGS();
    }
}

/**
 * Display Menu
 */
void Display_Menu(void)
{
    sprintf(msg, "\r\n--- Test Modes (tekan button) ---\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "PA0: Normal mode (auto-refresh)\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "PA1: Too Early Refresh Test\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "PA2: Too Late Refresh Test\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

/**
 * Process Button Input
 */
void Process_Button(void)
{
    static uint32_t last_button_time = 0;
    uint32_t now = HAL_GetTick();

    /* Debounce: 200ms */
    if((now - last_button_time) < 200)
        return;

    /* Check PA0 - Normal mode */
    if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET)
    {
        current_mode = MODE_NORMAL;
        sprintf(msg, "\r\n[MODE] Normal Auto-Refresh\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        last_button_time = now;
    }

    /* Check PA1 - Too Early */
    if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_RESET)
    {
        current_mode = MODE_TOO_EARLY;
        sprintf(msg, "\r\n[MODE] Testing Too Early Refresh...\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        last_button_time = now;
    }

    /* Check PA2 - Too Late */
    if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2) == GPIO_PIN_RESET)
    {
        current_mode = MODE_TOO_LATE;
        sprintf(msg, "\r\n[MODE] Testing Too Late Refresh...\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        last_button_time = now;
    }
}

/**
 * WWDG Early Wakeup Callback
 */
void HAL_WWDG_EarlyWakeupCallback(WWDG_HandleTypeDef *hwwdg)
{
    /* Early wakeup interrupt dipanggil saat counter = 0x40 */
    /* Ini adalah warning bahwa refresh harus segera dilakukan */
    early_wakeup_count++;
}

/**
 * WWDG IRQ Handler
 */
void WWDG_IRQHandler(void)
{
    HAL_WWDG_IRQHandler(&hwwdg);
}

/**
 * Error Handler
 */
void Error_Handler(void)
{
    while(1)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_Delay(100);
    }
}

/**
 * SysTick Handler
 */
void SysTick_Handler(void)
{
    HAL_IncTick();
}
