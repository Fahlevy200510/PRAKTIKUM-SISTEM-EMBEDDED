/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : FreeRTOS Event Groups - Multi-task Synchronization
 * @date           : 2026-01-22
 ******************************************************************************
 * Program 37: Event Groups untuk synchronize multiple tasks
 * 
 * Features:
 * - 3 buttons set 3 different event bits
 * - Wait-for-all pattern: All 3 buttons pressed
 * - Wait-for-any pattern: Any button pressed
 * - LED indicators untuk each event
 ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include <stdio.h>

#define EVENT_BIT_0  (1 << 0)  // Button 1
#define EVENT_BIT_1  (1 << 1)  // Button 2
#define EVENT_BIT_2  (1 << 2)  // Button 3
#define EVENT_ALL_BITS (EVENT_BIT_0 | EVENT_BIT_1 | EVENT_BIT_2)

UART_HandleTypeDef huart1;
EventGroupHandle_t xEventGroup;

void SystemClock_Config(void);
void GPIO_Init(void);
void UART_Init(void);
void vTaskWaitAll(void *pvParameters);
void vTaskWaitAny(void *pvParameters);

int _write(int file, char *ptr, int len) {
    HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}

/* Button interrupt callbacks */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    switch(GPIO_Pin) {
        case GPIO_PIN_0:
            xEventGroupSetBitsFromISR(xEventGroup, EVENT_BIT_0, &xHigherPriorityTaskWoken);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
            printf("[ISR] Button 1 pressed - Bit 0 set\r\n");
            break;
        case GPIO_PIN_1:
            xEventGroupSetBitsFromISR(xEventGroup, EVENT_BIT_1, &xHigherPriorityTaskWoken);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);
            printf("[ISR] Button 2 pressed - Bit 1 set\r\n");
            break;
        case GPIO_PIN_2:
            xEventGroupSetBitsFromISR(xEventGroup, EVENT_BIT_2, &xHigherPriorityTaskWoken);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_RESET);
            printf("[ISR] Button 3 pressed - Bit 2 set\r\n");
            break;
    }
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    UART_Init();
    
    printf("\r\n=== FreeRTOS Event Groups ===\r\n");
    printf("Program 37: Multi-task Synchronization\r\n\r\n");
    printf("Press buttons:\r\n");
    printf("  PA0: Set Bit 0 (LED PC13)\r\n");
    printf("  PA1: Set Bit 1 (LED PC14)\r\n");
    printf("  PA2: Set Bit 2 (LED PC15)\r\n\r\n");
    
    xEventGroup = xEventGroupCreate();
    
    if (xEventGroup == NULL) {
        printf("ERROR: Failed to create event group!\r\n");
        Error_Handler();
    }
    
    xTaskCreate(vTaskWaitAll, "WaitAll", 256, NULL, 2, NULL);
    xTaskCreate(vTaskWaitAny, "WaitAny", 256, NULL, 1, NULL);
    
    printf("Tasks created. Waiting for events...\r\n\r\n");
    
    vTaskStartScheduler();
    while (1) {}
}

/* Task: Wait for ALL bits */
void vTaskWaitAll(void *pvParameters) {
    EventBits_t uxBits;
    
    for (;;) {
        /* Wait for ALL 3 bits to be set */
        uxBits = xEventGroupWaitBits(
            xEventGroup,
            EVENT_ALL_BITS,
            pdTRUE,   // Clear bits on exit
            pdTRUE,   // Wait for ALL bits
            portMAX_DELAY
        );
        
        if ((uxBits & EVENT_ALL_BITS) == EVENT_ALL_BITS) {
            printf("\r\n>>> [WAIT-ALL] ALL BUTTONS PRESSED! <<<\r\n\r\n");
            
            /* Blink all LEDs */
            for (int i = 0; i < 3; i++) {
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);
                vTaskDelay(pdMS_TO_TICKS(200));
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_SET);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
        }
    }
}

/* Task: Wait for ANY bit */
void vTaskWaitAny(void *pvParameters) {
    EventBits_t uxBits;
    
    for (;;) {
        /* Wait for ANY of the 3 bits */
        uxBits = xEventGroupWaitBits(
            xEventGroup,
            EVENT_ALL_BITS,
            pdFALSE,  // Don't clear bits
            pdFALSE,  // Wait for ANY bit
            portMAX_DELAY
        );
        
        printf("[WAIT-ANY] Event bits: 0x%02lX | ", (unsigned long)uxBits);
        
        if (uxBits & EVENT_BIT_0) printf("B1 ");
        if (uxBits & EVENT_BIT_1) printf("B2 ");
        if (uxBits & EVENT_BIT_2) printf("B3 ");
        printf("\r\n");
        
        vTaskDelay(pdMS_TO_TICKS(100));
        
        /* Turn off LEDs after reading */
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_SET);
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
    
    /* PC13, PC14, PC15 as outputs (LEDs) */
    GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_SET);
    
    /* PA0, PA1, PA2 as inputs with interrupts (Buttons) */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
    HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);
    HAL_NVIC_SetPriority(EXTI2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI2_IRQn);
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
void EXTI2_IRQHandler(void) { HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2); }

void Error_Handler(void) {
    __disable_irq();
    while (1) {}
}
