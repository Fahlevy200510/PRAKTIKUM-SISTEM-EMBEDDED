/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 32-ISR_Stream_Buffer
 * 
 * DESKRIPSI:
 * Demo penggunaan Stream Buffer untuk komunikasi ISR ke Task.
 * Timer interrupt mensimulasikan penerimaan data dari peripheral,
 * kemudian mengirim ke task melalui stream buffer.
 * 
 * ============================================================================
 * ARSITEKTUR SISTEM
 * ============================================================================
 * 
 *    ┌──────────────────────────────────────────────────────────────────────┐
 *    │                    ISR STREAM BUFFER DEMO                            │
 *    │                                                                      │
 *    │   Hardware Timer                          Processor Task             │
 *    │   ┌─────────────────┐                     ┌─────────────────┐        │
 *    │   │  TIM2 @ 100Hz   │                     │  Process data   │        │
 *    │   │                 │                     │  from ISR       │        │
 *    │   │  Simulate       │                     │                 │        │
 *    │   │  ADC/sensor     │                     │  - Validate     │        │
 *    │   │  data capture   │                     │  - Calculate    │        │
 *    │   └────────┬────────┘                     │  - Display      │        │
 *    │            │                              └────────▲────────┘        │
 *    │            │ ISR                                   │                 │
 *    │            ▼                                       │ Task            │
 *    │   ┌───────────────────────────────────────────────┴┐                 │
 *    │   │  xStreamBufferSendFromISR()                    │                 │
 *    │   │                                                │                 │
 *    │   │  ┌──────────────────────────────────────────┐  │                 │
 *    │   │  │▓▓▓▓▓▓▓▓▓▓▓▓▓▓│░░░░░░░░░░░░░░░░░░░░░░░░░░│  │                 │
 *    │   │  └──────────────────────────────────────────┘  │                 │
 *    │   │     ↑                                          │                 │
 *    │   │  ISR adds data                                 │                 │
 *    │   │                  xStreamBufferReceive()        │                 │
 *    │   └────────────────────────────────────────────────┘                 │
 *    │                                                                      │
 *    └──────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * DATA FLOW
 * ============================================================================
 * 
 *    TIM2 IRQ                          Task
 *    ════════                          ════
 *       │                                │
 *       │ [10ms] Generate sample         │ (blocked, waiting trigger)
 *       ├───► Send 2 bytes ──────────────│
 *       │                                │
 *       │ [10ms] Generate sample         │ 
 *       ├───► Send 2 bytes ──────────────│
 *       │                                │
 *       │ [10ms] Generate sample         │
 *       ├───► Send 2 bytes ──────────────│
 *       │                                │
 *       │ [10ms] Generate sample         │
 *       ├───► Send 2 bytes ──────────────│
 *       │     (buffer now has 8 bytes)   │
 *       │                                ├──► UNBLOCK! Process 8 bytes
 *       │                                │    Display: "Samples: 4"
 *       ▼                                ▼
 * 
 * ============================================================================
 * SAMPLE DATA FORMAT
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │  Each sample = 2 bytes:                                            │
 *    │                                                                     │
 *    │  ┌────────────────┬────────────────┐                               │
 *    │  │  Sample ID     │  Sample Value  │                               │
 *    │  │   (uint8_t)    │   (uint8_t)    │                               │
 *    │  └────────────────┴────────────────┘                               │
 *    │                                                                     │
 *    │  Trigger level = 8 bytes = 4 samples                               │
 *    │  Task processes 4 samples at a time for efficiency                 │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * CONTOH OUTPUT SERIAL
 * ============================================================================
 * 
 *    === ISR STREAM BUFFER DEMO ===
 *    
 *    [ISR] Sample #1: value=127
 *    [ISR] Sample #2: value=134
 *    [ISR] Sample #3: value=141
 *    [ISR] Sample #4: value=148
 *    [Task] ▶ Received 8 bytes (4 samples)
 *           Sample 1: ID=1, Value=127
 *           Sample 2: ID=2, Value=134
 *           Sample 3: ID=3, Value=141
 *           Sample 4: ID=4, Value=148
 *           Average: 137
 * 
 * ============================================================================
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

/* Sample data structure - 2 bytes each */
typedef struct __attribute__((packed)) {
    uint8_t ucId;
    uint8_t ucValue;
} SampleData_t;

/* ============================================================================
 * GLOBAL VARIABLES
 * ============================================================================ */
UART_HandleTypeDef huart1;
TIM_HandleTypeDef htim2;
StreamBufferHandle_t xIsrBuffer = NULL;

/* ISR statistics */
static volatile uint32_t ulIsrCount = 0;
static volatile uint32_t ulBytesSentFromIsr = 0;
static volatile uint32_t ulYieldCount = 0;

/* Simulated sensor value */
static volatile uint8_t ucSimulatedValue = 128;

/* ============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================ */
static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART1_Init(void);
static void TIM2_Init(void);
static void prvProcessorTask(void *pvParameters);
static void prvMonitorTask(void *pvParameters);
static void UART_SendString(const char *str);

/* ============================================================================
 * UART HELPER
 * ============================================================================ */
static void UART_SendString(const char *str)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}

/* ============================================================================
 * TIM2 INTERRUPT HANDLER
 * Simulates receiving sensor data and sends to stream buffer
 * ============================================================================ */
void TIM2_IRQHandler(void)
{
    if (__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_UPDATE) != RESET)
    {
        __HAL_TIM_CLEAR_IT(&htim2, TIM_IT_UPDATE);
        
        ulIsrCount++;
        
        /* Generate simulated sensor sample */
        SampleData_t sample;
        sample.ucId = (uint8_t)(ulIsrCount & 0xFF);
        sample.ucValue = ucSimulatedValue;
        
        /* Simulate changing sensor value */
        ucSimulatedValue += 7;
        if (ucSimulatedValue > 250) ucSimulatedValue = 100;
        
        /*
         * SEND FROM ISR
         * 
         * xStreamBufferSendFromISR():
         *   - ISR-safe version of xStreamBufferSend
         *   - No blocking! Returns immediately
         *   - pxHigherPriorityTaskWoken indicates if context switch needed
         */
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        
        size_t xBytesSent = xStreamBufferSendFromISR(xIsrBuffer,
                                                      &sample,
                                                      sizeof(sample),
                                                      &xHigherPriorityTaskWoken);
        
        if (xBytesSent == sizeof(sample))
        {
            ulBytesSentFromIsr += xBytesSent;
        }
        
        /* Toggle LED to show ISR activity */
        if ((ulIsrCount % 10) == 0)
        {
            HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
        }
        
        /*
         * YIELD FROM ISR IF NEEDED
         * 
         * If sending to buffer caused a higher priority task to unblock,
         * we should yield to that task immediately
         */
        if (xHigherPriorityTaskWoken == pdTRUE)
        {
            ulYieldCount++;
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

/* ============================================================================
 * MAIN FUNCTION
 * ============================================================================ */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    UART1_Init();
    
    char msg[150];
    
    UART_SendString("\r\n\r\n");
    UART_SendString("╔══════════════════════════════════════════════════════════╗\r\n");
    UART_SendString("║          32. ISR TO TASK STREAM BUFFER                   ║\r\n");
    UART_SendString("╚══════════════════════════════════════════════════════════╝\r\n\r\n");
    
    UART_SendString("[Config] Timer: TIM2 @ 100Hz (10ms period)\r\n");
    snprintf(msg, sizeof(msg), "[Config] Buffer size: %d bytes, Trigger: %d bytes\r\n",
             ISR_BUFFER_SIZE, TRIGGER_LEVEL);
    UART_SendString(msg);
    snprintf(msg, sizeof(msg), "[Config] Sample size: %d bytes (trigger = %d samples)\r\n\r\n",
             (int)sizeof(SampleData_t), TRIGGER_LEVEL / (int)sizeof(SampleData_t));
    UART_SendString(msg);
    
    /* Create stream buffer for ISR communication */
    xIsrBuffer = xStreamBufferCreate(ISR_BUFFER_SIZE, TRIGGER_LEVEL);
    
    if (xIsrBuffer == NULL)
    {
        UART_SendString("[ERROR] Failed to create ISR buffer!\r\n");
        while (1);
    }
    UART_SendString("[OK] ISR Stream Buffer created\r\n");
    
    /* Create tasks */
    xTaskCreate(prvProcessorTask, "Processor", TASK_STACK, NULL, 3, NULL);
    xTaskCreate(prvMonitorTask, "Monitor", TASK_STACK, NULL, 1, NULL);
    
    /* Initialize and start timer AFTER buffer is ready */
    TIM2_Init();
    
    UART_SendString("[INFO] Starting scheduler...\r\n\r\n");
    UART_SendString("────────────────────────────────────────────────────────────\r\n");
    
    vTaskStartScheduler();
    
    while (1);
}

/* ============================================================================
 * PROCESSOR TASK
 * Receives and processes samples from ISR
 * ============================================================================ */
static void prvProcessorTask(void *pvParameters)
{
    (void)pvParameters;
    
    SampleData_t samples[16];  /* Can receive multiple samples */
    char msg[150];
    uint32_t ulBatchNum = 0;
    
    for (;;)
    {
        /*
         * RECEIVE FROM BUFFER
         * 
         * Will block until TRIGGER_LEVEL bytes available
         * This batches processing for efficiency
         */
        size_t xBytesReceived = xStreamBufferReceive(xIsrBuffer,
                                                      samples,
                                                      sizeof(samples),
                                                      portMAX_DELAY);
        
        if (xBytesReceived > 0)
        {
            ulBatchNum++;
            
            uint32_t ulNumSamples = xBytesReceived / sizeof(SampleData_t);
            
            snprintf(msg, sizeof(msg), 
                     "\r\n[Task] ▶ Batch #%lu: Received %u bytes (%lu samples)\r\n",
                     ulBatchNum, (unsigned)xBytesReceived, ulNumSamples);
            UART_SendString(msg);
            
            /* Process samples */
            uint32_t ulSum = 0;
            uint8_t ucMin = 255, ucMax = 0;
            
            for (uint32_t i = 0; i < ulNumSamples; i++)
            {
                ulSum += samples[i].ucValue;
                if (samples[i].ucValue < ucMin) ucMin = samples[i].ucValue;
                if (samples[i].ucValue > ucMax) ucMax = samples[i].ucValue;
                
                /* Print first few samples */
                if (i < 4)
                {
                    snprintf(msg, sizeof(msg), "        Sample[%lu]: ID=%u, Value=%u\r\n",
                             i, samples[i].ucId, samples[i].ucValue);
                    UART_SendString(msg);
                }
            }
            
            if (ulNumSamples > 4)
            {
                snprintf(msg, sizeof(msg), "        ... (%lu more samples)\r\n",
                         ulNumSamples - 4);
                UART_SendString(msg);
            }
            
            /* Statistics */
            uint32_t ulAverage = ulSum / ulNumSamples;
            snprintf(msg, sizeof(msg), 
                     "        Statistics: Avg=%lu, Min=%u, Max=%u\r\n",
                     ulAverage, ucMin, ucMax);
            UART_SendString(msg);
        }
    }
}

/* ============================================================================
 * MONITOR TASK
 * Reports ISR and buffer statistics
 * ============================================================================ */
static void prvMonitorTask(void *pvParameters)
{
    (void)pvParameters;
    
    char msg[150];
    const TickType_t xPeriod = pdMS_TO_TICKS(5000);
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
        
        size_t xSpaceAvailable = xStreamBufferSpacesAvailable(xIsrBuffer);
        
        UART_SendString("\r\n┌─── ISR STREAM BUFFER STATISTICS ────────────────────────┐\r\n");
        
        snprintf(msg, sizeof(msg), "│  ISR triggers: %lu                                     │\r\n",
                 ulIsrCount);
        UART_SendString(msg);
        
        snprintf(msg, sizeof(msg), "│  Bytes sent from ISR: %lu                              │\r\n",
                 ulBytesSentFromIsr);
        UART_SendString(msg);
        
        snprintf(msg, sizeof(msg), "│  Context switch yields: %lu                            │\r\n",
                 ulYieldCount);
        UART_SendString(msg);
        
        snprintf(msg, sizeof(msg), "│  Buffer space: %u/%d bytes                             │\r\n",
                 (unsigned)xSpaceAvailable, ISR_BUFFER_SIZE);
        UART_SendString(msg);
        
        UART_SendString("└──────────────────────────────────────────────────────────┘\r\n");
    }
}

/* ============================================================================
 * TIMER INITIALIZATION
 * ============================================================================ */
static void TIM2_Init(void)
{
    __HAL_RCC_TIM2_CLK_ENABLE();
    
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = TIM2_PRESCALER;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = TIM2_PERIOD;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_Base_Init(&htim2);
    
    /* Configure interrupt priority - must be within FreeRTOS range */
    HAL_NVIC_SetPriority(TIM2_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
    
    /* Start timer with interrupt */
    HAL_TIM_Base_Start_IT(&htim2);
    
    UART_SendString("[OK] TIM2 started @ 100Hz\r\n");
}

/* ============================================================================
 * SYSTEM CONFIGURATION
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
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = LED_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);
    
    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_SET);
}

static void UART1_Init(void)
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

/* ============================================================================
 * FreeRTOS HOOKS
 * ============================================================================ */
void vApplicationMallocFailedHook(void)
{
    UART_SendString("[FATAL] Malloc failed!\r\n");
    taskDISABLE_INTERRUPTS();
    for (;;);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    char msg[50];
    snprintf(msg, sizeof(msg), "[FATAL] Stack overflow: %s\r\n", pcTaskName);
    UART_SendString(msg);
    taskDISABLE_INTERRUPTS();
    for (;;);
}
