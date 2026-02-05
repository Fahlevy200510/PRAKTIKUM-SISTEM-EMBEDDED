/**
 * =============================================================================
 * PROGRAM 45: DATA LOGGER - Circular Buffer Data Logging dengan FreeRTOS
 * =============================================================================
 * 
 * Demo data logger dengan circular buffer untuk log data ADC simulasi,
 * event counter, dan system statistics.
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

/* Log entry structure */
typedef struct {
    uint32_t ulTimestamp;     /* Tick count (ms since boot) */
    uint8_t  ucChannel;       /* Source channel (0-7) */
    uint8_t  ucType;          /* Data type (ADC/EVENT/SYS) */
    uint16_t usValue;         /* Actual data value */
    uint8_t  ucFlags;         /* Status flags */
    uint8_t  ucPadding[3];    /* Alignment padding */
} LogEntry_t;

/* Circular buffer structure */
typedef struct {
    LogEntry_t xEntries[LOG_BUFFER_SIZE];
    uint32_t ulHead;          /* Write position */
    uint32_t ulTail;          /* Read position */
    uint32_t ulCount;         /* Current count */
    uint32_t ulTotalWrites;   /* Total writes */
    uint32_t ulOverwrites;    /* Overwrites count */
    uint32_t ulDropped;       /* Dropped entries (queue full) */
} CircularBuffer_t;

/* Buffer statistics */
typedef struct {
    uint32_t ulChannelCounts[4];  /* Per-channel entry counts */
    uint32_t ulLastExportTime;    /* Last export timestamp */
    uint32_t ulExportCount;       /* Total exports done */
} BufferStats_t;

/* ========================== GLOBALS ======================================= */

UART_HandleTypeDef huart1;

/* Circular buffer */
static CircularBuffer_t xLogBuffer;

/* Buffer access mutex */
static SemaphoreHandle_t xBufferMutex;

/* Log queue from producers to logger task */
static QueueHandle_t xLogQueue;

/* Task handles */
static TaskHandle_t xAdcTaskHandle;
static TaskHandle_t xEventTaskHandle;
static TaskHandle_t xSystemTaskHandle;
static TaskHandle_t xLoggerTaskHandle;

/* Buffer statistics */
static volatile BufferStats_t xStats;

/* Random seed for simulation */
static uint32_t ulRandomSeed = 12345;

/* Export requested flag */
static volatile BaseType_t xExportRequested = pdFALSE;

/* ========================== FUNCTION PROTOTYPES =========================== */

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART1_Init(void);
static void UART_SendString(const char *str);

static void vAdcTask(void *pvParameters);
static void vEventTask(void *pvParameters);
static void vSystemTask(void *pvParameters);
static void vLoggerTask(void *pvParameters);

static BaseType_t xBufferWrite(const LogEntry_t *pxEntry);
static BaseType_t xBufferRead(LogEntry_t *pxEntry);
static uint32_t ulBufferGetCount(void);
static void vExportBuffer(void);

/* ========================== RANDOM NUMBER GENERATOR ======================= */

static uint32_t ulRandom(void)
{
    ulRandomSeed = ulRandomSeed * 1103515245 + 12345;
    return (ulRandomSeed >> 16) & 0x7FFF;
}

/* ========================== CIRCULAR BUFFER FUNCTIONS ===================== */

/**
 * Initialize circular buffer
 */
static void vBufferInit(void)
{
    memset(&xLogBuffer, 0, sizeof(xLogBuffer));
    memset((void *)&xStats, 0, sizeof(xStats));
}

/**
 * Write entry to circular buffer
 * Returns pdTRUE if successful
 */
static BaseType_t xBufferWrite(const LogEntry_t *pxEntry)
{
    BaseType_t xResult = pdFALSE;
    
    if (xSemaphoreTake(xBufferMutex, pdMS_TO_TICKS(50)) == pdTRUE)
    {
        /* Copy entry to buffer */
        memcpy(&xLogBuffer.xEntries[xLogBuffer.ulHead], pxEntry, sizeof(LogEntry_t));
        
        /* Advance head */
        xLogBuffer.ulHead = (xLogBuffer.ulHead + 1) % LOG_BUFFER_SIZE;
        xLogBuffer.ulTotalWrites++;
        
        /* Handle overflow (overwrite old data) */
        if (xLogBuffer.ulCount >= LOG_BUFFER_SIZE)
        {
            /* Buffer full, advance tail (lose oldest entry) */
            xLogBuffer.ulTail = (xLogBuffer.ulTail + 1) % LOG_BUFFER_SIZE;
            xLogBuffer.ulOverwrites++;
        }
        else
        {
            xLogBuffer.ulCount++;
        }
        
        /* Update channel stats */
        if (pxEntry->ucChannel < 4)
        {
            xStats.ulChannelCounts[pxEntry->ucChannel]++;
        }
        
        xResult = pdTRUE;
        xSemaphoreGive(xBufferMutex);
    }
    
    return xResult;
}

/**
 * Read and remove entry from circular buffer
 * Returns pdTRUE if entry available
 */
static BaseType_t xBufferRead(LogEntry_t *pxEntry)
{
    BaseType_t xResult = pdFALSE;
    
    if (xSemaphoreTake(xBufferMutex, pdMS_TO_TICKS(50)) == pdTRUE)
    {
        if (xLogBuffer.ulCount > 0)
        {
            /* Copy entry from buffer */
            memcpy(pxEntry, &xLogBuffer.xEntries[xLogBuffer.ulTail], sizeof(LogEntry_t));
            
            /* Advance tail */
            xLogBuffer.ulTail = (xLogBuffer.ulTail + 1) % LOG_BUFFER_SIZE;
            xLogBuffer.ulCount--;
            
            xResult = pdTRUE;
        }
        xSemaphoreGive(xBufferMutex);
    }
    
    return xResult;
}

/**
 * Get current buffer count
 */
static uint32_t ulBufferGetCount(void)
{
    uint32_t ulCount = 0;
    
    if (xSemaphoreTake(xBufferMutex, pdMS_TO_TICKS(10)) == pdTRUE)
    {
        ulCount = xLogBuffer.ulCount;
        xSemaphoreGive(xBufferMutex);
    }
    
    return ulCount;
}

/**
 * Export buffer contents to UART (CSV format)
 */
static void vExportBuffer(void)
{
    LogEntry_t xEntry;
    char cBuffer[80];
    uint32_t ulExported = 0;
    const char *pcTypes[] = {"ADC", "EVENT", "SYS"};
    
    UART_SendString("\r\n[EXPORT] Starting data export...\r\n");
    UART_SendString("TIMESTAMP,CHANNEL,TYPE,VALUE,FLAGS\r\n");
    
    while (xBufferRead(&xEntry) == pdTRUE)
    {
        const char *pcType = (xEntry.ucType < 3) ? pcTypes[xEntry.ucType] : "???";
        
        snprintf(cBuffer, sizeof(cBuffer), "%lu,%d,%s,%u,%d\r\n",
                 xEntry.ulTimestamp,
                 xEntry.ucChannel,
                 pcType,
                 xEntry.usValue,
                 xEntry.ucFlags);
        UART_SendString(cBuffer);
        
        ulExported++;
    }
    
    snprintf(cBuffer, sizeof(cBuffer), 
             "[EXPORT] Complete. %lu entries exported.\r\n", ulExported);
    UART_SendString(cBuffer);
    
    xStats.ulLastExportTime = xTaskGetTickCount();
    xStats.ulExportCount++;
}

/* ========================== TASKS ========================================= */

/**
 * ADC Task - Simulates ADC readings
 */
static void vAdcTask(void *pvParameters)
{
    (void)pvParameters;
    
    TickType_t xLastWakeTime;
    LogEntry_t xEntry;
    uint16_t usBaseValue = 2048;  /* 12-bit ADC mid-point */
    
    UART_SendString("[ADC] Task started\r\n");
    
    xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        /* Simulate ADC channel 0 reading */
        xEntry.ulTimestamp = xTaskGetTickCount();
        xEntry.ucChannel = LOG_CHANNEL_ADC0;
        xEntry.ucType = LOG_TYPE_ADC;
        xEntry.usValue = usBaseValue + (ulRandom() % 200) - 100; /* 1948-2148 */
        xEntry.ucFlags = 0;
        
        /* Set flag if exceeds threshold */
        if (xEntry.usValue > 2100)
        {
            xEntry.ucFlags = 1;
        }
        
        /* Send to logger queue */
        if (xQueueSend(xLogQueue, &xEntry, pdMS_TO_TICKS(10)) != pdPASS)
        {
            xLogBuffer.ulDropped++;
        }
        
        /* Simulate ADC channel 1 reading */
        xEntry.ucChannel = LOG_CHANNEL_ADC1;
        xEntry.usValue = usBaseValue + (ulRandom() % 400) - 200; /* 1848-2248 */
        xEntry.ucFlags = (xEntry.usValue > 2100 || xEntry.usValue < 1900) ? 1 : 0;
        
        if (xQueueSend(xLogQueue, &xEntry, pdMS_TO_TICKS(10)) != pdPASS)
        {
            xLogBuffer.ulDropped++;
        }
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(ADC_SAMPLE_PERIOD_MS));
    }
}

/**
 * Event Task - Simulates event counting
 */
static void vEventTask(void *pvParameters)
{
    (void)pvParameters;
    
    LogEntry_t xEntry;
    uint16_t usEventCount = 0;
    
    UART_SendString("[Event] Task started\r\n");
    
    for (;;)
    {
        /* Random delay between events (150-500ms) */
        vTaskDelay(pdMS_TO_TICKS(150 + (ulRandom() % 350)));
        
        usEventCount++;
        
        xEntry.ulTimestamp = xTaskGetTickCount();
        xEntry.ucChannel = LOG_CHANNEL_EVENT;
        xEntry.ucType = LOG_TYPE_EVENT;
        xEntry.usValue = usEventCount;
        xEntry.ucFlags = 0;
        
        if (xQueueSend(xLogQueue, &xEntry, pdMS_TO_TICKS(10)) != pdPASS)
        {
            xLogBuffer.ulDropped++;
        }
    }
}

/**
 * System Task - Logs system statistics
 */
static void vSystemTask(void *pvParameters)
{
    (void)pvParameters;
    
    TickType_t xLastWakeTime;
    LogEntry_t xEntry;
    
    UART_SendString("[System] Task started\r\n");
    
    xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        /* Log heap free */
        xEntry.ulTimestamp = xTaskGetTickCount();
        xEntry.ucChannel = LOG_CHANNEL_SYSTEM;
        xEntry.ucType = LOG_TYPE_SYSTEM;
        xEntry.usValue = (uint16_t)(xPortGetFreeHeapSize() & 0xFFFF);
        xEntry.ucFlags = 0;
        
        if (xQueueSend(xLogQueue, &xEntry, pdMS_TO_TICKS(10)) != pdPASS)
        {
            xLogBuffer.ulDropped++;
        }
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(SYSTEM_LOG_PERIOD_MS));
    }
}

/**
 * Logger Task - Receives log entries and writes to buffer
 */
static void vLoggerTask(void *pvParameters)
{
    (void)pvParameters;
    
    LogEntry_t xEntry;
    char cBuffer[100];
    uint32_t ulEntryNumber = 0;
    const char *pcTypes[] = {"ADC", "EVENT", "SYS"};
    TickType_t xLastStatusTime = 0;
    
    UART_SendString("[Logger] Task started\r\n");
    
    for (;;)
    {
        /* Wait for log entry */
        if (xQueueReceive(xLogQueue, &xEntry, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            /* Write to circular buffer */
            if (xBufferWrite(&xEntry) == pdTRUE)
            {
                ulEntryNumber++;
                
                /* Print entry info */
                const char *pcType = (xEntry.ucType < 3) ? pcTypes[xEntry.ucType] : "???";
                snprintf(cBuffer, sizeof(cBuffer), 
                         "[LOG] #%lu: CH%d %s=%u @ %lums",
                         ulEntryNumber, xEntry.ucChannel, pcType,
                         xEntry.usValue, xEntry.ulTimestamp);
                
                if (xEntry.ucFlags)
                {
                    strcat(cBuffer, " [!]");
                }
                strcat(cBuffer, "\r\n");
                UART_SendString(cBuffer);
                
                /* LED blink on log */
                HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
            }
        }
        
        /* Check for export trigger */
        if (xExportRequested || ulBufferGetCount() >= EXPORT_TRIGGER_ENTRIES)
        {
            xExportRequested = pdFALSE;
            vExportBuffer();
        }
        
        /* Periodic buffer status (every 5 seconds) */
        if ((xTaskGetTickCount() - xLastStatusTime) >= pdMS_TO_TICKS(5000))
        {
            xLastStatusTime = xTaskGetTickCount();
            
            if (xSemaphoreTake(xBufferMutex, pdMS_TO_TICKS(50)) == pdTRUE)
            {
                UART_SendString("\r\n=== BUFFER STATUS ===\r\n");
                snprintf(cBuffer, sizeof(cBuffer), 
                         "Used: %lu/%d (%lu%%)\r\n",
                         xLogBuffer.ulCount, LOG_BUFFER_SIZE,
                         (xLogBuffer.ulCount * 100) / LOG_BUFFER_SIZE);
                UART_SendString(cBuffer);
                
                snprintf(cBuffer, sizeof(cBuffer), 
                         "Total writes: %lu\r\n", xLogBuffer.ulTotalWrites);
                UART_SendString(cBuffer);
                
                snprintf(cBuffer, sizeof(cBuffer), 
                         "Overwrites: %lu\r\n", xLogBuffer.ulOverwrites);
                UART_SendString(cBuffer);
                
                snprintf(cBuffer, sizeof(cBuffer), 
                         "Dropped: %lu\r\n", xLogBuffer.ulDropped);
                UART_SendString(cBuffer);
                
                UART_SendString("Channel stats:\r\n");
                snprintf(cBuffer, sizeof(cBuffer), 
                         "  CH0 (ADC0): %lu\r\n", xStats.ulChannelCounts[0]);
                UART_SendString(cBuffer);
                snprintf(cBuffer, sizeof(cBuffer), 
                         "  CH1 (ADC1): %lu\r\n", xStats.ulChannelCounts[1]);
                UART_SendString(cBuffer);
                snprintf(cBuffer, sizeof(cBuffer), 
                         "  CH2 (EVENT): %lu\r\n", xStats.ulChannelCounts[2]);
                UART_SendString(cBuffer);
                snprintf(cBuffer, sizeof(cBuffer), 
                         "  CH3 (SYS): %lu\r\n", xStats.ulChannelCounts[3]);
                UART_SendString(cBuffer);
                
                snprintf(cBuffer, sizeof(cBuffer), 
                         "Heap free: %u bytes\r\n", 
                         (unsigned int)xPortGetFreeHeapSize());
                UART_SendString(cBuffer);
                
                xSemaphoreGive(xBufferMutex);
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
    
    /* PA9 = TX, PA10 = RX */
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
    char cBuffer[80];
    
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    UART1_Init();

    UART_SendString("\r\n");
    UART_SendString("============================================\r\n");
    UART_SendString("    PROGRAM 45: DATA LOGGER DEMO\r\n");
    UART_SendString("============================================\r\n");
    UART_SendString("Circular buffer data logging system\r\n\r\n");

    /* Initialize circular buffer */
    vBufferInit();
    snprintf(cBuffer, sizeof(cBuffer), 
             "Buffer size: %d entries (%u bytes)\r\n",
             LOG_BUFFER_SIZE, (unsigned int)(LOG_BUFFER_SIZE * sizeof(LogEntry_t)));
    UART_SendString(cBuffer);

    /* Create buffer mutex */
    xBufferMutex = xSemaphoreCreateMutex();
    if (xBufferMutex == NULL)
    {
        UART_SendString("ERROR: Failed to create buffer mutex!\r\n");
        for (;;);
    }
    UART_SendString("Buffer mutex created\r\n");

    /* Create log queue */
    xLogQueue = xQueueCreate(LOG_QUEUE_LENGTH, sizeof(LogEntry_t));
    if (xLogQueue == NULL)
    {
        UART_SendString("ERROR: Failed to create log queue!\r\n");
        for (;;);
    }
    UART_SendString("Log queue created\r\n");

    /* Create tasks */
    UART_SendString("Creating tasks...\r\n");
    
    if (xTaskCreate(vAdcTask, "ADC", configMINIMAL_STACK_SIZE + 30, 
                    NULL, 2, &xAdcTaskHandle) != pdPASS)
    {
        UART_SendString("ERROR: Failed to create ADC task!\r\n");
        for (;;);
    }
    UART_SendString("  ADC task created\r\n");
    
    if (xTaskCreate(vEventTask, "Event", configMINIMAL_STACK_SIZE + 30, 
                    NULL, 2, &xEventTaskHandle) != pdPASS)
    {
        UART_SendString("ERROR: Failed to create Event task!\r\n");
        for (;;);
    }
    UART_SendString("  Event task created\r\n");
    
    if (xTaskCreate(vSystemTask, "System", configMINIMAL_STACK_SIZE + 30, 
                    NULL, 1, &xSystemTaskHandle) != pdPASS)
    {
        UART_SendString("ERROR: Failed to create System task!\r\n");
        for (;;);
    }
    UART_SendString("  System task created\r\n");
    
    if (xTaskCreate(vLoggerTask, "Logger", configMINIMAL_STACK_SIZE + 80, 
                    NULL, 3, &xLoggerTaskHandle) != pdPASS)
    {
        UART_SendString("ERROR: Failed to create Logger task!\r\n");
        for (;;);
    }
    UART_SendString("  Logger task created\r\n");

    UART_SendString("\r\nStarting scheduler...\r\n");
    UART_SendString("----------------------------------------\r\n\r\n");

    vTaskStartScheduler();

    /* Should never reach here */
    for (;;);
}
