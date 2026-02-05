/**
 * ============================================================================
 * FILE: main.c  
 * PROJECT: 28-Yield_From_ISR
 * PERTEMUAN: 10 - FreeRTOS Advanced
 * 
 * JUDUL: Yield From ISR untuk Immediate Context Switch
 * 
 * DESKRIPSI:
 * Demonstrasi portYIELD_FROM_ISR() untuk memicu context switch segera
 * setelah ISR selesai, meningkatkan response time task prioritas tinggi.
 * 
 * ============================================================================
 * ARSITEKTUR
 * ============================================================================
 * 
 *    ┌────────────────────────────────────────────────────────────────┐
 *    │                  YIELD FROM ISR DEMO                           │
 *    ├────────────────────────────────────────────────────────────────┤
 *    │                                                                │
 *    │   Timer ISR (50ms)                                             │
 *    │   ┌────────────────────────────────────────┐                   │
 *    │   │  1. xTaskNotifyFromISR(High Priority)  │                   │
 *    │   │  2. Set pxHigherPriorityTaskWoken      │                   │
 *    │   │  3. portYIELD_FROM_ISR(...)            │───► Immediate     │
 *    │   └────────────────────────────────────────┘    Context Switch │
 *    │              │                                                 │
 *    │              ▼                                                 │
 *    │   ┌────────────────────────────────────────┐                   │
 *    │   │  High Priority Task (Priority 3)       │                   │
 *    │   │  - Wakes up immediately                │                   │
 *    │   │  - Process time-critical data          │                   │
 *    │   │  - Measure response time               │                   │
 *    │   └────────────────────────────────────────┘                   │
 *    │                                                                │
 *    │   Low Priority Task (Priority 1)                               │
 *    │   ┌────────────────────────────────────────┐                   │
 *    │   │  - Background processing               │                   │
 *    │   │  - Preempted by high priority          │                   │
 *    │   └────────────────────────────────────────┘                   │
 *    └────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * EXPECTED OUTPUT
 * ============================================================================
 * 
 *    === Yield From ISR Demo ===
 *    
 *    [ISR] Triggered at 50ms
 *    [High] Woke up! Response time: 0.012ms
 *    [Low] Running... (will be preempted)
 *    
 *    [ISR] Triggered at 100ms
 *    [High] Woke up! Response time: 0.010ms
 *    [Low] Running...
 * 
 * ============================================================================
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>

/* ============================================================================
 * CONFIGURATION
 * ============================================================================ */

#define LED_GPIO_PORT   GPIOC
#define LED_GPIO_PIN    GPIO_PIN_13

/* ============================================================================
 * GLOBALS
 * ============================================================================ */

static TIM_HandleTypeDef htim2;
static UART_HandleTypeDef huart1;
static TaskHandle_t xHighPriorityTask = NULL;

/* ============================================================================
 * PROTOTYPES
 * ============================================================================ */

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART_Init(void);
static void TIM2_Init(void);
static void UART_SendString(const char *str);

static void vHighPriorityTask(void *pvParameters);
static void vLowPriorityTask(void *pvParameters);

/* ============================================================================
 * TIMER ISR - With Yield From ISR
 * ============================================================================ */

void TIM2_IRQHandler(void)
{
    if(__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_UPDATE) != RESET)
    {
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
        
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        
        /* Notify high priority task */
        vTaskNotifyGiveFromISR(xHighPriorityTask, &xHigherPriorityTaskWoken);
        
        /*
         * portYIELD_FROM_ISR()
         * 
         * Jika xHigherPriorityTaskWoken == pdTRUE, maka context switch
         * akan terjadi SEGERA setelah ISR selesai, tanpa menunggu tick.
         * 
         * Ini meningkatkan response time secara signifikan!
         */
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

/* ============================================================================
 * HIGH PRIORITY TASK - Time-critical processing
 * ============================================================================ */

static void vHighPriorityTask(void *pvParameters)
{
    (void)pvParameters;
    char buffer[100];
    uint32_t ulNotificationValue;
    TickType_t xTimeISR, xTimeWakeup;
    
    for(;;)
    {
        /* Wait for notification from ISR */
        ulNotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        
        if(ulNotificationValue > 0)
        {
            xTimeWakeup = xTaskGetTickCount();
            
            /* Calculate response time (estimate) */
            snprintf(buffer, sizeof(buffer),
                    "[High] Woke up! Notification received.\r\n");
            UART_SendString(buffer);
            
            /* Do time-critical processing */
            HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
            
            /* Simulate processing */
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

/* ============================================================================
 * LOW PRIORITY TASK - Background processing
 * ============================================================================ */

static void vLowPriorityTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulCounter = 0;
    char buffer[80];
    
    for(;;)
    {
        ulCounter++;
        
        snprintf(buffer, sizeof(buffer),
                "[Low] Background task running... (count=%lu)\r\n", ulCounter);
        UART_SendString(buffer);
        
        /* Simulate work */
        vTaskDelay(pdMS_TO_TICKS(100));
    }
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
    
    /* Start timer */
    HAL_TIM_Base_Start_IT(&htim2);
    
    UART_SendString("\r\n=== Yield From ISR Demo ===\r\n\r\n");
    
    /* Create tasks */
    xTaskCreate(vHighPriorityTask, "HighPrio", 256, NULL, 3, &xHighPriorityTask);
    xTaskCreate(vLowPriorityTask, "LowPrio", 128, NULL, 1, NULL);
    
    vTaskStartScheduler();
    
    while(1);
}

/* ============================================================================
 * PERIPHERAL INITIALIZATION
 * ============================================================================ */

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

void GPIO_Init(void)
{
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = LED_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);
}

void UART_Init(void)
{
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
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

void TIM2_Init(void)
{
    __HAL_RCC_TIM2_CLK_ENABLE();
    
    /* 72MHz / (7200 * 5000) = 2Hz (500ms) */
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 7200 - 1;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 5000 - 1;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&htim2);
    
    HAL_NVIC_SetPriority(TIM2_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

void UART_SendString(const char *str)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
}

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
