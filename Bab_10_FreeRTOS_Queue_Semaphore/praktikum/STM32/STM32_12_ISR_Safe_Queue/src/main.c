/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 26-ISR_Safe_Queue
 * 
 * JUDUL: ISR-Safe Queue Operations Demo
 * 
 * DESKRIPSI:
 * Demo penggunaan xQueueSendFromISR() untuk mengirim data dari ISR ke task.
 * Timer interrupt mengirim data ke queue, task menerima dan memproses.
 * 
 * ============================================================================
 * ARSITEKTUR PROGRAM
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                    ISR SAFE QUEUE DEMO                             │
 *    ├─────────────────────────────────────────────────────────────────────┤
 *    │                                                                     │
 *    │   ┌─────────────────────────────────────────────────────────────┐   │
 *    │   │                      TIM2 ISR                               │   │
 *    │   │                   Period: 100ms                             │   │
 *    │   │                                                             │   │
 *    │   │   void TIM2_IRQHandler(void) {                              │   │
 *    │   │       // Capture data                                       │   │
 *    │   │       event.value = counter++;                              │   │
 *    │   │       event.timestamp = xTaskGetTickCountFromISR();         │   │
 *    │   │                                                             │   │
 *    │   │       // Send ke queue - FromISR version!                   │   │
 *    │   │       BaseType_t xWoken = pdFALSE;                          │   │
 *    │   │       xQueueSendFromISR(queue, &event, &xWoken);            │   │
 *    │   │                                                             │   │
 *    │   │       // Yield jika higher priority task ready              │   │
 *    │   │       portYIELD_FROM_ISR(xWoken);                           │   │
 *    │   │   }                                                         │   │
 *    │   │                                                             │   │
 *    │   └────────────────────────────┬────────────────────────────────┘   │
 *    │                                │                                    │
 *    │                                │ xQueueSendFromISR                  │
 *    │                                ▼                                    │
 *    │                         ┌──────────────┐                            │
 *    │                         │    QUEUE     │                            │
 *    │                         │  Depth: 5    │                            │
 *    │                         └──────┬───────┘                            │
 *    │                                │                                    │
 *    │                                │ xQueueReceive                      │
 *    │                                ▼                                    │
 *    │   ┌─────────────────────────────────────────────────────────────┐   │
 *    │   │                    vReceiverTask                            │   │
 *    │   │                   Priority: HIGH                            │   │
 *    │   │                                                             │   │
 *    │   │   - Block on queue                                          │   │
 *    │   │   - Receive event dari ISR                                  │   │
 *    │   │   - Process dan print                                       │   │
 *    │   │   - Toggle LED                                              │   │
 *    │   │                                                             │   │
 *    │   └─────────────────────────────────────────────────────────────┘   │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * FromISR API PATTERN
 * ============================================================================
 * 
 *    void SomeInterruptHandler(void)
 *    {
 *        BaseType_t xHigherPriorityTaskWoken = pdFALSE;  // WAJIB init!
 *        
 *        // Operasi FreeRTOS...
 *        xQueueSendFromISR(queue, &data, &xHigherPriorityTaskWoken);
 *        xSemaphoreGiveFromISR(sem, &xHigherPriorityTaskWoken);
 *        vTaskNotifyGiveFromISR(task, &xHigherPriorityTaskWoken);
 *        
 *        // Di AKHIR ISR - request context switch jika needed
 *        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
 *    }
 * 
 * ============================================================================
 * EXPECTED OUTPUT
 * ============================================================================
 * 
 *    === ISR Safe Queue Demo ===
 *    
 *    [0.100s] ISR: Sent event #1 to queue
 *    [0.100s] TASK: Received event #1 (timestamp: 100)
 *    
 *    [0.200s] ISR: Sent event #2 to queue
 *    [0.200s] TASK: Received event #2 (timestamp: 200)
 *    ...
 * 
 * ============================================================================
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * EVENT STRUCTURE
 * ============================================================================ */

typedef struct {
    uint32_t   eventNumber;
    TickType_t timestamp;
    uint16_t   adcValue;
} ISREvent_t;

/* ============================================================================
 * GLOBAL VARIABLES
 * ============================================================================ */

static TIM_HandleTypeDef htim2;
static UART_HandleTypeDef huart1;
static ADC_HandleTypeDef hadc1;

static QueueHandle_t xISRQueue = NULL;
static TaskHandle_t xReceiverTaskHandle = NULL;

static volatile uint32_t ulEventCounter = 0;

/* ============================================================================
 * PROTOTYPE
 * ============================================================================ */

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART_Init(void);
static void ADC_Init(void);
static void TIM2_Init(void);
static void UART_SendString(const char *str);

static void vReceiverTask(void *pvParameters);

/* ============================================================================
 * TIMER ISR - Menggunakan FromISR API
 * ============================================================================ */

/**
 * @brief TIM2 ISR - Mengirim data ke queue menggunakan FromISR API
 * 
 * CATATAN PENTING:
 * ═══════════════════════════════════════════════════════════════════════════
 * 
 *    1. HARUS gunakan versi FromISR dari semua FreeRTOS API
 *    2. HARUS deklarasi xHigherPriorityTaskWoken dan init ke pdFALSE
 *    3. HARUS panggil portYIELD_FROM_ISR() di akhir ISR
 *    4. TIDAK BOLEH block (timeout selalu 0)
 * 
 * ═══════════════════════════════════════════════════════════════════════════
 */
void TIM2_IRQHandler(void)
{
    if(__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_UPDATE) != RESET)
    {
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
        
        /*
         * STEP 1: Initialize xHigherPriorityTaskWoken
         * 
         * HARUS pdFALSE! Akan di-set pdTRUE oleh FreeRTOS jika
         * operasi menyebabkan task dengan priority lebih tinggi
         * menjadi ready.
         */
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        
        ulEventCounter++;
        
        /* Baca ADC cepat */
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, 1);
        uint16_t adcValue = HAL_ADC_GetValue(&hadc1);
        HAL_ADC_Stop(&hadc1);
        
        /*
         * STEP 2: Buat event
         * 
         * Gunakan xTaskGetTickCountFromISR() untuk mendapat tick
         * dari context ISR.
         */
        ISREvent_t event = {
            .eventNumber = ulEventCounter,
            .timestamp = xTaskGetTickCountFromISR(),
            .adcValue = adcValue
        };
        
        /*
         * STEP 3: Kirim ke queue menggunakan FromISR version
         * 
         * xQueueSendFromISR TIDAK AKAN BLOCK!
         * Parameter ke-3: pointer ke xHigherPriorityTaskWoken
         */
        BaseType_t result = xQueueSendFromISR(xISRQueue, &event, 
                                              &xHigherPriorityTaskWoken);
        
        if(result != pdPASS)
        {
            /* Queue penuh! Event hilang. */
            /* Di production: increment error counter */
        }
        
        /*
         * STEP 4: Yield jika higher priority task siap
         * 
         * portYIELD_FROM_ISR() akan trigger context switch jika
         * xHigherPriorityTaskWoken == pdTRUE
         * 
         * HARUS dipanggil di AKHIR ISR!
         */
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

/* ============================================================================
 * RECEIVER TASK
 * ============================================================================ */

static void vReceiverTask(void *pvParameters)
{
    (void)pvParameters;
    
    ISREvent_t event;
    char buffer[100];
    
    UART_SendString("[TASK] Receiver task started, waiting for ISR events...\r\n\r\n");
    
    for(;;)
    {
        /*
         * Task bisa BLOCK karena ini TASK context (bukan ISR)
         * Gunakan API normal (bukan FromISR)
         */
        if(xQueueReceive(xISRQueue, &event, portMAX_DELAY) == pdPASS)
        {
            /* Toggle LED */
            HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
            
            float seconds = (float)event.timestamp / 1000.0f;
            
            snprintf(buffer, sizeof(buffer),
                     "[%.3fs] TASK: Event #%lu received (ADC=%u)\r\n",
                     seconds, event.eventNumber, event.adcValue);
            UART_SendString(buffer);
        }
    }
}

/* ============================================================================
 * KONFIGURASI HARDWARE
 * ============================================================================ */

static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
    
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
    
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
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

static void ADC_Init(void)
{
    __HAL_RCC_ADC1_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    HAL_ADC_Init(&hadc1);
    
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_7CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
}

static void TIM2_Init(void)
{
    __HAL_RCC_TIM2_CLK_ENABLE();
    
    /* 72MHz / 7200 = 10kHz, 1000 counts = 100ms */
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 7199;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 999;  /* 100ms */
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_Base_Init(&htim2);
    
    /* Priority >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY (5) */
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
    ADC_Init();
    TIM2_Init();
    
    UART_SendString("\r\n================================\r\n");
    UART_SendString("26-ISR_Safe_Queue\r\n");
    UART_SendString("STM32F103 + FreeRTOS\r\n");
    UART_SendString("================================\r\n\r\n");
    UART_SendString("=== ISR Safe Queue Demo ===\r\n\r\n");
    UART_SendString("TIM2 ISR sends events to queue every 100ms\r\n");
    UART_SendString("Receiver task processes events\r\n\r\n");
    
    /* Buat queue untuk ISR → Task communication */
    xISRQueue = xQueueCreate(ISR_QUEUE_LENGTH, sizeof(ISREvent_t));
    
    if(xISRQueue == NULL)
    {
        UART_SendString("ERROR: Failed to create queue!\r\n");
        while(1);
    }
    
    /* Buat receiver task dengan priority tinggi */
    xTaskCreate(vReceiverTask, "Receiver", TASK_STACK, NULL,
                configMAX_PRIORITIES - 2, &xReceiverTaskHandle);
    
    /* Start timer interrupt */
    HAL_TIM_Base_Start_IT(&htim2);
    
    /* Start scheduler */
    vTaskStartScheduler();
    
    for(;;);
}
