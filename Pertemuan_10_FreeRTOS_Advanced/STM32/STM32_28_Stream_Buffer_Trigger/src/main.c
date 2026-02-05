/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 31-Stream_Buffer_Trigger
 * 
 * DESKRIPSI:
 * Demo Stream Buffer dengan berbagai trigger level untuk menunjukkan
 * trade-off antara latency dan efficiency. Trigger level dapat diubah
 * runtime menggunakan xStreamBufferSetTriggerLevel().
 * 
 * ============================================================================
 * ARSITEKTUR SISTEM
 * ============================================================================
 * 
 *    ┌──────────────────────────────────────────────────────────────────────┐
 *    │                   TRIGGER LEVEL DEMO                                 │
 *    │                                                                      │
 *    │   Producer Task                           Consumer Task              │
 *    │   ┌─────────────────┐                     ┌─────────────────┐        │
 *    │   │  Generate       │                     │  Wait for       │        │
 *    │   │  5 bytes        │                     │  trigger level  │        │
 *    │   │  per send       │                     │  to process     │        │
 *    │   └────────┬────────┘                     └────────▲────────┘        │
 *    │            │                                       │                 │
 *    │            ▼                                       │                 │
 *    │   ┌────────────────────────────────────────────────┴────────┐        │
 *    │   │                    STREAM BUFFER                        │        │
 *    │   │  ┌──────────────────────────────────────────────────┐   │        │
 *    │   │  │▓▓▓▓▓▓▓▓▓▓▓▓▓│░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░│   │        │
 *    │   │  └──────────────────────────────────────────────────┘   │        │
 *    │   │        ↑               ↑                                │        │
 *    │   │    used bytes     trigger level                         │        │
 *    │   └─────────────────────────────────────────────────────────┘        │
 *    │                                                                      │
 *    │   Control Task                                                       │
 *    │   ┌─────────────────┐                                                │
 *    │   │  Change trigger │  ← Cycles through LOW/MEDIUM/HIGH              │
 *    │   │  level at       │                                                │
 *    │   │  runtime        │                                                │
 *    │   └─────────────────┘                                                │
 *    │                                                                      │
 *    └──────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * TRIGGER LEVEL COMPARISON
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │  Trigger = 1 (LOW)     Trigger = 20 (MED)    Trigger = 50 (HIGH)   │
 *    │  ━━━━━━━━━━━━━━━━━     ━━━━━━━━━━━━━━━━━     ━━━━━━━━━━━━━━━━━━━   │
 *    │                                                                     │
 *    │  Send #1: 5 bytes      Send #1: 5 bytes      Send #1: 5 bytes      │
 *    │  ▶ UNBLOCK             Waiting...            Waiting...             │
 *    │                                                                     │
 *    │  Send #2: 5 bytes      Send #2: 10 bytes     Send #2: 10 bytes     │
 *    │  ▶ UNBLOCK             Waiting...            Waiting...             │
 *    │                                                                     │
 *    │  Send #3: 5 bytes      Send #3: 15 bytes     Send #3: 15 bytes     │
 *    │  ▶ UNBLOCK             Waiting...            Waiting...             │
 *    │                                                                     │
 *    │  Send #4: 5 bytes      Send #4: 20 bytes     Send #4: 20 bytes     │
 *    │  ▶ UNBLOCK             ▶ UNBLOCK             Waiting...             │
 *    │                                                                     │
 *    │  ...                   ...                   ...                    │
 *    │                                                                     │
 *    │  Switches: 10x         Switches: 2x          Switches: 1x          │
 *    │  Latency: 0-5 bytes    Latency: 0-20 bytes   Latency: 0-50 bytes   │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * CONTOH OUTPUT SERIAL
 * ============================================================================
 * 
 *    === TRIGGER LEVEL DEMO ===
 *    
 *    [Control] Setting trigger level: LOW (1 byte)
 *    [Producer] Sent 5 bytes (total: 5)
 *    [Consumer] Unblocked! Received 5 bytes (immediate)
 *    [Producer] Sent 5 bytes (total: 5)
 *    [Consumer] Unblocked! Received 5 bytes (immediate)
 *    
 *    [Control] Setting trigger level: MEDIUM (20 bytes)
 *    [Producer] Sent 5 bytes (total: 5)
 *    [Producer] Sent 5 bytes (total: 10)
 *    [Producer] Sent 5 bytes (total: 15)
 *    [Producer] Sent 5 bytes (total: 20)
 *    [Consumer] Unblocked! Received 20 bytes (batch)
 *    
 *    [Control] Setting trigger level: HIGH (50 bytes)
 *    ... waits for 50 bytes ...
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

/* Current trigger level setting */
static volatile size_t xCurrentTrigger = TRIGGER_LOW;
static const char *pcTriggerNames[] = {"LOW (1)", "MEDIUM (20)", "HIGH (50)"};
static volatile uint32_t ulTriggerIndex = 0;

/* Statistics per trigger level */
typedef struct {
    uint32_t ulUnblockCount;
    uint32_t ulTotalBytesReceived;
    TickType_t xStartTime;
} TriggerStats_t;

static TriggerStats_t xStats = {0};

/* ============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================ */
static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART1_Init(void);
static void prvProducerTask(void *pvParameters);
static void prvConsumerTask(void *pvParameters);
static void prvControlTask(void *pvParameters);
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
    
    char msg[150];
    
    UART_SendString("\r\n\r\n");
    UART_SendString("╔══════════════════════════════════════════════════════════╗\r\n");
    UART_SendString("║       31. STREAM BUFFER TRIGGER LEVEL DEMO               ║\r\n");
    UART_SendString("╚══════════════════════════════════════════════════════════╝\r\n\r\n");
    
    UART_SendString("[Config] Buffer size: 100 bytes\r\n");
    snprintf(msg, sizeof(msg), "[Config] Trigger levels: LOW=%d, MED=%d, HIGH=%d\r\n\r\n",
             TRIGGER_LOW, TRIGGER_MEDIUM, TRIGGER_HIGH);
    UART_SendString(msg);
    
    /* Create with LOW trigger initially */
    xStreamBuffer = xStreamBufferCreate(STREAM_BUFFER_SIZE, TRIGGER_LOW);
    
    if (xStreamBuffer == NULL)
    {
        UART_SendString("[ERROR] Failed to create Stream Buffer!\r\n");
        while (1);
    }
    UART_SendString("[OK] Stream Buffer created (trigger=LOW)\r\n\r\n");
    
    /* Create tasks */
    xTaskCreate(prvProducerTask, "Producer", TASK_STACK, NULL, 2, NULL);
    xTaskCreate(prvConsumerTask, "Consumer", TASK_STACK, NULL, 3, NULL);
    xTaskCreate(prvControlTask, "Control", TASK_STACK, NULL, 1, NULL);
    
    UART_SendString("[INFO] Starting scheduler...\r\n\r\n");
    UART_SendString("════════════════════════════════════════════════════════════\r\n");
    
    vTaskStartScheduler();
    
    while (1);
}

/* ============================================================================
 * PRODUCER TASK
 * Sends fixed-size chunks at regular interval
 * ============================================================================ */
static void prvProducerTask(void *pvParameters)
{
    (void)pvParameters;
    
    char txData[] = "DATA_";  /* 5 bytes per send */
    char msg[80];
    static uint32_t ulSendCount = 0;
    
    const TickType_t xSendInterval = pdMS_TO_TICKS(200);  /* Fast producer */
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        size_t xBytesSent = xStreamBufferSend(xStreamBuffer,
                                               txData,
                                               sizeof(txData) - 1,  /* Exclude null */
                                               pdMS_TO_TICKS(50));
        
        if (xBytesSent > 0)
        {
            ulSendCount++;
            
            /* Get current buffer fill level */
            size_t xBytesInBuffer = STREAM_BUFFER_SIZE - 
                                    xStreamBufferSpacesAvailable(xStreamBuffer);
            
            snprintf(msg, sizeof(msg), "[Producer] Sent %u bytes (buffer: %u/%d)\r\n",
                     (unsigned)xBytesSent, (unsigned)xBytesInBuffer, STREAM_BUFFER_SIZE);
            UART_SendString(msg);
        }
        
        vTaskDelayUntil(&xLastWakeTime, xSendInterval);
    }
}

/* ============================================================================
 * CONSUMER TASK
 * Receives data, blocked until trigger level reached
 * ============================================================================ */
static void prvConsumerTask(void *pvParameters)
{
    (void)pvParameters;
    
    char rxBuffer[STREAM_BUFFER_SIZE];
    char msg[120];
    
    for (;;)
    {
        TickType_t xWaitStart = xTaskGetTickCount();
        
        /*
         * RECEIVE - will block until trigger level bytes available
         * or timeout (we use shorter timeout to show partial receives)
         */
        size_t xBytesReceived = xStreamBufferReceive(xStreamBuffer,
                                                      rxBuffer,
                                                      sizeof(rxBuffer) - 1,
                                                      pdMS_TO_TICKS(2000));
        
        if (xBytesReceived > 0)
        {
            TickType_t xWaitTime = xTaskGetTickCount() - xWaitStart;
            
            rxBuffer[xBytesReceived] = '\0';
            
            xStats.ulUnblockCount++;
            xStats.ulTotalBytesReceived += xBytesReceived;
            
            snprintf(msg, sizeof(msg), 
                     "[Consumer] ▶ UNBLOCK! Got %u bytes after %lu ms (trigger=%s)\r\n",
                     (unsigned)xBytesReceived, xWaitTime, pcTriggerNames[ulTriggerIndex]);
            UART_SendString(msg);
            
            /* Toggle LED on receive */
            HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
        }
        else
        {
            UART_SendString("[Consumer] Timeout - no trigger\r\n");
        }
    }
}

/* ============================================================================
 * CONTROL TASK
 * Periodically changes trigger level to demonstrate different behaviors
 * ============================================================================ */
static void prvControlTask(void *pvParameters)
{
    (void)pvParameters;
    
    char msg[150];
    const TickType_t xPeriod = pdMS_TO_TICKS(10000);  /* Change every 10 seconds */
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    /* Initial stats reset */
    xStats.xStartTime = xTaskGetTickCount();
    
    for (;;)
    {
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
        
        /* Print statistics for current trigger level */
        TickType_t xElapsed = xTaskGetTickCount() - xStats.xStartTime;
        
        UART_SendString("\r\n┌─── TRIGGER LEVEL STATISTICS ────────────────────────────┐\r\n");
        snprintf(msg, sizeof(msg), "│  Trigger level: %s                              │\r\n",
                 pcTriggerNames[ulTriggerIndex]);
        UART_SendString(msg);
        snprintf(msg, sizeof(msg), "│  Unblock count: %lu times                              │\r\n",
                 xStats.ulUnblockCount);
        UART_SendString(msg);
        snprintf(msg, sizeof(msg), "│  Total received: %lu bytes                             │\r\n",
                 xStats.ulTotalBytesReceived);
        UART_SendString(msg);
        if (xStats.ulUnblockCount > 0)
        {
            snprintf(msg, sizeof(msg), "│  Avg per unblock: %lu bytes                            │\r\n",
                     xStats.ulTotalBytesReceived / xStats.ulUnblockCount);
            UART_SendString(msg);
        }
        UART_SendString("└──────────────────────────────────────────────────────────┘\r\n\r\n");
        
        /* Cycle to next trigger level */
        ulTriggerIndex = (ulTriggerIndex + 1) % 3;
        
        switch (ulTriggerIndex)
        {
            case 0: xCurrentTrigger = TRIGGER_LOW; break;
            case 1: xCurrentTrigger = TRIGGER_MEDIUM; break;
            case 2: xCurrentTrigger = TRIGGER_HIGH; break;
        }
        
        /*
         * SET NEW TRIGGER LEVEL
         * 
         * xStreamBufferSetTriggerLevel():
         *   - Changes trigger level at runtime
         *   - Returns pdTRUE if success
         *   - Does not affect currently blocked task
         */
        if (xStreamBufferSetTriggerLevel(xStreamBuffer, xCurrentTrigger) == pdTRUE)
        {
            snprintf(msg, sizeof(msg), 
                     "════════════════════════════════════════════════════════════\r\n"
                     "[Control] ★ NEW TRIGGER LEVEL: %s\r\n"
                     "════════════════════════════════════════════════════════════\r\n\r\n",
                     pcTriggerNames[ulTriggerIndex]);
            UART_SendString(msg);
        }
        
        /* Reset statistics */
        xStats.ulUnblockCount = 0;
        xStats.ulTotalBytesReceived = 0;
        xStats.xStartTime = xTaskGetTickCount();
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
