/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 27-Critical_Section_ISR
 * PERTEMUAN: 10 - FreeRTOS Advanced
 * 
 * JUDUL: Critical Section untuk Protect Shared Data (Task & ISR)
 * 
 * DESKRIPSI:
 * Demonstrasi penggunaan taskENTER_CRITICAL/taskEXIT_CRITICAL untuk
 * melindungi data yang diakses oleh task dan ISR secara bersamaan.
 * 
 * ============================================================================
 * ARSITEKTUR PROGRAM
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                   CRITICAL SECTION DEMO                             │
 *    ├─────────────────────────────────────────────────────────────────────┤
 *    │                                                                     │
 *    │   SHARED DATA:                                                      │
 *    │   ┌───────────────────────────────────────────────────────────────┐ │
 *    │   │  struct SharedData {                                          │ │
 *    │   │      uint32_t counter;      // Incremented by task & ISR      │ │
 *    │   │      uint32_t taskWrites;   // Task write count               │ │
 *    │   │      uint32_t isrWrites;    // ISR write count                │ │
 *    │   │  }                                                            │ │
 *    │   └───────────────────────────────────────────────────────────────┘ │
 *    │                      ↑                                              │
 *    │          ┌───────────┴───────────┐                                  │
 *    │          │                       │                                  │
 *    │   ┌──────┴──────┐         ┌──────┴──────┐                           │
 *    │   │   TASK      │         │    ISR      │                           │
 *    │   │             │         │   (TIM2)    │                           │
 *    │   │ ENTER_CRIT  │         │ENTER_CRIT   │                           │
 *    │   │ counter++   │         │FROM_ISR     │                           │
 *    │   │ taskWrites++│         │ counter++   │                           │
 *    │   │ EXIT_CRIT   │         │isrWrites++  │                           │
 *    │   │             │         │EXIT_CRIT    │                           │
 *    │   │             │         │FROM_ISR     │                           │
 *    │   └─────────────┘         └─────────────┘                           │
 *    │                                                                     │
 *    │   Monitor Task                                                      │
 *    │   ┌──────────────────────────────────────────────────────────────┐  │
 *    │   │  Every 1s: Verify integrity                                  │  │
 *    │   │  counter == taskWrites + isrWrites                           │  │
 *    │   └──────────────────────────────────────────────────────────────┘  │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * KONSEP PENTING
 * ============================================================================
 * 
 * 1. CRITICAL SECTION:
 *    - Disable interrupt untuk protect shared data
 *    - Harus singkat (< 100us ideal)
 *    - Delay task scheduling
 *    
 * 2. TASK CONTEXT:
 *    taskENTER_CRITICAL() / taskEXIT_CRITICAL()
 *    
 * 3. ISR CONTEXT:
 *    taskENTER_CRITICAL_FROM_ISR() / taskEXIT_CRITICAL_FROM_ISR()
 *    HARUS simpan return value di ENTER dan pass ke EXIT!
 * 
 * ============================================================================
 * EXPECTED OUTPUT
 * ============================================================================
 * 
 *    === Critical Section Demo ===
 *    Protecting shared data between Task and ISR
 *    
 *    [1.000s] Stats: counter=50, task=30, isr=20
 *             Integrity: OK (50 = 30 + 20)
 *    
 *    [2.000s] Stats: counter=100, task=60, isr=40
 *             Integrity: OK (100 = 60 + 40)
 *    
 *    [3.000s] Stats: counter=150, task=90, isr=60
 *             Integrity: OK (150 = 90 + 60)
 * 
 * ============================================================================
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * HARDWARE CONFIGURATION
 * ============================================================================ */

#define LED_GPIO_PORT   GPIOC
#define LED_GPIO_PIN    GPIO_PIN_13

/* ============================================================================
 * SHARED DATA STRUCTURE
 * ============================================================================ */

typedef struct {
    uint32_t counter;
    uint32_t taskWrites;
    uint32_t isrWrites;
} SharedData_t;

static volatile SharedData_t xSharedData = {0, 0, 0};

/* ============================================================================
 * GLOBAL HANDLES
 * ============================================================================ */

static TIM_HandleTypeDef htim2;
static UART_HandleTypeDef huart1;

/* ============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================ */

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART_Init(void);
static void TIM2_Init(void);
static void UART_SendString(const char *str);

static void vWriterTask(void *pvParameters);
static void vMonitorTask(void *pvParameters);

/* ============================================================================
 * TIMER 2 ISR - Increment counter dengan Critical Section
 * ============================================================================ */

void TIM2_IRQHandler(void)
{
    if(__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_UPDATE) != RESET)
    {
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
        
        /*
         * Critical Section FROM ISR
         * Return value HARUS disimpan dan passed ke EXIT
         */
        UBaseType_t uxSavedInterruptStatus;
        uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
        {
            /* Multi-step atomic operation */
            xSharedData.counter++;
            xSharedData.isrWrites++;
        }
        taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
        
        /* Toggle LED */
        HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
    }
}

/* ============================================================================
 * WRITER TASK - Menulis ke shared data dengan Critical Section
 * ============================================================================ */

static void vWriterTask(void *pvParameters)
{
    (void)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for(;;)
    {
        /*
         * Critical Section dari TASK context
         * Disable semua interrupt
         */
        taskENTER_CRITICAL();
        {
            xSharedData.counter++;
            xSharedData.taskWrites++;
        }
        taskEXIT_CRITICAL();
        
        /* Delay 50ms */
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(50));
    }
}

/* ============================================================================
 * MONITOR TASK - Verify data integrity
 * ============================================================================ */

static void vMonitorTask(void *pvParameters)
{
    (void)pvParameters;
    char buffer[120];
    uint32_t ulSeconds = 0;
    
    UART_SendString("\r\n=== Critical Section Demo ===\r\n");
    UART_SendString("Protecting shared data between Task and ISR\r\n\r\n");
    
    for(;;)
    {
        ulSeconds++;
        
        /* Read data dalam critical section untuk consistency */
        uint32_t counter, taskWrites, isrWrites;
        
        taskENTER_CRITICAL();
        {
            counter = xSharedData.counter;
            taskWrites = xSharedData.taskWrites;
            isrWrites = xSharedData.isrWrites;
        }
        taskEXIT_CRITICAL();
        
        /* Verify integrity */
        const char *status = (counter == taskWrites + isrWrites) ? "OK" : "ERROR!";
        
        snprintf(buffer, sizeof(buffer),
                "[%lu.000s] Stats: counter=%lu, task=%lu, isr=%lu\r\n"
                "         Integrity: %s (%lu = %lu + %lu)\r\n\r\n",
                ulSeconds, counter, taskWrites, isrWrites,
                status, counter, taskWrites, isrWrites);
        UART_SendString(buffer);
        
        vTaskDelay(pdMS_TO_TICKS(1000));
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
    
    /* Start Timer 2 interrupt (20Hz = 50ms) */
    HAL_TIM_Base_Start_IT(&htim2);
    
    /* Create tasks */
    xTaskCreate(vWriterTask, "Writer", 128, NULL, 2, NULL);
    xTaskCreate(vMonitorTask, "Monitor", 256, NULL, 1, NULL);
    
    /* Start scheduler */
    vTaskStartScheduler();
    
    while(1);
}

/* ============================================================================
 * SYSTEM CLOCK CONFIGURATION - 72MHz
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

/* ============================================================================
 * GPIO INITIALIZATION
 * ============================================================================ */

void GPIO_Init(void)
{
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = LED_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);
    
    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_SET);
}

/* ============================================================================
 * UART1 INITIALIZATION - 115200 baud
 * ============================================================================ */

void UART_Init(void)
{
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    /* PA9: TX, PA10: RX */
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

/* ============================================================================
 * TIMER 2 INITIALIZATION - 20Hz interrupt (50ms period)
 * ============================================================================ */

void TIM2_Init(void)
{
    __HAL_RCC_TIM2_CLK_ENABLE();
    
    /* Timer config: 72MHz / (7200 * 500) = 20Hz */
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 7200 - 1;    /* 72MHz / 7200 = 10kHz */
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 500 - 1;        /* 10kHz / 500 = 20Hz */
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&htim2);
    
    /* Enable interrupt */
    HAL_NVIC_SetPriority(TIM2_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

/* ============================================================================
 * UART HELPER
 * ============================================================================ */

void UART_SendString(const char *str)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
}

/* ============================================================================
 * FREERTOS HOOKS
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
