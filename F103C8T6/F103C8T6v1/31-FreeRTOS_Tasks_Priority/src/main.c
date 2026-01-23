/**
 * Program 31: FreeRTOS Task Priority Management
 * 
 * Deskripsi:
 * Program ini mendemonstrasikan manajemen prioritas task dalam FreeRTOS,
 * termasuk preemptive scheduling, priority inversion, dan dynamic priority change.
 * 
 * Fitur:
 * - Multiple tasks dengan berbagai priority level
 * - Preemptive scheduling demonstration
 * - Priority inheritance untuk mutex
 * - Dynamic priority change (vTaskPrioritySet)
 * - CPU utilization monitoring
 * - Task state monitoring
 * 
 * Tasks:
 * - High Priority Task (Priority 3): Critical, fast execution
 * - Medium Priority Task (Priority 2): Normal processing
 * - Low Priority Task (Priority 1): Background task
 * - Idle Task (Priority 0): System idle (automatic)
 * - Monitor Task (Priority 4): System monitoring
 * 
 * Concepts:
 * - Priority levels: 0 (lowest) to configMAX_PRIORITIES-1 (highest)
 * - Preemption: Higher priority task dapat interrupt lower priority
 * - Time slicing: Tasks dengan priority sama share CPU time
 * - Starvation: Low priority task mungkin tidak dapat CPU time
 * 
 * Hardware:
 * - STM32F103C8T6 Blue Pill
 * - UART USB adapter (PA9/PA10)
 * - 3x LED (PC13, PB0, PB1) untuk visualisasi task
 * 
 * Pin Configuration:
 * - PA9: UART TX
 * - PA10: UART RX
 * - PC13: High priority task LED
 * - PB0: Medium priority task LED
 * - PB1: Low priority task LED
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <string.h>
#include <stdio.h>

/* UART handle */
UART_HandleTypeDef huart1;

/* Task handles */
TaskHandle_t xHighPriorityTask;
TaskHandle_t xMediumPriorityTask;
TaskHandle_t xLowPriorityTask;
TaskHandle_t xMonitorTask;

/* Mutex untuk shared resource */
SemaphoreHandle_t xMutex;

/* Shared counter */
volatile uint32_t shared_counter = 0;

/* Task execution counters */
volatile uint32_t high_task_count = 0;
volatile uint32_t medium_task_count = 0;
volatile uint32_t low_task_count = 0;

/* Function prototypes */
void SystemClock_Config(void);
void GPIO_Init(void);
void UART_Init(void);
void UART_Print(const char *str);

/* Task functions */
void vHighPriorityTask(void *pvParameters);
void vMediumPriorityTask(void *pvParameters);
void vLowPriorityTask(void *pvParameters);
void vMonitorTask(void *pvParameters);

/* Buffer */
char msg[200];

/**
 * Main function
 */
int main(void)
{
    /* Initialize HAL */
    HAL_Init();

    /* Configure system clock */
    SystemClock_Config();

    /* Initialize peripherals */
    GPIO_Init();
    UART_Init();

    UART_Print("\r\n\r\n");
    UART_Print("╔═══════════════════════════════════════╗\r\n");
    UART_Print("║   FreeRTOS Task Priority Manager     ║\r\n");
    UART_Print("║   STM32F103C8T6                      ║\r\n");
    UART_Print("╚═══════════════════════════════════════╝\r\n");
    UART_Print("\r\nInitializing FreeRTOS...\r\n");

    /* Create mutex */
    xMutex = xSemaphoreCreateMutex();
    
    if(xMutex == NULL)
    {
        UART_Print("ERROR: Failed to create mutex!\r\n");
        while(1);
    }

    /* Create tasks */
    xTaskCreate(vHighPriorityTask, "HighTask", 128, NULL, 3, &xHighPriorityTask);
    xTaskCreate(vMediumPriorityTask, "MediumTask", 128, NULL, 2, &xMediumPriorityTask);
    xTaskCreate(vLowPriorityTask, "LowTask", 128, NULL, 1, &xLowPriorityTask);
    xTaskCreate(vMonitorTask, "Monitor", 256, NULL, 4, &xMonitorTask);

    UART_Print("\r\nTasks created:\r\n");
    UART_Print("- High Priority Task   (Priority 3)\r\n");
    UART_Print("- Medium Priority Task (Priority 2)\r\n");
    UART_Print("- Low Priority Task    (Priority 1)\r\n");
    UART_Print("- Monitor Task         (Priority 4)\r\n");
    UART_Print("\r\nStarting FreeRTOS scheduler...\r\n\r\n");

    /* Start scheduler */
    vTaskStartScheduler();

    /* Should never reach here */
    while(1)
    {
        UART_Print("ERROR: Scheduler failed!\r\n");
        HAL_Delay(1000);
    }
}

/**
 * High Priority Task (Priority 3)
 * Runs frequently, critical task
 */
void vHighPriorityTask(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(500); // 500ms period

    while(1)
    {
        /* Wait for next cycle */
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        /* Acquire mutex */
        if(xSemaphoreTake(xMutex, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            /* Critical section */
            shared_counter++;
            high_task_count++;
            
            /* LED toggle */
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
            
            /* Simulate work */
            vTaskDelay(pdMS_TO_TICKS(50));
            
            /* Release mutex */
            xSemaphoreGive(xMutex);
        }
    }
}

/**
 * Medium Priority Task (Priority 2)
 * Normal processing task
 */
void vMediumPriorityTask(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(1000); // 1000ms period

    while(1)
    {
        /* Wait for next cycle */
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        /* Acquire mutex */
        if(xSemaphoreTake(xMutex, pdMS_TO_TICKS(200)) == pdTRUE)
        {
            /* Critical section */
            shared_counter += 10;
            medium_task_count++;
            
            /* LED toggle */
            HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
            
            /* Simulate work */
            vTaskDelay(pdMS_TO_TICKS(100));
            
            /* Release mutex */
            xSemaphoreGive(xMutex);
        }
    }
}

/**
 * Low Priority Task (Priority 1)
 * Background task, may get starved
 */
void vLowPriorityTask(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(2000); // 2000ms period

    while(1)
    {
        /* Wait for next cycle */
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        /* Acquire mutex */
        if(xSemaphoreTake(xMutex, pdMS_TO_TICKS(500)) == pdTRUE)
        {
            /* Critical section */
            shared_counter += 100;
            low_task_count++;
            
            /* LED toggle */
            HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
            
            /* Simulate work */
            vTaskDelay(pdMS_TO_TICKS(200));
            
            /* Release mutex */
            xSemaphoreGive(xMutex);
        }
        else
        {
            /* Mutex tidak didapat (timeout) */
            sprintf(msg, "[LOW] Mutex timeout! High priority blocking.\r\n");
            UART_Print(msg);
        }
    }
}

/**
 * Monitor Task (Priority 4)
 * Monitors system status dan task statistics
 */
void vMonitorTask(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(5000); // 5000ms period

    while(1)
    {
        /* Wait for next cycle */
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        /* Print statistics */
        UART_Print("\r\n╔═══════════════════════════════════════╗\r\n");
        UART_Print("║       Task Statistics Report         ║\r\n");
        UART_Print("╠═══════════════════════════════════════╣\r\n");

        sprintf(msg, "║ Shared Counter:    %10lu        ║\r\n", shared_counter);
        UART_Print(msg);
        
        UART_Print("╠═══════════════════════════════════════╣\r\n");
        
        sprintf(msg, "║ High Task Count:   %10lu        ║\r\n", high_task_count);
        UART_Print(msg);
        
        sprintf(msg, "║ Medium Task Count: %10lu        ║\r\n", medium_task_count);
        UART_Print(msg);
        
        sprintf(msg, "║ Low Task Count:    %10lu        ║\r\n", low_task_count);
        UART_Print(msg);
        
        UART_Print("╠═══════════════════════════════════════╣\r\n");

        /* Get task states */
        eTaskState highState = eTaskGetState(xHighPriorityTask);
        eTaskState mediumState = eTaskGetState(xMediumPriorityTask);
        eTaskState lowState = eTaskGetState(xLowPriorityTask);

        const char* stateStr[] = {"Running", "Ready", "Blocked", "Suspended", "Deleted"};
        
        sprintf(msg, "║ High Task State:   %-18s║\r\n", stateStr[highState]);
        UART_Print(msg);
        
        sprintf(msg, "║ Medium Task State: %-18s║\r\n", stateStr[mediumState]);
        UART_Print(msg);
        
        sprintf(msg, "║ Low Task State:    %-18s║\r\n", stateStr[lowState]);
        UART_Print(msg);
        
        UART_Print("╠═══════════════════════════════════════╣\r\n");

        /* Free heap space */
        size_t freeHeap = xPortGetFreeHeapSize();
        sprintf(msg, "║ Free Heap:         %10u bytes ║\r\n", freeHeap);
        UART_Print(msg);
        
        UART_Print("╚═══════════════════════════════════════╝\r\n\r\n");

        /* Test dynamic priority change setiap 10 detik */
        static uint8_t priority_test = 0;
        if(priority_test++ >= 2)
        {
            priority_test = 0;
            
            /* Swap priority antara medium dan low task */
            UBaseType_t mediumPrio = uxTaskPriorityGet(xMediumPriorityTask);
            UBaseType_t lowPrio = uxTaskPriorityGet(xLowPriorityTask);
            
            vTaskPrioritySet(xMediumPriorityTask, lowPrio);
            vTaskPrioritySet(xLowPriorityTask, mediumPrio);
            
            UART_Print(">>> Priority swapped between Medium and Low tasks! <<<\r\n\r\n");
        }
    }
}

/**
 * System Clock Configuration
 */
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

/**
 * GPIO Initialization
 */
void GPIO_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* PC13, PB0, PB1 - LEDs */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/**
 * UART Initialization
 */
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

/**
 * UART Print Helper
 */
void UART_Print(const char *str)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
}

/**
 * SysTick Handler (required by HAL)
 */
void SysTick_Handler(void)
{
    HAL_IncTick();
}
