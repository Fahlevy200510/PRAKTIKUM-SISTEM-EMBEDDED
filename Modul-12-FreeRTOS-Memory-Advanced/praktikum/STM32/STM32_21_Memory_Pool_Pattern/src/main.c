/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 37-Memory_Pool_Pattern
 * 
 * DESKRIPSI:
 * Implementasi Memory Pool pattern untuk fixed-size block allocation.
 * Menunjukkan O(1) allocation/free dengan zero fragmentation.
 * 
 * ============================================================================
 * ARSITEKTUR SISTEM
 * ============================================================================
 * 
 *    ┌──────────────────────────────────────────────────────────────────────┐
 *    │                      MEMORY POOL DEMO                                │
 *    │                                                                      │
 *    │   Producer Task                           Consumer Task              │
 *    │   ┌─────────────────┐                     ┌─────────────────┐        │
 *    │   │  Alloc block    │                     │  Receive ptr    │        │
 *    │   │  Fill with data │                     │  Process data   │        │
 *    │   │  Send via queue │                     │  Free block     │        │
 *    │   └────────┬────────┘                     └────────▲────────┘        │
 *    │            │                                       │                 │
 *    │            ▼                                       │                 │
 *    │   ┌────────────────────────────────────────────────┴────────┐        │
 *    │   │                    MEMORY POOL                          │        │
 *    │   │                                                         │        │
 *    │   │  ┌──────┬──────┬──────┬──────┬──────┬──────┬──────┬──┐ │        │
 *    │   │  │ U    │  F   │  U   │  F   │  F   │  U   │  F   │...│ │        │
 *    │   │  └──────┴──────┴──────┴──────┴──────┴──────┴──────┴──┘ │        │
 *    │   │     ↑             ↑      ↑             ↑               │        │
 *    │   │    Used         Free   Free          Used              │        │
 *    │   │                                                         │        │
 *    │   │  Free List: F1 → F3 → F4 → F6 → NULL                   │        │
 *    │   └─────────────────────────────────────────────────────────┘        │
 *    │                                                                      │
 *    └──────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * CONTOH OUTPUT SERIAL
 * ============================================================================
 * 
 *    === MEMORY POOL DEMO ===
 *    Pool: 10 blocks × 32 bytes
 *    
 *    [Pool] Alloc block 0 (9 remaining)
 *    [Producer] Sent message in block 0
 *    [Pool] Alloc block 1 (8 remaining)
 *    [Pool] Free block 0 (9 remaining)
 *    [Consumer] Processed message from block 0
 *    
 *    [Monitor] Pool status: 8/10 free (80%)
 * 
 * ============================================================================
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * MEMORY POOL IMPLEMENTATION
 * ============================================================================ */

/* Block header for free list linkage */
typedef struct PoolBlock {
    struct PoolBlock *pxNext;
} PoolBlock_t;

/* Memory pool structure */
typedef struct {
    uint8_t *pucMemory;           /* Pool memory area */
    PoolBlock_t *pxFreeList;      /* Free block list */
    size_t xBlockSize;            /* Size per block */
    size_t xBlockCount;           /* Total blocks */
    size_t xFreeCount;            /* Free blocks */
    SemaphoreHandle_t xMutex;     /* Thread protection */
} MemoryPool_t;

/* Pool statistics */
typedef struct {
    uint32_t ulAllocations;
    uint32_t ulFrees;
    uint32_t ulAllocFails;
    size_t xMinFree;
} PoolStats_t;

static PoolStats_t xPoolStats = {0, 0, 0, POOL_BLOCK_COUNT};

/* Memory pool storage (static allocation) */
static uint8_t ucPoolMemory[POOL_BLOCK_COUNT * POOL_BLOCK_SIZE];
static MemoryPool_t xPool;

/*
 * Initialize memory pool
 */
static BaseType_t MemoryPool_Init(MemoryPool_t *pxPool, 
                                   uint8_t *pucMemory,
                                   size_t xBlockSize, 
                                   size_t xBlockCount)
{
    /* Block must be large enough for free list pointer */
    if (xBlockSize < sizeof(PoolBlock_t))
    {
        xBlockSize = sizeof(PoolBlock_t);
    }
    
    pxPool->pucMemory = pucMemory;
    pxPool->xBlockSize = xBlockSize;
    pxPool->xBlockCount = xBlockCount;
    pxPool->xFreeCount = xBlockCount;
    
    /* Create mutex for thread safety */
    pxPool->xMutex = xSemaphoreCreateMutex();
    if (pxPool->xMutex == NULL)
    {
        return pdFALSE;
    }
    
    /* Initialize free list - link all blocks */
    pxPool->pxFreeList = NULL;
    for (size_t i = 0; i < xBlockCount; i++)
    {
        PoolBlock_t *pxBlock = (PoolBlock_t *)(pucMemory + i * xBlockSize);
        pxBlock->pxNext = pxPool->pxFreeList;
        pxPool->pxFreeList = pxBlock;
    }
    
    return pdTRUE;
}

/*
 * Allocate a block from pool - O(1)
 */
static void *MemoryPool_Alloc(MemoryPool_t *pxPool)
{
    void *pvBlock = NULL;
    
    if (xSemaphoreTake(pxPool->xMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        if (pxPool->pxFreeList != NULL)
        {
            /* Get block from head of free list */
            pvBlock = pxPool->pxFreeList;
            pxPool->pxFreeList = pxPool->pxFreeList->pxNext;
            pxPool->xFreeCount--;
            
            /* Update statistics */
            xPoolStats.ulAllocations++;
            if (pxPool->xFreeCount < xPoolStats.xMinFree)
            {
                xPoolStats.xMinFree = pxPool->xFreeCount;
            }
        }
        else
        {
            xPoolStats.ulAllocFails++;
        }
        
        xSemaphoreGive(pxPool->xMutex);
    }
    
    return pvBlock;
}

/*
 * Free a block back to pool - O(1)
 */
static void MemoryPool_Free(MemoryPool_t *pxPool, void *pvBlock)
{
    if (pvBlock == NULL) return;
    
    if (xSemaphoreTake(pxPool->xMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        /* Add block to head of free list */
        PoolBlock_t *pxBlock = (PoolBlock_t *)pvBlock;
        pxBlock->pxNext = pxPool->pxFreeList;
        pxPool->pxFreeList = pxBlock;
        pxPool->xFreeCount++;
        
        xPoolStats.ulFrees++;
        
        xSemaphoreGive(pxPool->xMutex);
    }
}

/* ============================================================================
 * APPLICATION DATA
 * ============================================================================ */

/* Message structure that fits in pool block */
typedef struct {
    uint32_t ulId;
    uint32_t ulTimestamp;
    uint8_t ucData[24];  /* Total = 32 bytes */
} Message_t;

/* ============================================================================
 * GLOBAL VARIABLES
 * ============================================================================ */
UART_HandleTypeDef huart1;
QueueHandle_t xPtrQueue = NULL;  /* Queue of pointers */

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
    
    char msg[120];
    
    UART_SendString("\r\n\r\n");
    UART_SendString("╔══════════════════════════════════════════════════════════╗\r\n");
    UART_SendString("║           37. MEMORY POOL PATTERN DEMO                   ║\r\n");
    UART_SendString("╚══════════════════════════════════════════════════════════╝\r\n\r\n");
    
    snprintf(msg, sizeof(msg), "[Config] Pool: %d blocks × %d bytes = %d bytes\r\n",
             POOL_BLOCK_COUNT, POOL_BLOCK_SIZE, 
             POOL_BLOCK_COUNT * POOL_BLOCK_SIZE);
    UART_SendString(msg);
    
    /* Initialize memory pool */
    if (MemoryPool_Init(&xPool, ucPoolMemory, POOL_BLOCK_SIZE, POOL_BLOCK_COUNT))
    {
        UART_SendString("[OK] Memory Pool initialized\r\n");
    }
    else
    {
        UART_SendString("[ERROR] Pool init failed!\r\n");
        while (1);
    }
    
    /* Create queue for message pointers */
    xPtrQueue = xQueueCreate(POOL_BLOCK_COUNT, sizeof(Message_t *));
    if (xPtrQueue == NULL)
    {
        UART_SendString("[ERROR] Queue creation failed!\r\n");
        while (1);
    }
    
    /* Create tasks */
    xTaskCreate(prvProducerTask, "Producer", TASK_STACK, NULL, 2, NULL);
    xTaskCreate(prvConsumerTask, "Consumer", TASK_STACK, NULL, 3, NULL);
    xTaskCreate(prvMonitorTask, "Monitor", TASK_STACK, NULL, 1, NULL);
    
    UART_SendString("\r\n────────────────────────────────────────────────────────────\r\n");
    
    vTaskStartScheduler();
    
    while (1);
}

/* ============================================================================
 * PRODUCER TASK
 * Allocates blocks from pool, fills with data, sends pointer
 * ============================================================================ */
static void prvProducerTask(void *pvParameters)
{
    (void)pvParameters;
    
    char msg[100];
    uint32_t ulMsgId = 0;
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        ulMsgId++;
        
        /* Allocate from pool - O(1) */
        Message_t *pxMsg = (Message_t *)MemoryPool_Alloc(&xPool);
        
        if (pxMsg != NULL)
        {
            /* Fill message */
            pxMsg->ulId = ulMsgId;
            pxMsg->ulTimestamp = xTaskGetTickCount();
            snprintf((char *)pxMsg->ucData, sizeof(pxMsg->ucData), 
                     "Msg#%lu", ulMsgId);
            
            snprintf(msg, sizeof(msg), 
                     "[Producer] Alloc: ID=%lu, Free=%u/%d\r\n",
                     ulMsgId, (unsigned)xPool.xFreeCount, POOL_BLOCK_COUNT);
            UART_SendString(msg);
            
            /* Send pointer (not copy!) to consumer */
            if (xQueueSend(xPtrQueue, &pxMsg, pdMS_TO_TICKS(100)) != pdTRUE)
            {
                /* Queue full - free the block */
                MemoryPool_Free(&xPool, pxMsg);
                UART_SendString("         [WARN] Queue full, freed block\r\n");
            }
        }
        else
        {
            UART_SendString("[Producer] Pool exhausted! Waiting...\r\n");
        }
        
        HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));
    }
}

/* ============================================================================
 * CONSUMER TASK
 * Receives pointer, processes data, frees block back to pool
 * ============================================================================ */
static void prvConsumerTask(void *pvParameters)
{
    (void)pvParameters;
    
    char msg[100];
    Message_t *pxMsg;
    
    for (;;)
    {
        /* Receive pointer */
        if (xQueueReceive(xPtrQueue, &pxMsg, portMAX_DELAY) == pdTRUE)
        {
            /* Process message */
            snprintf(msg, sizeof(msg), 
                     "[Consumer] Process: ID=%lu, Time=%lu, Data='%s'\r\n",
                     pxMsg->ulId, pxMsg->ulTimestamp, pxMsg->ucData);
            UART_SendString(msg);
            
            /* Simulate processing time */
            vTaskDelay(pdMS_TO_TICKS(100));
            
            /* Free block back to pool - O(1) */
            MemoryPool_Free(&xPool, pxMsg);
            
            snprintf(msg, sizeof(msg), 
                     "          Freed block (Free=%u/%d)\r\n",
                     (unsigned)xPool.xFreeCount, POOL_BLOCK_COUNT);
            UART_SendString(msg);
        }
    }
}

/* ============================================================================
 * MONITOR TASK
 * Reports pool statistics
 * ============================================================================ */
static void prvMonitorTask(void *pvParameters)
{
    (void)pvParameters;
    
    char msg[120];
    const TickType_t xPeriod = pdMS_TO_TICKS(5000);
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
        
        UART_SendString("\r\n┌─── MEMORY POOL STATISTICS ───────────────────────────────┐\r\n");
        
        snprintf(msg, sizeof(msg), "│  Pool size: %d blocks × %d bytes                         │\r\n",
                 POOL_BLOCK_COUNT, POOL_BLOCK_SIZE);
        UART_SendString(msg);
        
        snprintf(msg, sizeof(msg), "│  Free blocks: %u/%d (%u%%)                                │\r\n",
                 (unsigned)xPool.xFreeCount, POOL_BLOCK_COUNT,
                 (unsigned)(xPool.xFreeCount * 100 / POOL_BLOCK_COUNT));
        UART_SendString(msg);
        
        snprintf(msg, sizeof(msg), "│  Min free ever: %u blocks                                │\r\n",
                 (unsigned)xPoolStats.xMinFree);
        UART_SendString(msg);
        
        snprintf(msg, sizeof(msg), "│  Total allocations: %lu                                   │\r\n",
                 xPoolStats.ulAllocations);
        UART_SendString(msg);
        
        snprintf(msg, sizeof(msg), "│  Total frees: %lu                                         │\r\n",
                 xPoolStats.ulFrees);
        UART_SendString(msg);
        
        snprintf(msg, sizeof(msg), "│  Allocation failures: %lu                                 │\r\n",
                 xPoolStats.ulAllocFails);
        UART_SendString(msg);
        
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
