/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 27-Critical_Section_ISR
 * 
 * JUDUL: Critical Section Demo
 * 
 * DESKRIPSI:
 * Demo penggunaan critical section untuk protect shared data
 * antara task dan ISR.
 * 
 * ============================================================================
 * ARSITEKTUR PROGRAM
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                   CRITICAL SECTION DEMO                            │
 *    ├─────────────────────────────────────────────────────────────────────┤
 *    │                                                                     │
 *    │   SHARED DATA:                                                      │
 *    │   ┌───────────────────────────────────────────────────────────────┐ │
 *    │   │  struct SharedData {                                          │ │
 *    │   │      uint32_t counter;      // Increment by task & ISR        │ │
 *    │   │      uint32_t taskWrites;   // Count task writes              │ │
 *    │   │      uint32_t isrWrites;    // Count ISR writes               │ │
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
 *    │   │ EXIT_CRIT   │         │ counter++   │                           │
 *    │   │             │         │EXIT_CRIT    │                           │
 *    │   │             │         │FROM_ISR     │                           │
 *    │   └─────────────┘         └─────────────┘                           │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * EXPECTED OUTPUT
 * ============================================================================
 * 
 *    === Critical Section Demo ===
 *    
 *    [5.000s] Stats: counter=150, task=100, isr=50
 *             Integrity: OK (counter = task + isr)
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
 * SHARED DATA STRUCTURE (Protected by Critical Section)
 * ============================================================================ */

typedef struct {
    uint32_t counter;
    uint32_t taskWrites;
    uint32_t isrWrites;
} SharedData_t;

static volatile SharedData_t xSharedData = {0, 0, 0};

/* ============================================================================
 * GLOBAL VARIABLES
 * ============================================================================ */

static TIM_HandleTypeDef htim2;
static UART_HandleTypeDef huart1;

/* ============================================================================
 * PROTOTYPE
 * ============================================================================ */

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART_Init(void);
static void TIM2_Init(void);
static void UART_SendString(const char *str);

static void vWriterTask(void *pvParameters);
static void vMonitorTask(void *pvParameters);

/* ============================================================================
 * TIMER ISR - Menggunakan Critical Section FROM_ISR
 * ============================================================================ */

/**
 * @brief TIM2 ISR - Increment shared counter dengan protection
 */
void TIM2_IRQHandler(void)
{
    if(__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_UPDATE) != RESET)
    {
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
        
        /*
         * taskENTER_CRITICAL_FROM_ISR / taskEXIT_CRITICAL_FROM_ISR
         * 
         * PENTING: Return value HARUS disimpan dan dipass ke EXIT!
         * Ini menyimpan interrupt status sebelum masking.
         */
        UBaseType_t uxSavedInterruptStatus;
        uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
        {
            /* 
             * Critical Section - Interrupt dengan priority lebih rendah
             * TIDAK bisa preempt di sini.
             * 
             * Multi-step operation yang harus atomic:
             */
            xSharedData.counter++;
            xSharedData.isrWrites++;
            
            /* Bisa tambah operasi lain yang perlu atomic */
        }
        taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
        
        /* Toggle LED */
        HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
    }
}

/* ============================================================================
 * WRITER TASK - Menggunakan Critical Section
 * ============================================================================ */

/**
 * @brief Task yang menulis ke shared data dengan protection
 */
static void vWriterTask(void *pvParameters)
{
    (void)pvParameters;
    
    for(;;)
    {
        /*
         * taskENTER_CRITICAL / taskEXIT_CRITICAL
         * 
         * Untuk TASK context (bukan ISR).
         * Disable interrupt sampai configMAX_SYSCALL_INTERRUPT_PRIORITY.
         * Bisa nested - FreeRTOS track dengan counter.
         */
        taskENTER_CRITICAL();
        {
            /*
             * CRITICAL SECTION
             * 
             * Di sini interrupt DISABLED (yang priority >= MAX_SYSCALL)
             * TIM2 ISR tidak bisa preempt!
             */
            xSharedData.counter++;
            xSharedData.taskWrites++;
            
            /* 
             * CATATAN: Jangan terlalu lama di critical section!
             * Interrupt tertunda → latency buruk
             */
        }
        taskEXIT_CRITICAL();
        
        /* Delay sebentar */
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

/* ============================================================================
 * MONITOR TASK
 * ============================================================================ */

static void vMonitorTask(void *pvParameters)
{
    (void)pvParameters;
    
    char buffer[120];
    
    for(;;)
    {
        vTaskDelay(pdMS_TO_TICKS(5000));
        
        /* Baca shared data dengan protection */
        uint32_t counter, taskWrites, isrWrites;
        
        taskENTER_CRITICAL();
        {
            counter = xSharedData.counter;
            taskWrites = xSharedData.taskWrites;
            isrWrites = xSharedData.isrWrites;
        }
        taskEXIT_CRITICAL();
        
        /* Check integrity */
        const char *integrity = (counter == taskWrites + isrWrites) ? "OK" : "CORRUPTED!";
        
        float seconds = (float)xTaskGetTickCount() / 1000.0f;
        snprintf(buffer, sizeof(buffer),
                 "[%.1fs] Stats: counter=%lu, task=%lu, isr=%lu\r\n"
                 "        Integrity: %s (counter should = task + isr)\r\n\r\n",
                 seconds, counter, taskWrites, isrWrites, integrity);
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

static void TIM2_Init(void)
{
    __HAL_RCC_TIM2_CLK_ENABLE();
    
    /* 100ms period */
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 7199;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 999;
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
    UART_SendString("27-Critical_Section_ISR\r\n");
    UART_SendString("STM32F103 + FreeRTOS\r\n");
    UART_SendString("================================\r\n\r\n");
    UART_SendString("=== Critical Section Demo ===\r\n\r\n");
    UART_SendString("Task and ISR both increment shared counter\r\n");
    UART_SendString("Critical sections prevent race conditions\r\n\r\n");
    
    xTaskCreate(vWriterTask, "Writer", TASK_STACK, NULL,
                tskIDLE_PRIORITY + 1, NULL);
    
    xTaskCreate(vMonitorTask, "Monitor", TASK_STACK, NULL,
                tskIDLE_PRIORITY + 1, NULL);
    
    HAL_TIM_Base_Start_IT(&htim2);
    
    vTaskStartScheduler();
    
    for(;;);
}
