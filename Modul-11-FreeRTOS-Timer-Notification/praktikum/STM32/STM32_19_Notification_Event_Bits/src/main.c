/**
 * Program 36: Notification Event Bits
 * Concept: Core functionality demonstration
 * 
 * Learning Points:
 * - Hardware & FreeRTOS configuration
 * - Task management & synchronization
 * - Data exchange patterns
 * - Error handling
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "config.h"
#include <stdio.h>
#include <string.h>

/* Handle declarations */
UART_HandleTypeDef huart1;

/* FreeRTOS objects */
TaskHandle_t task_handle_1 = NULL;
TaskHandle_t task_handle_2 = NULL;
QueueHandle_t queue_handle = NULL;
SemaphoreHandle_t semaphore_handle = NULL;
TimerHandle_t timer_handle = NULL;

/* Function prototypes */
void SystemClock_Config(void);
void UART_Init(void);
void LED_Init(void);
void task_function_1(void *pvParameters);
void task_function_2(void *pvParameters);
void timer_callback(TimerHandle_t xTimer);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    UART_Init();
    LED_Init();
    
    printf("\n=== Program 36: Notification Event Bits ===\n");
    printf("FreeRTOS v%s\n", tskKERNEL_VERSION_NUMBER);
    
    /* Create FreeRTOS objects */
    queue_handle = xQueueCreate(10, sizeof(uint32_t));
    semaphore_handle = xSemaphoreCreateBinary();
    timer_handle = xTimerCreate("Timer", pdMS_TO_TICKS(1000), pdTRUE, NULL, timer_callback);
    
    /* Create tasks */
    xTaskCreate(task_function_1, "Task1", 512, NULL, 2, &task_handle_1);
    xTaskCreate(task_function_2, "Task2", 512, NULL, 1, &task_handle_2);
    
    /* Start timer */
    if(timer_handle != NULL)
        xTimerStart(timer_handle, 0);
    
    printf("Starting FreeRTOS scheduler...\n");
    vTaskStartScheduler();
    
    /* Should never reach here */
    while(1);
    
    return 0;
}

void task_function_1(void *pvParameters)
{
    (void)pvParameters;
    
    while(1)
    {
        printf("[Task1] Running...\n");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void task_function_2(void *pvParameters)
{
    (void)pvParameters;
    
    while(1)
    {
        printf("[Task2] Running...\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void timer_callback(TimerHandle_t xTimer)
{
    (void)xTimer;
    printf("[Timer] Callback\n");
}

void UART_Init(void)
{
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    HAL_UART_Init(&huart1);
}

void LED_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = LED_RED | LED_YEL | LED_GRN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
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
    
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                   RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}

void vAssertCalled(const char *pcFile, unsigned long ulLine)
{
    printf("ASSERT: %s:%lu\n", pcFile, ulLine);
    taskDISABLE_INTERRUPTS();
    while(1);
}

void Error_Handler(void)
{
    while(1);
}
