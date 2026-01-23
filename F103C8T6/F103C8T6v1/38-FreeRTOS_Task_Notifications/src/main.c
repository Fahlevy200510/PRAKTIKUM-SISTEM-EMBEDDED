/**
 * Program 38: FreeRTOS Task Notifications
 * Lightweight IPC mechanism - 75% faster than semaphores!
 * 
 * Hardware: PA0, PA1 (buttons), PC13-PC15 (LEDs)
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

UART_HandleTypeDef huart1;
TaskHandle_t xTask1Handle = NULL;
TaskHandle_t xTask2Handle = NULL;

void SystemClock_Config(void);
void GPIO_Init(void);
void UART_Init(void);
void vTask1(void *pvParameters);
void vTask2(void *pvParameters);

int _write(int file, char *ptr, int len) {
    HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    if (GPIO_Pin == GPIO_PIN_0 && xTask1Handle != NULL) {
        /* Notify Task 1 with value */
        xTaskNotifyFromISR(xTask1Handle, 0x01, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
        printf("[ISR] Task 1 notified\r\n");
    } else if (GPIO_Pin == GPIO_PIN_1 && xTask2Handle != NULL) {
        /* Binary semaphore alternative */
        vTaskNotifyGiveFromISR(xTask2Handle, &xHigherPriorityTaskWoken);
        printf("[ISR] Task 2 notified\r\n");
    }
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    UART_Init();
    
    printf("\r\n=== FreeRTOS Task Notifications ===\r\n");
    printf("Program 38: Lightweight Direct-to-Task Signaling\r\n\r\n");
    printf("Performance: 75%% faster than semaphores!\r\n");
    printf("Press PA0 → Notify Task 1\r\n");
    printf("Press PA1 → Notify Task 2\r\n\r\n");
    
    xTaskCreate(vTask1, "Task1", 256, NULL, 2, &xTask1Handle);
    xTaskCreate(vTask2, "Task2", 256, NULL, 2, &xTask2Handle);
    
    vTaskStartScheduler();
    while (1) {}
}

void vTask1(void *pvParameters) {
    uint32_t ulNotifiedValue;
    
    for (;;) {
        /* Wait for notification with value */
        if (xTaskNotifyWait(0x00, 0xFFFFFFFF, &ulNotifiedValue, portMAX_DELAY) == pdTRUE) {
            printf("[TASK1] Received notification: 0x%02lX\r\n", ulNotifiedValue);
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        }
    }
}

void vTask2(void *pvParameters) {
    uint32_t ulNotificationValue;
    
    for (;;) {
        /* Binary semaphore alternative - wait for notification */
        ulNotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        
        if (ulNotificationValue > 0) {
            printf("[TASK2] Notification taken (count: %lu)\r\n", ulNotificationValue);
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_14);
        }
    }
}

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    __HAL_RCC_PWR_CLK_ENABLE();
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

void GPIO_Init(void) {
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
    HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);
}

void UART_Init(void) {
    __HAL_RCC_USART1_CLK_ENABLE();
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart1);
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void EXTI0_IRQHandler(void) { HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0); }
void EXTI1_IRQHandler(void) { HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1); }
void Error_Handler(void) { __disable_irq(); while (1) {} }
