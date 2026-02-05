/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 25-Interrupt_Nesting_Priority
 * 
 * JUDUL: Interrupt Nesting dan Priority Demo
 * 
 * DESKRIPSI:
 * Demo yang menunjukkan interrupt nesting menggunakan TIM2 dan TIM3.
 * TIM2 memiliki priority lebih tinggi dan akan preempt TIM3.
 * 
 * ============================================================================
 * ARSITEKTUR PROGRAM
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │               INTERRUPT NESTING DEMO                               │
 *    ├─────────────────────────────────────────────────────────────────────┤
 *    │                                                                     │
 *    │   ┌─────────────────────────────────────────────────────────────┐   │
 *    │   │                      TIM3 ISR                               │   │
 *    │   │                   Priority: 10 (Low)                        │   │
 *    │   │                   Period: 1000ms                            │   │
 *    │   │                                                             │   │
 *    │   │   - Print "TIM3 ISR Start"                                  │   │
 *    │   │   - Do some work (busy loop 10ms)                           │   │
 *    │   │   - Send to queue via FromISR                               │   │
 *    │   │   - Print "TIM3 ISR End"                                    │   │
 *    │   │                                                             │   │
 *    │   │   ┌───────────────────────────────────────────────────────┐ │   │
 *    │   │   │ DAPAT DI-PREEMPT oleh TIM2 ISR (Priority 6)!          │ │   │
 *    │   │   └───────────────────────────────────────────────────────┘ │   │
 *    │   │                                                             │   │
 *    │   └─────────────────────────────────────────────────────────────┘   │
 *    │                                                                     │
 *    │   ┌─────────────────────────────────────────────────────────────┐   │
 *    │   │                      TIM2 ISR                               │   │
 *    │   │                   Priority: 6 (High)                        │   │
 *    │   │                   Period: 500ms                             │   │
 *    │   │                                                             │   │
 *    │   │   - Print "TIM2 ISR" (quick)                                │   │
 *    │   │   - Toggle LED                                              │   │
 *    │   │   - Send notification via FromISR                           │   │
 *    │   │   - Exit cepat                                              │   │
 *    │   │                                                             │   │
 *    │   │   Bisa PREEMPT TIM3!                                        │   │
 *    │   │                                                             │   │
 *    │   └─────────────────────────────────────────────────────────────┘   │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * TIMELINE NESTING
 * ============================================================================
 * 
 *    Time:  0      500     1000    1500    2000    2500 ms
 *           │       │       │       │       │       │
 *    TIM2:  │       ●       │       ●       │       ●   (500ms period)
 *           │       │       │       │       │       │
 *    TIM3:  │       │       ●───────────────●       │   (1000ms period)
 *           │       │       │               │       │
 *                           │               │
 *                           │ TIM3 ISR      │ TIM3 ISR
 *                           │ berjalan      │ berjalan
 *                           │               │
 *                           │◄── TIM2 ──►   │◄── TIM2
 *                           │   preempts!   │   preempts!
 * 
 * ============================================================================
 * EXPECTED OUTPUT
 * ============================================================================
 * 
 *    === Interrupt Nesting Demo ===
 *    
 *    [0.500s] TIM2 ISR (Pri-6) - Quick toggle
 *    [1.000s] TIM3 ISR (Pri-10) Start - Processing...
 *    [1.005s] TIM2 ISR (Pri-6) - Preempted TIM3!    ← TIM2 muncul di tengah TIM3
 *    [1.010s] TIM3 ISR (Pri-10) End - Resume after preemption
 * 
 * ============================================================================
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * GLOBAL VARIABLES
 * ============================================================================ */

static TIM_HandleTypeDef htim2;
static TIM_HandleTypeDef htim3;
static UART_HandleTypeDef huart1;

static QueueHandle_t xEventQueue = NULL;
static TaskHandle_t xHandlerTaskHandle = NULL;

static volatile uint32_t ulTIM2Count = 0;
static volatile uint32_t ulTIM3Count = 0;
static volatile uint8_t ucTIM3Running = 0;  /* Flag TIM3 sedang berjalan */

/* ============================================================================
 * PROTOTYPE
 * ============================================================================ */

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART_Init(void);
static void TIM2_Init(void);
static void TIM3_Init(void);
static void UART_SendString(const char *str);

static void vHandlerTask(void *pvParameters);

/* ============================================================================
 * TIMER ISR HANDLERS
 * ============================================================================ */

/**
 * @brief TIM2 ISR - HIGH Priority (6)
 * 
 * Karena priority lebih tinggi (angka lebih kecil),
 * ISR ini BISA preempt TIM3 ISR yang sedang berjalan!
 */
void TIM2_IRQHandler(void)
{
    if(__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_UPDATE) != RESET)
    {
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
        
        ulTIM2Count++;
        
        /* Toggle LED */
        HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
        
        char buffer[80];
        float seconds = (float)xTaskGetTickCount() / 1000.0f;
        
        if(ucTIM3Running)
        {
            /* TIM3 sedang berjalan - kita preempt! */
            snprintf(buffer, sizeof(buffer),
                     "[%.3fs] TIM2 ISR (Pri-%d) - PREEMPTED TIM3!\r\n",
                     seconds, HIGH_PRIORITY_INT);
        }
        else
        {
            snprintf(buffer, sizeof(buffer),
                     "[%.3fs] TIM2 ISR (Pri-%d) - Quick toggle\r\n",
                     seconds, HIGH_PRIORITY_INT);
        }
        
        /* UART transmit - dalam ISR (untuk demo) */
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 10);
        
        /* Notify handler task */
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        vTaskNotifyGiveFromISR(xHandlerTaskHandle, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

/**
 * @brief TIM3 ISR - LOW Priority (10)
 * 
 * ISR ini sengaja dibuat LAMA untuk demo preemption.
 * TIM2 dengan priority lebih tinggi akan preempt di tengah-tengah.
 */
void TIM3_IRQHandler(void)
{
    if(__HAL_TIM_GET_FLAG(&htim3, TIM_FLAG_UPDATE) != RESET)
    {
        __HAL_TIM_CLEAR_FLAG(&htim3, TIM_FLAG_UPDATE);
        
        ulTIM3Count++;
        ucTIM3Running = 1;  /* Set flag */
        
        char buffer[80];
        float seconds = (float)xTaskGetTickCount() / 1000.0f;
        
        snprintf(buffer, sizeof(buffer),
                 "[%.3fs] TIM3 ISR (Pri-%d) Start - Processing...\r\n",
                 seconds, LOW_PRIORITY_INT);
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 10);
        
        /*
         * Busy loop untuk simulasi processing lama
         * Selama loop ini, TIM2 bisa preempt!
         */
        volatile uint32_t delay = 500000;  /* ~10ms at 72MHz */
        while(delay--);
        
        seconds = (float)xTaskGetTickCount() / 1000.0f;
        snprintf(buffer, sizeof(buffer),
                 "[%.3fs] TIM3 ISR (Pri-%d) End - Done\r\n\r\n",
                 seconds, LOW_PRIORITY_INT);
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 10);
        
        ucTIM3Running = 0;  /* Clear flag */
    }
}

/* ============================================================================
 * HANDLER TASK
 * ============================================================================ */

static void vHandlerTask(void *pvParameters)
{
    (void)pvParameters;
    
    char buffer[50];
    
    for(;;)
    {
        /* Wait for notification from TIM2 ISR */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        
        snprintf(buffer, sizeof(buffer),
                 "[TASK] Got notification #%lu\r\n",
                 ulTIM2Count);
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

/**
 * @brief TIM2 Init - High Priority Timer (500ms)
 */
static void TIM2_Init(void)
{
    __HAL_RCC_TIM2_CLK_ENABLE();
    
    /* 72MHz / 7200 = 10kHz, 5000 counts = 500ms */
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 7199;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 4999;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_Base_Init(&htim2);
    
    /* Set NVIC priority - HIGH (6) */
    HAL_NVIC_SetPriority(TIM2_IRQn, HIGH_PRIORITY_INT, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

/**
 * @brief TIM3 Init - Low Priority Timer (1000ms)
 */
static void TIM3_Init(void)
{
    __HAL_RCC_TIM3_CLK_ENABLE();
    
    /* 72MHz / 7200 = 10kHz, 10000 counts = 1000ms */
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 7199;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 9999;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_Base_Init(&htim3);
    
    /* Set NVIC priority - LOW (10) */
    HAL_NVIC_SetPriority(TIM3_IRQn, LOW_PRIORITY_INT, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
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
    TIM3_Init();
    
    UART_SendString("\r\n================================\r\n");
    UART_SendString("25-Interrupt_Nesting_Priority\r\n");
    UART_SendString("STM32F103 + FreeRTOS\r\n");
    UART_SendString("================================\r\n\r\n");
    UART_SendString("=== Interrupt Nesting Demo ===\r\n\r\n");
    
    char buffer[80];
    snprintf(buffer, sizeof(buffer),
             "TIM2: Period=500ms, Priority=%d (HIGH)\r\n",
             HIGH_PRIORITY_INT);
    UART_SendString(buffer);
    snprintf(buffer, sizeof(buffer),
             "TIM3: Period=1000ms, Priority=%d (LOW)\r\n\r\n",
             LOW_PRIORITY_INT);
    UART_SendString(buffer);
    
    UART_SendString("TIM2 will PREEMPT TIM3 when both run!\r\n\r\n");
    
    /* Buat handler task */
    xTaskCreate(vHandlerTask, "Handler", TASK_STACK, NULL,
                tskIDLE_PRIORITY + 2, &xHandlerTaskHandle);
    
    /* Start timers dengan interrupt enabled */
    HAL_TIM_Base_Start_IT(&htim2);
    HAL_TIM_Base_Start_IT(&htim3);
    
    /* Start scheduler */
    vTaskStartScheduler();
    
    for(;;);
}
