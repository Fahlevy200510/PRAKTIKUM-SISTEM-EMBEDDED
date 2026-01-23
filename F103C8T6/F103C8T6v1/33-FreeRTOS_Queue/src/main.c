/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : FreeRTOS Queue - Inter-Task Communication
 * @author         : Based on "Mastering STM32" by Carmine Noviello
 * @date           : 2026-01-22
 ******************************************************************************
 * @description
 * Program 33: FreeRTOS Queue untuk Inter-Task Communication
 * 
 * Hardware Requirements:
 * - STM32F103C8T6 Blue Pill
 * - Potentiometer di PA0 (ADC1_IN0)
 * - LED di PC13 (Queue full indicator)
 * - USB-UART adapter di PA9/PA10
 * 
 * Features:
 * - Producer task: Membaca ADC setiap 500ms
 * - Consumer task: Menerima data dari queue dan log via UART
 * - Queue length: 10 items
 * - Queue full detection dengan LED indicator
 * 
 * Pin Configuration:
 * - PA0  : ADC1_IN0 (Potentiometer input)
 * - PA9  : USART1_TX
 * - PA10 : USART1_RX
 * - PC13 : LED (Queue full indicator - active LOW)
 ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include <string.h>

/* Private defines */
#define QUEUE_LENGTH    10
#define ITEM_SIZE       sizeof(ADC_Data_t)

/* Data structure untuk queue */
typedef struct {
    uint32_t timestamp;    // Tick count saat data diambil
    uint16_t adc_value;    // Nilai ADC (0-4095)
    float voltage;         // Voltage (0-3.3V)
} ADC_Data_t;

/* Private variables */
UART_HandleTypeDef huart1;
ADC_HandleTypeDef hadc1;

QueueHandle_t xADCQueue = NULL;

/* Task handles */
TaskHandle_t xProducerTaskHandle = NULL;
TaskHandle_t xConsumerTaskHandle = NULL;

/* Statistics variables */
static uint32_t produced_count = 0;
static uint32_t consumed_count = 0;
static uint32_t queue_full_count = 0;

/* Function prototypes */
void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART_Init(void);
static void ADC_Init(void);
void vProducerTask(void *pvParameters);
void vConsumerTask(void *pvParameters);

/* Helper functions */
int _write(int file, char *ptr, int len) {
    HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}

/**
 * @brief  Main program
 */
int main(void) {
    /* Reset of all peripherals, Initializes the Flash interface and Systick */
    HAL_Init();
    
    /* Configure the system clock to 72 MHz */
    SystemClock_Config();
    
    /* Initialize all configured peripherals */
    GPIO_Init();
    UART_Init();
    ADC_Init();
    
    /* Print startup message */
    printf("\r\n=== FreeRTOS Queue Demo ===\r\n");
    printf("Program 33: Inter-Task Communication\r\n");
    printf("Queue Length: %d items\r\n", QUEUE_LENGTH);
    printf("Producer: ADC sampling @ 2Hz\r\n");
    printf("Consumer: UART logging\r\n\r\n");
    
    /* Create the queue */
    xADCQueue = xQueueCreate(QUEUE_LENGTH, ITEM_SIZE);
    
    if (xADCQueue == NULL) {
        printf("ERROR: Failed to create queue!\r\n");
        Error_Handler();
    }
    
    printf("Queue created successfully\r\n\r\n");
    
    /* Create Producer Task (ADC sampling) - Priority 2 */
    xTaskCreate(vProducerTask, 
                "Producer", 
                256, 
                NULL, 
                2, 
                &xProducerTaskHandle);
    
    /* Create Consumer Task (UART logging) - Priority 1 */
    xTaskCreate(vConsumerTask, 
                "Consumer", 
                256, 
                NULL, 
                1, 
                &xConsumerTaskHandle);
    
    /* Start the scheduler */
    printf("Starting FreeRTOS scheduler...\r\n\r\n");
    vTaskStartScheduler();
    
    /* Should never reach here */
    while (1) {
        Error_Handler();
    }
}

/**
 * @brief  Producer Task - Samples ADC and sends to queue
 * @param  pvParameters: Not used
 */
void vProducerTask(void *pvParameters) {
    ADC_Data_t adc_data;
    BaseType_t xStatus;
    TickType_t xLastWakeTime;
    
    /* Initialize the xLastWakeTime variable with the current time */
    xLastWakeTime = xTaskGetTickCount();
    
    printf("[PRODUCER] Task started\r\n");
    
    for (;;) {
        /* Start ADC conversion */
        HAL_ADC_Start(&hadc1);
        
        /* Wait for conversion complete */
        if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK) {
            /* Get ADC value */
            adc_data.adc_value = HAL_ADC_GetValue(&hadc1);
            adc_data.voltage = (adc_data.adc_value * 3.3f) / 4095.0f;
            adc_data.timestamp = xTaskGetTickCount();
            
            /* Try to send to queue (non-blocking) */
            xStatus = xQueueSend(xADCQueue, &adc_data, 0);
            
            if (xStatus == pdPASS) {
                produced_count++;
                
                /* Turn off LED (queue not full) */
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
            } else {
                /* Queue is full! */
                queue_full_count++;
                
                /* Turn on LED (queue full) */
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
                
                printf("[PRODUCER] WARNING: Queue full! Data lost.\r\n");
            }
        }
        
        HAL_ADC_Stop(&hadc1);
        
        /* Wait for 500ms (2Hz sampling rate) */
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));
    }
}

/**
 * @brief  Consumer Task - Receives from queue and logs via UART
 * @param  pvParameters: Not used
 */
void vConsumerTask(void *pvParameters) {
    ADC_Data_t received_data;
    BaseType_t xStatus;
    UBaseType_t uxMessagesWaiting;
    
    printf("[CONSUMER] Task started\r\n\r\n");
    
    for (;;) {
        /* Wait for data in queue (block indefinitely) */
        xStatus = xQueueReceive(xADCQueue, &received_data, portMAX_DELAY);
        
        if (xStatus == pdPASS) {
            consumed_count++;
            
            /* Get number of messages waiting in queue */
            uxMessagesWaiting = uxQueueMessagesWaiting(xADCQueue);
            
            /* Print received data */
            printf("[%lu] ADC: %4u | %.3fV | Queue: %u/%d | P:%lu C:%lu Lost:%lu\r\n",
                   received_data.timestamp,
                   received_data.adc_value,
                   received_data.voltage,
                   (unsigned int)uxMessagesWaiting,
                   QUEUE_LENGTH,
                   produced_count,
                   consumed_count,
                   queue_full_count);
            
            /* Simulate processing time (variable delay based on ADC value) */
            uint32_t process_time = 100 + (received_data.adc_value / 41); // 100-200ms
            vTaskDelay(pdMS_TO_TICKS(process_time));
        }
    }
}

/**
 * @brief  System Clock Configuration to 72 MHz
 */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* Configure the main internal regulator output voltage */
    __HAL_RCC_PWR_CLK_ENABLE();

    /* Initializes the CPU, AHB and APB busses clocks */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;  // 8MHz * 9 = 72MHz
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /* Initializes the CPU, AHB and APB busses clocks */
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

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* Configure PC13 as output (LED - Queue full indicator) */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    /* LED OFF initially */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}

/**
 * @brief  UART1 Initialization (115200 baud, 8N1)
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
    
    /* Configure UART pins PA9 (TX) and PA10 (RX) */
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
 * @brief  ADC1 Initialization (PA0 - Channel 0)
 */
static void ADC_Init(void) {
    ADC_ChannelConfTypeDef sConfig = {0};
    
    __HAL_RCC_ADC1_CLK_ENABLE();
    
    /* Configure ADC */
    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    
    if (HAL_ADC_Init(&hadc1) != HAL_OK) {
        Error_Handler();
    }
    
    /* Configure ADC Channel 0 (PA0) */
    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;
    
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        Error_Handler();
    }
    
    /* Configure PA0 as analog input */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/**
 * @brief  This function is executed in case of error occurrence
 */
void Error_Handler(void) {
    __disable_irq();
    while (1) {
        /* LED blink fast to indicate error */
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_Delay(100);
    }
}

/* FreeRTOS hooks */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    printf("ERROR: Stack overflow in task %s\r\n", pcTaskName);
    Error_Handler();
}

void vApplicationMallocFailedHook(void) {
    printf("ERROR: Malloc failed!\r\n");
    Error_Handler();
}
