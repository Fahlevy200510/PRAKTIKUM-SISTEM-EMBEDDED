/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 39-Tickless_Idle
 * 
 * DESKRIPSI:
 * Demonstrasi Tickless Idle mode untuk power saving. MCU akan masuk
 * low-power sleep saat tidak ada task yang perlu dijalankan.
 * 
 * ============================================================================
 * ARSITEKTUR SISTEM
 * ============================================================================
 * 
 *    ┌──────────────────────────────────────────────────────────────────────┐
 *    │                    TICKLESS IDLE DEMO                                │
 *    │                                                                      │
 *    │    TIME ───────────────────────────────────────────────────────▶     │
 *    │                                                                      │
 *    │    │ Task  │ Idle │ Task  │          Idle          │ Task  │        │
 *    │    │ Work  │Sleep │ Work  │       Deep Sleep       │ Work  │        │
 *    │    ├───────┼──────┼───────┼────────────────────────┼───────┤        │
 *    │    │ 100ms │ 5s   │ 100ms │          5s            │ 100ms │        │
 *    │                                                                      │
 *    │    Power Consumption:                                                │
 *    │    ┌───────┐      ┌───────┐                        ┌───────┐        │
 *    │    │ HIGH  │      │ HIGH  │                        │ HIGH  │        │
 *    │    │  50mA │      │  50mA │                        │  50mA │        │
 *    │    ├───────┼──────┼───────┼────────────────────────┼───────┤        │
 *    │            │ LOW  │       │         LOW            │                 │
 *    │            │ ~2mA │       │         ~2mA           │                 │
 *    │            └──────┘       └────────────────────────┘                 │
 *    │                                                                      │
 *    │    Average power: ~5mA vs ~50mA (10x improvement!)                  │
 *    │                                                                      │
 *    └──────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * SLEEP STATE DIAGRAM
 * ============================================================================
 * 
 *              ┌──────────────────────────────────────────┐
 *              │               RUNNING                     │
 *              │  (Task executing, full power)            │
 *              └─────────────────┬────────────────────────┘
 *                                │
 *                    All tasks blocked/delayed
 *                                │
 *                                ▼
 *              ┌──────────────────────────────────────────┐
 *              │         IDLE TASK RUNNING                 │
 *              │  (Check if can enter tickless)           │
 *              └─────────────────┬────────────────────────┘
 *                                │
 *            Expected idle time >= configEXPECTED_IDLE_TIME_BEFORE_SLEEP?
 *                                │
 *                   ┌────────────┴────────────┐
 *                   │YES                      │NO
 *                   ▼                         ▼
 *    ┌──────────────────────────┐    ┌────────────────────┐
 *    │   TICKLESS SLEEP         │    │   NORMAL IDLE      │
 *    │                          │    │   (WFI, wake on    │
 *    │ • Stop SysTick           │    │    next tick)      │
 *    │ • Program wake timer     │    │                    │
 *    │ • Enter low power mode   │    └────────────────────┘
 *    │ • ZZZ...                 │
 *    │ • Wake up                │
 *    │ • Compensate tick count  │
 *    │ • Restart SysTick        │
 *    └──────────────────────────┘
 * 
 * ============================================================================
 * CONTOH OUTPUT SERIAL
 * ============================================================================
 * 
 *    === TICKLESS IDLE DEMO ===
 *    
 *    [Tick: 0] Worker task started
 *    [Tick: 100] Work complete, sleeping 5000ms...
 *    
 *    [PRE-SLEEP] Expected idle: 4900 ticks
 *    [PRE-SLEEP] Entering low-power mode
 *    
 *    ... MCU dalam sleep mode (hemat daya) ...
 *    
 *    [POST-SLEEP] Woke up, slept ~4900 ticks
 *    
 *    [Tick: 5100] Worker task resumed
 *    [Tick: 5200] Work complete, sleeping 5000ms...
 *    
 *    [Monitor] Sleep statistics:
 *              Total sleep entries: 5
 *              Total sleep time: 24500 ticks
 *              Avg sleep duration: 4900 ticks
 * 
 * ============================================================================
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * GLOBAL VARIABLES
 * ============================================================================ */
UART_HandleTypeDef huart1;

/* Sleep statistics */
static volatile uint32_t ulSleepEntries = 0;
static volatile uint32_t ulTotalSleepTicks = 0;
static volatile uint32_t ulLastExpectedSleep = 0;

/* ============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================ */
static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART1_Init(void);
static void prvWorkerTask(void *pvParameters);
static void prvMonitorTask(void *pvParameters);
static void UART_SendString(const char *str);

/* ============================================================================
 * UART HELPER
 * ============================================================================ */
static void UART_SendString(const char *str)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}

/* ============================================================================
 * MAIN FUNCTION
 * ============================================================================ */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    UART1_Init();
    
    UART_SendString("\r\n\r\n");
    UART_SendString("╔══════════════════════════════════════════════════════════╗\r\n");
    UART_SendString("║               39. TICKLESS IDLE DEMO                     ║\r\n");
    UART_SendString("╚══════════════════════════════════════════════════════════╝\r\n\r\n");
    
    UART_SendString("[Config] configUSE_TICKLESS_IDLE = 1\r\n");
    UART_SendString("[Config] configEXPECTED_IDLE_TIME_BEFORE_SLEEP = 2\r\n");
    UART_SendString("[Config] configPRE_SLEEP_PROCESSING = enabled\r\n");
    UART_SendString("[Config] configPOST_SLEEP_PROCESSING = enabled\r\n\r\n");
    
    UART_SendString("[Info] MCU akan masuk low-power sleep saat idle\r\n");
    UART_SendString("[Info] Perhatikan LED: mati = sleeping, nyala = active\r\n\r\n");
    
    /* Create tasks */
    xTaskCreate(prvWorkerTask, "Worker", TASK_STACK, NULL, 2, NULL);
    xTaskCreate(prvMonitorTask, "Monitor", TASK_STACK, NULL, 1, NULL);
    
    UART_SendString("────────────────────────────────────────────────────────────\r\n\r\n");
    
    vTaskStartScheduler();
    
    while (1);
}

/* ============================================================================
 * WORKER TASK
 * Does periodic work then sleeps for long period
 * ============================================================================ */
static void prvWorkerTask(void *pvParameters)
{
    (void)pvParameters;
    
    char msg[100];
    const TickType_t xWorkTime = pdMS_TO_TICKS(200);    /* Simulate work */
    const TickType_t xSleepTime = pdMS_TO_TICKS(5000);  /* Long sleep */
    
    uint32_t ulWorkCount = 0;
    
    UART_SendString("[Worker] Task started\r\n\r\n");
    
    for (;;)
    {
        ulWorkCount++;
        
        /* LED ON - indicate we're active */
        HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_RESET);
        
        snprintf(msg, sizeof(msg), "[Tick: %lu] Worker cycle #%lu - doing work...\r\n",
                 (unsigned long)xTaskGetTickCount(), ulWorkCount);
        UART_SendString(msg);
        
        /* Simulate some work */
        vTaskDelay(xWorkTime);
        
        snprintf(msg, sizeof(msg), "[Tick: %lu] Work done, sleeping %lu ms\r\n",
                 (unsigned long)xTaskGetTickCount(),
                 (unsigned long)xSleepTime);
        UART_SendString(msg);
        
        /* LED OFF - indicate we're sleeping */
        HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_SET);
        
        UART_SendString("\r\n");
        
        /* Long sleep - this will trigger tickless idle */
        vTaskDelay(xSleepTime);
        
        UART_SendString("\r\n");
    }
}

/* ============================================================================
 * MONITOR TASK
 * Reports sleep statistics periodically
 * ============================================================================ */
static void prvMonitorTask(void *pvParameters)
{
    (void)pvParameters;
    
    char msg[120];
    const TickType_t xPeriod = pdMS_TO_TICKS(15000);  /* Every 15 seconds */
    
    vTaskDelay(pdMS_TO_TICKS(10000));  /* Initial delay */
    
    for (;;)
    {
        UART_SendString("┌─── TICKLESS IDLE STATISTICS ─────────────────────────────┐\r\n");
        
        snprintf(msg, sizeof(msg), "│  Current tick: %lu                                       │\r\n",
                 (unsigned long)xTaskGetTickCount());
        UART_SendString(msg);
        
        snprintf(msg, sizeof(msg), "│  Sleep entries: %lu                                       │\r\n",
                 ulSleepEntries);
        UART_SendString(msg);
        
        snprintf(msg, sizeof(msg), "│  Total sleep time: %lu ticks                              │\r\n",
                 ulTotalSleepTicks);
        UART_SendString(msg);
        
        if (ulSleepEntries > 0)
        {
            snprintf(msg, sizeof(msg), "│  Avg sleep duration: %lu ticks                            │\r\n",
                     ulTotalSleepTicks / ulSleepEntries);
            UART_SendString(msg);
        }
        
        snprintf(msg, sizeof(msg), "│  Last expected sleep: %lu ticks                           │\r\n",
                 ulLastExpectedSleep);
        UART_SendString(msg);
        
        /* Calculate theoretical power saving */
        if (xTaskGetTickCount() > 0)
        {
            uint32_t ulSleepPercent = (ulTotalSleepTicks * 100) / xTaskGetTickCount();
            snprintf(msg, sizeof(msg), "│  Sleep ratio: ~%lu%% of time                               │\r\n",
                     ulSleepPercent);
            UART_SendString(msg);
        }
        
        UART_SendString("└──────────────────────────────────────────────────────────┘\r\n\r\n");
        
        vTaskDelay(xPeriod);
    }
}

/* ============================================================================
 * TICKLESS IDLE HOOKS
 * ============================================================================ */

/**
 * vApplicationPreSleepProcessing
 * 
 * Called BEFORE entering tickless sleep mode.
 * Use this to:
 * - Disable peripherals to save more power
 * - Log sleep entry
 * - Prepare for low power mode
 * 
 * @param ulExpectedIdleTime Expected sleep time in ticks
 */
void vApplicationPreSleepProcessing(uint32_t ulExpectedIdleTime)
{
    char msg[80];
    
    ulSleepEntries++;
    ulLastExpectedSleep = ulExpectedIdleTime;
    ulTotalSleepTicks += ulExpectedIdleTime;
    
    snprintf(msg, sizeof(msg), "  [PRE-SLEEP] Entry #%lu, expected: %lu ticks (~%lu ms)\r\n",
             ulSleepEntries, ulExpectedIdleTime, ulExpectedIdleTime);
    UART_SendString(msg);
    
    /*
     * Di sini bisa tambahkan:
     * - Disable unused GPIO
     * - Stop unused timers
     * - Disable ADC
     * - Switch to lower clock (jika perlu)
     */
    
    /* CATATAN PENTING:
     * Jangan disable UART jika masih perlu logging!
     * Untuk power saving maksimal, disable semua yang tidak perlu.
     */
}

/**
 * vApplicationPostSleepProcessing
 * 
 * Called AFTER waking up from tickless sleep.
 * Use this to:
 * - Re-enable peripherals
 * - Log wake-up
 * - Restore normal operation
 * 
 * @param ulExpectedIdleTime Expected sleep time (same value as pre-sleep)
 */
void vApplicationPostSleepProcessing(uint32_t ulExpectedIdleTime)
{
    char msg[60];
    
    snprintf(msg, sizeof(msg), "  [POST-SLEEP] Woke up (expected %lu ticks)\r\n",
             ulExpectedIdleTime);
    UART_SendString(msg);
    
    /*
     * Di sini bisa tambahkan:
     * - Re-enable peripherals
     * - Restore clocks
     * - Resume any suspended operations
     */
}

/* ============================================================================
 * IDLE HOOK
 * Called from idle task (before tickless check)
 * ============================================================================ */
void vApplicationIdleHook(void)
{
    /* 
     * Idle hook dipanggil dari idle task.
     * Untuk tickless idle, ini dipanggil SEBELUM masuk sleep.
     * 
     * Bisa digunakan untuk:
     * - Blink slow LED
     * - Check battery level
     * - Housekeeping tasks
     * 
     * JANGAN block di sini! Harus return cepat.
     */
    
    /* Toggle idle indicator - jangan terlalu sering karena overhead */
    static uint32_t ulIdleCount = 0;
    ulIdleCount++;
}

/* ============================================================================
 * SYSTEM CONFIGURATION
 * ============================================================================ */
static void SystemClock_Config(void)
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
    
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

static void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = LED_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);
    
    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_SET);
}

static void UART1_Init(void)
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

/* ============================================================================
 * FreeRTOS HOOKS
 * ============================================================================ */
void vApplicationMallocFailedHook(void)
{
    UART_SendString("[FATAL] Malloc failed!\r\n");
    taskDISABLE_INTERRUPTS();
    for (;;);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    char msg[50];
    snprintf(msg, sizeof(msg), "[FATAL] Stack overflow: %s\r\n", pcTaskName);
    UART_SendString(msg);
    taskDISABLE_INTERRUPTS();
    for (;;);
}
