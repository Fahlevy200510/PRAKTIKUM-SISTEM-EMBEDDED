/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 29-Stream_Buffer_Basics
 * 
 * DESKRIPSI:
 * Demo Stream Buffer untuk transfer continuous byte stream antara producer
 * dan consumer task. Berbeda dengan queue yang transfer fixed-size items,
 * stream buffer ideal untuk UART data, audio, atau log messages.
 * 
 * ============================================================================
 * ARSITEKTUR SISTEM
 * ============================================================================
 * 
 *    ┌──────────────────────────────────────────────────────────────────────┐
 *    │                      STREAM BUFFER DEMO                              │
 *    │                                                                      │
 *    │   Producer Task                           Consumer Task              │
 *    │   ┌─────────────────┐                     ┌─────────────────┐        │
 *    │   │  Generate data  │                     │  Process data   │        │
 *    │   │  (sensor log,   │                     │  (print to      │        │
 *    │   │   messages)     │                     │   UART)         │        │
 *    │   └────────┬────────┘                     └────────▲────────┘        │
 *    │            │                                       │                 │
 *    │            │ xStreamBufferSend()    xStreamBufferReceive()           │
 *    │            │                                       │                 │
 *    │            ▼                                       │                 │
 *    │   ┌────────────────────────────────────────────────┴────────┐        │
 *    │   │                    STREAM BUFFER                        │        │
 *    │   │  ┌──────────────────────────────────────────────────┐   │        │
 *    │   │  │ H e l l o   W o r l d ! \n S e n s o r = 1 2 3   │   │        │
 *    │   │  └──────────────────────────────────────────────────┘   │        │
 *    │   │                 (100 bytes capacity)                    │        │
 *    │   │                 (trigger = 10 bytes)                    │        │
 *    │   └─────────────────────────────────────────────────────────┘        │
 *    │                                                                      │
 *    └──────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * FLOW OPERASI
 * ============================================================================
 * 
 *    Timeline:
 *    
 *    Producer                  Stream Buffer              Consumer
 *    ═══════                   ═════════════              ════════
 *       │                           │                         │
 *       │ Send "Temp=25.3\n"        │                         │
 *       │ (10 bytes)                │                         │
 *       ├──────────────────────────►│                         │
 *       │                           │ (bytes < trigger)       │
 *       │                           │                         │ (blocked)
 *       │                           │                         │
 *       │ Send "Humid=80%\n"        │                         │
 *       │ (10 bytes)                │                         │
 *       ├──────────────────────────►│                         │
 *       │                           │ (bytes >= trigger!)     │
 *       │                           ├────────────────────────►│ UNBLOCK!
 *       │                           │                         │
 *       │                           │         Receive all     │
 *       │                           │◄────────────────────────┤
 *       │                           │                         │ Process
 *       ▼                           ▼                         ▼
 * 
 * ============================================================================
 * TRIGGER LEVEL BEHAVIOR
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │  Trigger Level = 10 bytes                                          │
 *    │                                                                     │
 *    │  State 1: 5 bytes in buffer                                        │
 *    │  ┌─────┬─────────────────────────────────────────────────────┐     │
 *    │  │A B C│░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░│     │
 *    │  └─────┴─────────────────────────────────────────────────────┘     │
 *    │  Consumer: BLOCKED (waiting for trigger)                           │
 *    │                                                                     │
 *    │  State 2: 12 bytes in buffer (>= trigger)                          │
 *    │  ┌───────────────────────────────────────────────────────────┐     │
 *    │  │A B C D E F G H I J K L│░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░│     │
 *    │  └───────────────────────────────────────────────────────────┘     │
 *    │  Consumer: UNBLOCKED! Can receive up to 12 bytes                   │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * CONTOH OUTPUT SERIAL
 * ============================================================================
 * 
 *    === STREAM BUFFER BASICS ===
 *    Stream Buffer size: 100 bytes
 *    Trigger level: 10 bytes
 *    
 *    [Producer] Sending 'Msg#001: Hello Stream!'
 *    [Producer] Sent 22 bytes, Total sent: 22
 *    [Consumer] Waiting for data...
 *    [Consumer] Received 22 bytes: 'Msg#001: Hello Stream!'
 *    
 *    [Producer] Sending 'Msg#002: Temperature=25.3'
 *    [Producer] Sent 24 bytes, Total sent: 46
 *    [Consumer] Received 24 bytes: 'Msg#002: Temperature=25.3'
 *    
 *    [Producer] Sending 'Msg#003: Humidity=80%'
 *    [Producer] Sent 21 bytes, Total sent: 67
 *    [Consumer] Received 21 bytes: 'Msg#003: Humidity=80%'
 *    
 *    [Producer] Sending short 'AB' (trigger demo)
 *    [Producer] Sent 2 bytes (below trigger, consumer may timeout)
 *    [Consumer] Timeout - only 2 bytes available
 *    [Consumer] Received 2 bytes: 'AB'
 * 
 * ============================================================================
 * CATATAN PENTING
 * ============================================================================
 * 
 *    1. SINGLE WRITER ONLY: Stream Buffer dirancang untuk 1 producer
 *       Jika multiple producer diperlukan, gunakan mutex atau separate buffers
 * 
 *    2. TRIGGER LEVEL: Set sesuai batch size yang optimal
 *       - Kecil: Lebih responsive, tapi lebih sering context switch
 *       - Besar: Lebih efisien, tapi latency lebih tinggi
 * 
 *    3. PARTIAL READS: Receive dapat return kurang dari yang diminta
 *       Selalu check return value!
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
 * GLOBAL VARIABLES
 * ============================================================================ */
UART_HandleTypeDef huart1;
StreamBufferHandle_t xStreamBuffer = NULL;

/* Statistics */
static uint32_t ulTotalBytesSent = 0;
static uint32_t ulTotalBytesReceived = 0;
static uint32_t ulSendCount = 0;
static uint32_t ulReceiveCount = 0;

/* ============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================ */
static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART1_Init(void);
static void prvProducerTask(void *pvParameters);
static void prvConsumerTask(void *pvParameters);
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
 * MAIN FUNCTION
 * ============================================================================ */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    UART1_Init();
    
    char msg[200];
    
    UART_SendString("\r\n\r\n");
    UART_SendString("╔══════════════════════════════════════════════════════════╗\r\n");
    UART_SendString("║         29. STREAM BUFFER - BYTE STREAM DEMO             ║\r\n");
    UART_SendString("╚══════════════════════════════════════════════════════════╝\r\n\r\n");
    
    /* Display configuration */
    snprintf(msg, sizeof(msg), "[Config] Stream Buffer size: %d bytes\r\n", STREAM_BUFFER_SIZE);
    UART_SendString(msg);
    snprintf(msg, sizeof(msg), "[Config] Trigger level: %d bytes\r\n\r\n", TRIGGER_LEVEL);
    UART_SendString(msg);
    
    /*
     * CREATE STREAM BUFFER
     * 
     * xStreamBufferCreate(size, trigger):
     *   - size: Total buffer capacity in bytes
     *   - trigger: Minimum bytes before receiver unblocks
     */
    xStreamBuffer = xStreamBufferCreate(STREAM_BUFFER_SIZE, TRIGGER_LEVEL);
    
    if (xStreamBuffer == NULL)
    {
        UART_SendString("[ERROR] Failed to create Stream Buffer!\r\n");
        while (1);
    }
    UART_SendString("[OK] Stream Buffer created\r\n\r\n");
    
    /* Create tasks */
    xTaskCreate(prvProducerTask, "Producer", TASK_STACK, NULL, 2, NULL);
    xTaskCreate(prvConsumerTask, "Consumer", TASK_STACK, NULL, 3, NULL);  /* Higher priority */
    xTaskCreate(prvMonitorTask, "Monitor", TASK_STACK, NULL, 1, NULL);
    
    UART_SendString("[INFO] Tasks created. Starting scheduler...\r\n\r\n");
    UART_SendString("────────────────────────────────────────────────────────────\r\n");
    
    vTaskStartScheduler();
    
    while (1);
}

/* ============================================================================
 * PRODUCER TASK
 * Generates messages and sends to Stream Buffer
 * ============================================================================ */
static void prvProducerTask(void *pvParameters)
{
    (void)pvParameters;
    
    char txBuffer[64];
    char msg[120];
    uint32_t ulMsgNum = 0;
    size_t xBytesSent;
    
    const TickType_t xSendDelay = pdMS_TO_TICKS(1000);  /* Send every 1 second */
    
    /* Simulated sensor data patterns */
    const char *patterns[] = {
        "TEMP:%.1f,HUM:%.1f",    /* Temperature, Humidity */
        "ADC:%04d,V:%.2f",       /* ADC reading, Voltage */
        "STATUS:OK,CNT:%lu",     /* Status message */
        "GPS:%.6f,%.6f",         /* Lat, Long */
        "AB"                     /* Short message - trigger demo */
    };
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        ulMsgNum++;
        
        /* Generate message based on pattern */
        int patternIdx = ulMsgNum % 5;
        
        switch (patternIdx)
        {
            case 0:
                snprintf(txBuffer, sizeof(txBuffer), patterns[0], 
                         25.0f + (ulMsgNum % 100) * 0.1f,
                         60.0f + (ulMsgNum % 40));
                break;
            case 1:
                snprintf(txBuffer, sizeof(txBuffer), patterns[1],
                         1000 + (ulMsgNum % 3000),
                         3.3f * (float)((ulMsgNum % 4096)) / 4096.0f);
                break;
            case 2:
                snprintf(txBuffer, sizeof(txBuffer), patterns[2], ulMsgNum);
                break;
            case 3:
                snprintf(txBuffer, sizeof(txBuffer), patterns[3],
                         -6.2f + (ulMsgNum % 100) * 0.001f,
                         106.8f + (ulMsgNum % 100) * 0.001f);
                break;
            case 4:
                /* Short message to demonstrate trigger behavior */
                strcpy(txBuffer, "AB");
                break;
        }
        
        size_t xDataLen = strlen(txBuffer);
        
        snprintf(msg, sizeof(msg), "[Producer] Sending (#%lu, %u bytes): '%s'\r\n",
                 ulMsgNum, (unsigned)xDataLen, txBuffer);
        UART_SendString(msg);
        
        /*
         * SEND TO STREAM BUFFER
         * 
         * xStreamBufferSend(handle, data, length, timeout):
         *   Returns: Number of bytes actually sent
         *   May be less than requested if buffer full
         */
        xBytesSent = xStreamBufferSend(xStreamBuffer,
                                        txBuffer,
                                        xDataLen,
                                        pdMS_TO_TICKS(100));
        
        if (xBytesSent == xDataLen)
        {
            ulTotalBytesSent += xBytesSent;
            ulSendCount++;
            
            snprintf(msg, sizeof(msg), "          Sent: %u bytes (total: %lu)\r\n",
                     (unsigned)xBytesSent, ulTotalBytesSent);
            UART_SendString(msg);
            
            /* Short message demo - explain trigger behavior */
            if (xDataLen < TRIGGER_LEVEL)
            {
                UART_SendString("          [Note: Below trigger level - consumer may timeout]\r\n");
            }
        }
        else
        {
            snprintf(msg, sizeof(msg), "          [WARN] Partial send: %u/%u bytes\r\n",
                     (unsigned)xBytesSent, (unsigned)xDataLen);
            UART_SendString(msg);
        }
        
        vTaskDelayUntil(&xLastWakeTime, xSendDelay);
    }
}

/* ============================================================================
 * CONSUMER TASK
 * Receives data from Stream Buffer and processes
 * ============================================================================ */
static void prvConsumerTask(void *pvParameters)
{
    (void)pvParameters;
    
    char rxBuffer[64];
    char msg[120];
    size_t xBytesReceived;
    
    /* 
     * Wait timeout - if trigger not met, will timeout after this
     * Consumer can then process partial data
     */
    const TickType_t xReceiveTimeout = pdMS_TO_TICKS(1500);
    
    for (;;)
    {
        /*
         * RECEIVE FROM STREAM BUFFER
         * 
         * xStreamBufferReceive(handle, buffer, maxLen, timeout):
         *   Returns: Number of bytes actually received
         *   Blocks until trigger level reached OR timeout
         */
        xBytesReceived = xStreamBufferReceive(xStreamBuffer,
                                               rxBuffer,
                                               sizeof(rxBuffer) - 1,  /* Leave room for null */
                                               xReceiveTimeout);
        
        if (xBytesReceived > 0)
        {
            /* Null terminate for string operations */
            rxBuffer[xBytesReceived] = '\0';
            
            ulTotalBytesReceived += xBytesReceived;
            ulReceiveCount++;
            
            /* Check if this was a timeout-triggered partial receive */
            if (xBytesReceived < TRIGGER_LEVEL)
            {
                snprintf(msg, sizeof(msg), "[Consumer] Timeout-receive %u bytes: '%s'\r\n",
                         (unsigned)xBytesReceived, rxBuffer);
            }
            else
            {
                snprintf(msg, sizeof(msg), "[Consumer] Received %u bytes: '%s'\r\n",
                         (unsigned)xBytesReceived, rxBuffer);
            }
            UART_SendString(msg);
            
            /* Toggle LED to indicate activity */
            HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
        }
        else
        {
            /* True timeout - no data at all */
            UART_SendString("[Consumer] Timeout - no data received\r\n");
        }
    }
}

/* ============================================================================
 * MONITOR TASK
 * Periodically reports Stream Buffer statistics
 * ============================================================================ */
static void prvMonitorTask(void *pvParameters)
{
    (void)pvParameters;
    
    char msg[150];
    const TickType_t xPeriod = pdMS_TO_TICKS(10000);  /* Every 10 seconds */
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
        
        /* Get buffer space available */
        size_t xSpaceAvailable = xStreamBufferSpacesAvailable(xStreamBuffer);
        size_t xBytesWaiting = STREAM_BUFFER_SIZE - xSpaceAvailable;
        
        UART_SendString("\r\n");
        UART_SendString("┌─── STREAM BUFFER STATISTICS ────────────────────────────┐\r\n");
        
        snprintf(msg, sizeof(msg), "│  Buffer usage: %u/%d bytes (%u%% full)               │\r\n",
                 (unsigned)xBytesWaiting, STREAM_BUFFER_SIZE,
                 (unsigned)(xBytesWaiting * 100 / STREAM_BUFFER_SIZE));
        UART_SendString(msg);
        
        snprintf(msg, sizeof(msg), "│  Total sent: %lu bytes (%lu messages)                │\r\n",
                 ulTotalBytesSent, ulSendCount);
        UART_SendString(msg);
        
        snprintf(msg, sizeof(msg), "│  Total received: %lu bytes (%lu receives)            │\r\n",
                 ulTotalBytesReceived, ulReceiveCount);
        UART_SendString(msg);
        
        /* Check if buffer is empty (data properly flowing) */
        if (xStreamBufferIsEmpty(xStreamBuffer))
        {
            UART_SendString("│  Status: Buffer empty - data flowing smoothly          │\r\n");
        }
        else if (xStreamBufferIsFull(xStreamBuffer))
        {
            UART_SendString("│  [WARN] Buffer FULL - consumer may be too slow!        │\r\n");
        }
        else
        {
            UART_SendString("│  Status: Buffer has pending data                        │\r\n");
        }
        
        UART_SendString("└──────────────────────────────────────────────────────────┘\r\n\r\n");
    }
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
    
    /* LED PC13 */
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
    
    /* TX PA9 */
    GPIO_InitStruct.Pin = DEBUG_UART_TX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DEBUG_UART_TX_PORT, &GPIO_InitStruct);
    
    /* RX PA10 */
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
