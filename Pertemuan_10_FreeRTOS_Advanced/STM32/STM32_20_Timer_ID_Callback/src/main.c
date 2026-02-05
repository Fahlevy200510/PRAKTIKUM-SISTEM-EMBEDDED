/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 22-Timer_ID_Callback
 * 
 * JUDUL: Timer ID dan Shared Callback Demo
 * 
 * DESKRIPSI:
 * Demo penggunaan Timer ID untuk:
 * 1. Mengidentifikasi timer di shared callback
 * 2. Menyimpan context data per-timer
 * 3. Dynamic update Timer ID runtime
 * 
 * ============================================================================
 * ARSITEKTUR PROGRAM
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                   TIMER ID CALLBACK DEMO                           │
 *    ├─────────────────────────────────────────────────────────────────────┤
 *    │                                                                     │
 *    │   ┌───────────────────────────────────────────────────────────────┐ │
 *    │   │                    TIMER CONTEXTS                             │ │
 *    │   │                                                               │ │
 *    │   │   TimerCtx[0]          TimerCtx[1]          TimerCtx[2]       │ │
 *    │   │   ┌─────────┐          ┌─────────┐          ┌─────────┐       │ │
 *    │   │   │name:"A" │          │name:"B" │          │name:"C" │       │ │
 *    │   │   │period:500│          │period:800│          │period:1200│     │ │
 *    │   │   │count:0  │          │count:0  │          │count:0  │       │ │
 *    │   │   └────┬────┘          └────┬────┘          └────┬────┘       │ │
 *    │   │        │                    │                    │            │ │
 *    │   │        ▼                    ▼                    ▼            │ │
 *    │   │   ┌─────────┐          ┌─────────┐          ┌─────────┐       │ │
 *    │   │   │Timer A  │          │Timer B  │          │Timer C  │       │ │
 *    │   │   │ID=&ctx[0]│          │ID=&ctx[1]│          │ID=&ctx[2]│      │ │
 *    │   │   └────┬────┘          └────┬────┘          └────┬────┘       │ │
 *    │   │        │                    │                    │            │ │
 *    │   └────────┼────────────────────┼────────────────────┼────────────┘ │
 *    │            │                    │                    │              │
 *    │            └────────────────────┼────────────────────┘              │
 *    │                                 │                                   │
 *    │                                 ▼                                   │
 *    │                    ┌────────────────────────┐                       │
 *    │                    │  vSharedTimerCallback  │                       │
 *    │                    │                        │                       │
 *    │                    │  ctx = pvTimerGetID()  │                       │
 *    │                    │  ctx->count++          │                       │
 *    │                    │  print(ctx->name)      │                       │
 *    │                    └────────────────────────┘                       │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * EXPECTED OUTPUT
 * ============================================================================
 * 
 *    === Timer ID Callback Demo ===
 *    
 *    [0.500s] Timer A triggered! Count: 1, Period: 500ms
 *    [0.800s] Timer B triggered! Count: 1, Period: 800ms
 *    [1.000s] Timer A triggered! Count: 2, Period: 500ms
 *    [1.200s] Timer C triggered! Count: 1, Period: 1200ms
 *    [1.500s] Timer A triggered! Count: 3, Period: 500ms
 *    [1.600s] Timer B triggered! Count: 2, Period: 800ms
 *    ...
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
 * TIMER CONTEXT STRUCTURE
 * ============================================================================ */

/**
 * @brief Struktur yang menyimpan context per-timer
 * 
 * KEUNTUNGAN menggunakan context struct:
 * - Setiap timer bisa punya state sendiri
 * - Satu callback untuk multiple timers
 * - Mudah extend tanpa ubah callback signature
 */
typedef struct {
    const char *name;       /* Nama timer untuk display */
    uint32_t    period_ms;  /* Period dalam ms */
    uint32_t    count;      /* Counter trigger */
    uint8_t     toggle;     /* State untuk toggle */
} TimerContext_t;

/* ============================================================================
 * GLOBAL VARIABLES
 * ============================================================================ */

static TimerHandle_t xTimers[NUM_TIMERS];

/* Array of timer contexts */
static TimerContext_t xTimerContexts[NUM_TIMERS] = {
    { .name = "A", .period_ms = 500,  .count = 0, .toggle = 0 },
    { .name = "B", .period_ms = 800,  .count = 0, .toggle = 0 },
    { .name = "C", .period_ms = 1200, .count = 0, .toggle = 0 }
};

static UART_HandleTypeDef huart1;

/* ============================================================================
 * PROTOTYPE
 * ============================================================================ */

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART_Init(void);
static void UART_SendString(const char *str);

static void vSharedTimerCallback(TimerHandle_t xTimer);

/* ============================================================================
 * SHARED TIMER CALLBACK
 * ============================================================================ */

/**
 * @brief Callback yang digunakan oleh semua timer
 * 
 * ILUSTRASI PENGGUNAAN pvTimerGetTimerID():
 * ═══════════════════════════════════════════════════════════════════════════
 * 
 *    Timer A (ID = &ctx[0])          Timer B (ID = &ctx[1])
 *    ┌───────────────────┐           ┌───────────────────┐
 *    │ xTimerCreate()    │           │ xTimerCreate()    │
 *    │ ...               │           │ ...               │
 *    │ ID = &ctx[0]      │           │ ID = &ctx[1]      │
 *    │ cb = vSharedCB    │           │ cb = vSharedCB    │
 *    └─────────┬─────────┘           └─────────┬─────────┘
 *              │                               │
 *              │ 500ms expires                 │ 800ms expires
 *              ▼                               ▼
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                      vSharedTimerCallback(xTimer)                   │
 *    │                                                                     │
 *    │   // xTimer bisa Timer A ATAU Timer B                              │
 *    │   // Gunakan ID untuk bedakan!                                     │
 *    │                                                                     │
 *    │   TimerContext_t *ctx = (TimerContext_t*)pvTimerGetTimerID(xTimer);│
 *    │   //                                                               │
 *    │   // Jika dipanggil dari Timer A:                                  │
 *    │   //   ctx = &ctx[0] → name="A", period=500                        │
 *    │   //                                                               │
 *    │   // Jika dipanggil dari Timer B:                                  │
 *    │   //   ctx = &ctx[1] → name="B", period=800                        │
 *    │                                                                     │
 *    │   ctx->count++;                                                     │
 *    │   printf("Timer %s count: %d\n", ctx->name, ctx->count);           │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ═══════════════════════════════════════════════════════════════════════════
 */
static void vSharedTimerCallback(TimerHandle_t xTimer)
{
    /*
     * Dapatkan Timer ID - ini adalah pointer ke TimerContext_t
     * yang kita set saat xTimerCreate()
     */
    TimerContext_t *pxContext = (TimerContext_t*)pvTimerGetTimerID(xTimer);
    
    if(pxContext != NULL)
    {
        /* Increment counter */
        pxContext->count++;
        
        /* Toggle LED (hanya untuk timer pertama) */
        if(pxContext == &xTimerContexts[0])
        {
            HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
        }
        
        /* Print info */
        char buffer[80];
        float seconds = (float)xTaskGetTickCount() / 1000.0f;
        
        snprintf(buffer, sizeof(buffer),
                 "[%.3fs] Timer %s triggered! Count: %lu, Period: %lums\r\n",
                 seconds,
                 pxContext->name,
                 pxContext->count,
                 pxContext->period_ms);
        UART_SendString(buffer);
    }
}

/* ============================================================================
 * KONFIGURASI HARDWARE
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
    
    UART_SendString("\r\n================================\r\n");
    UART_SendString("22-Timer_ID_Callback\r\n");
    UART_SendString("STM32F103 + FreeRTOS\r\n");
    UART_SendString("================================\r\n\r\n");
    UART_SendString("=== Timer ID Callback Demo ===\r\n\r\n");
    
    /*
     * Buat multiple timer dengan SHARED CALLBACK
     * Timer ID diset ke pointer ke context masing-masing
     */
    for(int i = 0; i < NUM_TIMERS; i++)
    {
        char timerName[10];
        snprintf(timerName, sizeof(timerName), "Timer%s", xTimerContexts[i].name);
        
        xTimers[i] = xTimerCreate(
            timerName,                                    /* Name */
            pdMS_TO_TICKS(xTimerContexts[i].period_ms),  /* Period from context */
            pdTRUE,                                       /* Auto-reload */
            (void*)&xTimerContexts[i],                   /* Timer ID = pointer ke context */
            vSharedTimerCallback                         /* SHARED callback! */
        );
        
        if(xTimers[i] == NULL)
        {
            UART_SendString("ERROR: Failed to create timer!\r\n");
            while(1);
        }
        
        char buffer[50];
        snprintf(buffer, sizeof(buffer),
                 "Created Timer %s (period: %lums)\r\n",
                 xTimerContexts[i].name,
                 xTimerContexts[i].period_ms);
        UART_SendString(buffer);
    }
    
    UART_SendString("\r\nStarting all timers...\r\n\r\n");
    
    /* Start semua timer */
    for(int i = 0; i < NUM_TIMERS; i++)
    {
        xTimerStart(xTimers[i], 0);
    }
    
    /* Start scheduler */
    vTaskStartScheduler();
    
    for(;;);
}
