/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 21-Autoreload_Timer
 * 
 * JUDUL: Auto-Reload Software Timer Demo
 * 
 * DESKRIPSI:
 * Demo penggunaan 3 auto-reload timer dengan period berbeda.
 * Setiap timer memiliki callback yang dipanggil secara periodik.
 * 
 * ============================================================================
 * ARSITEKTUR PROGRAM
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                   AUTO-RELOAD TIMER DEMO                           │
 *    ├─────────────────────────────────────────────────────────────────────┤
 *    │                                                                     │
 *    │   ┌─────────────────────────────────────────────────────────────┐   │
 *    │   │                 TIMER SERVICE TASK                          │   │
 *    │   │              (Created by FreeRTOS)                          │   │
 *    │   └───────────────────────┬─────────────────────────────────────┘   │
 *    │                           │                                         │
 *    │           ┌───────────────┼───────────────┐                         │
 *    │           ▼               ▼               ▼                         │
 *    │   ┌─────────────┐ ┌─────────────┐ ┌─────────────┐                   │
 *    │   │ Heartbeat   │ │  Sensor     │ │  Stats      │                   │
 *    │   │ Timer       │ │  Timer      │ │  Timer      │                   │
 *    │   │ Period:500ms│ │ Period:2s   │ │ Period:5s   │                   │
 *    │   └──────┬──────┘ └──────┬──────┘ └──────┬──────┘                   │
 *    │          │               │               │                          │
 *    │          ▼               ▼               ▼                          │
 *    │   ┌─────────────┐ ┌─────────────┐ ┌─────────────┐                   │
 *    │   │ vHeartbeat  │ │ vSensor     │ │ vStats      │                   │
 *    │   │ Callback()  │ │ Callback()  │ │ Callback()  │                   │
 *    │   │             │ │             │ │             │                   │
 *    │   │ - Toggle LED│ │ - Read ADC  │ │ - Print     │                   │
 *    │   │ - Print beat│ │ - Print temp│ │   statistics│                   │
 *    │   └─────────────┘ └─────────────┘ └─────────────┘                   │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * TIMELINE EKSEKUSI
 * ============================================================================
 * 
 *    Time(s):  0    0.5    1    1.5    2    2.5    3    3.5    4    4.5    5
 *              │     │     │     │     │     │     │     │     │     │     │
 *    Heartbeat ●─────●─────●─────●─────●─────●─────●─────●─────●─────●─────●
 *    (500ms)   │     │     │     │     │     │     │     │     │     │     │
 *              │     │     │     │     │     │     │     │     │     │     │
 *    Sensor    ●─────────────────●─────────────────●─────────────────●─────
 *    (2000ms)  │                 │                 │                 │
 *              │                 │                 │                 │
 *    Stats     ●─────────────────────────────────────────────────────●─────
 *    (5000ms)  │                                                     │
 * 
 * ============================================================================
 * EXPECTED OUTPUT
 * ============================================================================
 * 
 *    === Auto-Reload Timer Demo ===
 *    
 *    [0.500s] Heartbeat #1
 *    [1.000s] Heartbeat #2
 *    [1.500s] Heartbeat #3
 *    [2.000s] Heartbeat #4
 *    [2.000s] Sensor reading: 25.3C
 *    [2.500s] Heartbeat #5
 *    [3.000s] Heartbeat #6
 *    ...
 *    [5.000s] === Statistics ===
 *             Heartbeat count: 10
 *             Sensor count: 2
 *             Uptime: 5.0s
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
 * HANDLE GLOBAL
 * ============================================================================ */

static TimerHandle_t xHeartbeatTimer = NULL;
static TimerHandle_t xSensorTimer = NULL;
static TimerHandle_t xStatsTimer = NULL;

static UART_HandleTypeDef huart1;
static ADC_HandleTypeDef hadc1;

/* Statistik */
static volatile uint32_t ulHeartbeatCount = 0;
static volatile uint32_t ulSensorCount = 0;

/* ============================================================================
 * PROTOTYPE
 * ============================================================================ */

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART_Init(void);
static void ADC_Init(void);
static void UART_SendString(const char *str);

static void vHeartbeatCallback(TimerHandle_t xTimer);
static void vSensorCallback(TimerHandle_t xTimer);
static void vStatsCallback(TimerHandle_t xTimer);

/* ============================================================================
 * TIMER CALLBACKS
 * ============================================================================ */

/**
 * @brief Heartbeat Timer Callback - Dipanggil setiap 500ms
 * 
 * CATATAN PENTING tentang Timer Callback:
 * ═══════════════════════════════════════════════════════════════════════════
 * 
 *    Timer callbacks berjalan di TIMER SERVICE TASK context!
 *    
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │ TIMER SERVICE TASK                                                  │
 *    │ ┌───────────────────────────────────────────────────────────────┐   │
 *    │ │ while(1) {                                                    │   │
 *    │ │     // Block sampai timer expired atau command received       │   │
 *    │ │     xQueueReceive(timerQueue, ...);                           │   │
 *    │ │                                                               │   │
 *    │ │     // Panggil callback DALAM context task ini                │   │
 *    │ │     timer->pxCallbackFunction(timer);                         │   │
 *    │ │     ↑                                                         │   │
 *    │ │     Callback berjalan di sini, bukan di interrupt!            │   │
 *    │ │ }                                                             │   │
 *    │ └───────────────────────────────────────────────────────────────┘   │
 *    └─────────────────────────────────────────────────────────────────────┘
 *    
 *    IMPLIKASI:
 *    - Callback BISA memanggil blocking API (xQueueSend, dll)
 *    - Callback TIDAK BOLEH terlalu lama (block timer lain)
 *    - Prioritas callback = prioritas Timer Service Task
 * 
 * ═══════════════════════════════════════════════════════════════════════════
 */
static void vHeartbeatCallback(TimerHandle_t xTimer)
{
    (void)xTimer;
    
    ulHeartbeatCount++;
    
    /* Toggle LED */
    HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
    
    char buffer[50];
    float seconds = (float)xTaskGetTickCount() / 1000.0f;
    snprintf(buffer, sizeof(buffer), 
             "[%.3fs] Heartbeat #%lu\r\n", 
             seconds, ulHeartbeatCount);
    UART_SendString(buffer);
}

/**
 * @brief Sensor Timer Callback - Dipanggil setiap 2000ms
 * 
 * Simulasi pembacaan sensor dengan ADC.
 */
static void vSensorCallback(TimerHandle_t xTimer)
{
    (void)xTimer;
    
    ulSensorCount++;
    
    /* Baca ADC */
    HAL_ADC_Start(&hadc1);
    if(HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
    {
        uint32_t adcValue = HAL_ADC_GetValue(&hadc1);
        /* Convert ke "temperature" (simulasi) */
        float temp = 20.0f + (adcValue * 15.0f / 4095.0f);
        
        char buffer[60];
        float seconds = (float)xTaskGetTickCount() / 1000.0f;
        snprintf(buffer, sizeof(buffer),
                 "[%.3fs] Sensor reading #%lu: %.1fC\r\n",
                 seconds, ulSensorCount, temp);
        UART_SendString(buffer);
    }
    HAL_ADC_Stop(&hadc1);
}

/**
 * @brief Stats Timer Callback - Dipanggil setiap 5000ms
 * 
 * Mencetak ringkasan statistik sistem.
 */
static void vStatsCallback(TimerHandle_t xTimer)
{
    (void)xTimer;
    
    float uptime = (float)xTaskGetTickCount() / 1000.0f;
    
    char buffer[150];
    snprintf(buffer, sizeof(buffer),
             "\r\n[%.3fs] === STATISTICS ===\r\n"
             "  Heartbeat count: %lu\r\n"
             "  Sensor count: %lu\r\n"
             "  Uptime: %.1f seconds\r\n"
             "=======================\r\n\r\n",
             uptime, ulHeartbeatCount, ulSensorCount, uptime);
    UART_SendString(buffer);
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
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
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
    UART_SendString("21-Autoreload_Timer\r\n");
    UART_SendString("STM32F103 + FreeRTOS\r\n");
    UART_SendString("================================\r\n\r\n");
    UART_SendString("=== Auto-Reload Timer Demo ===\r\n\r\n");
    UART_SendString("Timers:\r\n");
    UART_SendString("  - Heartbeat: 500ms (toggle LED)\r\n");
    UART_SendString("  - Sensor: 2000ms (read ADC)\r\n");
    UART_SendString("  - Stats: 5000ms (print statistics)\r\n\r\n");
    
    /*
     * Buat Auto-Reload Timers
     * 
     * xTimerCreate parameters:
     * 1. pcTimerName - nama untuk debug
     * 2. xTimerPeriod - period dalam ticks
     * 3. uxAutoReload - pdTRUE = auto-reload, pdFALSE = one-shot
     * 4. pvTimerID - ID untuk identifikasi (bisa pointer ke data)
     * 5. pxCallbackFunction - fungsi callback
     */
    
    xHeartbeatTimer = xTimerCreate(
        "Heartbeat",                        /* Name */
        pdMS_TO_TICKS(HEARTBEAT_PERIOD_MS), /* Period */
        pdTRUE,                             /* Auto-reload = YES */
        (void*)0,                           /* Timer ID */
        vHeartbeatCallback                  /* Callback */
    );
    
    xSensorTimer = xTimerCreate(
        "Sensor",
        pdMS_TO_TICKS(SENSOR_PERIOD_MS),
        pdTRUE,                             /* Auto-reload = YES */
        (void*)1,
        vSensorCallback
    );
    
    xStatsTimer = xTimerCreate(
        "Stats",
        pdMS_TO_TICKS(STATS_PERIOD_MS),
        pdTRUE,                             /* Auto-reload = YES */
        (void*)2,
        vStatsCallback
    );
    
    if(xHeartbeatTimer == NULL || xSensorTimer == NULL || xStatsTimer == NULL)
    {
        UART_SendString("ERROR: Failed to create timers!\r\n");
        while(1);
    }
    
    /*
     * Start semua timer
     * 
     * CATATAN: Timer harus di-start setelah scheduler berjalan,
     * KECUALI jika kita menggunakan block time = 0 (tidak block)
     * Karena scheduler belum jalan, gunakan timeout = 0
     */
    xTimerStart(xHeartbeatTimer, 0);
    xTimerStart(xSensorTimer, 0);
    xTimerStart(xStatsTimer, 0);
    
    UART_SendString("All timers started!\r\n\r\n");
    
    /* Start scheduler - timer service task akan dibuat otomatis */
    vTaskStartScheduler();
    
    for(;;);
}
