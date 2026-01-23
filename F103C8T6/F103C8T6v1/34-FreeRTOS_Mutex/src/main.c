/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : FreeRTOS Mutex - Mutual Exclusion & Priority Inheritance
 * @author         : Based on "Mastering STM32" by Carmine Noviello
 * @date           : 2026-01-22
 ******************************************************************************
 * @description
 * Program 34: FreeRTOS Mutex untuk Protect Shared Resource
 * 
 * Hardware Requirements:
 * - STM32F103C8T6 Blue Pill
 * - 2x LED di PC13 & PC14
 * - USB-UART adapter di PA9/PA10 (shared resource)
 * 
 * Features:
 * - 3 Tasks dengan priority berbeda
 * - UART sebagai shared resource (protected by mutex)
 * - Demonstrasi priority inheritance
 * - Deadlock prevention
 * 
 * Tasks:
 * - High Priority Task  : Periodic reporting (2s)
 * - Medium Priority Task: Busy computing (1s)
 * - Low Priority Task   : Background logging (500ms)
 * 
 * Pin Configuration:
 * - PA9  : USART1_TX (shared resource)
 * - PA10 : USART1_RX
 * - PC13 : LED High Priority Task
 * - PC14 : LED Medium Priority Task
 ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdio.h>
#include <string.h>

/* Private variables */
UART_HandleTypeDef huart1;

SemaphoreHandle_t xUARTMutex = NULL;

/* Task handles */
TaskHandle_t xHighPriorityTask = NULL;
TaskHandle_t xMediumPriorityTask = NULL;
TaskHandle_t xLowPriorityTask = NULL;

/* Statistics */
static uint32_t high_task_count = 0;
static uint32_t medium_task_count = 0;
static uint32_t low_task_count = 0;
static uint32_t mutex_timeout_count = 0;

/* Function prototypes */
void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART_Init(void);
void vHighPriorityTask(void *pvParameters);
void vMediumPriorityTask(void *pvParameters);
void vLowPriorityTask(void *pvParameters);
void UART_Print_Protected(const char *message);

/* Helper function for protected UART print */
void UART_Print_Protected(const char *message) {
    /* Take mutex (wait maximum 1 second) */
    if (xSemaphoreTake(xUARTMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        /* Critical section - use UART */
        HAL_UART_Transmit(&huart1, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
        
        /* Give mutex back */
        xSemaphoreGive(xUARTMutex);
    } else {
        /* Mutex timeout */
        mutex_timeout_count++;
    }
}

/**
 * @brief  Main program
 */
int main(void) {
    /* HAL Init */
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    UART_Init();
    
    /* Print startup message (before RTOS starts) */
    const char *msg = "\r\n=== FreeRTOS Mutex Demo ===\r\n"
                      "Program 34: Mutual Exclusion & Priority Inheritance\r\n\r\n"
                      "Tasks:\r\n"
                      "  HIGH (P3)   : Periodic report @ 2s\r\n"
                      "  MEDIUM (P2) : Busy computing @ 1s\r\n"
                      "  LOW (P1)    : Background log @ 500ms\r\n\r\n"
                      "UART is shared resource (protected by mutex)\r\n\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    
    /* Create mutex with priority inheritance */
    xUARTMutex = xSemaphoreCreateMutex();
    
    if (xUARTMutex == NULL) {
        const char *err = "ERROR: Failed to create mutex!\r\n";
        HAL_UART_Transmit(&huart1, (uint8_t*)err, strlen(err), HAL_MAX_DELAY);
        Error_Handler();
    }
    
    const char *ok = "Mutex created successfully\r\n\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t*)ok, strlen(ok), HAL_MAX_DELAY);
    
    /* Create tasks */
    xTaskCreate(vHighPriorityTask, "HighTask", 256, NULL, 3, &xHighPriorityTask);
    xTaskCreate(vMediumPriorityTask, "MediumTask", 256, NULL, 2, &xMediumPriorityTask);
    xTaskCreate(vLowPriorityTask, "LowTask", 256, NULL, 1, &xLowPriorityTask);
    
    /* Start scheduler */
    const char *start = "Starting scheduler...\r\n\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t*)start, strlen(start), HAL_MAX_DELAY);
    
    vTaskStartScheduler();
    
    /* Should never reach here */
    while (1) {
        Error_Handler();
    }
}

/**
 * @brief  High Priority Task - Periodic reporting
 */
void vHighPriorityTask(void *pvParameters) {
    char buffer[100];
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for (;;) {
        high_task_count++;
        
        /* LED ON */
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
        
        /* Create message */
        snprintf(buffer, sizeof(buffer), 
                "[HIGH-%lu] Tick:%lu | H:%lu M:%lu L:%lu | Timeout:%lu\r\n",
                high_task_count,
                (unsigned long)xTaskGetTickCount(),
                high_task_count,
                medium_task_count,
                low_task_count,
                mutex_timeout_count);
        
        /* Print dengan mutex protection */
        UART_Print_Protected(buffer);
        
        /* LED OFF */
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
        
        /* Wait 2 seconds */
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(2000));
    }
}

/**
 * @brief  Medium Priority Task - Busy computing (no UART access)
 * @note   This task does NOT use UART, showing priority inheritance
 */
void vMediumPriorityTask(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    volatile uint32_t computation = 0;
    
    for (;;) {
        medium_task_count++;
        
        /* LED ON */
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);
        
        /* Simulate heavy computation (no UART, no mutex needed) */
        for (uint32_t i = 0; i < 100000; i++) {
            computation += i;
        }
        
        /* LED OFF */
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_SET);
        
        /* Wait 1 second */
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
    }
}

/**
 * @brief  Low Priority Task - Background logging
 */
void vLowPriorityTask(void *pvParameters) {
    char buffer[80];
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for (;;) {
        low_task_count++;
        
        /* Create log message */
        snprintf(buffer, sizeof(buffer), 
                "[LOW-%lu] Background log | Tick:%lu\r\n",
                low_task_count,
                (unsigned long)xTaskGetTickCount());
        
        /* Print dengan mutex protection */
        UART_Print_Protected(buffer);
        
        /* Wait 500ms */
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));
    }
}

/**
 * @brief  System Clock Configuration to 72 MHz
 */
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
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief  GPIO Initialization
 */
static void GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* Configure PC13 & PC14 as outputs (LEDs) */
    GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    /* LEDs OFF initially */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13 | GPIO_PIN_14, GPIO_PIN_SET);
}

/**
 * @brief  UART1 Initialization
 */
static void UART_Init(void) {
    __HAL_RCC_USART1_CLK_ENABLE();
    
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (HAL_UART_Init(&huart1) != HAL_OK) {
        Error_Handler();
    }
    
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

/**
 * @brief  Error Handler
 */
void Error_Handler(void) {
    __disable_irq();
    while (1) {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_Delay(100);
    }
}

/* FreeRTOS hooks */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    Error_Handler();
}

void vApplicationMallocFailedHook(void) {
    Error_Handler();
}
