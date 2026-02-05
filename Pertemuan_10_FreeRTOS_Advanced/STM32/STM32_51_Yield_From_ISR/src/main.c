/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 28-Yield_From_ISR
 * 
 * JUDUL: portYIELD_FROM_ISR Demo
 * 
 * DESKRIPSI:
 * Demo yang menunjukkan pentingnya portYIELD_FROM_ISR untuk
 * mendapatkan immediate context switch setelah ISR.
 * 
 * ============================================================================
 * ARSITEKTUR PROGRAM
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                  portYIELD_FROM_ISR DEMO                           │
 *    ├─────────────────────────────────────────────────────────────────────┤
 *    │                                                                     │
 *    │   ┌─────────────────────────────────────────────────────────────┐   │
 *    │   │                      TIM2 ISR                               │   │
 *    │   │                   Period: 500ms                             │   │
 *    │   │                                                             │   │
 *    │   │   void TIM2_IRQHandler(void) {                              │   │
 *    │   │       BaseType_t woken = pdFALSE;                           │   │
 *    │   │                                                             │   │
 *    │   │       // Unblock high priority task                         │   │
 *    │   │       xSemaphoreGiveFromISR(sem, &woken);                   │   │
 *    │   │                                                             │   │
 *    │   │       // CRITICAL: Request context switch!                  │   │
 *    │   │       portYIELD_FROM_ISR(woken);                            │   │
 *    │   │   }                                                         │   │
 *    │   │                                                             │   │
 *    │   └────────────────────────────────┬────────────────────────────┘   │
 *    │                                    │                                │
 *    │                                    │ Semaphore Give                 │
 *    │                                    ▼                                │
 *    │   ┌─────────────────────────────────────────────────────────────┐   │
 *    │   │                   vHighPriorityTask                         │   │
 *    │   │                   Priority: HIGH (3)                        │   │
 *    │   │                                                             │   │
 *    │   │   - Blocked on semaphore                                    │   │
 *    │   │   - Gets unblocked by ISR                                   │   │
 *    │   │   - Runs IMMEDIATELY (with YIELD)                           │   │
 *    │   │   - Measures latency from ISR                               │   │
 *    │   │                                                             │   │
 *    │   └─────────────────────────────────────────────────────────────┘   │
 *    │                                                                     │
 *    │   ┌─────────────────────────────────────────────────────────────┐   │
 *    │   │                   vLowPriorityTask                          │   │
 *    │   │                   Priority: LOW (1)                         │   │
 *    │   │                                                             │   │
 *    │   │   - Busy loop (consuming CPU)                               │   │
 *    │   │   - Should be preempted when high priority unblocks         │   │
 *    │   │                                                             │   │
 *    │   └─────────────────────────────────────────────────────────────┘   │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * EXPECTED OUTPUT
 * ============================================================================
 * 
 *    === Yield From ISR Demo ===
 *    
 *    [0.500s] ISR: Giving semaphore...
 *    [0.500s] HIGH: Got semaphore! Latency: 0 ticks
 *    
 *    [1.000s] ISR: Giving semaphore...
 *    [1.000s] HIGH: Got semaphore! Latency: 0 ticks
 * 
 * ============================================================================
 */

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * GLOBAL VARIABLES
 * ============================================================================ */

static TIM_HandleTypeDef htim2;
static UART_HandleTypeDef huart1;

static SemaphoreHandle_t xSyncSemaphore = NULL;

static volatile TickType_t xISRTimestamp = 0;

/* ============================================================================
 * PROTOTYPE
 * ============================================================================ */

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART_Init(void);
static void TIM2_Init(void);
static void UART_SendString(const char *str);

static void vHighPriorityTask(void *pvParameters);
static void vLowPriorityTask(void *pvParameters);

/* ============================================================================
 * TIMER ISR dengan portYIELD_FROM_ISR
 * ============================================================================ */

void TIM2_IRQHandler(void)
{
    if(__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_UPDATE) != RESET)
    {
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
        
        /* Capture timestamp saat ISR */
        xISRTimestamp = xTaskGetTickCountFromISR();
        
        /* Toggle LED */
        HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
        
        /*
         * STEP 1: Initialize flag
         */
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        
        /*
         * STEP 2: Give semaphore - ini akan unblock high priority task
         * 
         * xHigherPriorityTaskWoken akan di-set pdTRUE karena
         * task yang di-unblock (priority 3) lebih tinggi dari
         * task yang sedang running sebelum ISR (priority 1)
         */
        xSemaphoreGiveFromISR(xSyncSemaphore, &xHigherPriorityTaskWoken);
        
        /*
         * STEP 3: Request context switch jika diperlukan
         * 
         * TANPA ini: ISR kembali ke Low Priority Task
         *           High Priority Task baru jalan di next tick
         *           
         * DENGAN ini: ISR langsung switch ke High Priority Task
         *            Low Priority Task di-preempt
         */
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

/* ============================================================================
 * HIGH PRIORITY TASK
 * ============================================================================ */

static void vHighPriorityTask(void *pvParameters)
{
    (void)pvParameters;
    
    char buffer[80];
    
    UART_SendString("[HIGH] High priority task started (Pri 3)\r\n\r\n");
    
    for(;;)
    {
        /* Block on semaphore */
        if(xSemaphoreTake(xSyncSemaphore, portMAX_DELAY) == pdPASS)
        {
            /* Hitung latency dari ISR */
            TickType_t now = xTaskGetTickCount();
            TickType_t latency = now - xISRTimestamp;
            
            float seconds = (float)now / 1000.0f;
            snprintf(buffer, sizeof(buffer),
                     "[%.3fs] HIGH: Got semaphore! Latency: %lu ticks\r\n\r\n",
                     seconds, latency);
            UART_SendString(buffer);
        }
    }
}

/* ============================================================================
 * LOW PRIORITY TASK
 * ============================================================================ */

static void vLowPriorityTask(void *pvParameters)
{
    (void)pvParameters;
    
    UART_SendString("[LOW] Low priority task started (Pri 1)\r\n");
    UART_SendString("[LOW] Running busy loop...\r\n\r\n");
    
    volatile uint32_t dummy = 0;
    
    for(;;)
    {
        /*
         * Busy loop - consuming CPU
         * 
         * Tanpa portYIELD_FROM_ISR, setelah ISR selesai,
         * CPU akan kembali ke sini, bukan ke high priority task!
         */
        dummy++;
        if(dummy > 100000)
        {
            dummy = 0;
            /* Small yield untuk tidak block scheduler sepenuhnya */
            taskYIELD();
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

static void TIM2_Init(void)
{
    __HAL_RCC_TIM2_CLK_ENABLE();
    
    /* 500ms period */
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 7199;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 4999;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_Base_Init(&htim2);
    
    HAL_NVIC_SetPriority(TIM2_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
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
    TIM2_Init();
    
    UART_SendString("\r\n================================\r\n");
    UART_SendString("28-Yield_From_ISR\r\n");
    UART_SendString("STM32F103 + FreeRTOS\r\n");
    UART_SendString("================================\r\n\r\n");
    UART_SendString("=== portYIELD_FROM_ISR Demo ===\r\n\r\n");
    
    /* Buat binary semaphore */
    xSyncSemaphore = xSemaphoreCreateBinary();
    
    if(xSyncSemaphore == NULL)
    {
        UART_SendString("ERROR: Failed to create semaphore!\r\n");
        while(1);
    }
    
    /* Buat tasks dengan priority berbeda */
    xTaskCreate(vHighPriorityTask, "High", TASK_STACK, NULL,
                tskIDLE_PRIORITY + 3, NULL);  /* High priority */
    
    xTaskCreate(vLowPriorityTask, "Low", TASK_STACK, NULL,
                tskIDLE_PRIORITY + 1, NULL);  /* Low priority */
    
    /* Start timer */
    HAL_TIM_Base_Start_IT(&htim2);
    
    vTaskStartScheduler();
    
    for(;;);
}
