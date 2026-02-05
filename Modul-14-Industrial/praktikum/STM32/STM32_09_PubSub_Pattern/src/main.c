/**
 * =============================================================================
 * PROGRAM 47: PUB/SUB PATTERN - Publish-Subscribe dengan FreeRTOS
 * =============================================================================
 * 
 * Demo Publish-Subscribe pattern dengan broker yang mengelola
 * multiple topics dan subscribers.
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdio.h>
#include <string.h>

/* ========================== HARDWARE CONFIGURATION ======================== */
#define LED_PIN         GPIO_PIN_13
#define LED_PORT        GPIOC

/* ========================== DATA STRUCTURES =============================== */

/* Pub/Sub message structure */
typedef struct {
    uint8_t  ucTopicId;       /* Topic identifier */
    uint8_t  ucPublisherId;   /* Publisher ID */
    uint8_t  ucPriority;      /* Message priority 0-3 */
    uint8_t  ucReserved;      /* Padding */
    uint32_t ulTimestamp;     /* When published */
    uint32_t ulData;          /* Message payload */
} PubSubMessage_t;

/* Subscriber registration */
typedef struct {
    uint8_t        ucSubscriberId;
    uint8_t        ucTopicMask;     /* Bitmask of subscribed topics */
    QueueHandle_t  xQueue;          /* Subscriber's message queue */
    const char    *pcName;          /* Subscriber name for debug */
} Subscriber_t;

/* Broker statistics */
typedef struct {
    uint32_t ulPublished;           /* Total messages published */
    uint32_t ulDispatched;          /* Total messages dispatched */
    uint32_t ulDropped;             /* Dropped due to full queue */
    uint32_t ulTopicCounts[PUBSUB_MAX_TOPICS];
} BrokerStats_t;

/* ========================== GLOBALS ======================================= */

UART_HandleTypeDef huart1;

/* Broker queue */
static QueueHandle_t xBrokerQueue;

/* Subscriber list */
static Subscriber_t xSubscribers[PUBSUB_MAX_SUBSCRIBERS];
static uint8_t ucSubscriberCount = 0;

/* Broker statistics */
static volatile BrokerStats_t xBrokerStats;

/* Access mutex */
static SemaphoreHandle_t xBrokerMutex;

/* Task handles */
static TaskHandle_t xBrokerTaskHandle;
static TaskHandle_t xTempPubTaskHandle;
static TaskHandle_t xHumidPubTaskHandle;
static TaskHandle_t xEventPubTaskHandle;

/* Random seed */
static uint32_t ulRandomSeed = 12345;

/* ========================== FUNCTION PROTOTYPES =========================== */

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART1_Init(void);
static void UART_SendString(const char *str);

static void vBrokerTask(void *pvParameters);
static void vTempPublisherTask(void *pvParameters);
static void vHumidPublisherTask(void *pvParameters);
static void vEventPublisherTask(void *pvParameters);
static void vDisplaySubscriberTask(void *pvParameters);
static void vLoggerSubscriberTask(void *pvParameters);
static void vAlarmSubscriberTask(void *pvParameters);

static BaseType_t xPubSubPublish(uint8_t ucTopic, uint8_t ucPublisher, 
                                  uint32_t ulData, uint8_t ucPriority);
static BaseType_t xPubSubSubscribe(uint8_t ucSubscriberId, const char *pcName,
                                    uint8_t ucTopicMask, QueueHandle_t *pxQueue);

/* ========================== HELPER FUNCTIONS ============================== */

static uint32_t ulRandom(void)
{
    ulRandomSeed = ulRandomSeed * 1103515245 + 12345;
    return (ulRandomSeed >> 16) & 0x7FFF;
}

static const char* pcTopicName(uint8_t ucTopic)
{
    switch (ucTopic)
    {
        case TOPIC_TEMPERATURE: return "TEMP";
        case TOPIC_HUMIDITY: return "HUMID";
        case TOPIC_EVENT: return "EVENT";
        case TOPIC_SYSTEM: return "SYSTEM";
        default: return "???";
    }
}

/* ========================== PUB/SUB API =================================== */

/**
 * Subscribe to topics
 * Returns pdTRUE if successful
 */
static BaseType_t xPubSubSubscribe(uint8_t ucSubscriberId, const char *pcName,
                                    uint8_t ucTopicMask, QueueHandle_t *pxQueue)
{
    BaseType_t xResult = pdFALSE;
    
    if (ucSubscriberCount >= PUBSUB_MAX_SUBSCRIBERS)
    {
        return pdFALSE;
    }
    
    /* Create subscriber queue */
    *pxQueue = xQueueCreate(PUBSUB_SUBSCRIBER_QUEUE_SIZE, sizeof(PubSubMessage_t));
    if (*pxQueue == NULL)
    {
        return pdFALSE;
    }
    
    if (xSemaphoreTake(xBrokerMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        xSubscribers[ucSubscriberCount].ucSubscriberId = ucSubscriberId;
        xSubscribers[ucSubscriberCount].ucTopicMask = ucTopicMask;
        xSubscribers[ucSubscriberCount].xQueue = *pxQueue;
        xSubscribers[ucSubscriberCount].pcName = pcName;
        ucSubscriberCount++;
        xResult = pdTRUE;
        
        xSemaphoreGive(xBrokerMutex);
    }
    
    return xResult;
}

/**
 * Publish message to topic
 * Returns pdTRUE if message was queued to broker
 */
static BaseType_t xPubSubPublish(uint8_t ucTopic, uint8_t ucPublisher, 
                                  uint32_t ulData, uint8_t ucPriority)
{
    PubSubMessage_t xMsg;
    
    xMsg.ucTopicId = ucTopic;
    xMsg.ucPublisherId = ucPublisher;
    xMsg.ucPriority = ucPriority;
    xMsg.ulTimestamp = xTaskGetTickCount();
    xMsg.ulData = ulData;
    
    return xQueueSend(xBrokerQueue, &xMsg, pdMS_TO_TICKS(10));
}

/* ========================== TASKS ========================================= */

/**
 * Broker Task - Receives published messages and dispatches to subscribers
 */
static void vBrokerTask(void *pvParameters)
{
    (void)pvParameters;
    
    PubSubMessage_t xMsg;
    char cBuffer[100];
    uint8_t i;
    TickType_t xLastStatsTime = 0;
    
    UART_SendString("[BROKER] Task started\r\n");
    
    for (;;)
    {
        /* Wait for published message */
        if (xQueueReceive(xBrokerQueue, &xMsg, pdMS_TO_TICKS(500)) == pdTRUE)
        {
            xBrokerStats.ulPublished++;
            if (xMsg.ucTopicId < PUBSUB_MAX_TOPICS)
            {
                xBrokerStats.ulTopicCounts[xMsg.ucTopicId]++;
            }
            
            snprintf(cBuffer, sizeof(cBuffer), 
                     "\r\n[BROKER] Recv: Topic=%s, Data=%lu\r\n",
                     pcTopicName(xMsg.ucTopicId), xMsg.ulData);
            UART_SendString(cBuffer);
            
            /* Dispatch to subscribers */
            uint8_t ucDispatchCount = 0;
            
            if (xSemaphoreTake(xBrokerMutex, pdMS_TO_TICKS(50)) == pdTRUE)
            {
                for (i = 0; i < ucSubscriberCount; i++)
                {
                    /* Check if subscriber is interested in this topic */
                    uint8_t ucInterested = 0;
                    
                    if (xSubscribers[i].ucTopicMask == TOPIC_ALL)
                    {
                        ucInterested = 1;  /* Wildcard subscriber */
                    }
                    else if (xSubscribers[i].ucTopicMask & (1 << xMsg.ucTopicId))
                    {
                        ucInterested = 1;  /* Topic matches */
                    }
                    
                    if (ucInterested)
                    {
                        if (xQueueSend(xSubscribers[i].xQueue, &xMsg, 0) == pdPASS)
                        {
                            ucDispatchCount++;
                            xBrokerStats.ulDispatched++;
                        }
                        else
                        {
                            xBrokerStats.ulDropped++;
                        }
                    }
                }
                xSemaphoreGive(xBrokerMutex);
            }
            
            snprintf(cBuffer, sizeof(cBuffer), 
                     "[BROKER] Dispatched to %d subscribers\r\n", ucDispatchCount);
            UART_SendString(cBuffer);
            
            HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
        }
        
        /* Print statistics every 10 seconds */
        if ((xTaskGetTickCount() - xLastStatsTime) >= pdMS_TO_TICKS(10000))
        {
            xLastStatsTime = xTaskGetTickCount();
            
            UART_SendString("\r\n=== BROKER STATISTICS ===\r\n");
            snprintf(cBuffer, sizeof(cBuffer), 
                     "Published: %lu, Dispatched: %lu, Dropped: %lu\r\n",
                     xBrokerStats.ulPublished, xBrokerStats.ulDispatched,
                     xBrokerStats.ulDropped);
            UART_SendString(cBuffer);
            
            snprintf(cBuffer, sizeof(cBuffer), 
                     "Topics: TEMP=%lu, HUMID=%lu, EVENT=%lu, SYS=%lu\r\n",
                     xBrokerStats.ulTopicCounts[0], xBrokerStats.ulTopicCounts[1],
                     xBrokerStats.ulTopicCounts[2], xBrokerStats.ulTopicCounts[3]);
            UART_SendString(cBuffer);
            
            snprintf(cBuffer, sizeof(cBuffer), 
                     "Active Subscribers: %d\r\n", ucSubscriberCount);
            UART_SendString(cBuffer);
        }
    }
}

/**
 * Temperature Publisher Task
 */
static void vTempPublisherTask(void *pvParameters)
{
    (void)pvParameters;
    
    TickType_t xLastWakeTime;
    uint32_t ulTemp;
    
    UART_SendString("[TEMP PUB] Started\r\n");
    
    xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        /* Simulate temperature 20.0-30.0°C (x10) */
        ulTemp = 200 + (ulRandom() % 100);
        
        xPubSubPublish(TOPIC_TEMPERATURE, PUB_TEMP_SENSOR, ulTemp, 1);
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(2000));
    }
}

/**
 * Humidity Publisher Task
 */
static void vHumidPublisherTask(void *pvParameters)
{
    (void)pvParameters;
    
    TickType_t xLastWakeTime;
    uint32_t ulHumid;
    
    UART_SendString("[HUMID PUB] Started\r\n");
    
    xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        /* Simulate humidity 40-70% (x10) */
        ulHumid = 400 + (ulRandom() % 300);
        
        xPubSubPublish(TOPIC_HUMIDITY, PUB_HUMID_SENSOR, ulHumid, 1);
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(3000));
    }
}

/**
 * Event Publisher Task - Simulates button events
 */
static void vEventPublisherTask(void *pvParameters)
{
    (void)pvParameters;
    
    uint32_t ulEventId = 0;
    
    UART_SendString("[EVENT PUB] Started\r\n");
    
    for (;;)
    {
        /* Random event delay 5-10 seconds */
        vTaskDelay(pdMS_TO_TICKS(5000 + (ulRandom() % 5000)));
        
        ulEventId++;
        xPubSubPublish(TOPIC_EVENT, PUB_BUTTON, ulEventId, 2);
    }
}

/**
 * Display Subscriber Task - Shows TEMP and HUMID
 */
static void vDisplaySubscriberTask(void *pvParameters)
{
    QueueHandle_t xMyQueue = (QueueHandle_t)pvParameters;
    PubSubMessage_t xMsg;
    char cBuffer[80];
    
    UART_SendString("[DISPLAY] Subscriber started\r\n");
    
    for (;;)
    {
        if (xQueueReceive(xMyQueue, &xMsg, portMAX_DELAY) == pdTRUE)
        {
            switch (xMsg.ucTopicId)
            {
                case TOPIC_TEMPERATURE:
                    snprintf(cBuffer, sizeof(cBuffer), 
                             "[DISPLAY] Temp: %.1f°C\r\n", xMsg.ulData / 10.0f);
                    break;
                    
                case TOPIC_HUMIDITY:
                    snprintf(cBuffer, sizeof(cBuffer), 
                             "[DISPLAY] Humidity: %.1f%%\r\n", xMsg.ulData / 10.0f);
                    break;
                    
                default:
                    snprintf(cBuffer, sizeof(cBuffer), 
                             "[DISPLAY] Unknown: %lu\r\n", xMsg.ulData);
                    break;
            }
            UART_SendString(cBuffer);
        }
    }
}

/**
 * Logger Subscriber Task - Logs all messages
 */
static void vLoggerSubscriberTask(void *pvParameters)
{
    QueueHandle_t xMyQueue = (QueueHandle_t)pvParameters;
    PubSubMessage_t xMsg;
    char cBuffer[100];
    
    UART_SendString("[LOGGER] Subscriber started\r\n");
    
    for (;;)
    {
        if (xQueueReceive(xMyQueue, &xMsg, portMAX_DELAY) == pdTRUE)
        {
            snprintf(cBuffer, sizeof(cBuffer), 
                     "[LOGGER] [%s] Value=%lu @ %lums\r\n",
                     pcTopicName(xMsg.ucTopicId), xMsg.ulData, xMsg.ulTimestamp);
            UART_SendString(cBuffer);
        }
    }
}

/**
 * Alarm Subscriber Task - Monitors TEMP and EVENT for alarms
 */
static void vAlarmSubscriberTask(void *pvParameters)
{
    QueueHandle_t xMyQueue = (QueueHandle_t)pvParameters;
    PubSubMessage_t xMsg;
    char cBuffer[80];
    
    UART_SendString("[ALARM] Subscriber started\r\n");
    
    for (;;)
    {
        if (xQueueReceive(xMyQueue, &xMsg, portMAX_DELAY) == pdTRUE)
        {
            switch (xMsg.ucTopicId)
            {
                case TOPIC_TEMPERATURE:
                    {
                        float fTemp = xMsg.ulData / 10.0f;
                        const char *pcStatus = (fTemp > 28.0f) ? "HIGH!" : 
                                               (fTemp < 22.0f) ? "LOW!" : "OK";
                        snprintf(cBuffer, sizeof(cBuffer), 
                                 "[ALARM] Temp: %.1f°C - %s\r\n", fTemp, pcStatus);
                    }
                    break;
                    
                case TOPIC_EVENT:
                    snprintf(cBuffer, sizeof(cBuffer), 
                             "[ALARM] Event #%lu received!\r\n", xMsg.ulData);
                    break;
                    
                default:
                    cBuffer[0] = '\0';
                    break;
            }
            
            if (cBuffer[0])
            {
                UART_SendString(cBuffer);
            }
        }
    }
}

/* ========================== UART FUNCTIONS ================================ */

static void UART_SendString(const char *str)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}

/* ========================== HOOK FUNCTIONS ================================ */

void vApplicationMallocFailedHook(void)
{
    UART_SendString("MALLOC FAILED!\r\n");
    for (;;);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    UART_SendString("STACK OVERFLOW: ");
    UART_SendString(pcTaskName);
    UART_SendString("\r\n");
    for (;;);
}

/* ========================== HARDWARE INIT ================================= */

static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
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
    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);
    
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET); /* LED OFF */
}

static void UART1_Init(void)
{
    __HAL_RCC_USART1_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart1);
}

/* ========================== MAIN ========================================== */

int main(void)
{
    QueueHandle_t xDisplayQueue, xLoggerQueue, xAlarmQueue;
    
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    UART1_Init();

    UART_SendString("\r\n");
    UART_SendString("============================================\r\n");
    UART_SendString("    PROGRAM 47: PUB/SUB PATTERN DEMO\r\n");
    UART_SendString("============================================\r\n");
    UART_SendString("Publish-Subscribe messaging pattern\r\n\r\n");

    /* Initialize stats */
    memset((void *)&xBrokerStats, 0, sizeof(xBrokerStats));

    /* Create broker mutex */
    xBrokerMutex = xSemaphoreCreateMutex();
    if (xBrokerMutex == NULL)
    {
        UART_SendString("ERROR: Failed to create mutex!\r\n");
        for (;;);
    }
    UART_SendString("Broker mutex created\r\n");

    /* Create broker queue */
    xBrokerQueue = xQueueCreate(PUBSUB_BROKER_QUEUE_SIZE, sizeof(PubSubMessage_t));
    if (xBrokerQueue == NULL)
    {
        UART_SendString("ERROR: Failed to create broker queue!\r\n");
        for (;;);
    }
    UART_SendString("Broker queue created\r\n");

    /* Register subscribers */
    UART_SendString("\r\nRegistering subscribers...\r\n");
    
    /* Display subscribes to TEMP and HUMID */
    if (xPubSubSubscribe(SUB_DISPLAY, "Display", 
                         (1 << TOPIC_TEMPERATURE) | (1 << TOPIC_HUMIDITY), 
                         &xDisplayQueue) != pdTRUE)
    {
        UART_SendString("ERROR: Display subscribe failed!\r\n");
        for (;;);
    }
    UART_SendString("  Display: TEMP, HUMID\r\n");
    
    /* Logger subscribes to ALL (wildcard) */
    if (xPubSubSubscribe(SUB_LOGGER, "Logger", TOPIC_ALL, &xLoggerQueue) != pdTRUE)
    {
        UART_SendString("ERROR: Logger subscribe failed!\r\n");
        for (;;);
    }
    UART_SendString("  Logger: ALL (wildcard)\r\n");
    
    /* Alarm subscribes to TEMP and EVENT */
    if (xPubSubSubscribe(SUB_ALARM, "Alarm",
                         (1 << TOPIC_TEMPERATURE) | (1 << TOPIC_EVENT),
                         &xAlarmQueue) != pdTRUE)
    {
        UART_SendString("ERROR: Alarm subscribe failed!\r\n");
        for (;;);
    }
    UART_SendString("  Alarm: TEMP, EVENT\r\n");

    /* Create tasks */
    UART_SendString("\r\nCreating tasks...\r\n");
    
    /* Broker task */
    xTaskCreate(vBrokerTask, "Broker", configMINIMAL_STACK_SIZE + 80, 
                NULL, 3, &xBrokerTaskHandle);
    
    /* Publisher tasks */
    xTaskCreate(vTempPublisherTask, "TempPub", configMINIMAL_STACK_SIZE + 30, 
                NULL, 2, &xTempPubTaskHandle);
    xTaskCreate(vHumidPublisherTask, "HumidPub", configMINIMAL_STACK_SIZE + 30, 
                NULL, 2, &xHumidPubTaskHandle);
    xTaskCreate(vEventPublisherTask, "EventPub", configMINIMAL_STACK_SIZE + 30, 
                NULL, 2, &xEventPubTaskHandle);
    
    /* Subscriber tasks */
    xTaskCreate(vDisplaySubscriberTask, "Display", configMINIMAL_STACK_SIZE + 30, 
                xDisplayQueue, 1, NULL);
    xTaskCreate(vLoggerSubscriberTask, "Logger", configMINIMAL_STACK_SIZE + 30, 
                xLoggerQueue, 1, NULL);
    xTaskCreate(vAlarmSubscriberTask, "Alarm", configMINIMAL_STACK_SIZE + 30, 
                xAlarmQueue, 1, NULL);

    UART_SendString("\r\nStarting scheduler...\r\n");
    UART_SendString("----------------------------------------\r\n\r\n");

    vTaskStartScheduler();

    for (;;);
}
