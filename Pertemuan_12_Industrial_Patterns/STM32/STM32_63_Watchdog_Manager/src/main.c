/**
 * =============================================================================
 * PROGRAM 49: WATCHDOG MANAGER - Multi-Task Watchdog dengan FreeRTOS
 * =============================================================================
 * 
 * Demo Watchdog Manager yang memonitor multiple tasks dan hanya
 * memberi makan hardware watchdog jika semua task aktif.
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdio.h>
#include <string.h>

/* ========================== HARDWARE CONFIGURATION ======================== */
#define LED_PIN         GPIO_PIN_13
#define LED_PORT        GPIOC

/* ========================== DATA STRUCTURES =============================== */

/* Task status */
typedef enum {
    WDM_STATUS_INACTIVE = 0,
    WDM_STATUS_OK,
    WDM_STATUS_WARNING,
    WDM_STATUS_TIMEOUT
} WdmTaskStatus_t;

/* Monitored task info */
typedef struct {
    const char     *pcName;           /* Task name */
    uint32_t        ulTimeoutMs;      /* Timeout period */
    TickType_t      xLastCheckIn;     /* Last check-in time */
    WdmTaskStatus_t eStatus;          /* Current status */
    uint32_t        ulCheckInCount;   /* Total check-ins */
    uint32_t        ulTimeoutCount;   /* Timeout occurrences */
    BaseType_t      xActive;          /* Task registered */
} WdmTaskInfo_t;

/* Watchdog Manager state */
typedef enum {
    WDM_STATE_INIT = 0,
    WDM_STATE_RUNNING,
    WDM_STATE_WARNING,
    WDM_STATE_FAILURE
} WdmState_t;

/* ========================== GLOBALS ======================================= */

UART_HandleTypeDef huart1;
#if WDM_USE_HARDWARE_WDT
IWDG_HandleTypeDef hiwdg;
#endif

/* Watchdog Manager data */
static WdmTaskInfo_t xWdmTasks[WDM_MAX_TASKS];
static WdmState_t eWdmState = WDM_STATE_INIT;
static SemaphoreHandle_t xWdmMutex;
static uint32_t ulWdtFeedCount = 0;
static TickType_t xGracePeriodStart = 0;

/* Simulated hang flag */
static volatile BaseType_t xSimulateHang = pdFALSE;

/* ========================== FUNCTION PROTOTYPES =========================== */

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART1_Init(void);
static void UART_SendString(const char *str);
#if WDM_USE_HARDWARE_WDT
static void IWDG_Init(void);
#endif

static void vWatchdogManagerTask(void *pvParameters);
static void vSensorTask(void *pvParameters);
static void vDisplayTask(void *pvParameters);
static void vCommTask(void *pvParameters);

static BaseType_t xWDM_Register(uint8_t ucTaskId, const char *pcName, uint32_t ulTimeoutMs);
static void vWDM_CheckIn(uint8_t ucTaskId);
static WdmTaskStatus_t xWDM_GetStatus(uint8_t ucTaskId);

/* ========================== WATCHDOG MANAGER API ========================== */

/**
 * Register a task for monitoring
 */
static BaseType_t xWDM_Register(uint8_t ucTaskId, const char *pcName, uint32_t ulTimeoutMs)
{
    char cBuffer[60];
    
    if (ucTaskId >= WDM_MAX_TASKS)
    {
        return pdFALSE;
    }
    
    if (xSemaphoreTake(xWdmMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        xWdmTasks[ucTaskId].pcName = pcName;
        xWdmTasks[ucTaskId].ulTimeoutMs = ulTimeoutMs;
        xWdmTasks[ucTaskId].xLastCheckIn = xTaskGetTickCount();
        xWdmTasks[ucTaskId].eStatus = WDM_STATUS_OK;
        xWdmTasks[ucTaskId].ulCheckInCount = 0;
        xWdmTasks[ucTaskId].ulTimeoutCount = 0;
        xWdmTasks[ucTaskId].xActive = pdTRUE;
        
        xSemaphoreGive(xWdmMutex);
        
        snprintf(cBuffer, sizeof(cBuffer), 
                 "[WDM] Task %d \"%s\" registered (timeout: %lums)\r\n",
                 ucTaskId, pcName, ulTimeoutMs);
        UART_SendString(cBuffer);
        
        return pdTRUE;
    }
    
    return pdFALSE;
}

/**
 * Task check-in (heartbeat)
 */
static void vWDM_CheckIn(uint8_t ucTaskId)
{
    if (ucTaskId >= WDM_MAX_TASKS)
    {
        return;
    }
    
    if (xSemaphoreTake(xWdmMutex, pdMS_TO_TICKS(10)) == pdTRUE)
    {
        if (xWdmTasks[ucTaskId].xActive)
        {
            xWdmTasks[ucTaskId].xLastCheckIn = xTaskGetTickCount();
            xWdmTasks[ucTaskId].eStatus = WDM_STATUS_OK;
            xWdmTasks[ucTaskId].ulCheckInCount++;
        }
        xSemaphoreGive(xWdmMutex);
    }
}

/**
 * Get task status
 */
static WdmTaskStatus_t xWDM_GetStatus(uint8_t ucTaskId)
{
    WdmTaskStatus_t eStatus = WDM_STATUS_INACTIVE;
    
    if (ucTaskId < WDM_MAX_TASKS)
    {
        if (xSemaphoreTake(xWdmMutex, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            eStatus = xWdmTasks[ucTaskId].eStatus;
            xSemaphoreGive(xWdmMutex);
        }
    }
    
    return eStatus;
}

/* ========================== TASKS ========================================= */

/**
 * Watchdog Manager Task
 */
static void vWatchdogManagerTask(void *pvParameters)
{
    (void)pvParameters;
    
    TickType_t xLastWakeTime;
    TickType_t xCurrentTime;
    TickType_t xElapsed;
    uint8_t i;
    uint8_t ucAllOK;
    uint8_t ucTimeoutTask;
    char cBuffer[100];
    
    UART_SendString("[WDM] Watchdog Manager started\r\n");
    
    eWdmState = WDM_STATE_RUNNING;
    xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        xCurrentTime = xTaskGetTickCount();
        ucAllOK = 1;
        ucTimeoutTask = 0xFF;
        
        /* Check all registered tasks */
        if (xSemaphoreTake(xWdmMutex, pdMS_TO_TICKS(50)) == pdTRUE)
        {
            for (i = 0; i < WDM_MAX_TASKS; i++)
            {
                if (xWdmTasks[i].xActive)
                {
                    xElapsed = xCurrentTime - xWdmTasks[i].xLastCheckIn;
                    
                    if (xElapsed > pdMS_TO_TICKS(xWdmTasks[i].ulTimeoutMs))
                    {
                        /* Task timeout! */
                        if (xWdmTasks[i].eStatus != WDM_STATUS_TIMEOUT)
                        {
                            xWdmTasks[i].eStatus = WDM_STATUS_TIMEOUT;
                            xWdmTasks[i].ulTimeoutCount++;
                            ucTimeoutTask = i;
                        }
                        ucAllOK = 0;
                    }
                    else if (xElapsed > pdMS_TO_TICKS(xWdmTasks[i].ulTimeoutMs * 80 / 100))
                    {
                        /* Near timeout - warning */
                        xWdmTasks[i].eStatus = WDM_STATUS_WARNING;
                    }
                }
            }
            xSemaphoreGive(xWdmMutex);
        }
        
        /* State machine */
        switch (eWdmState)
        {
            case WDM_STATE_RUNNING:
                if (ucAllOK)
                {
                    /* All tasks OK - feed watchdog */
                    #if WDM_USE_HARDWARE_WDT
                    HAL_IWDG_Refresh(&hiwdg);
                    #endif
                    ulWdtFeedCount++;
                    
                    if ((ulWdtFeedCount % 20) == 0)  /* Every 2 seconds */
                    {
                        snprintf(cBuffer, sizeof(cBuffer), 
                                 "[WDM] All OK - WDT fed #%lu\r\n", ulWdtFeedCount);
                        UART_SendString(cBuffer);
                    }
                    
                    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET); /* LED ON */
                }
                else
                {
                    /* Task timeout detected */
                    if (ucTimeoutTask < WDM_MAX_TASKS)
                    {
                        snprintf(cBuffer, sizeof(cBuffer), 
                                 "[WDM] Task %d \"%s\" TIMEOUT!\r\n",
                                 ucTimeoutTask, xWdmTasks[ucTimeoutTask].pcName);
                        UART_SendString(cBuffer);
                    }
                    
                    UART_SendString("[WDM] WARNING: Entering grace period\r\n");
                    xGracePeriodStart = xCurrentTime;
                    eWdmState = WDM_STATE_WARNING;
                }
                break;
                
            case WDM_STATE_WARNING:
                /* Still feed WDT during grace period */
                #if WDM_USE_HARDWARE_WDT
                HAL_IWDG_Refresh(&hiwdg);
                #endif
                ulWdtFeedCount++;
                
                HAL_GPIO_TogglePin(LED_PORT, LED_PIN); /* LED blink */
                
                if (ucAllOK)
                {
                    /* Recovered! */
                    UART_SendString("[WDM] RECOVERY: All tasks OK again\r\n");
                    eWdmState = WDM_STATE_RUNNING;
                }
                else if ((xCurrentTime - xGracePeriodStart) > pdMS_TO_TICKS(WDM_GRACE_PERIOD_MS))
                {
                    /* Grace period expired */
                    UART_SendString("[WDM] FAILURE: Grace period expired!\r\n");
                    eWdmState = WDM_STATE_FAILURE;
                    
                    /* Print status of all tasks */
                    UART_SendString("[WDM] === TASK STATUS ===\r\n");
                    for (i = 0; i < WDM_MAX_TASKS; i++)
                    {
                        if (xWdmTasks[i].xActive)
                        {
                            const char *pcStatus;
                            switch (xWdmTasks[i].eStatus)
                            {
                                case WDM_STATUS_OK: pcStatus = "OK"; break;
                                case WDM_STATUS_WARNING: pcStatus = "WARN"; break;
                                case WDM_STATUS_TIMEOUT: pcStatus = "TIMEOUT"; break;
                                default: pcStatus = "???"; break;
                            }
                            snprintf(cBuffer, sizeof(cBuffer), 
                                     "  [%d] %s: %s (checkins: %lu, timeouts: %lu)\r\n",
                                     i, xWdmTasks[i].pcName, pcStatus,
                                     xWdmTasks[i].ulCheckInCount,
                                     xWdmTasks[i].ulTimeoutCount);
                            UART_SendString(cBuffer);
                        }
                    }
                }
                break;
                
            case WDM_STATE_FAILURE:
                /* DON'T feed watchdog! */
                HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET); /* LED OFF */
                
                UART_SendString("[WDM] !!! NOT FEEDING WDT - RESET IMMINENT !!!\r\n");
                
                #if WDM_USE_HARDWARE_WDT
                /* Hardware WDT will reset the system */
                #else
                /* For demo, simulate reset after a while */
                vTaskDelay(pdMS_TO_TICKS(2000));
                UART_SendString("[WDM] (Would reset here if IWDG enabled)\r\n");
                eWdmState = WDM_STATE_RUNNING; /* For demo only */
                xSimulateHang = pdFALSE;
                #endif
                break;
                
            default:
                break;
        }
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(WDM_CHECK_PERIOD_MS));
    }
}

/**
 * Sensor Task
 */
static void vSensorTask(void *pvParameters)
{
    (void)pvParameters;
    
    TickType_t xLastWakeTime;
    
    /* Register with watchdog manager */
    xWDM_Register(0, "Sensor", TASK_SENSOR_TIMEOUT);
    
    UART_SendString("[Sensor] Task started\r\n");
    
    xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        /* Simulate work */
        vTaskDelay(pdMS_TO_TICKS(10 + (xTaskGetTickCount() % 20)));
        
        /* Check in with watchdog manager */
        vWDM_CheckIn(0);
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
    }
}

/**
 * Display Task
 */
static void vDisplayTask(void *pvParameters)
{
    (void)pvParameters;
    
    TickType_t xLastWakeTime;
    
    /* Register with watchdog manager */
    xWDM_Register(1, "Display", TASK_DISPLAY_TIMEOUT);
    
    UART_SendString("[Display] Task started\r\n");
    
    xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        /* Simulate work */
        vTaskDelay(pdMS_TO_TICKS(20 + (xTaskGetTickCount() % 30)));
        
        /* Check in with watchdog manager */
        vWDM_CheckIn(1);
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(200));
    }
}

/**
 * Communication Task - Will simulate hang
 */
static void vCommTask(void *pvParameters)
{
    (void)pvParameters;
    
    TickType_t xLastWakeTime;
    TickType_t xStartTime;
    
    /* Register with watchdog manager */
    xWDM_Register(2, "Comm", TASK_COMM_TIMEOUT);
    
    UART_SendString("[Comm] Task started\r\n");
    
    xLastWakeTime = xTaskGetTickCount();
    xStartTime = xTaskGetTickCount();
    
    for (;;)
    {
        /* Check if we should simulate hang */
        if (!xSimulateHang && 
            (xTaskGetTickCount() - xStartTime) > pdMS_TO_TICKS(SIMULATE_HANG_AFTER_MS))
        {
            UART_SendString("\r\n[Comm] !!! SIMULATING HANG !!!\r\n");
            xSimulateHang = pdTRUE;
        }
        
        if (xSimulateHang)
        {
            /* Hang - don't check in! */
            while (xSimulateHang)
            {
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        }
        
        /* Simulate work */
        vTaskDelay(pdMS_TO_TICKS(50 + (xTaskGetTickCount() % 50)));
        
        /* Check in with watchdog manager */
        vWDM_CheckIn(2);
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));
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

#if WDM_USE_HARDWARE_WDT
static void IWDG_Init(void)
{
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
    hiwdg.Init.Reload = 468;  /* ~3 seconds timeout */
    HAL_IWDG_Init(&hiwdg);
    
    UART_SendString("IWDG initialized (timeout: ~3s)\r\n");
}
#endif

/* ========================== MAIN ========================================== */

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    UART1_Init();

    UART_SendString("\r\n");
    UART_SendString("============================================\r\n");
    UART_SendString("    PROGRAM 49: WATCHDOG MANAGER DEMO\r\n");
    UART_SendString("============================================\r\n");
    UART_SendString("Multi-task watchdog monitoring system\r\n\r\n");

    #if WDM_USE_HARDWARE_WDT
    IWDG_Init();
    #else
    UART_SendString("(Hardware WDT disabled for demo)\r\n\r\n");
    #endif

    /* Initialize task info */
    memset(xWdmTasks, 0, sizeof(xWdmTasks));

    /* Create mutex */
    xWdmMutex = xSemaphoreCreateMutex();
    if (xWdmMutex == NULL)
    {
        UART_SendString("ERROR: Failed to create mutex!\r\n");
        for (;;);
    }
    UART_SendString("WDM mutex created\r\n");

    /* Create tasks */
    UART_SendString("\r\nCreating tasks...\r\n");
    
    xTaskCreate(vWatchdogManagerTask, "WDM", configMINIMAL_STACK_SIZE + 80, 
                NULL, 4, NULL);
    UART_SendString("  Watchdog Manager task created\r\n");
    
    xTaskCreate(vSensorTask, "Sensor", configMINIMAL_STACK_SIZE + 30, 
                NULL, 2, NULL);
    
    xTaskCreate(vDisplayTask, "Display", configMINIMAL_STACK_SIZE + 30, 
                NULL, 2, NULL);
    
    xTaskCreate(vCommTask, "Comm", configMINIMAL_STACK_SIZE + 30, 
                NULL, 2, NULL);

    UART_SendString("\r\n");
    UART_SendString("NOTE: Comm task will hang after 15 seconds to\r\n");
    UART_SendString("      demonstrate watchdog detection.\r\n");
    
    UART_SendString("\r\nStarting scheduler...\r\n");
    UART_SendString("----------------------------------------\r\n\r\n");

    vTaskStartScheduler();

    for (;;);
}
