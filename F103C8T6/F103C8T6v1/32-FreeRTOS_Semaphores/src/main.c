/**
 * Program 32: FreeRTOS Semaphores (Binary & Counting)
 * 
 * Berdasarkan: Mastering STM32 - Chapter 16 (FreeRTOS)
 * 
 * Deskripsi:
 * Program ini mendemonstrasikan penggunaan semaphore dalam FreeRTOS untuk
 * sinkronisasi antar task dan resource protection.
 * 
 * Types of Semaphores:
 * 1. Binary Semaphore - Signal/Event notification (0 atau 1)
 * 2. Counting Semaphore - Resource counting (0 to N)
 * 3. Mutex - Mutual exclusion dengan priority inheritance
 * 
 * Fitur:
 * - Binary semaphore untuk interrupt-to-task signaling
 * - Counting semaphore untuk resource pool management
 * - Producer-Consumer pattern
 * - ISR safe API (xSemaphoreGiveFromISR)
 * - Task synchronization
 * 
 * Scenario:
 * - Button ISR gives binary semaphore
 * - Handler task waits for semaphore
 * - Producer task gives counting semaphore
 * - Consumer tasks take from semaphore pool
 * 
 * Hardware:
 * - STM32F103C8T6 Blue Pill
 * - Button pada PA0 (dengan pull-up)
 * - 3x LED (PC13, PB0, PB1)
 * - UART (PA9/PA10) untuk monitoring
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <string.h>
#include <stdio.h>

/* Handles */
UART_HandleTypeDef huart1;

/* Semaphore handles */
SemaphoreHandle_t xBinarySemaphore;
SemaphoreHandle_t xCountingSemaphore;
SemaphoreHandle_t xMutex;

/* Task handles */
TaskHandle_t xButtonHandlerTask;
TaskHandle_t xProducerTask;
TaskHandle_t xConsumer1Task;
TaskHandle_t xConsumer2Task;
TaskHandle_t xMonitorTask;

/* Statistics */
volatile uint32_t button_press_count = 0;
volatile uint32_t items_produced = 0;
volatile uint32_t items_consumed_1 = 0;
volatile uint32_t items_consumed_2 = 0;
volatile uint32_t semaphore_errors = 0;

/* Function prototypes */
void SystemClock_Config(void);
void GPIO_Init(void);
void UART_Init(void);
void UART_Print(const char *str);
void vButtonHandlerTask(void *pvParameters);
void vProducerTask(void *pvParameters);
void vConsumer1Task(void *pvParameters);
void vConsumer2Task(void *pvParameters);
void vMonitorTask(void *pvParameters);

char msg[200];

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    UART_Init();

    UART_Print("\r\n=== FreeRTOS Semaphores Demo ===\r\n");
    UART_Print("Binary & Counting Semaphores\r\n\r\n");

    /* Create Binary Semaphore untuk button interrupt */
    xBinarySemaphore = xSemaphoreCreateBinary();
    if(xBinarySemaphore == NULL)
    {
        UART_Print("ERROR: Failed to create binary semaphore!\r\n");
        while(1);
    }

    /* Create Counting Semaphore (max 5 items) */
    xCountingSemaphore = xSemaphoreCreateCounting(5, 0);
    if(xCountingSemaphore == NULL)
    {
        UART_Print("ERROR: Failed to create counting semaphore!\r\n");
        while(1);
    }

    /* Create Mutex untuk shared resource protection */
    xMutex = xSemaphoreCreateMutex();
    if(xMutex == NULL)
    {
        UART_Print("ERROR: Failed to create mutex!\r\n");
        while(1);
    }

    /* Create tasks */
    xTaskCreate(vButtonHandlerTask, "ButtonHandler", 128, NULL, 3, &xButtonHandlerTask);
    xTaskCreate(vProducerTask, "Producer", 128, NULL, 2, &xProducerTask);
    xTaskCreate(vConsumer1Task, "Consumer1", 128, NULL, 1, &xConsumer1Task);
    xTaskCreate(vConsumer2Task, "Consumer2", 128, NULL, 1, &xConsumer2Task);
    xTaskCreate(vMonitorTask, "Monitor", 256, NULL, 4, &xMonitorTask);

    UART_Print("Tasks & Semaphores created!\r\n");
    UART_Print("Press button (PA0) to test binary semaphore\r\n\r\n");

    /* Start scheduler */
    vTaskStartScheduler();

    while(1);
}

/**
 * Button Handler Task
 * Waits for binary semaphore from button ISR
 */
void vButtonHandlerTask(void *pvParameters)
{
    while(1)
    {
        /* Wait for binary semaphore (from button ISR) */
        if(xSemaphoreTake(xBinarySemaphore, portMAX_DELAY) == pdTRUE)
        {
            /* Button pressed! */
            if(xSemaphoreTake(xMutex, pdMS_TO_TICKS(100)) == pdTRUE)
            {
                button_press_count++;
                
                HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
                
                sprintf(msg, "[BUTTON] Press #%lu detected!\r\n", button_press_count);
                UART_Print(msg);
                
                xSemaphoreGive(xMutex);
            }
        }
    }
}

/**
 * Producer Task
 * Produces items and gives counting semaphore
 */
void vProducerTask(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    while(1)
    {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(2000));
        
        /* Try to produce item (give counting semaphore) */
        if(xSemaphoreGive(xCountingSemaphore) == pdTRUE)
        {
            if(xSemaphoreTake(xMutex, pdMS_TO_TICKS(100)) == pdTRUE)
            {
                items_produced++;
                
                UBaseType_t available = uxSemaphoreGetCount(xCountingSemaphore);
                sprintf(msg, "[PRODUCER] Item produced. Available: %u/5\r\n", 
                        (unsigned int)available);
                UART_Print(msg);
                
                xSemaphoreGive(xMutex);
            }
        }
        else
        {
            /* Buffer full */
            if(xSemaphoreTake(xMutex, pdMS_TO_TICKS(100)) == pdTRUE)
            {
                semaphore_errors++;
                UART_Print("[PRODUCER] Buffer FULL! Cannot produce.\r\n");
                xSemaphoreGive(xMutex);
            }
        }
    }
}

/**
 * Consumer 1 Task
 * Consumes items from counting semaphore
 */
void vConsumer1Task(void *pvParameters)
{
    while(1)
    {
        /* Try to consume item (take counting semaphore) */
        if(xSemaphoreTake(xCountingSemaphore, pdMS_TO_TICKS(3000)) == pdTRUE)
        {
            if(xSemaphoreTake(xMutex, pdMS_TO_TICKS(100)) == pdTRUE)
            {
                items_consumed_1++;
                
                HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
                
                sprintf(msg, "[CONSUMER1] Item consumed. Total: %lu\r\n", 
                        items_consumed_1);
                UART_Print(msg);
                
                xSemaphoreGive(xMutex);
            }
            
            /* Simulate processing */
            vTaskDelay(pdMS_TO_TICKS(500));
        }
        else
        {
            /* Timeout - no items available */
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

/**
 * Consumer 2 Task
 * Consumes items from counting semaphore (slower)
 */
void vConsumer2Task(void *pvParameters)
{
    while(1)
    {
        if(xSemaphoreTake(xCountingSemaphore, pdMS_TO_TICKS(4000)) == pdTRUE)
        {
            if(xSemaphoreTake(xMutex, pdMS_TO_TICKS(100)) == pdTRUE)
            {
                items_consumed_2++;
                
                HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
                
                sprintf(msg, "[CONSUMER2] Item consumed. Total: %lu\r\n", 
                        items_consumed_2);
                UART_Print(msg);
                
                xSemaphoreGive(xMutex);
            }
            
            /* Simulate slower processing */
            vTaskDelay(pdMS_TO_TICKS(800));
        }
        else
        {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

/**
 * Monitor Task
 * Displays statistics
 */
void vMonitorTask(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    while(1)
    {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(5000));
        
        if(xSemaphoreTake(xMutex, pdMS_TO_TICKS(200)) == pdTRUE)
        {
            UART_Print("\r\n╔═══════════════════════════════════╗\r\n");
            UART_Print("║    Semaphore Statistics          ║\r\n");
            UART_Print("╠═══════════════════════════════════╣\r\n");
            
            sprintf(msg, "║ Button Presses:    %10lu  ║\r\n", button_press_count);
            UART_Print(msg);
            
            sprintf(msg, "║ Items Produced:    %10lu  ║\r\n", items_produced);
            UART_Print(msg);
            
            sprintf(msg, "║ Consumed (Task1):  %10lu  ║\r\n", items_consumed_1);
            UART_Print(msg);
            
            sprintf(msg, "║ Consumed (Task2):  %10lu  ║\r\n", items_consumed_2);
            UART_Print(msg);
            
            UBaseType_t available = uxSemaphoreGetCount(xCountingSemaphore);
            sprintf(msg, "║ Available Items:   %10u  ║\r\n", (unsigned int)available);
            UART_Print(msg);
            
            sprintf(msg, "║ Semaphore Errors:  %10lu  ║\r\n", semaphore_errors);
            UART_Print(msg);
            
            UART_Print("╚═══════════════════════════════════╝\r\n\r\n");
            
            xSemaphoreGive(xMutex);
        }
    }
}

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
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* LEDs */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Button PA0 dengan EXTI */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

void UART_Init(void)
{
    __HAL_RCC_USART1_CLK_ENABLE();

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

void UART_Print(const char *str)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
}

/**
 * EXTI0 IRQ Handler (Button)
 */
void EXTI0_IRQHandler(void)
{
    if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_0) != RESET)
    {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
        
        /* Give binary semaphore from ISR */
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(xBinarySemaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}
