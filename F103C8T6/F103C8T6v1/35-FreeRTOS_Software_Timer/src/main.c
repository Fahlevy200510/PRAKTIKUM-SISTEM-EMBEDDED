/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : FreeRTOS Software Timer - Periodic & One-shot Timers
 * @date           : 2026-01-22
 ******************************************************************************
 * Program 35: FreeRTOS Software Timer
 * 
 * Features:
 * - Periodic timer (1s) untuk LED blink di PC13
 * - One-shot timer (5s) untuk LED pulse di PC14
 * - Auto-reload timer untuk heartbeat di PC15
 * - Timer statistics via UART
 ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include <stdio.h>

UART_HandleTypeDef huart1;

TimerHandle_t xPeriodicTimer;
TimerHandle_t xOneShotTimer;
TimerHandle_t xHeartbeatTimer;

uint32_t periodic_count = 0;
uint32_t oneshot_count = 0;
uint32_t heartbeat_count = 0;

void SystemClock_Config(void);
void GPIO_Init(void);
void UART_Init(void);
void vPeriodicTimerCallback(TimerHandle_t xTimer);
void vOneShotTimerCallback(TimerHandle_t xTimer);
void vHeartbeatCallback(TimerHandle_t xTimer);

int _write(int file, char *ptr, int len) {
    HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    UART_Init();
    
    printf("\r\n=== FreeRTOS Software Timer ===\r\n");
    printf("Program 35: Periodic & One-shot Timers\r\n\r\n");
    
    /* Create Periodic Timer (1 second, auto-reload) */
    xPeriodicTimer = xTimerCreate("Periodic", pdMS_TO_TICKS(1000), pdTRUE, 
                                   (void*)0, vPeriodicTimerCallback);
    
    /* Create One-shot Timer (5 seconds) */
    xOneShotTimer = xTimerCreate("OneShot", pdMS_TO_TICKS(5000), pdFALSE, 
                                  (void*)1, vOneShotTimerCallback);
    
    /* Create Heartbeat Timer (200ms, auto-reload) */
    xHeartbeatTimer = xTimerCreate("Heartbeat", pdMS_TO_TICKS(200), pdTRUE, 
                                    (void*)2, vHeartbeatCallback);
    
    if (xPeriodicTimer == NULL || xOneShotTimer == NULL || xHeartbeatTimer == NULL) {
        printf("ERROR: Failed to create timers!\r\n");
        Error_Handler();
    }
    
    /* Start timers */
    xTimerStart(xPeriodicTimer, 0);
    xTimerStart(xOneShotTimer, 0);
    xTimerStart(xHeartbeatTimer, 0);
    
    printf("All timers started!\r\n\r\n");
    
    vTaskStartScheduler();
    
    while (1) {}
}

void vPeriodicTimerCallback(TimerHandle_t xTimer) {
    periodic_count++;
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    printf("[PERIODIC] Count: %lu | Tick: %lu\r\n", 
           periodic_count, (unsigned long)xTaskGetTickCount());
}

void vOneShotTimerCallback(TimerHandle_t xTimer) {
    oneshot_count++;
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);
    printf("[ONE-SHOT] Fired! Count: %lu | Tick: %lu\r\n", 
           oneshot_count, (unsigned long)xTaskGetTickCount());
    
    vTaskDelay(pdMS_TO_TICKS(100));
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_SET);
    
    /* Restart one-shot timer */
    xTimerStart(xTimer, 0);
}

void vHeartbeatCallback(TimerHandle_t xTimer) {
    heartbeat_count++;
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_15);
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
    
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
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
    
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15, GPIO_PIN_SET);
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

void Error_Handler(void) {
    __disable_irq();
    while (1) {}
}
