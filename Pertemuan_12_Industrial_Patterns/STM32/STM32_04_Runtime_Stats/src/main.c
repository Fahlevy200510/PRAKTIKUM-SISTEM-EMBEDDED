/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 42-Runtime_Stats
 * 
 * DESKRIPSI:
 * Demonstrasi penggunaan vTaskGetRunTimeStats() untuk melihat berapa persen
 * waktu CPU yang digunakan oleh setiap task.
 * 
 * ============================================================================
 * ARSITEKTUR SISTEM
 * ============================================================================
 * 
 *    ┌──────────────────────────────────────────────────────────────────────┐
 *    │                    RUNTIME STATS DEMO                                │
 *    │                                                                      │
 *    │   ┌────────────────────┐     ┌────────────────────┐                 │
 *    │   │   CPU Intensive    │     │   Light Worker     │                 │
 *    │   │   (Heavy work)     │     │   (Quick work)     │                 │
 *    │   │   Uses ~40-50%     │     │   Uses ~10%        │                 │
 *    │   └────────────────────┘     └────────────────────┘                 │
 *    │                                                                      │
 *    │   ┌────────────────────┐     ┌────────────────────┐                 │
 *    │   │   IO Simulator     │     │   Stats Monitor    │                 │
 *    │   │   (I/O wait)       │     │   (Report stats)   │                 │
 *    │   │   Uses ~15%        │     │   Uses ~5%         │                 │
 *    │   └────────────────────┘     └────────────────────┘                 │
 *    │                                                                      │
 *    │   ┌─────────────────────────────────────────────────────────────┐   │
 *    │   │                    IDLE TASK                                │   │
 *    │   │                    Uses remaining ~20-30%                   │   │
 *    │   └─────────────────────────────────────────────────────────────┘   │
 *    │                                                                      │
 *    │   Runtime Counter: ulHighFrequencyTimerTicks                        │
 *    │   ┌──────────────────────────────────────────────────────────────┐  │
 *    │   │ ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓ │  │
 *    │   │ Increments 10x per tick for higher resolution              │  │
 *    │   └──────────────────────────────────────────────────────────────┘  │
 *    │                                                                      │
 *    └──────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * CONTOH OUTPUT SERIAL
 * ============================================================================
 * 
 *    === RUNTIME STATS DEMO ===
 *    
 *    [CPU] Heavy computation...
 *    [Light] Quick work done
 *    [IO] Simulating I/O wait...
 *    
 *    ┌─── RUNTIME STATISTICS ─────────────────────────────────────────────┐
 *    │ Task            Abs Time        % Time                             │
 *    │ ─────────────────────────────────────────                          │
 *    │ CPUTask         12500034        42%                                │
 *    │ LightTask       2981234         10%                                │
 *    │ IOTask          4462345         15%                                │
 *    │ StatsTask       1487823         5%                                 │
 *    │ IDLE            8023456         27%                                │
 *    │ Tmr Svc         148723          1%                                 │
 *    └────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * RUNTIME COUNTER
 * ============================================================================ */
/* 
 * High frequency counter for runtime stats.
 * Incremented in tick hook (10x per tick for better resolution).
 */
volatile uint32_t ulHighFrequencyTimerTicks = 0UL;

/* ============================================================================
 * GLOBAL VARIABLES
 * ============================================================================ */
UART_HandleTypeDef huart1;

/* Stats buffer */
static char pcStatsBuffer[STATS_BUFFER_SIZE];

/* ============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================ */
static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART1_Init(void);
static void prvCPUIntensiveTask(void *pvParameters);
static void prvLightTask(void *pvParameters);
static void prvIOTask(void *pvParameters);
static void prvStatsTask(void *pvParameters);
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
    UART_SendString("║            42. RUNTIME STATISTICS DEMO                   ║\r\n");
    UART_SendString("╚══════════════════════════════════════════════════════════╝\r\n\r\n");
    
    UART_SendString("[Info] configGENERATE_RUN_TIME_STATS = 1\r\n");
    UART_SendString("[Info] Using tick hook for high-res counter\r\n");
    UART_SendString("[Info] Stats reported every 5 seconds\r\n\r\n");
    
    /* Create tasks with different CPU loads */
    xTaskCreate(prvCPUIntensiveTask, "CPUTask", TASK_STACK, NULL, 2, NULL);
    xTaskCreate(prvLightTask, "LightTask", TASK_STACK, NULL, 1, NULL);
    xTaskCreate(prvIOTask, "IOTask", TASK_STACK, NULL, 1, NULL);
    xTaskCreate(prvStatsTask, "StatsTask", TASK_STACK + 128, NULL, 3, NULL);
    
    UART_SendString("────────────────────────────────────────────────────────────\r\n\r\n");
    
    vTaskStartScheduler();
    
    while (1);
}

/* ============================================================================
 * CPU INTENSIVE TASK
 * Uses significant CPU time doing calculations
 * ============================================================================ */
static void prvCPUIntensiveTask(void *pvParameters)
{
    (void)pvParameters;
    
    volatile uint32_t ulDummy = 0;
    uint32_t ulCycle = 0;
    char msg[60];
    
    for (;;)
    {
        ulCycle++;
        
        /* CPU intensive work - takes significant time */
        for (volatile uint32_t i = 0; i < 50000; i++)
        {
            ulDummy += i;
            ulDummy ^= (i << 4);
        }
        
        if (ulCycle % 10 == 0)
        {
            snprintf(msg, sizeof(msg), "[CPU] Heavy computation cycle %lu\r\n", ulCycle);
            UART_SendString(msg);
        }
        
        HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
        
        /* Small delay to prevent starving other tasks */
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/* ============================================================================
 * LIGHT TASK
 * Does minimal work, mostly sleeping
 * ============================================================================ */
static void prvLightTask(void *pvParameters)
{
    (void)pvParameters;
    
    uint32_t ulCycle = 0;
    char msg[60];
    
    for (;;)
    {
        ulCycle++;
        
        /* Light work */
        for (volatile uint32_t i = 0; i < 1000; i++)
        {
            /* Minimal computation */
        }
        
        if (ulCycle % 20 == 0)
        {
            snprintf(msg, sizeof(msg), "[Light] Quick work cycle %lu\r\n", ulCycle);
            UART_SendString(msg);
        }
        
        /* Mostly sleeping */
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* ============================================================================
 * IO TASK
 * Simulates I/O bound work (mostly waiting)
 * ============================================================================ */
static void prvIOTask(void *pvParameters)
{
    (void)pvParameters;
    
    uint32_t ulCycle = 0;
    char msg[60];
    
    for (;;)
    {
        ulCycle++;
        
        /* Simulate I/O operation with some CPU work */
        for (volatile uint32_t i = 0; i < 10000; i++)
        {
            /* Simulating data processing */
        }
        
        if (ulCycle % 10 == 0)
        {
            snprintf(msg, sizeof(msg), "[IO] I/O operation cycle %lu\r\n", ulCycle);
            UART_SendString(msg);
        }
        
        /* Wait for "I/O completion" */
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

/* ============================================================================
 * STATS TASK
 * Reports runtime statistics periodically
 * ============================================================================ */
static void prvStatsTask(void *pvParameters)
{
    (void)pvParameters;
    
    char msg[80];
    const TickType_t xPeriod = pdMS_TO_TICKS(5000);
    uint32_t ulReportCount = 0;
    
    /* Wait for tasks to run a bit */
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    for (;;)
    {
        ulReportCount++;
        
        UART_SendString("\r\n");
        UART_SendString("┌─── RUNTIME STATISTICS ─────────────────────────────────────┐\r\n");
        
        snprintf(msg, sizeof(msg), "│  Report #%lu at tick %lu                                    │\r\n",
                 ulReportCount, (unsigned long)xTaskGetTickCount());
        UART_SendString(msg);
        
        snprintf(msg, sizeof(msg), "│  Runtime counter: %lu                                      │\r\n",
                 ulHighFrequencyTimerTicks);
        UART_SendString(msg);
        
        UART_SendString("│                                                             │\r\n");
        UART_SendString("│  Task            Abs Time           %% Time                 │\r\n");
        UART_SendString("│  ─────────────────────────────────────────                  │\r\n");
        
        /* Get and print runtime stats */
        vTaskGetRunTimeStats(pcStatsBuffer);
        
        /* Parse and print each line */
        char *pcLine = strtok(pcStatsBuffer, "\r\n");
        while (pcLine != NULL)
        {
            UART_SendString("│  ");
            UART_SendString(pcLine);
            UART_SendString("\r\n");
            pcLine = strtok(NULL, "\r\n");
        }
        
        UART_SendString("└─────────────────────────────────────────────────────────────┘\r\n");
        
        /* Also show number of tasks */
        snprintf(msg, sizeof(msg), "\n[Info] Total tasks: %lu\r\n\r\n",
                 (unsigned long)uxTaskGetNumberOfTasks());
        UART_SendString(msg);
        
        vTaskDelay(xPeriod);
    }
}

/* ============================================================================
 * TICK HOOK
 * Increments high-resolution counter for runtime stats
 * ============================================================================ */
void vApplicationTickHook(void)
{
    /* 
     * Increment counter multiple times per tick for higher resolution.
     * In real applications, you would use a hardware timer for this.
     * We use 10x increment to simulate 10KHz resolution with 1KHz tick.
     */
    ulHighFrequencyTimerTicks += 10;
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
