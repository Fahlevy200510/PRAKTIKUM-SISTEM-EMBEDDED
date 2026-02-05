/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 23-Debounce_Timer
 * 
 * JUDUL: Debounce Button dengan One-Shot Timer
 * 
 * DESKRIPSI:
 * Demo debouncing menggunakan one-shot timer dan ADC threshold.
 * Karena Blue Pill tidak punya button fisik, kita gunakan ADC
 * untuk simulasi: voltage > threshold = "pressed"
 * 
 * ============================================================================
 * ARSITEKTUR PROGRAM
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                    DEBOUNCE TIMER DEMO                             │
 *    ├─────────────────────────────────────────────────────────────────────┤
 *    │                                                                     │
 *    │   ┌─────────────────────────────────────────────────────────────┐   │
 *    │   │                   vPollTask                                 │   │
 *    │   │                   (Poll ADC)                                │   │
 *    │   │                                                             │   │
 *    │   │   while(1) {                                                │   │
 *    │   │       Read ADC value                                        │   │
 *    │   │       if(rising edge detected) {                            │   │
 *    │   │           xTimerReset(debounceTimer)  ───────┐              │   │
 *    │   │       }                                       │              │   │
 *    │   │       vTaskDelay(10ms)                        │              │   │
 *    │   │   }                                           │              │   │
 *    │   └───────────────────────────────────────────────┼──────────────┘   │
 *    │                                                   │                  │
 *    │                                                   ▼                  │
 *    │   ┌─────────────────────────────────────────────────────────────┐   │
 *    │   │               xDebounceTimer (One-Shot)                     │   │
 *    │   │                                                             │   │
 *    │   │   Period: 50ms                                              │   │
 *    │   │   Auto-reload: pdFALSE (one-shot)                           │   │
 *    │   │                                                             │   │
 *    │   │   Every reset restarts the 50ms countdown!                  │   │
 *    │   │                                                             │   │
 *    │   └───────────────────────────────────────────────┬──────────────┘   │
 *    │                                                   │                  │
 *    │                                                   ▼ (after 50ms)     │
 *    │   ┌─────────────────────────────────────────────────────────────┐   │
 *    │   │               vDebounceCallback                             │   │
 *    │   │                                                             │   │
 *    │   │   - Read ADC again (confirm still pressed)                  │   │
 *    │   │   - If confirmed: Process button event                      │   │
 *    │   │   - Toggle LED                                              │   │
 *    │   │   - Print message                                           │   │
 *    │   │                                                             │   │
 *    │   └─────────────────────────────────────────────────────────────┘   │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * TIMELINE DEBOUNCE
 * ============================================================================
 * 
 *    ADC Value:
 *    
 *    High ────────┐     ┌─┐ ┌─┐ ┌────────────────────────────
 *                 │     │ │ │ │ │
 *    Low          └─────┘ └─┘ └─┘
 *                 │     │   │   │           │
 *    Edges:       │ E1  │E2 │E3 │E4         │
 *                 ▼     ▼   ▼   ▼           │
 *                 │                         │
 *    Timer:       │◄── 50ms ──────────────►│
 *                 Start           Each edge │
 *                                 resets!   ▼
 *                                        Callback!
 *                                        (Confirmed press)
 * 
 * ============================================================================
 * EXPECTED OUTPUT
 * ============================================================================
 * 
 *    === Debounce Timer Demo ===
 *    
 *    Touch PA0 pin to simulate button press (above threshold)
 *    
 *    [5.230s] Edge detected! Starting debounce...
 *    [5.235s] Edge detected! Resetting debounce...
 *    [5.237s] Edge detected! Resetting debounce...
 *    [5.290s] DEBOUNCED: Button Press #1 confirmed!
 *    
 *    [8.500s] Edge detected! Starting debounce...
 *    [8.555s] DEBOUNCED: Button Press #2 confirmed!
 * 
 * ============================================================================
 */

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * DEFINISI KONSTANTA
 * ============================================================================ */

#define ADC_THRESHOLD     2048    /* Nilai tengah 12-bit ADC */

/* ============================================================================
 * GLOBAL VARIABLES
 * ============================================================================ */

static TimerHandle_t xDebounceTimer = NULL;

static UART_HandleTypeDef huart1;
static ADC_HandleTypeDef hadc1;

static volatile uint32_t ulButtonPressCount = 0;
static volatile uint8_t  ucLastButtonState = 0;   /* 0 = released, 1 = pressed */
static volatile uint8_t  ucDebouncing = 0;         /* Flag sedang debounce */

/* ============================================================================
 * PROTOTYPE
 * ============================================================================ */

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART_Init(void);
static void ADC_Init(void);
static void UART_SendString(const char *str);
static uint16_t ADC_Read(void);

static void vPollTask(void *pvParameters);
static void vDebounceCallback(TimerHandle_t xTimer);

/* ============================================================================
 * DEBOUNCE TIMER CALLBACK
 * ============================================================================ */

/**
 * @brief Callback setelah debounce period selesai
 * 
 * CATATAN PENTING:
 * ═══════════════════════════════════════════════════════════════════════════
 * 
 *    Callback ini dipanggil HANYA jika:
 *    - 50ms telah berlalu TANPA ada edge baru
 *    - Artinya signal sudah stabil
 *    
 *    Di callback ini, kita:
 *    1. Baca state button lagi (untuk konfirmasi)
 *    2. Jika masih "pressed" → valid button press!
 *    3. Jika sudah "released" → noise atau very short press
 * 
 * ═══════════════════════════════════════════════════════════════════════════
 */
static void vDebounceCallback(TimerHandle_t xTimer)
{
    (void)xTimer;
    
    /* Clear debouncing flag */
    ucDebouncing = 0;
    
    /* Baca ADC untuk konfirmasi */
    uint16_t adcValue = ADC_Read();
    uint8_t currentState = (adcValue > ADC_THRESHOLD) ? 1 : 0;
    
    char buffer[80];
    float seconds = (float)xTaskGetTickCount() / 1000.0f;
    
    if(currentState == 1)  /* Still pressed */
    {
        ulButtonPressCount++;
        
        /* Toggle LED */
        HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
        
        snprintf(buffer, sizeof(buffer),
                 "[%.3fs] DEBOUNCED: Button Press #%lu confirmed!\r\n\r\n",
                 seconds, ulButtonPressCount);
    }
    else
    {
        snprintf(buffer, sizeof(buffer),
                 "[%.3fs] Debounce done, but button released (noise?)\r\n",
                 seconds);
    }
    
    UART_SendString(buffer);
    
    /* Update last state */
    ucLastButtonState = currentState;
}

/* ============================================================================
 * POLL TASK
 * ============================================================================ */

/**
 * @brief Task yang poll ADC untuk deteksi rising edge
 * 
 * DALAM IMPLEMENTASI REAL:
 * ═══════════════════════════════════════════════════════════════════════════
 * 
 *    Biasanya kita gunakan EXTI interrupt, bukan polling:
 *    
 *    void EXTI0_IRQHandler(void)
 *    {
 *        if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_0) != RESET)
 *        {
 *            __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
 *            
 *            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
 *            
 *            // Reset timer dari ISR
 *            xTimerResetFromISR(xDebounceTimer, &xHigherPriorityTaskWoken);
 *            
 *            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
 *        }
 *    }
 *    
 *    Tapi karena ini demo, kita gunakan polling + ADC
 * 
 * ═══════════════════════════════════════════════════════════════════════════
 */
static void vPollTask(void *pvParameters)
{
    (void)pvParameters;
    
    char buffer[80];
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    UART_SendString("Poll task started. Touch PA0 to simulate button...\r\n\r\n");
    
    for(;;)
    {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(POLL_PERIOD_MS));
        
        /* Baca ADC */
        uint16_t adcValue = ADC_Read();
        uint8_t currentState = (adcValue > ADC_THRESHOLD) ? 1 : 0;
        
        /* Deteksi rising edge (0 → 1) */
        if(currentState == 1 && ucLastButtonState == 0)
        {
            float seconds = (float)xTaskGetTickCount() / 1000.0f;
            
            if(!ucDebouncing)
            {
                /* First edge - start debounce timer */
                ucDebouncing = 1;
                snprintf(buffer, sizeof(buffer),
                         "[%.3fs] Edge detected! Starting debounce...\r\n",
                         seconds);
                UART_SendString(buffer);
                
                xTimerReset(xDebounceTimer, 0);
            }
            else
            {
                /* Subsequent edge during debounce - just reset timer */
                snprintf(buffer, sizeof(buffer),
                         "[%.3fs] Edge detected! Resetting debounce...\r\n",
                         seconds);
                UART_SendString(buffer);
                
                xTimerReset(xDebounceTimer, 0);
            }
        }
        
        /* Update last state (untuk falling edge jika tidak debouncing) */
        if(!ucDebouncing)
        {
            ucLastButtonState = currentState;
        }
    }
}

/* ============================================================================
 * KONFIGURASI HARDWARE
 * ============================================================================ */

static void SystemClock_Config(void)
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
    
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
    
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
}

static void GPIO_Init(void)
{
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = LED_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);
    
    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_SET);
}

static void UART_Init(void)
{
    __HAL_RCC_USART1_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = DEBUG_UART_TX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DEBUG_UART_TX_PORT, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = DEBUG_UART_RX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DEBUG_UART_RX_PORT, &GPIO_InitStruct);
    
    huart1.Instance = DEBUG_UART_INSTANCE;
    huart1.Init.BaudRate = DEBUG_UART_BAUDRATE;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart1);
}

static void ADC_Init(void)
{
    __HAL_RCC_ADC1_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = ADC_BUTTON_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(ADC_BUTTON_PORT, &GPIO_InitStruct);
    
    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    HAL_ADC_Init(&hadc1);
    
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
}

static void UART_SendString(const char *str)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
}

static uint16_t ADC_Read(void)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    uint16_t value = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    return value;
}

/* ============================================================================
 * HOOKS FreeRTOS
 * ============================================================================ */

void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();
    for(;;);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    taskDISABLE_INTERRUPTS();
    for(;;);
}

/* ============================================================================
 * MAIN
 * ============================================================================ */

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    UART_Init();
    ADC_Init();
    
    UART_SendString("\r\n================================\r\n");
    UART_SendString("23-Debounce_Timer\r\n");
    UART_SendString("STM32F103 + FreeRTOS\r\n");
    UART_SendString("================================\r\n\r\n");
    UART_SendString("=== Debounce Timer Demo ===\r\n\r\n");
    UART_SendString("Touch PA0 pin to simulate button press\r\n");
    UART_SendString("(voltage > 1.65V = pressed)\r\n\r\n");
    
    /*
     * Buat ONE-SHOT debounce timer
     * 
     * PENTING: pdFALSE = one-shot (tidak auto-reload)
     * Timer hanya trigger sekali, lalu stop.
     */
    xDebounceTimer = xTimerCreate(
        "Debounce",                           /* Name */
        pdMS_TO_TICKS(DEBOUNCE_PERIOD_MS),   /* 50ms debounce period */
        pdFALSE,                             /* ONE-SHOT! */
        NULL,                                /* Timer ID not used */
        vDebounceCallback                    /* Callback */
    );
    
    if(xDebounceTimer == NULL)
    {
        UART_SendString("ERROR: Failed to create debounce timer!\r\n");
        while(1);
    }
    
    /* Buat poll task */
    xTaskCreate(vPollTask, "Poll", TASK_STACK, NULL,
                tskIDLE_PRIORITY + 1, NULL);
    
    /* Start scheduler */
    vTaskStartScheduler();
    
    for(;;);
}
