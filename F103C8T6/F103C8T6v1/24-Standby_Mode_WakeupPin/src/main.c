/**
 * Program 24: Standby Mode dengan WKUP Pin
 * 
 * Deskripsi:
 * Program ini mendemonstrasikan STANDBY mode (mode hemat daya terendah) 
 * pada STM32F103C8T6 dengan wake-up menggunakan pin WKUP (PA0).
 * 
 * Fitur:
 * - STANDBY mode entry (konsumsi ~2-3 µA)
 * - Wake-up dari WKUP pin (PA0) dengan rising edge
 * - Backup register untuk menyimpan data counter
 * - Reset flag detection (STANDBY wake-up vs power-on reset)
 * - LED indikator wake-up
 * 
 * Catatan Penting:
 * - STANDBY mode akan mereset semua RAM dan peripheral
 * - Hanya backup registers yang tetap tersimpan
 * - Wake-up dari STANDBY = system reset dengan SBF flag set
 * - PA0 harus rising edge untuk wake-up
 * 
 * Hardware:
 * - STM32F103C8T6 Blue Pill
 * - Button pada PA0 (WKUP pin, pull-down resistor)
 * - LED pada PC13 (onboard)
 * - Multimeter untuk mengukur arus (optional)
 * 
 * Pin Configuration:
 * - PA0: WKUP pin (Button, active HIGH)
 * - PC13: LED indikator
 * - PA9: UART TX (debug, optional)
 * - PA10: UART RX (debug, optional)
 */

#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* UART handle untuk debug */
UART_HandleTypeDef huart1;

/* RTC handle untuk backup register */
RTC_HandleTypeDef hrtc;

/* Function prototypes */
void SystemClock_Config(void);
void GPIO_Init(void);
void UART_Init(void);
void RTC_Init(void);
void Enter_Standby_Mode(void);
void Check_Wakeup_Flags(void);

/* Buffer untuk UART */
char msg[100];

/**
 * Main function
 */
int main(void)
{
    /* Reset of all peripherals, Initializes the Flash interface and the Systick */
    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();

    /* Initialize peripherals */
    GPIO_Init();
    UART_Init();
    RTC_Init();

    /* LED ON untuk indikasi wake-up */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
    HAL_Delay(200);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

    /* Check wake-up flags */
    Check_Wakeup_Flags();

    /* Baca backup register untuk counter */
    uint32_t standby_counter = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1);

    sprintf(msg, "\r\n=== STM32 STANDBY Mode Demo ===\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    sprintf(msg, "Standby Counter: %lu\r\n", standby_counter);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    sprintf(msg, "System akan masuk STANDBY mode dalam 5 detik...\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    sprintf(msg, "Tekan button pada PA0 (WKUP) untuk wake-up!\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    /* LED blink countdown */
    for(int i = 5; i > 0; i--)
    {
        sprintf(msg, "%d... ", i);
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_Delay(500);
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_Delay(500);
    }

    sprintf(msg, "\r\nMasuk STANDBY mode sekarang!\r\n\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    HAL_Delay(100); // Wait UART transmission complete

    /* Increment counter dan simpan ke backup register */
    standby_counter++;
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, standby_counter);

    /* Enter STANDBY mode */
    Enter_Standby_Mode();

    /* Kode di bawah ini tidak akan pernah dieksekusi */
    /* karena wake-up dari STANDBY = system reset */
    while(1)
    {
        // Never reached
    }
}

/**
 * System Clock Configuration
 * HSE 8MHz -> PLL x9 -> SYSCLK 72MHz
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* Configure HSE and PLL */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSI;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9; // 8MHz * 9 = 72MHz
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    /* Configure SYSCLK, HCLK, PCLK1, PCLK2 */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;   // 72MHz
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;    // 36MHz (max)
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;    // 72MHz
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

/**
 * GPIO Initialization
 */
void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIO clocks */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* Configure PC13 as output (LED) */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* Set LED OFF (active LOW) */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

    /* Configure PA0 sebagai WKUP pin sudah otomatis dihandle oleh PWR peripheral */
}

/**
 * UART Initialization
 * PA9: TX, PA10: RX
 */
void UART_Init(void)
{
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* Configure GPIO pins for UART */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // TX
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // RX
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Configure UART */
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
 * RTC Initialization untuk backup registers
 */
void RTC_Init(void)
{
    /* Enable PWR and BKP clocks */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_RCC_BKP_CLK_ENABLE();

    /* Enable access to backup domain */
    HAL_PWR_EnableBkUpAccess();

    /* RTC clock configuration menggunakan LSI */
    hrtc.Instance = RTC;
    hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
    hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
    HAL_RTC_Init(&hrtc);

    /* Check jika ini pertama kali (backup register kosong) */
    if(HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) == 0xFFFFFFFF)
    {
        // First boot, initialize counter to 0
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0);
    }
}

/**
 * Enter STANDBY Mode
 */
void Enter_Standby_Mode(void)
{
    /* Enable PWR clock */
    __HAL_RCC_PWR_CLK_ENABLE();

    /* Clear wake-up flag jika ada */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

    /* Clear standby flag */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);

    /* Enable WKUP pin (PA0) untuk wake-up */
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);

    /* Request to enter STANDBY mode */
    HAL_PWR_EnterSTANDBYMode();

    /* System akan reset setelah wake-up */
    /* Eksekusi akan mulai dari awal main() */
}

/**
 * Check Wake-up Flags
 */
void Check_Wakeup_Flags(void)
{
    /* Check jika wake-up dari STANDBY mode */
    if(__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
    {
        sprintf(msg, ">>> Wake-up dari STANDBY mode! <<<\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

        /* Clear standby flag */
        __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
        __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    }
    else
    {
        sprintf(msg, ">>> Power-On Reset atau normal reset <<<\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    }
}

/**
 * SysTick Handler
 */
void SysTick_Handler(void)
{
    HAL_IncTick();
}
