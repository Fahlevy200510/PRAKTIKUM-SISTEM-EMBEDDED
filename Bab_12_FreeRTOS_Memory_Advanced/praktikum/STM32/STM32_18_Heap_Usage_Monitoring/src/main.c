/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 34-Heap_Usage_Monitoring
 * 
 * DESKRIPSI:
 * Demo pemantauan penggunaan heap FreeRTOS. Menunjukkan cara menggunakan
 * xPortGetFreeHeapSize(), xPortGetMinimumEverFreeHeapSize(), dan
 * vPortGetHeapStats() untuk analisis memory.
 * 
 * ============================================================================
 * ARSITEKTUR SISTEM
 * ============================================================================
 * 
 *    ┌──────────────────────────────────────────────────────────────────────┐
 *    │                    HEAP MONITORING DEMO                              │
 *    │                                                                      │
 *    │   Allocator Task                          Monitor Task               │
 *    │   ┌─────────────────┐                     ┌─────────────────┐        │
 *    │   │  Periodic       │                     │  Display heap   │        │
 *    │   │  allocations:   │                     │  statistics:    │        │
 *    │   │  - pvPortMalloc │                     │  - Free bytes   │        │
 *    │   │  - vPortFree    │                     │  - Min ever     │        │
 *    │   │  (simulate      │                     │  - Fragmentation│        │
 *    │   │   dynamic use)  │                     │                 │        │
 *    │   └────────┬────────┘                     └────────▲────────┘        │
 *    │            │                                       │                 │
 *    │            ▼                                       │                 │
 *    │   ┌────────────────────────────────────────────────┴────────┐        │
 *    │   │                       HEAP                              │        │
 *    │   │  ┌─────────────────────────────────────────────────────┐│        │
 *    │   │  │▓▓▓▓▓▓▓▓▓▓▓▓▓│░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░││        │
 *    │   │  └─────────────────────────────────────────────────────┘│        │
 *    │   │    Used          Free                                   │        │
 *    │   └─────────────────────────────────────────────────────────┘        │
 *    │                                                                      │
 *    └──────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * HEAP STATISTICS STRUCTURE
 * ============================================================================
 * 
 *    typedef struct HeapStats_t {
 *        size_t xAvailableHeapSpaceInBytes;      // Current free
 *        size_t xSizeOfLargestFreeBlockInBytes;  // Largest contiguous
 *        size_t xSizeOfSmallestFreeBlockInBytes; // Smallest fragment
 *        size_t xNumberOfFreeBlocks;             // Fragmentation indicator
 *        size_t xMinimumEverFreeBytesRemaining;  // High water mark
 *        size_t xNumberOfSuccessfulAllocations;  // Total mallocs
 *        size_t xNumberOfSuccessfulFrees;        // Total frees
 *    } HeapStats_t;
 * 
 * ============================================================================
 * CONTOH OUTPUT SERIAL
 * ============================================================================
 * 
 *    === HEAP MONITORING DEMO ===
 *    
 *    [Monitor] Initial heap state:
 *              Total heap: 10240 bytes
 *              Free: 6144 bytes (60%)
 *              Min ever: 6144 bytes
 *    
 *    [Allocator] Allocated 256 bytes at 0x20001234
 *    [Allocator] Allocated 128 bytes at 0x20001340
 *    
 *    [Monitor] Current heap state:
 *              Free: 5760 bytes (56%)
 *              Min ever: 5760 bytes
 *              Free blocks: 1
 *    
 *    [Allocator] Freed 256 bytes
 *    
 *    [Monitor] After free:
 *              Free: 6016 bytes (59%)
 *              Free blocks: 2 (fragmented!)
 *              Largest block: 5760 bytes
 * 
 * ============================================================================
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * ALLOCATION TRACKER
 * ============================================================================ */

#define MAX_ALLOCATIONS  8

typedef struct {
    void *pvPtr;
    size_t xSize;
    uint32_t ulAllocTime;
} AllocationRecord_t;

static AllocationRecord_t xAllocations[MAX_ALLOCATIONS] = {0};
static uint32_t ulAllocCount = 0;

/* ============================================================================
 * GLOBAL VARIABLES
 * ============================================================================ */
UART_HandleTypeDef huart1;

/* ============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================ */
static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART1_Init(void);
static void prvAllocatorTask(void *pvParameters);
static void prvMonitorTask(void *pvParameters);
static void UART_SendString(const char *str);
static void PrintHeapStats(const char *label);

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
    
    char msg[100];
    
    UART_SendString("\r\n\r\n");
    UART_SendString("╔══════════════════════════════════════════════════════════╗\r\n");
    UART_SendString("║          34. HEAP USAGE MONITORING DEMO                  ║\r\n");
    UART_SendString("╚══════════════════════════════════════════════════════════╝\r\n\r\n");
    
    /* Show initial heap state BEFORE any FreeRTOS objects created */
    snprintf(msg, sizeof(msg), "[Config] Total heap: %d bytes\r\n", configTOTAL_HEAP_SIZE);
    UART_SendString(msg);
    
    size_t xInitialFree = xPortGetFreeHeapSize();
    snprintf(msg, sizeof(msg), "[Config] Initial free: %u bytes\r\n\r\n", (unsigned)xInitialFree);
    UART_SendString(msg);
    
    /* Create tasks - this will allocate from heap */
    UART_SendString("[INFO] Creating tasks (allocating from heap)...\r\n");
    
    xTaskCreate(prvAllocatorTask, "Allocator", TASK_STACK, NULL, 2, NULL);
    xTaskCreate(prvMonitorTask, "Monitor", TASK_STACK, NULL, 1, NULL);
    
    /* Show heap after task creation */
    size_t xAfterTasks = xPortGetFreeHeapSize();
    snprintf(msg, sizeof(msg), "[INFO] After task creation: %u bytes free\r\n",
             (unsigned)xAfterTasks);
    UART_SendString(msg);
    snprintf(msg, sizeof(msg), "[INFO] Tasks used: %u bytes\r\n\r\n",
             (unsigned)(xInitialFree - xAfterTasks));
    UART_SendString(msg);
    
    UART_SendString("────────────────────────────────────────────────────────────\r\n");
    
    vTaskStartScheduler();
    
    while (1);
}

/* ============================================================================
 * PRINT HEAP STATISTICS
 * ============================================================================ */
static void PrintHeapStats(const char *label)
{
    char msg[150];
    
    size_t xFree = xPortGetFreeHeapSize();
    size_t xMinEver = xPortGetMinimumEverFreeHeapSize();
    
    /* Calculate percentages */
    uint32_t ulFreePercent = (xFree * 100) / configTOTAL_HEAP_SIZE;
    uint32_t ulMinPercent = (xMinEver * 100) / configTOTAL_HEAP_SIZE;
    
    UART_SendString("\r\n┌─── HEAP STATISTICS ─────────────────────────────────────┐\r\n");
    
    snprintf(msg, sizeof(msg), "│  %s                                           │\r\n", label);
    UART_SendString(msg);
    
    UART_SendString("│                                                          │\r\n");
    
    snprintf(msg, sizeof(msg), "│  Total heap:      %5u bytes                           │\r\n",
             (unsigned)configTOTAL_HEAP_SIZE);
    UART_SendString(msg);
    
    snprintf(msg, sizeof(msg), "│  Current free:    %5u bytes (%lu%%)                     │\r\n",
             (unsigned)xFree, ulFreePercent);
    UART_SendString(msg);
    
    snprintf(msg, sizeof(msg), "│  Current used:    %5u bytes (%lu%%)                     │\r\n",
             (unsigned)(configTOTAL_HEAP_SIZE - xFree), 100 - ulFreePercent);
    UART_SendString(msg);
    
    snprintf(msg, sizeof(msg), "│  Min ever free:   %5u bytes (%lu%%)                     │\r\n",
             (unsigned)xMinEver, ulMinPercent);
    UART_SendString(msg);
    
    /* Visual bar */
    UART_SendString("│                                                          │\r\n");
    UART_SendString("│  ");
    
    uint32_t usedBlocks = (configTOTAL_HEAP_SIZE - xFree) * 40 / configTOTAL_HEAP_SIZE;
    for (uint32_t i = 0; i < 40; i++)
    {
        if (i < usedBlocks)
            UART_SendString("▓");
        else
            UART_SendString("░");
    }
    UART_SendString("  │\r\n");
    
    UART_SendString("│  └─── Used ───┴─── Free ─────────────────────────────┘  │\r\n");
    
    UART_SendString("└──────────────────────────────────────────────────────────┘\r\n\r\n");
}

/* ============================================================================
 * ALLOCATOR TASK
 * Simulates dynamic memory usage
 * ============================================================================ */
static void prvAllocatorTask(void *pvParameters)
{
    (void)pvParameters;
    
    char msg[120];
    static uint32_t ulCycle = 0;
    
    /* Allocation sizes to test */
    const size_t xSizes[] = {64, 128, 256, 100, 50, 200};
    const uint32_t ulNumSizes = sizeof(xSizes) / sizeof(xSizes[0]);
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        ulCycle++;
        
        /* Alternate between allocate and free phases */
        if ((ulCycle % 2) == 1)
        {
            /* ALLOCATION PHASE */
            UART_SendString("\r\n[Allocator] ═══ ALLOCATION PHASE ═══\r\n");
            
            uint32_t ulNumToAlloc = 2 + (ulCycle % 3);  /* Allocate 2-4 blocks */
            
            for (uint32_t i = 0; i < ulNumToAlloc && ulAllocCount < MAX_ALLOCATIONS; i++)
            {
                size_t xSize = xSizes[(ulCycle + i) % ulNumSizes];
                
                /*
                 * pvPortMalloc():
                 *   - Allocate from FreeRTOS heap
                 *   - Returns NULL if failed
                 *   - Thread-safe (no need for mutex)
                 */
                void *pvMem = pvPortMalloc(xSize);
                
                if (pvMem != NULL)
                {
                    /* Record allocation */
                    xAllocations[ulAllocCount].pvPtr = pvMem;
                    xAllocations[ulAllocCount].xSize = xSize;
                    xAllocations[ulAllocCount].ulAllocTime = xTaskGetTickCount();
                    ulAllocCount++;
                    
                    snprintf(msg, sizeof(msg), 
                             "            Allocated %3u bytes at %p (total: %lu blocks)\r\n",
                             (unsigned)xSize, pvMem, ulAllocCount);
                    UART_SendString(msg);
                    
                    /* Write pattern to memory (helps detect corruption) */
                    memset(pvMem, 0xAA, xSize);
                }
                else
                {
                    UART_SendString("            [WARN] Allocation failed!\r\n");
                }
            }
        }
        else
        {
            /* FREE PHASE */
            UART_SendString("\r\n[Allocator] ═══ FREE PHASE ═══\r\n");
            
            /* Free half of allocations (random-ish pattern) */
            uint32_t ulNumToFree = ulAllocCount / 2;
            if (ulNumToFree < 1 && ulAllocCount > 0) ulNumToFree = 1;
            
            for (uint32_t i = 0; i < ulNumToFree && ulAllocCount > 0; i++)
            {
                /* Free from end (LIFO) or middle based on cycle */
                uint32_t idx = (ulCycle % 2 == 0) ? ulAllocCount - 1 : i;
                if (idx >= ulAllocCount) idx = ulAllocCount - 1;
                
                if (xAllocations[idx].pvPtr != NULL)
                {
                    snprintf(msg, sizeof(msg), 
                             "            Freeing %3u bytes at %p\r\n",
                             (unsigned)xAllocations[idx].xSize, 
                             xAllocations[idx].pvPtr);
                    UART_SendString(msg);
                    
                    /*
                     * vPortFree():
                     *   - Return memory to heap
                     *   - heap_4 will merge adjacent free blocks
                     */
                    vPortFree(xAllocations[idx].pvPtr);
                    
                    /* Shift remaining allocations */
                    for (uint32_t j = idx; j < ulAllocCount - 1; j++)
                    {
                        xAllocations[j] = xAllocations[j + 1];
                    }
                    ulAllocCount--;
                    xAllocations[ulAllocCount].pvPtr = NULL;
                }
            }
        }
        
        /* Toggle LED */
        HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(3000));
    }
}

/* ============================================================================
 * MONITOR TASK
 * Periodically reports heap statistics
 * ============================================================================ */
static void prvMonitorTask(void *pvParameters)
{
    (void)pvParameters;
    
    char msg[100];
    const TickType_t xPeriod = pdMS_TO_TICKS(5000);
    static uint32_t ulReportNum = 0;
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
        
        ulReportNum++;
        snprintf(msg, sizeof(msg), "Report #%lu", ulReportNum);
        PrintHeapStats(msg);
        
        /* Active allocations */
        snprintf(msg, sizeof(msg), "[Monitor] Active allocations: %lu blocks\r\n", ulAllocCount);
        UART_SendString(msg);
        
        size_t xTotalAllocated = 0;
        for (uint32_t i = 0; i < ulAllocCount; i++)
        {
            xTotalAllocated += xAllocations[i].xSize;
        }
        snprintf(msg, sizeof(msg), "[Monitor] Total tracked: %u bytes\r\n", 
                 (unsigned)xTotalAllocated);
        UART_SendString(msg);
        
        /* Margin check */
        size_t xMinEver = xPortGetMinimumEverFreeHeapSize();
        if (xMinEver < 1024)
        {
            UART_SendString("\r\n[WARNING] Heap margin < 1KB! Consider increasing heap size.\r\n");
        }
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
    UART_SendString("\r\n[FATAL] ═══ MALLOC FAILED! ═══\r\n");
    UART_SendString("[FATAL] Heap exhausted. Consider:\r\n");
    UART_SendString("        - Increase configTOTAL_HEAP_SIZE\r\n");
    UART_SendString("        - Reduce allocation sizes\r\n");
    UART_SendString("        - Free unused memory earlier\r\n");
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
