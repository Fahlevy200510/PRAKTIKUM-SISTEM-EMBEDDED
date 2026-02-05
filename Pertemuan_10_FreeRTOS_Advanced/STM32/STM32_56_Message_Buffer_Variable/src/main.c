/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 33-Message_Buffer_Variable
 * 
 * DESKRIPSI:
 * Demo Message Buffer untuk variable length messages. Menunjukkan bagaimana
 * Message Buffer secara otomatis menangani messages dengan ukuran berbeda
 * dan menjaga message boundaries.
 * 
 * ============================================================================
 * ARSITEKTUR SISTEM
 * ============================================================================
 * 
 *    ┌──────────────────────────────────────────────────────────────────────┐
 *    │                VARIABLE LENGTH MESSAGE DEMO                          │
 *    │                                                                      │
 *    │   Logger Task                             Collector Task             │
 *    │   ┌─────────────────┐                     ┌─────────────────┐        │
 *    │   │  Generate logs  │                     │  Collect and    │        │
 *    │   │  of varying     │                     │  display logs   │        │
 *    │   │  lengths        │                     │                 │        │
 *    │   └────────┬────────┘                     └────────▲────────┘        │
 *    │            │                                       │                 │
 *    │            ▼                                       │                 │
 *    │   ┌────────────────────────────────────────────────┴────────┐        │
 *    │   │                   MESSAGE BUFFER                        │        │
 *    │   │                                                         │        │
 *    │   │  ┌───┬────────┐┌───┬───────────────────────┐┌───┬───┐   │        │
 *    │   │  │10 │INFO:OK ││35 │WARN: Temp high...    ││ 5 │DBG │   │        │
 *    │   │  └───┴────────┘└───┴───────────────────────┘└───┴───┘   │        │
 *    │   │   Small msg     Medium msg                  Short msg   │        │
 *    │   │                                                         │        │
 *    │   └─────────────────────────────────────────────────────────┘        │
 *    │                                                                      │
 *    └──────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * MESSAGE TYPES
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │  Log Message Structure:                                            │
 *    │                                                                     │
 *    │  ┌─────────┬──────────┬───────────────────────────────────────────┐│
 *    │  │ Level   │ Timestamp│           Log Message (variable)          ││
 *    │  │ 1 byte  │ 4 bytes  │           1-74 bytes                      ││
 *    │  └─────────┴──────────┴───────────────────────────────────────────┘│
 *    │                                                                     │
 *    │  Levels:                                                            │
 *    │    0 = DEBUG   (short messages)                                    │
 *    │    1 = INFO    (medium messages)                                   │
 *    │    2 = WARNING (can be long)                                       │
 *    │    3 = ERROR   (detailed with context)                             │
 *    │                                                                     │
 *    │  Size range: 6 bytes (min) to 80 bytes (max)                       │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * CONTOH OUTPUT SERIAL
 * ============================================================================
 * 
 *    === VARIABLE LENGTH MESSAGE DEMO ===
 *    
 *    [Logger] Sending: "D" (6 bytes)
 *    [Logger] Sending: "INFO: System started OK" (28 bytes)
 *    [Logger] Sending: "WARN: Temperature 45.2C exceeds threshold" (46 bytes)
 *    
 *    [Collector] Received 6 bytes:
 *                Level=DEBUG, Time=1234, Msg="D"
 *    [Collector] Received 28 bytes:
 *                Level=INFO, Time=1235, Msg="INFO: System started OK"
 *    [Collector] Received 46 bytes:
 *                Level=WARN, Time=1236, Msg="WARN: Temperature..."
 * 
 * ============================================================================
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * LOG MESSAGE STRUCTURE
 * ============================================================================ */

typedef enum {
    LOG_DEBUG   = 0,
    LOG_INFO    = 1,
    LOG_WARNING = 2,
    LOG_ERROR   = 3,
} LogLevel_t;

/* Variable length log message */
typedef struct __attribute__((packed)) {
    uint8_t  ucLevel;
    uint32_t ulTimestamp;
    char     cMessage[MAX_MESSAGE_SIZE - 5];  /* Variable length */
} LogMessage_t;

static const char *pcLevelNames[] = {"DEBUG", "INFO", "WARN", "ERROR"};

/* ============================================================================
 * GLOBAL VARIABLES
 * ============================================================================ */
UART_HandleTypeDef huart1;
MessageBufferHandle_t xLogBuffer = NULL;

/* Statistics */
static uint32_t ulTotalMessagesSent = 0;
static uint32_t ulTotalBytesUsed = 0;
static size_t   xSmallestMsg = MAX_MESSAGE_SIZE;
static size_t   xLargestMsg = 0;

/* ============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================ */
static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART1_Init(void);
static void prvLoggerTask(void *pvParameters);
static void prvCollectorTask(void *pvParameters);
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
    
    char msg[150];
    
    UART_SendString("\r\n\r\n");
    UART_SendString("╔══════════════════════════════════════════════════════════╗\r\n");
    UART_SendString("║      33. VARIABLE LENGTH MESSAGE BUFFER DEMO             ║\r\n");
    UART_SendString("╚══════════════════════════════════════════════════════════╝\r\n\r\n");
    
    snprintf(msg, sizeof(msg), "[Config] Buffer size: %d bytes\r\n", MESSAGE_BUFFER_SIZE);
    UART_SendString(msg);
    snprintf(msg, sizeof(msg), "[Config] Max message size: %d bytes\r\n\r\n", MAX_MESSAGE_SIZE);
    UART_SendString(msg);
    
    /* Create message buffer */
    xLogBuffer = xMessageBufferCreate(MESSAGE_BUFFER_SIZE);
    
    if (xLogBuffer == NULL)
    {
        UART_SendString("[ERROR] Failed to create message buffer!\r\n");
        while (1);
    }
    UART_SendString("[OK] Message Buffer created\r\n\r\n");
    
    /* Create tasks */
    xTaskCreate(prvLoggerTask, "Logger", TASK_STACK, NULL, 2, NULL);
    xTaskCreate(prvCollectorTask, "Collector", TASK_STACK, NULL, 3, NULL);
    xTaskCreate(prvMonitorTask, "Monitor", TASK_STACK, NULL, 1, NULL);
    
    UART_SendString("[INFO] Starting scheduler...\r\n\r\n");
    UART_SendString("────────────────────────────────────────────────────────────\r\n");
    
    vTaskStartScheduler();
    
    while (1);
}

/* ============================================================================
 * LOGGER TASK
 * Generates log messages of varying lengths
 * ============================================================================ */
static void prvLoggerTask(void *pvParameters)
{
    (void)pvParameters;
    
    LogMessage_t logMsg;
    char uart_msg[100];
    uint32_t ulMsgNum = 0;
    
    /* Sample log messages of different lengths */
    const char *debugMsgs[] = {"D", "OK", "CHK"};
    const char *infoMsgs[] = {
        "System ready",
        "Task started successfully",
        "Configuration loaded from flash"
    };
    const char *warnMsgs[] = {
        "High CPU usage detected: 85%",
        "Temperature exceeds threshold: 45.2C, limit is 40.0C",
        "Queue almost full, consider increasing buffer size"
    };
    const char *errorMsgs[] = {
        "CRITICAL: Memory allocation failed for sensor buffer",
        "FATAL: Communication timeout after 3 retries, check connection",
        "ERROR: Stack overflow in Task_ADC, increase stack or optimize usage"
    };
    
    const TickType_t xSendDelay = pdMS_TO_TICKS(800);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        ulMsgNum++;
        
        /* Rotate through different log levels */
        LogLevel_t level = ulMsgNum % 4;
        logMsg.ucLevel = (uint8_t)level;
        logMsg.ulTimestamp = xTaskGetTickCount();
        
        /* Select message based on level */
        const char *text = NULL;
        switch (level)
        {
            case LOG_DEBUG:
                text = debugMsgs[ulMsgNum % 3];
                break;
            case LOG_INFO:
                text = infoMsgs[ulMsgNum % 3];
                break;
            case LOG_WARNING:
                text = warnMsgs[ulMsgNum % 3];
                break;
            case LOG_ERROR:
                text = errorMsgs[ulMsgNum % 3];
                break;
        }
        
        /* Copy message text */
        strncpy(logMsg.cMessage, text, sizeof(logMsg.cMessage) - 1);
        logMsg.cMessage[sizeof(logMsg.cMessage) - 1] = '\0';
        
        /* Calculate actual message size (header + actual text length) */
        size_t msgSize = 5 + strlen(logMsg.cMessage) + 1;  /* header + text + null */
        
        snprintf(uart_msg, sizeof(uart_msg), 
                 "\r\n[Logger] Sending %s (%u bytes): \"%s\"\r\n",
                 pcLevelNames[level], (unsigned)msgSize, logMsg.cMessage);
        UART_SendString(uart_msg);
        
        /*
         * SEND VARIABLE LENGTH MESSAGE
         * 
         * Message Buffer automatically stores length prefix
         * We only send actual data size, not full struct size!
         */
        size_t xBytesSent = xMessageBufferSend(xLogBuffer,
                                                &logMsg,
                                                msgSize,
                                                pdMS_TO_TICKS(100));
        
        if (xBytesSent == msgSize)
        {
            ulTotalMessagesSent++;
            ulTotalBytesUsed += msgSize + 4;  /* +4 for internal length prefix */
            
            if (msgSize < xSmallestMsg) xSmallestMsg = msgSize;
            if (msgSize > xLargestMsg) xLargestMsg = msgSize;
            
            snprintf(uart_msg, sizeof(uart_msg), 
                     "         Sent OK (in buffer: ~%u bytes used)\r\n",
                     (unsigned)(MESSAGE_BUFFER_SIZE - xMessageBufferSpacesAvailable(xLogBuffer)));
            UART_SendString(uart_msg);
        }
        else
        {
            UART_SendString("         [WARN] Buffer full, message dropped!\r\n");
        }
        
        vTaskDelayUntil(&xLastWakeTime, xSendDelay);
    }
}

/* ============================================================================
 * COLLECTOR TASK
 * Receives and processes variable length messages
 * ============================================================================ */
static void prvCollectorTask(void *pvParameters)
{
    (void)pvParameters;
    
    LogMessage_t rxMsg;
    char msg[150];
    static uint32_t ulRecvCount = 0;
    
    for (;;)
    {
        /*
         * RECEIVE VARIABLE LENGTH MESSAGE
         * 
         * Buffer must be large enough for largest expected message
         * Returns actual message size (not buffer size)
         */
        size_t xBytesReceived = xMessageBufferReceive(xLogBuffer,
                                                       &rxMsg,
                                                       sizeof(rxMsg),
                                                       portMAX_DELAY);
        
        if (xBytesReceived >= 5)  /* Minimum: header size */
        {
            ulRecvCount++;
            
            snprintf(msg, sizeof(msg), 
                     "[Collector] ▶ #%lu Received %u bytes\r\n",
                     ulRecvCount, (unsigned)xBytesReceived);
            UART_SendString(msg);
            
            snprintf(msg, sizeof(msg), 
                     "            Level=%s, Time=%lu\r\n",
                     pcLevelNames[rxMsg.ucLevel % 4], rxMsg.ulTimestamp);
            UART_SendString(msg);
            
            /* Truncate long messages for display */
            size_t textLen = xBytesReceived - 5;
            if (textLen > 40)
            {
                snprintf(msg, sizeof(msg), 
                         "            Msg=\"%.37s...\"\r\n", rxMsg.cMessage);
            }
            else
            {
                snprintf(msg, sizeof(msg), 
                         "            Msg=\"%s\"\r\n", rxMsg.cMessage);
            }
            UART_SendString(msg);
            
            /* Toggle LED based on severity */
            if (rxMsg.ucLevel >= LOG_WARNING)
            {
                HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_RESET);  /* LED ON */
            }
            else
            {
                HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_SET);   /* LED OFF */
            }
        }
    }
}

/* ============================================================================
 * MONITOR TASK
 * Reports message buffer statistics
 * ============================================================================ */
static void prvMonitorTask(void *pvParameters)
{
    (void)pvParameters;
    
    char msg[150];
    const TickType_t xPeriod = pdMS_TO_TICKS(8000);
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
        
        size_t xSpaceAvailable = xMessageBufferSpacesAvailable(xLogBuffer);
        
        UART_SendString("\r\n┌─── VARIABLE MESSAGE STATISTICS ─────────────────────────┐\r\n");
        
        snprintf(msg, sizeof(msg), "│  Messages sent: %lu                                      │\r\n",
                 ulTotalMessagesSent);
        UART_SendString(msg);
        
        snprintf(msg, sizeof(msg), "│  Total bytes used: %lu (incl. overhead)                 │\r\n",
                 ulTotalBytesUsed);
        UART_SendString(msg);
        
        snprintf(msg, sizeof(msg), "│  Message size range: %u - %u bytes                       │\r\n",
                 (unsigned)xSmallestMsg, (unsigned)xLargestMsg);
        UART_SendString(msg);
        
        if (ulTotalMessagesSent > 0)
        {
            snprintf(msg, sizeof(msg), "│  Average message: %lu bytes                              │\r\n",
                     ulTotalBytesUsed / ulTotalMessagesSent);
            UART_SendString(msg);
        }
        
        snprintf(msg, sizeof(msg), "│  Buffer space: %u/%d bytes free                         │\r\n",
                 (unsigned)xSpaceAvailable, MESSAGE_BUFFER_SIZE);
        UART_SendString(msg);
        
        UART_SendString("└──────────────────────────────────────────────────────────┘\r\n");
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
