/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 38-Malloc_Failed_Hook
 * 
 * DESKRIPSI:
 * Demonstrasi penggunaan vApplicationMallocFailedHook() untuk menangani
 * kondisi out-of-memory dengan graceful. Termasuk stress test alokasi.
 * 
 * ============================================================================
 * ARSITEKTUR SISTEM
 * ============================================================================
 * 
 *    ┌──────────────────────────────────────────────────────────────────────┐
 *    │                    MALLOC FAILED HOOK DEMO                           │
 *    │                                                                      │
 *    │    ┌─────────────────┐                                               │
 *    │    │  Allocator Task │                                               │
 *    │    │                 │                                               │
 *    │    │  1. Alloc 1KB  ───▶ pvPortMalloc(1024) ─▶ OK                   │
 *    │    │  2. Alloc 2KB  ───▶ pvPortMalloc(2048) ─▶ OK                   │
 *    │    │  3. Alloc 4KB  ───▶ pvPortMalloc(4096) ─▶ OK                   │
 *    │    │  4. Alloc 8KB  ───▶ pvPortMalloc(8192) ─▶ FAIL!               │
 *    │    │                 │                           │                   │
 *    │    └─────────────────┘                           ▼                   │
 *    │                                    ┌─────────────────────────────┐   │
 *    │                                    │ vApplicationMallocFailed()  │   │
 *    │                                    │                             │   │
 *    │                                    │  • Log to UART              │   │
 *    │                                    │  • LED rapid blink          │   │
 *    │                                    │  • System HALT              │   │
 *    │                                    └─────────────────────────────┘   │
 *    │                                                                      │
 *    └──────────────────────────────────────────────────────────────────────┘
 * 
 *    HEAP MEMORY VISUALIZATION:
 * 
 *    [0KB]  [1KB]  [2KB]  [3KB]  [4KB]  [5KB]  [6KB]  [7KB]  [8KB]  [9KB]  [10KB]
 *     |      |      |      |      |      |      |      |      |      |      |
 *     ├──────┴──────┼──────┴──────┴──────┼──────┴──────┴──────┴──────┤      │
 *     │   1KB Alloc │    2KB Alloc       │        4KB Alloc         │ FREE │
 *     └─────────────┴───────────────────┴────────────────────────────┴──────┘
 *                                                                      ↑
 *                                                        Request 8KB ──┘ FAIL!
 *                                                        (hanya 3KB tersisa)
 * 
 * ============================================================================
 * CONTOH OUTPUT SERIAL
 * ============================================================================
 * 
 *    === MALLOC FAILED HOOK DEMO ===
 *    [Heap] Total: 10240 bytes
 *    [Heap] Initial free: 9968 bytes
 *    
 *    [Step 1] Allocating 1024 bytes... OK
 *             Free: 8936 bytes, Min: 8936 bytes
 *    
 *    [Step 2] Allocating 2048 bytes... OK
 *             Free: 6880 bytes, Min: 6880 bytes
 *    
 *    [Step 3] Allocating 4096 bytes... OK
 *             Free: 2776 bytes, Min: 2776 bytes
 *    
 *    [Step 4] Allocating 8192 bytes...
 *    
 *    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *    [MALLOC FAILED HOOK TRIGGERED]
 *    Heap free: 2776 bytes
 *    Min ever : 2776 bytes
 *    Insufficient memory for 8192 byte allocation
 *    System halted for debugging
 *    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * 
 * ============================================================================
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * GLOBAL VARIABLES
 * ============================================================================ */
UART_HandleTypeDef huart1;

/* Track allocations for debugging */
static void *pvAllocations[10] = {NULL};
static uint8_t ucAllocIndex = 0;
static volatile uint32_t ulMallocFailCount = 0;

/* ============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================ */
static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART1_Init(void);
static void prvAllocatorTask(void *pvParameters);
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
    
    char msg[100];
    
    UART_SendString("\r\n\r\n");
    UART_SendString("╔══════════════════════════════════════════════════════════╗\r\n");
    UART_SendString("║            38. MALLOC FAILED HOOK DEMO                   ║\r\n");
    UART_SendString("╚══════════════════════════════════════════════════════════╝\r\n\r\n");
    
    snprintf(msg, sizeof(msg), "[Heap] Total configured: %u bytes\r\n",
             (unsigned)configTOTAL_HEAP_SIZE);
    UART_SendString(msg);
    
    snprintf(msg, sizeof(msg), "[Heap] Initial free: %u bytes\r\n\r\n",
             (unsigned)xPortGetFreeHeapSize());
    UART_SendString(msg);
    
    /* Create tasks - ini sendiri menggunakan heap! */
    xTaskCreate(prvAllocatorTask, "Allocator", TASK_STACK, NULL, 2, NULL);
    xTaskCreate(prvMonitorTask, "Monitor", TASK_STACK, NULL, 1, NULL);
    
    snprintf(msg, sizeof(msg), "[Heap] After task creation: %u bytes\r\n\r\n",
             (unsigned)xPortGetFreeHeapSize());
    UART_SendString(msg);
    
    UART_SendString("────────────────────────────────────────────────────────────\r\n\r\n");
    
    vTaskStartScheduler();
    
    while (1);
}

/* ============================================================================
 * ALLOCATOR TASK
 * Progressively allocates memory until malloc fails
 * ============================================================================ */
static void prvAllocatorTask(void *pvParameters)
{
    (void)pvParameters;
    
    char msg[120];
    
    /* Test allocation sizes */
    const size_t xTestSizes[] = {1024, 2048, 4096, 8192};
    const size_t xNumTests = sizeof(xTestSizes) / sizeof(xTestSizes[0]);
    
    /* Wait for monitor to start */
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    UART_SendString("[ALLOCATOR] Starting memory allocation stress test...\r\n\r\n");
    
    for (size_t i = 0; i < xNumTests; i++)
    {
        size_t xSize = xTestSizes[i];
        
        snprintf(msg, sizeof(msg), "[Step %u] Requesting %u bytes... ", 
                 (unsigned)(i + 1), (unsigned)xSize);
        UART_SendString(msg);
        
        /* Attempt allocation */
        void *pvMem = pvPortMalloc(xSize);
        
        if (pvMem != NULL)
        {
            /* Store pointer for cleanup */
            if (ucAllocIndex < 10)
            {
                pvAllocations[ucAllocIndex++] = pvMem;
            }
            
            /* Fill memory to ensure it's usable */
            memset(pvMem, 0xAA, xSize);
            
            UART_SendString("OK\r\n");
            
            snprintf(msg, sizeof(msg), 
                     "         Address: 0x%08lX\r\n"
                     "         Free: %u bytes, Min ever: %u bytes\r\n\r\n",
                     (unsigned long)pvMem,
                     (unsigned)xPortGetFreeHeapSize(),
                     (unsigned)xPortGetMinimumEverFreeHeapSize());
            UART_SendString(msg);
        }
        else
        {
            UART_SendString("FAILED!\r\n");
            /* Hook will be called by pvPortMalloc() */
        }
        
        /* LED blink for each allocation */
        HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
        
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    
    /* Cleanup - free all allocations */
    UART_SendString("[ALLOCATOR] Cleaning up allocations...\r\n");
    for (uint8_t i = 0; i < ucAllocIndex; i++)
    {
        if (pvAllocations[i] != NULL)
        {
            vPortFree(pvAllocations[i]);
            pvAllocations[i] = NULL;
        }
    }
    
    snprintf(msg, sizeof(msg), "[ALLOCATOR] After cleanup: %u bytes free\r\n\r\n",
             (unsigned)xPortGetFreeHeapSize());
    UART_SendString(msg);
    
    /* Test successful recovery */
    UART_SendString("[ALLOCATOR] Test complete. Repeating...\r\n\r\n");
    ucAllocIndex = 0;
    
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    /* Restart test */
    for (;;)
    {
        /* Now try allocation that will definitely fail */
        UART_SendString("[ALLOCATOR] Attempting 15KB allocation (will fail)...\r\n");
        
        void *pvBig = pvPortMalloc(15 * 1024);
        
        if (pvBig == NULL)
        {
            /* This won't print because hook will halt */
            UART_SendString("[ALLOCATOR] Returned NULL as expected\r\n");
        }
        
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

/* ============================================================================
 * MONITOR TASK
 * Reports heap status periodically
 * ============================================================================ */
static void prvMonitorTask(void *pvParameters)
{
    (void)pvParameters;
    
    char msg[100];
    const TickType_t xPeriod = pdMS_TO_TICKS(3000);
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
        
        size_t xFree = xPortGetFreeHeapSize();
        size_t xMinFree = xPortGetMinimumEverFreeHeapSize();
        size_t xUsed = configTOTAL_HEAP_SIZE - xFree;
        
        UART_SendString("┌─── HEAP MONITOR ─────────────────────────────────────────┐\r\n");
        
        snprintf(msg, sizeof(msg), "│  Used: %5u bytes (%2u%%)                                │\r\n",
                 (unsigned)xUsed, (unsigned)(xUsed * 100 / configTOTAL_HEAP_SIZE));
        UART_SendString(msg);
        
        snprintf(msg, sizeof(msg), "│  Free: %5u bytes (%2u%%)                                │\r\n",
                 (unsigned)xFree, (unsigned)(xFree * 100 / configTOTAL_HEAP_SIZE));
        UART_SendString(msg);
        
        snprintf(msg, sizeof(msg), "│  Min Free Ever: %5u bytes                              │\r\n",
                 (unsigned)xMinFree);
        UART_SendString(msg);
        
        snprintf(msg, sizeof(msg), "│  Malloc Failures: %lu                                     │\r\n",
                 ulMallocFailCount);
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
 * FreeRTOS HOOKS - THE STAR OF THIS DEMO
 * ============================================================================ */

/**
 * vApplicationMallocFailedHook
 * 
 * DIPANGGIL OLEH pvPortMalloc() SAAT ALOKASI GAGAL
 * 
 * Strategi di sini: HALT dengan diagnostic lengkap
 * - Log semua informasi heap
 * - LED rapid blink untuk visual indication
 * - Infinite loop untuk debugging
 * 
 * Alternatif strategi:
 * 1. Reset sistem via NVIC_SystemReset()
 * 2. Set flag untuk recovery task
 * 3. Trigger watchdog untuk clean reset
 */
void vApplicationMallocFailedHook(void)
{
    char msg[100];
    
    /* Increment failure counter */
    ulMallocFailCount++;
    
    UART_SendString("\r\n");
    UART_SendString("╔══════════════════════════════════════════════════════════╗\r\n");
    UART_SendString("║        ⚠️  MALLOC FAILED HOOK TRIGGERED! ⚠️               ║\r\n");
    UART_SendString("╚══════════════════════════════════════════════════════════╝\r\n");
    UART_SendString("\r\n");
    
    /* Detailed diagnostics */
    snprintf(msg, sizeof(msg), "[HOOK] Free heap now: %u bytes\r\n",
             (unsigned)xPortGetFreeHeapSize());
    UART_SendString(msg);
    
    snprintf(msg, sizeof(msg), "[HOOK] Min free ever: %u bytes\r\n",
             (unsigned)xPortGetMinimumEverFreeHeapSize());
    UART_SendString(msg);
    
    snprintf(msg, sizeof(msg), "[HOOK] Total failures: %lu\r\n",
             ulMallocFailCount);
    UART_SendString(msg);
    
    snprintf(msg, sizeof(msg), "[HOOK] Active allocations: %u\r\n",
             ucAllocIndex);
    UART_SendString(msg);
    
    UART_SendString("\r\n");
    UART_SendString("[HOOK] POSSIBLE CAUSES:\r\n");
    UART_SendString("       - Requested size > available heap\r\n");
    UART_SendString("       - Heap fragmentation\r\n");
    UART_SendString("       - Memory leak in application\r\n");
    UART_SendString("\r\n");
    UART_SendString("[HOOK] ACTION: System HALTED for debugging\r\n");
    UART_SendString("       Connect debugger to analyze heap state\r\n");
    UART_SendString("\r\n");
    
    /* Disable interrupts to prevent further issues */
    taskDISABLE_INTERRUPTS();
    
    /* Rapid LED blink to indicate failure */
    for (;;)
    {
        HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
        
        /* Busy wait since scheduler is stopped */
        for (volatile uint32_t i = 0; i < 500000; i++);
    }
}

/**
 * vApplicationStackOverflowHook
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    char msg[60];
    snprintf(msg, sizeof(msg), "[FATAL] Stack overflow in: %s\r\n", pcTaskName);
    UART_SendString(msg);
    taskDISABLE_INTERRUPTS();
    for (;;);
}
