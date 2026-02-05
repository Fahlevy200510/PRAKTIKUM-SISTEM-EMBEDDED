/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 36-Static_Memory_Allocation
 * 
 * DESKRIPSI:
 * Demo static memory allocation di FreeRTOS. Semua task, queue, dan semaphore
 * dialokasikan secara static (compile-time), tidak menggunakan heap.
 * 
 * ============================================================================
 * ARSITEKTUR SISTEM
 * ============================================================================
 * 
 *    ┌──────────────────────────────────────────────────────────────────────┐
 *    │                  STATIC MEMORY ALLOCATION                            │
 *    │                                                                      │
 *    │   COMPILE-TIME ALLOCATION (BSS/DATA section):                        │
 *    │   ┌────────────────────────────────────────────────────────────────┐ │
 *    │   │                                                                │ │
 *    │   │  Static Task 1:        Static Task 2:        Static Queue:    │ │
 *    │   │  ┌────────────────┐    ┌────────────────┐    ┌──────────────┐ │ │
 *    │   │  │ Stack[256]     │    │ Stack[256]     │    │ Storage[40]  │ │ │
 *    │   │  │ TCB (StaticTask)│    │ TCB            │    │ QueueStruct  │ │ │
 *    │   │  └────────────────┘    └────────────────┘    └──────────────┘ │ │
 *    │   │                                                                │ │
 *    │   │  Idle Task:            Timer Task:          Static Semaphore: │ │
 *    │   │  ┌────────────────┐    ┌────────────────┐    ┌──────────────┐ │ │
 *    │   │  │ Stack[128]     │    │ Stack[256]     │    │ SemStruct    │ │ │
 *    │   │  │ TCB            │    │ TCB            │    └──────────────┘ │ │
 *    │   │  └────────────────┘    └────────────────┘                      │ │
 *    │   │                                                                │ │
 *    │   └────────────────────────────────────────────────────────────────┘ │
 *    │                                                                      │
 *    │   NO HEAP FRAGMENTATION!  NO MALLOC FAILURES!                        │
 *    │                                                                      │
 *    └──────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * CONTOH OUTPUT SERIAL
 * ============================================================================
 * 
 *    === STATIC MEMORY ALLOCATION ===
 *    
 *    [Init] Creating static objects...
 *    [Init] Static Task 1: Stack at 0x20001000, TCB at 0x20001400
 *    [Init] Static Task 2: Stack at 0x20001500, TCB at 0x20001900
 *    [Init] Static Queue: Storage at 0x20001A00
 *    [Init] Static Semaphore created
 *    
 *    [Task1] Running (static allocation)
 *    [Task2] Running (static allocation)
 *    
 *    [Monitor] Memory comparison:
 *              Static objects: 2KB (fixed in BSS)
 *              Heap remaining: 8KB (untouched for static objects)
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
 * STATIC MEMORY BUFFERS
 * These are allocated in BSS section at compile time
 * ============================================================================ */

/* Static Task 1 */
static StackType_t xTask1Stack[STATIC_TASK_STACK];
static StaticTask_t xTask1TCB;

/* Static Task 2 */
static StackType_t xTask2Stack[STATIC_TASK_STACK];
static StaticTask_t xTask2TCB;

/* Static Queue (10 items of uint32_t) */
#define QUEUE_LENGTH    10
#define QUEUE_ITEM_SIZE sizeof(uint32_t)
static uint8_t ucQueueStorage[QUEUE_LENGTH * QUEUE_ITEM_SIZE];
static StaticQueue_t xQueueBuffer;

/* Static Binary Semaphore */
static StaticSemaphore_t xSemaphoreBuffer;

/* Static Mutex */
static StaticSemaphore_t xMutexBuffer;

/* Idle and Timer task memory (REQUIRED for static allocation) */
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];
static StaticTask_t xIdleTCB;
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];
static StaticTask_t xTimerTCB;

/* ============================================================================
 * GLOBAL VARIABLES
 * ============================================================================ */
UART_HandleTypeDef huart1;
QueueHandle_t xStaticQueue = NULL;
SemaphoreHandle_t xStaticSemaphore = NULL;
SemaphoreHandle_t xStaticMutex = NULL;
TaskHandle_t xTask1Handle = NULL;
TaskHandle_t xTask2Handle = NULL;

/* ============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================ */
static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART1_Init(void);
static void prvStaticTask1(void *pvParameters);
static void prvStaticTask2(void *pvParameters);
static void UART_SendString(const char *str);

/* ============================================================================
 * UART HELPER
 * ============================================================================ */
static void UART_SendString(const char *str)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}

/* ============================================================================
 * REQUIRED CALLBACKS FOR STATIC ALLOCATION
 * ============================================================================ */

/*
 * Provide memory for Idle task
 * Called by FreeRTOS kernel when configSUPPORT_STATIC_ALLOCATION=1
 */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
{
    *ppxIdleTaskTCBBuffer = &xIdleTCB;
    *ppxIdleTaskStackBuffer = xIdleStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/*
 * Provide memory for Timer task
 * Called when configUSE_TIMERS=1 and configSUPPORT_STATIC_ALLOCATION=1
 */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize)
{
    *ppxTimerTaskTCBBuffer = &xTimerTCB;
    *ppxTimerTaskStackBuffer = xTimerStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
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
    UART_SendString("║        36. STATIC MEMORY ALLOCATION DEMO                 ║\r\n");
    UART_SendString("╚══════════════════════════════════════════════════════════╝\r\n\r\n");
    
    UART_SendString("[Init] Creating STATIC FreeRTOS objects...\r\n\r\n");
    
    /* Show addresses of static buffers */
    snprintf(msg, sizeof(msg), "[Init] Task1 Stack: %p, TCB: %p\r\n",
             (void*)xTask1Stack, (void*)&xTask1TCB);
    UART_SendString(msg);
    
    snprintf(msg, sizeof(msg), "[Init] Task2 Stack: %p, TCB: %p\r\n",
             (void*)xTask2Stack, (void*)&xTask2TCB);
    UART_SendString(msg);
    
    snprintf(msg, sizeof(msg), "[Init] Queue Storage: %p, Struct: %p\r\n",
             (void*)ucQueueStorage, (void*)&xQueueBuffer);
    UART_SendString(msg);
    
    /*
     * CREATE STATIC QUEUE
     */
    xStaticQueue = xQueueCreateStatic(QUEUE_LENGTH,
                                       QUEUE_ITEM_SIZE,
                                       ucQueueStorage,
                                       &xQueueBuffer);
    if (xStaticQueue != NULL)
    {
        UART_SendString("[OK] Static Queue created\r\n");
    }
    
    /*
     * CREATE STATIC BINARY SEMAPHORE
     */
    xStaticSemaphore = xSemaphoreCreateBinaryStatic(&xSemaphoreBuffer);
    if (xStaticSemaphore != NULL)
    {
        xSemaphoreGive(xStaticSemaphore);  /* Initialize as available */
        UART_SendString("[OK] Static Semaphore created\r\n");
    }
    
    /*
     * CREATE STATIC MUTEX
     */
    xStaticMutex = xSemaphoreCreateMutexStatic(&xMutexBuffer);
    if (xStaticMutex != NULL)
    {
        UART_SendString("[OK] Static Mutex created\r\n");
    }
    
    /*
     * CREATE STATIC TASKS
     */
    xTask1Handle = xTaskCreateStatic(prvStaticTask1,
                                      "StaticT1",
                                      STATIC_TASK_STACK,
                                      NULL,
                                      2,
                                      xTask1Stack,
                                      &xTask1TCB);
    
    xTask2Handle = xTaskCreateStatic(prvStaticTask2,
                                      "StaticT2",
                                      STATIC_TASK_STACK,
                                      NULL,
                                      2,
                                      xTask2Stack,
                                      &xTask2TCB);
    
    UART_SendString("[OK] Static Tasks created\r\n\r\n");
    
    /* Memory summary */
    size_t xStaticUsed = sizeof(xTask1Stack) + sizeof(xTask1TCB) +
                         sizeof(xTask2Stack) + sizeof(xTask2TCB) +
                         sizeof(ucQueueStorage) + sizeof(xQueueBuffer) +
                         sizeof(xSemaphoreBuffer) + sizeof(xMutexBuffer) +
                         sizeof(xIdleStack) + sizeof(xIdleTCB) +
                         sizeof(xTimerStack) + sizeof(xTimerTCB);
    
    snprintf(msg, sizeof(msg), "[Info] Static memory used: %u bytes (in BSS)\r\n",
             (unsigned)xStaticUsed);
    UART_SendString(msg);
    
    snprintf(msg, sizeof(msg), "[Info] Heap remaining: %u bytes (untouched)\r\n",
             (unsigned)xPortGetFreeHeapSize());
    UART_SendString(msg);
    
    UART_SendString("\r\n────────────────────────────────────────────────────────────\r\n");
    
    vTaskStartScheduler();
    
    while (1);
}

/* ============================================================================
 * STATIC TASK 1 - Producer
 * ============================================================================ */
static void prvStaticTask1(void *pvParameters)
{
    (void)pvParameters;
    
    char msg[80];
    uint32_t ulValue = 0;
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        ulValue++;
        
        /* Use static mutex */
        if (xSemaphoreTake(xStaticMutex, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            /* Send to static queue */
            if (xQueueSend(xStaticQueue, &ulValue, pdMS_TO_TICKS(100)) == pdTRUE)
            {
                snprintf(msg, sizeof(msg), "[StaticT1] Sent: %lu\r\n", ulValue);
                UART_SendString(msg);
            }
            
            xSemaphoreGive(xStaticMutex);
        }
        
        HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
    }
}

/* ============================================================================
 * STATIC TASK 2 - Consumer
 * ============================================================================ */
static void prvStaticTask2(void *pvParameters)
{
    (void)pvParameters;
    
    char msg[80];
    uint32_t ulReceived;
    
    for (;;)
    {
        /* Receive from static queue */
        if (xQueueReceive(xStaticQueue, &ulReceived, portMAX_DELAY) == pdTRUE)
        {
            snprintf(msg, sizeof(msg), "[StaticT2] Received: %lu\r\n", ulReceived);
            UART_SendString(msg);
            
            /* Signal with static semaphore */
            xSemaphoreGive(xStaticSemaphore);
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
