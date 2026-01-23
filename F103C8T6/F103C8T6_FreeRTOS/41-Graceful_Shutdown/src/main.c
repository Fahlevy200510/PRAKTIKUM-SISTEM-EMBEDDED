/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 41-Graceful_Shutdown
 * 
 * DESKRIPSI:
 * Demonstrasi graceful shutdown pattern. Semua task diberi kesempatan untuk
 * cleanup sebelum sistem dimatikan dengan aman.
 * 
 * ============================================================================
 * ARSITEKTUR SISTEM
 * ============================================================================
 * 
 *    ┌──────────────────────────────────────────────────────────────────────┐
 *    │                   GRACEFUL SHUTDOWN DEMO                             │
 *    │                                                                      │
 *    │   ┌──────────────────────────────────────────────────────────────┐   │
 *    │   │                   SHUTDOWN MANAGER                           │   │
 *    │   │                                                              │   │
 *    │   │  Timeout 15s → Request Shutdown                              │   │
 *    │   │               ↓                                              │   │
 *    │   │  xTaskNotify(Task, SHUTDOWN) ke semua task                   │   │
 *    │   │               ↓                                              │   │
 *    │   │  Wait for ACK from semua task                                │   │
 *    │   │               ↓                                              │   │
 *    │   │  Disable peripherals → HALT                                  │   │
 *    │   └──────────────────────────────────────────────────────────────┘   │
 *    │                                                                      │
 *    │   ┌────────────┐    ┌────────────┐    ┌────────────┐                │
 *    │   │  Worker A  │    │  Worker B  │    │  Worker C  │                │
 *    │   │            │    │            │    │            │                │
 *    │   │ On notify: │    │ On notify: │    │ On notify: │                │
 *    │   │ • Stop work│    │ • Save data│    │ • Safe pos │                │
 *    │   │ • Send ACK │    │ • Send ACK │    │ • Send ACK │                │
 *    │   └────────────┘    └────────────┘    └────────────┘                │
 *    │                                                                      │
 *    └──────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * CONTOH OUTPUT SERIAL
 * ============================================================================
 * 
 *    === GRACEFUL SHUTDOWN DEMO ===
 *    
 *    [Worker A] Started - simulating EEPROM operations
 *    [Worker B] Started - simulating data transmission
 *    [Worker C] Started - simulating motor control
 *    
 *    [Worker A] Working cycle 1...
 *    [Worker B] Working cycle 1...
 *    [Worker C] Working cycle 1...
 *    ...
 *    
 *    [Shutdown] Initiating graceful shutdown...
 *    [Shutdown] Notifying Worker A...
 *    [Shutdown] Notifying Worker B...
 *    [Shutdown] Notifying Worker C...
 *    
 *    [Worker A] Shutdown received! Completing EEPROM write...
 *    [Worker A] Cleanup complete, ACK sent
 *    [Worker B] Shutdown received! Finishing transmission...
 *    [Worker B] Cleanup complete, ACK sent
 *    [Worker C] Shutdown received! Motor to safe position...
 *    [Worker C] Cleanup complete, ACK sent
 *    
 *    [Shutdown] All 3 workers acknowledged
 *    [Shutdown] Disabling UART...
 *    [Shutdown] LED off...
 *    [Shutdown] System halted safely ✓
 * 
 * ============================================================================
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <string.h>
#include <stdio.h>
#include <limits.h>

/* ============================================================================
 * WORKER TASK CONTEXT
 * ============================================================================ */
typedef struct {
    const char *pcName;
    uint32_t ulWorkCycles;
    TickType_t xCleanupTime;  /* Simulated cleanup time */
} WorkerContext_t;

/* ============================================================================
 * GLOBAL VARIABLES
 * ============================================================================ */
UART_HandleTypeDef huart1;

/* Worker task handles */
TaskHandle_t xWorkerTasks[WORKER_TASK_COUNT] = {NULL};
WorkerContext_t xWorkerContexts[WORKER_TASK_COUNT];

/* Shutdown coordination */
SemaphoreHandle_t xAckSemaphore = NULL;
volatile BaseType_t xShutdownRequested = pdFALSE;
volatile uint32_t ulAckCount = 0;

/* ============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================ */
static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART1_Init(void);
static void prvWorkerTask(void *pvParameters);
static void prvShutdownManagerTask(void *pvParameters);
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
    
    UART_SendString("\r\n\r\n");
    UART_SendString("╔══════════════════════════════════════════════════════════╗\r\n");
    UART_SendString("║           41. GRACEFUL SHUTDOWN DEMO                     ║\r\n");
    UART_SendString("╚══════════════════════════════════════════════════════════╝\r\n\r\n");
    
    UART_SendString("[Info] System will run for 15 seconds then shutdown\r\n");
    UART_SendString("[Info] All workers will cleanup before halt\r\n\r\n");
    
    /* Create ACK semaphore */
    xAckSemaphore = xSemaphoreCreateCounting(WORKER_TASK_COUNT, 0);
    if (xAckSemaphore == NULL)
    {
        UART_SendString("[ERROR] Semaphore creation failed!\r\n");
        while (1);
    }
    
    /* Initialize worker contexts */
    xWorkerContexts[0].pcName = "EEPROM_Writer";
    xWorkerContexts[0].ulWorkCycles = 0;
    xWorkerContexts[0].xCleanupTime = pdMS_TO_TICKS(500);
    
    xWorkerContexts[1].pcName = "Data_Sender";
    xWorkerContexts[1].ulWorkCycles = 0;
    xWorkerContexts[1].xCleanupTime = pdMS_TO_TICKS(300);
    
    xWorkerContexts[2].pcName = "Motor_Ctrl";
    xWorkerContexts[2].ulWorkCycles = 0;
    xWorkerContexts[2].xCleanupTime = pdMS_TO_TICKS(700);
    
    /* Create worker tasks */
    for (int i = 0; i < WORKER_TASK_COUNT; i++)
    {
        char taskName[16];
        snprintf(taskName, sizeof(taskName), "Worker%d", i);
        xTaskCreate(prvWorkerTask, taskName, TASK_STACK, &xWorkerContexts[i], 1, &xWorkerTasks[i]);
    }
    
    /* Create shutdown manager task */
    xTaskCreate(prvShutdownManagerTask, "Shutdown", TASK_STACK, NULL, 3, NULL);
    
    UART_SendString("────────────────────────────────────────────────────────────\r\n\r\n");
    
    vTaskStartScheduler();
    
    while (1);
}

/* ============================================================================
 * WORKER TASK
 * Simulates work and handles graceful shutdown
 * ============================================================================ */
static void prvWorkerTask(void *pvParameters)
{
    WorkerContext_t *pxContext = (WorkerContext_t *)pvParameters;
    char msg[80];
    uint32_t ulNotifyValue;
    
    snprintf(msg, sizeof(msg), "[%s] Started\r\n", pxContext->pcName);
    UART_SendString(msg);
    
    for (;;)
    {
        /* Check for shutdown notification with timeout */
        if (xTaskNotifyWait(0, ULONG_MAX, &ulNotifyValue, pdMS_TO_TICKS(1000)) == pdTRUE)
        {
            if (ulNotifyValue & NOTIFY_SHUTDOWN)
            {
                /* Shutdown requested! */
                snprintf(msg, sizeof(msg), "\r\n[%s] ⚠️ SHUTDOWN RECEIVED!\r\n", 
                         pxContext->pcName);
                UART_SendString(msg);
                
                /* Simulate cleanup work */
                snprintf(msg, sizeof(msg), "[%s] Performing cleanup (%lu ms)...\r\n",
                         pxContext->pcName, (unsigned long)pxContext->xCleanupTime);
                UART_SendString(msg);
                
                vTaskDelay(pxContext->xCleanupTime);
                
                snprintf(msg, sizeof(msg), "[%s] ✓ Cleanup complete (cycles: %lu)\r\n",
                         pxContext->pcName, pxContext->ulWorkCycles);
                UART_SendString(msg);
                
                /* Send ACK */
                xSemaphoreGive(xAckSemaphore);
                
                snprintf(msg, sizeof(msg), "[%s] ACK sent, suspending\r\n\r\n",
                         pxContext->pcName);
                UART_SendString(msg);
                
                /* Suspend self */
                vTaskSuspend(NULL);
            }
        }
        
        /* Normal work */
        if (!xShutdownRequested)
        {
            pxContext->ulWorkCycles++;
            
            if (pxContext->ulWorkCycles % 3 == 0)
            {
                snprintf(msg, sizeof(msg), "[%s] Work cycle %lu\r\n",
                         pxContext->pcName, pxContext->ulWorkCycles);
                UART_SendString(msg);
            }
            
            HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
        }
    }
}

/* ============================================================================
 * SHUTDOWN MANAGER TASK
 * Coordinates graceful shutdown
 * ============================================================================ */
static void prvShutdownManagerTask(void *pvParameters)
{
    (void)pvParameters;
    
    char msg[80];
    const TickType_t xShutdownDelay = pdMS_TO_TICKS(15000);  /* 15 seconds */
    const TickType_t xAckTimeout = pdMS_TO_TICKS(5000);      /* 5 second timeout */
    
    UART_SendString("[Shutdown] Manager started\r\n");
    UART_SendString("[Shutdown] System will shutdown in 15 seconds...\r\n\r\n");
    
    /* Wait before initiating shutdown */
    vTaskDelay(xShutdownDelay);
    
    /* ═══════════════════════════════════════════════════════════════════ */
    /* STEP 1: Request Shutdown */
    /* ═══════════════════════════════════════════════════════════════════ */
    UART_SendString("\r\n");
    UART_SendString("╔════════════════════════════════════════════════════════╗\r\n");
    UART_SendString("║           INITIATING GRACEFUL SHUTDOWN                 ║\r\n");
    UART_SendString("╚════════════════════════════════════════════════════════╝\r\n\r\n");
    
    xShutdownRequested = pdTRUE;
    
    /* ═══════════════════════════════════════════════════════════════════ */
    /* STEP 2: Notify All Workers */
    /* ═══════════════════════════════════════════════════════════════════ */
    UART_SendString("[Shutdown] Notifying all workers...\r\n");
    
    for (int i = 0; i < WORKER_TASK_COUNT; i++)
    {
        if (xWorkerTasks[i] != NULL)
        {
            snprintf(msg, sizeof(msg), "[Shutdown] Sending to worker %d (%s)\r\n",
                     i, xWorkerContexts[i].pcName);
            UART_SendString(msg);
            
            xTaskNotify(xWorkerTasks[i], NOTIFY_SHUTDOWN, eSetBits);
        }
    }
    
    /* ═══════════════════════════════════════════════════════════════════ */
    /* STEP 3: Wait for All ACKs */
    /* ═══════════════════════════════════════════════════════════════════ */
    UART_SendString("\r\n[Shutdown] Waiting for ACKs...\r\n");
    
    ulAckCount = 0;
    BaseType_t xAllAcked = pdTRUE;
    
    for (int i = 0; i < WORKER_TASK_COUNT; i++)
    {
        if (xSemaphoreTake(xAckSemaphore, xAckTimeout) == pdTRUE)
        {
            ulAckCount++;
            snprintf(msg, sizeof(msg), "[Shutdown] ACK %lu/%d received\r\n",
                     ulAckCount, WORKER_TASK_COUNT);
            UART_SendString(msg);
        }
        else
        {
            xAllAcked = pdFALSE;
            UART_SendString("[Shutdown] ⚠️ Timeout waiting for ACK!\r\n");
        }
    }
    
    /* ═══════════════════════════════════════════════════════════════════ */
    /* STEP 4: Final Status */
    /* ═══════════════════════════════════════════════════════════════════ */
    UART_SendString("\r\n");
    
    if (xAllAcked)
    {
        UART_SendString("┌────────────────────────────────────────────────────────┐\r\n");
        snprintf(msg, sizeof(msg), "│  ✓ All %d workers acknowledged shutdown               │\r\n",
                 WORKER_TASK_COUNT);
        UART_SendString(msg);
        UART_SendString("└────────────────────────────────────────────────────────┘\r\n\r\n");
    }
    else
    {
        UART_SendString("┌────────────────────────────────────────────────────────┐\r\n");
        UART_SendString("│  ⚠️ WARNING: Some workers did not acknowledge         │\r\n");
        UART_SendString("│  Force shutdown proceeding...                         │\r\n");
        UART_SendString("└────────────────────────────────────────────────────────┘\r\n\r\n");
    }
    
    /* ═══════════════════════════════════════════════════════════════════ */
    /* STEP 5: Peripheral Cleanup */
    /* ═══════════════════════════════════════════════════════════════════ */
    UART_SendString("[Shutdown] Disabling peripherals...\r\n");
    
    /* LED OFF */
    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_SET);
    UART_SendString("[Shutdown] LED: OFF\r\n");
    
    /* Final message */
    UART_SendString("\r\n");
    UART_SendString("╔════════════════════════════════════════════════════════╗\r\n");
    UART_SendString("║     ✓ SYSTEM HALTED SAFELY                             ║\r\n");
    UART_SendString("║                                                        ║\r\n");
    UART_SendString("║     All data saved, all operations complete            ║\r\n");
    UART_SendString("║     Safe to power off                                  ║\r\n");
    UART_SendString("╚════════════════════════════════════════════════════════╝\r\n");
    
    /* ═══════════════════════════════════════════════════════════════════ */
    /* STEP 6: Halt */
    /* ═══════════════════════════════════════════════════════════════════ */
    taskDISABLE_INTERRUPTS();
    
    /* LED slow blink to indicate halted state */
    for (;;)
    {
        HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
        for (volatile uint32_t i = 0; i < 2000000; i++);
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
