/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 35-Stack_Overflow_Detection
 * 
 * DESKRIPSI:
 * Demo deteksi stack overflow dan monitoring stack high water mark.
 * Menunjukkan cara menggunakan uxTaskGetStackHighWaterMark() untuk
 * menganalisis penggunaan stack dan sizing yang tepat.
 * 
 * ============================================================================
 * ARSITEKTUR SISTEM
 * ============================================================================
 * 
 *    ┌──────────────────────────────────────────────────────────────────────┐
 *    │                  STACK OVERFLOW DETECTION DEMO                       │
 *    │                                                                      │
 *    │   Safe Task            Tight Task            Monitor Task            │
 *    │   ┌──────────────┐     ┌──────────────┐     ┌──────────────┐         │
 *    │   │ Stack: 256W  │     │ Stack: 128W  │     │ Stack: 256W  │         │
 *    │   │              │     │              │     │              │         │
 *    │   │ Light work   │     │ Heavy work   │     │ Report HWM   │         │
 *    │   │ Small buffers│     │ Large buffers│     │ for all tasks│         │
 *    │   │              │     │ (stress test)│     │              │         │
 *    │   └──────────────┘     └──────────────┘     └──────────────┘         │
 *    │                                                                      │
 *    │   HIGH WATER MARK (HWM) = Minimum free stack ever                    │
 *    │                                                                      │
 *    │   ┌────────────────────────────────────────────────────────────────┐ │
 *    │   │         STACK USAGE VISUALIZATION                             │ │
 *    │   │                                                                │ │
 *    │   │   Safe Task:     [▓▓▓▓▓▓▓▓░░░░░░░░░░░░] HWM=48 words (OK)     │ │
 *    │   │   Tight Task:    [▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░] HWM=12 words (LOW!)   │ │
 *    │   │   Monitor Task:  [▓▓▓▓▓▓▓▓▓░░░░░░░░░░░] HWM=38 words (OK)     │ │
 *    │   │                                                                │ │
 *    │   └────────────────────────────────────────────────────────────────┘ │
 *    │                                                                      │
 *    └──────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * STACK SIZING GUIDELINES
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │  RECOMMENDED HWM MARGINS:                                          │
 *    │                                                                     │
 *    │  - Production: HWM > 20% of stack size                             │
 *    │  - Development: HWM > 30% (room for debug code)                    │
 *    │  - Critical tasks: HWM > 40%                                       │
 *    │                                                                     │
 *    │  FACTORS AFFECTING STACK USAGE:                                    │
 *    │  ✓ Local variables (especially arrays)                             │
 *    │  ✓ Function call depth (nested calls)                              │
 *    │  ✓ Interrupt pre-emption                                           │
 *    │  ✓ printf/sprintf (can use 100+ bytes!)                            │
 *    │  ✓ Floating point operations                                       │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * CONTOH OUTPUT SERIAL
 * ============================================================================
 * 
 *    === STACK OVERFLOW DETECTION ===
 *    
 *    [Monitor] Stack High Water Marks (in WORDS):
 *              ┌─────────────┬───────┬───────┬─────────┐
 *              │ Task        │ Total │ HWM   │ Status  │
 *              ├─────────────┼───────┼───────┼─────────┤
 *              │ SafeTask    │ 256   │  48   │ OK      │
 *              │ TightTask   │ 128   │  12   │ WARNING │
 *              │ Monitor     │ 256   │  38   │ OK      │
 *              └─────────────┴───────┴───────┴─────────┘
 *    
 *    [TightTask] Processing with large buffer...
 *    [WARNING] TightTask HWM very low: 8 words!
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

TaskHandle_t xSafeTaskHandle = NULL;
TaskHandle_t xTightTaskHandle = NULL;
TaskHandle_t xMonitorTaskHandle = NULL;

/* ============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================ */
static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART1_Init(void);
static void prvSafeTask(void *pvParameters);
static void prvTightTask(void *pvParameters);
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
    
    char msg[100];
    
    UART_SendString("\r\n\r\n");
    UART_SendString("╔══════════════════════════════════════════════════════════╗\r\n");
    UART_SendString("║        35. STACK OVERFLOW DETECTION DEMO                 ║\r\n");
    UART_SendString("╚══════════════════════════════════════════════════════════╝\r\n\r\n");
    
    snprintf(msg, sizeof(msg), "[Config] Stack overflow method: %d (watermark)\r\n",
             configCHECK_FOR_STACK_OVERFLOW);
    UART_SendString(msg);
    UART_SendString("[Config] SafeTask: 256 words, TightTask: 128 words\r\n\r\n");
    
    /* Create tasks with different stack sizes */
    xTaskCreate(prvSafeTask, "SafeTask", SAFE_TASK_STACK, NULL, 2, &xSafeTaskHandle);
    xTaskCreate(prvTightTask, "TightTask", TIGHT_TASK_STACK, NULL, 2, &xTightTaskHandle);
    xTaskCreate(prvMonitorTask, "Monitor", SAFE_TASK_STACK, NULL, 1, &xMonitorTaskHandle);
    
    UART_SendString("[OK] Tasks created\r\n");
    UART_SendString("────────────────────────────────────────────────────────────\r\n\r\n");
    
    vTaskStartScheduler();
    
    while (1);
}

/* ============================================================================
 * SAFE TASK
 * Uses modest stack - should always be safe
 * ============================================================================ */
static void prvSafeTask(void *pvParameters)
{
    (void)pvParameters;
    
    /* Moderate local variable usage */
    char buffer[32];
    uint32_t ulCounter = 0;
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        ulCounter++;
        
        /* Light processing */
        snprintf(buffer, sizeof(buffer), "Safe #%lu", ulCounter);
        
        /* Occasional activity report */
        if ((ulCounter % 20) == 0)
        {
            UART_SendString("[SafeTask] Running normally...\r\n");
        }
        
        HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));
    }
}

/* ============================================================================
 * TIGHT TASK
 * Uses more stack - closer to the limit
 * ============================================================================ */
static void prvTightTask(void *pvParameters)
{
    (void)pvParameters;
    
    char msg[80];
    uint32_t ulCounter = 0;
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        ulCounter++;
        
        /* Heavier processing that uses more stack */
        {
            /* This block uses additional stack */
            char localBuffer[48];  /* Significant stack usage */
            uint32_t ulSum = 0;
            
            /* Simulate computation */
            for (int i = 0; i < 48; i++)
            {
                localBuffer[i] = (char)(ulCounter + i);
                ulSum += localBuffer[i];
            }
            
            /* Occasional report */
            if ((ulCounter % 30) == 0)
            {
                snprintf(msg, sizeof(msg), "[TightTask] Heavy work done, sum=%lu\r\n", ulSum);
                UART_SendString(msg);
            }
        }
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));
    }
}

/* ============================================================================
 * MONITOR TASK
 * Reports stack high water marks for all tasks
 * ============================================================================ */
static void prvMonitorTask(void *pvParameters)
{
    (void)pvParameters;
    
    char msg[150];
    const TickType_t xPeriod = pdMS_TO_TICKS(5000);
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
        
        UART_SendString("\r\n┌─── STACK HIGH WATER MARKS ──────────────────────────────┐\r\n");
        UART_SendString("│                                                          │\r\n");
        UART_SendString("│  HWM = Minimum free stack ever (in WORDS)                │\r\n");
        UART_SendString("│  Lower HWM = closer to overflow!                         │\r\n");
        UART_SendString("│                                                          │\r\n");
        UART_SendString("├─────────────┬───────────┬──────────┬────────────────────┤\r\n");
        UART_SendString("│ Task        │ Stack Size│ HWM      │ Status             │\r\n");
        UART_SendString("├─────────────┼───────────┼──────────┼────────────────────┤\r\n");
        
        /* Check each task */
        struct {
            TaskHandle_t handle;
            const char *name;
            uint32_t stackSize;
        } tasks[] = {
            {xSafeTaskHandle, "SafeTask", SAFE_TASK_STACK},
            {xTightTaskHandle, "TightTask", TIGHT_TASK_STACK},
            {xMonitorTaskHandle, "Monitor", SAFE_TASK_STACK},
        };
        
        for (int i = 0; i < 3; i++)
        {
            if (tasks[i].handle != NULL)
            {
                /*
                 * uxTaskGetStackHighWaterMark():
                 *   - Returns minimum free stack in WORDS
                 *   - Scans for 0xA5 pattern from stack bottom
                 *   - Lower value = higher risk of overflow
                 */
                UBaseType_t uxHWM = uxTaskGetStackHighWaterMark(tasks[i].handle);
                
                /* Calculate percentage used */
                uint32_t ulUsedPercent = 100 - (uxHWM * 100 / tasks[i].stackSize);
                
                /* Determine status */
                const char *status;
                if (uxHWM < 10)
                    status = "CRITICAL!";
                else if (uxHWM < 20)
                    status = "WARNING";
                else if (uxHWM < tasks[i].stackSize * 20 / 100)
                    status = "LOW";
                else
                    status = "OK";
                
                snprintf(msg, sizeof(msg), 
                         "│ %-11s │ %4lu W    │ %4lu W   │ %s (%lu%% used)    │\r\n",
                         tasks[i].name, 
                         (unsigned long)tasks[i].stackSize,
                         (unsigned long)uxHWM,
                         status,
                         ulUsedPercent);
                UART_SendString(msg);
            }
        }
        
        UART_SendString("└─────────────┴───────────┴──────────┴────────────────────┘\r\n");
        
        /* Visual bar representation */
        UART_SendString("\r\n  Stack Usage Visualization:\r\n");
        
        for (int i = 0; i < 3; i++)
        {
            if (tasks[i].handle != NULL)
            {
                UBaseType_t uxHWM = uxTaskGetStackHighWaterMark(tasks[i].handle);
                uint32_t ulUsedWords = tasks[i].stackSize - uxHWM;
                uint32_t ulUsedBlocks = ulUsedWords * 20 / tasks[i].stackSize;
                
                snprintf(msg, sizeof(msg), "  %-10s [", tasks[i].name);
                UART_SendString(msg);
                
                for (uint32_t b = 0; b < 20; b++)
                {
                    if (b < ulUsedBlocks)
                        UART_SendString("▓");
                    else
                        UART_SendString("░");
                }
                
                snprintf(msg, sizeof(msg), "] HWM=%lu\r\n", (unsigned long)uxHWM);
                UART_SendString(msg);
            }
        }
        
        UART_SendString("\r\n");
    }
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

/*
 * STACK OVERFLOW HOOK
 * 
 * Called when FreeRTOS detects stack overflow.
 * At this point the stack is already corrupted!
 * 
 * Actions to take:
 *   1. Log the error (if possible)
 *   2. Halt the system
 *   3. During development: Increase the task's stack size
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    
    /* CRITICAL: Stack overflow detected! */
    UART_SendString("\r\n");
    UART_SendString("╔══════════════════════════════════════════════════════════╗\r\n");
    UART_SendString("║        !!!  STACK OVERFLOW DETECTED  !!!                 ║\r\n");
    UART_SendString("╚══════════════════════════════════════════════════════════╝\r\n");
    
    char msg[80];
    snprintf(msg, sizeof(msg), "\r\n  Task: %s\r\n", pcTaskName);
    UART_SendString(msg);
    
    UART_SendString("\r\n  Actions to fix:\r\n");
    UART_SendString("  1. Increase task stack size\r\n");
    UART_SendString("  2. Reduce local variable sizes\r\n");
    UART_SendString("  3. Use heap instead of stack for large buffers\r\n");
    UART_SendString("  4. Optimize function call depth\r\n");
    
    /* Halt */
    taskDISABLE_INTERRUPTS();
    for (;;);
}
