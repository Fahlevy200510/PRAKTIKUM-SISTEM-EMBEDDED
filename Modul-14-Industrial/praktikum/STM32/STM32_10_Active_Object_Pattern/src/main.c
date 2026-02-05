/**
 * =============================================================================
 * PROGRAM 48: ACTIVE OBJECT PATTERN - Encapsulated Concurrency dengan FreeRTOS
 * =============================================================================
 * 
 * Demo Active Object pattern dengan LED Controller, Button Monitor,
 * dan Temperature Monitor sebagai contoh Active Objects.
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include <stdio.h>
#include <string.h>

/* ========================== HARDWARE CONFIGURATION ======================== */
#define LED_PIN         GPIO_PIN_13
#define LED_PORT        GPIOC

/* ========================== EVENT SIGNALS ================================= */
typedef enum {
    SIG_NONE = 0,
    SIG_INIT,
    SIG_TIMER_TICK,
    SIG_BUTTON_PRESS,
    SIG_TEMP_UPDATE,
    SIG_SET_MODE,
    SIG_GET_STATUS,
    SIG_SHUTDOWN
} AO_Signal_t;

/* ========================== DATA STRUCTURES =============================== */

/* Forward declaration */
struct ActiveObject;

/* Event structure */
typedef struct {
    AO_Signal_t eSignal;          /* Event type */
    uint32_t    ulParam1;         /* Parameter 1 */
    uint32_t    ulParam2;         /* Parameter 2 */
    struct ActiveObject *pxSender; /* Sender (optional) */
} AO_Event_t;

/* Active Object event handler function type */
typedef void (*AO_Handler_t)(struct ActiveObject *pxAO, AO_Event_t *pxEvent);

/* Active Object structure */
typedef struct ActiveObject {
    QueueHandle_t  xQueue;        /* Event queue */
    TaskHandle_t   xTask;         /* Private thread */
    AO_Handler_t   pfnHandler;    /* Event handler */
    const char    *pcName;        /* Object name */
    uint8_t        ucId;          /* Object ID */
    uint32_t       ulEventsProcessed;
    void          *pvState;       /* Private state pointer */
} ActiveObject_t;

/* LED Controller State */
typedef struct {
    uint8_t  ucState;             /* LED state machine state */
    uint8_t  ucBlinkOn;           /* Current blink state */
    uint16_t usTemp;              /* Last temperature */
    TimerHandle_t xBlinkTimer;    /* Blink timer */
} LedState_t;

/* ========================== GLOBALS ======================================= */

UART_HandleTypeDef huart1;

/* Active Objects */
static ActiveObject_t xLedAO;
static ActiveObject_t xButtonAO;
static ActiveObject_t xTempAO;

/* LED state */
static LedState_t xLedState;

/* Random seed */
static uint32_t ulRandomSeed = 12345;

/* ========================== FUNCTION PROTOTYPES =========================== */

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART1_Init(void);
static void UART_SendString(const char *str);

static void vActiveObjectThread(void *pvParameters);
static BaseType_t xAO_Create(ActiveObject_t *pxAO, const char *pcName, 
                              uint8_t ucId, AO_Handler_t pfnHandler,
                              uint8_t ucPriority);
static BaseType_t xAO_Post(ActiveObject_t *pxAO, AO_Event_t *pxEvent);
static BaseType_t xAO_PostUrgent(ActiveObject_t *pxAO, AO_Event_t *pxEvent);

static void vLedHandler(ActiveObject_t *pxAO, AO_Event_t *pxEvent);
static void vButtonHandler(ActiveObject_t *pxAO, AO_Event_t *pxEvent);
static void vTempHandler(ActiveObject_t *pxAO, AO_Event_t *pxEvent);

static void vBlinkTimerCallback(TimerHandle_t xTimer);

/* ========================== HELPER FUNCTIONS ============================== */

static uint32_t ulRandom(void)
{
    ulRandomSeed = ulRandomSeed * 1103515245 + 12345;
    return (ulRandomSeed >> 16) & 0x7FFF;
}

static const char* pcSignalName(AO_Signal_t eSignal)
{
    switch (eSignal)
    {
        case SIG_INIT: return "INIT";
        case SIG_TIMER_TICK: return "TIMER_TICK";
        case SIG_BUTTON_PRESS: return "BUTTON_PRESS";
        case SIG_TEMP_UPDATE: return "TEMP_UPDATE";
        case SIG_SET_MODE: return "SET_MODE";
        case SIG_GET_STATUS: return "GET_STATUS";
        case SIG_SHUTDOWN: return "SHUTDOWN";
        default: return "???";
    }
}

static const char* pcLedStateName(uint8_t ucState)
{
    switch (ucState)
    {
        case LED_STATE_OFF: return "OFF";
        case LED_STATE_SOLID_ON: return "SOLID_ON";
        case LED_STATE_BLINK: return "BLINK";
        default: return "???";
    }
}

/* ========================== ACTIVE OBJECT API ============================= */

/**
 * Active Object thread - Event loop
 */
static void vActiveObjectThread(void *pvParameters)
{
    ActiveObject_t *pxAO = (ActiveObject_t *)pvParameters;
    AO_Event_t xEvent;
    
    for (;;)
    {
        /* Wait for event */
        if (xQueueReceive(pxAO->xQueue, &xEvent, portMAX_DELAY) == pdTRUE)
        {
            /* Dispatch to handler */
            if (pxAO->pfnHandler != NULL)
            {
                pxAO->pfnHandler(pxAO, &xEvent);
                pxAO->ulEventsProcessed++;
            }
            
            /* Check for shutdown */
            if (xEvent.eSignal == SIG_SHUTDOWN)
            {
                break;
            }
        }
    }
    
    /* Cleanup */
    vQueueDelete(pxAO->xQueue);
    vTaskDelete(NULL);
}

/**
 * Create Active Object
 */
static BaseType_t xAO_Create(ActiveObject_t *pxAO, const char *pcName, 
                              uint8_t ucId, AO_Handler_t pfnHandler,
                              uint8_t ucPriority)
{
    char cBuffer[50];
    
    /* Initialize structure */
    pxAO->pcName = pcName;
    pxAO->ucId = ucId;
    pxAO->pfnHandler = pfnHandler;
    pxAO->ulEventsProcessed = 0;
    pxAO->pvState = NULL;
    
    /* Create event queue */
    pxAO->xQueue = xQueueCreate(AO_QUEUE_SIZE, sizeof(AO_Event_t));
    if (pxAO->xQueue == NULL)
    {
        return pdFALSE;
    }
    
    /* Create private thread */
    if (xTaskCreate(vActiveObjectThread, pcName, AO_STACK_SIZE, 
                    pxAO, ucPriority, &pxAO->xTask) != pdPASS)
    {
        vQueueDelete(pxAO->xQueue);
        return pdFALSE;
    }
    
    snprintf(cBuffer, sizeof(cBuffer), "[%s] Active Object created\r\n", pcName);
    UART_SendString(cBuffer);
    
    /* Send init event */
    AO_Event_t xInitEvent = { .eSignal = SIG_INIT };
    xAO_Post(pxAO, &xInitEvent);
    
    return pdTRUE;
}

/**
 * Post event to Active Object (FIFO)
 */
static BaseType_t xAO_Post(ActiveObject_t *pxAO, AO_Event_t *pxEvent)
{
    return xQueueSend(pxAO->xQueue, pxEvent, pdMS_TO_TICKS(10));
}

/**
 * Post urgent event (LIFO - front of queue)
 */
static BaseType_t xAO_PostUrgent(ActiveObject_t *pxAO, AO_Event_t *pxEvent)
{
    return xQueueSendToFront(pxAO->xQueue, pxEvent, pdMS_TO_TICKS(10));
}

/* ========================== LED CONTROLLER ================================ */

/**
 * Blink timer callback
 */
static void vBlinkTimerCallback(TimerHandle_t xTimer)
{
    (void)xTimer;
    
    /* Post timer tick to LED AO */
    AO_Event_t xEvent = { .eSignal = SIG_TIMER_TICK };
    xAO_Post(&xLedAO, &xEvent);
}

/**
 * LED Controller event handler
 */
static void vLedHandler(ActiveObject_t *pxAO, AO_Event_t *pxEvent)
{
    char cBuffer[80];
    LedState_t *pxState = (LedState_t *)pxAO->pvState;
    uint8_t ucOldState = pxState->ucState;
    
    snprintf(cBuffer, sizeof(cBuffer), "[LED_AO] Event: %s\r\n", 
             pcSignalName(pxEvent->eSignal));
    UART_SendString(cBuffer);
    
    switch (pxEvent->eSignal)
    {
        case SIG_INIT:
            pxState->ucState = LED_STATE_OFF;
            pxState->ucBlinkOn = 0;
            HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET); /* LED OFF */
            UART_SendString("[LED_AO] Initialized, state: OFF\r\n");
            break;
            
        case SIG_BUTTON_PRESS:
            /* State machine transitions */
            switch (pxState->ucState)
            {
                case LED_STATE_OFF:
                    pxState->ucState = LED_STATE_SOLID_ON;
                    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
                    break;
                    
                case LED_STATE_SOLID_ON:
                    pxState->ucState = LED_STATE_BLINK;
                    xTimerStart(pxState->xBlinkTimer, 0);
                    break;
                    
                case LED_STATE_BLINK:
                    pxState->ucState = LED_STATE_OFF;
                    xTimerStop(pxState->xBlinkTimer, 0);
                    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
                    break;
            }
            
            if (ucOldState != pxState->ucState)
            {
                snprintf(cBuffer, sizeof(cBuffer), 
                         "[LED_AO] State: %s -> %s\r\n",
                         pcLedStateName(ucOldState), pcLedStateName(pxState->ucState));
                UART_SendString(cBuffer);
            }
            break;
            
        case SIG_TIMER_TICK:
            if (pxState->ucState == LED_STATE_BLINK)
            {
                pxState->ucBlinkOn = !pxState->ucBlinkOn;
                HAL_GPIO_WritePin(LED_PORT, LED_PIN, 
                                  pxState->ucBlinkOn ? GPIO_PIN_RESET : GPIO_PIN_SET);
            }
            break;
            
        case SIG_TEMP_UPDATE:
            pxState->usTemp = (uint16_t)pxEvent->ulParam1;
            snprintf(cBuffer, sizeof(cBuffer), 
                     "[LED_AO] Temp update: %.1f°C\r\n", pxState->usTemp / 10.0f);
            UART_SendString(cBuffer);
            
            /* Alert if high temperature */
            if (pxState->usTemp > 300) /* > 30.0°C */
            {
                if (pxState->ucState != LED_STATE_BLINK)
                {
                    UART_SendString("[LED_AO] HIGH TEMP! Enabling blink\r\n");
                    pxState->ucState = LED_STATE_BLINK;
                    xTimerStart(pxState->xBlinkTimer, 0);
                }
            }
            break;
            
        case SIG_SET_MODE:
            pxState->ucState = (uint8_t)pxEvent->ulParam1;
            snprintf(cBuffer, sizeof(cBuffer), 
                     "[LED_AO] Mode set to: %s\r\n", pcLedStateName(pxState->ucState));
            UART_SendString(cBuffer);
            break;
            
        case SIG_GET_STATUS:
            snprintf(cBuffer, sizeof(cBuffer), 
                     "[LED_AO] Status: State=%s, Events=%lu\r\n",
                     pcLedStateName(pxState->ucState), pxAO->ulEventsProcessed);
            UART_SendString(cBuffer);
            break;
            
        default:
            break;
    }
}

/* ========================== BUTTON MONITOR ================================ */

/**
 * Button Monitor event handler
 */
static void vButtonHandler(ActiveObject_t *pxAO, AO_Event_t *pxEvent)
{
    char cBuffer[60];
    static TickType_t xLastButtonTime = 0;
    static uint32_t ulButtonCount = 0;
    
    switch (pxEvent->eSignal)
    {
        case SIG_INIT:
            UART_SendString("[BTN_AO] Initialized, monitoring started\r\n");
            break;
            
        case SIG_TIMER_TICK:
            /* Simulate button press every ~5 seconds */
            if ((xTaskGetTickCount() - xLastButtonTime) >= pdMS_TO_TICKS(5000))
            {
                xLastButtonTime = xTaskGetTickCount();
                ulButtonCount++;
                
                snprintf(cBuffer, sizeof(cBuffer), 
                         "[BTN_AO] Button press #%lu detected\r\n", ulButtonCount);
                UART_SendString(cBuffer);
                
                /* Forward to LED controller */
                AO_Event_t xBtnEvent = { 
                    .eSignal = SIG_BUTTON_PRESS,
                    .ulParam1 = ulButtonCount,
                    .pxSender = pxAO
                };
                xAO_Post(&xLedAO, &xBtnEvent);
                
                UART_SendString("[BTN_AO] Posted BUTTON_PRESS to LED_AO\r\n");
            }
            break;
            
        case SIG_GET_STATUS:
            snprintf(cBuffer, sizeof(cBuffer), 
                     "[BTN_AO] Status: Presses=%lu, Events=%lu\r\n",
                     ulButtonCount, pxAO->ulEventsProcessed);
            UART_SendString(cBuffer);
            break;
            
        default:
            break;
    }
}

/* ========================== TEMPERATURE MONITOR =========================== */

/**
 * Temperature Monitor event handler
 */
static void vTempHandler(ActiveObject_t *pxAO, AO_Event_t *pxEvent)
{
    char cBuffer[60];
    static uint16_t usLastTemp = 250; /* 25.0°C */
    
    switch (pxEvent->eSignal)
    {
        case SIG_INIT:
            UART_SendString("[TEMP_AO] Initialized, sampling started\r\n");
            break;
            
        case SIG_TIMER_TICK:
            /* Simulate temperature reading (20.0-35.0°C) */
            usLastTemp = 200 + (ulRandom() % 150);
            
            snprintf(cBuffer, sizeof(cBuffer), 
                     "[TEMP_AO] Temperature: %.1f°C\r\n", usLastTemp / 10.0f);
            UART_SendString(cBuffer);
            
            /* Send to LED controller */
            AO_Event_t xTempEvent = {
                .eSignal = SIG_TEMP_UPDATE,
                .ulParam1 = usLastTemp,
                .pxSender = pxAO
            };
            xAO_Post(&xLedAO, &xTempEvent);
            break;
            
        case SIG_GET_STATUS:
            snprintf(cBuffer, sizeof(cBuffer), 
                     "[TEMP_AO] Status: LastTemp=%.1f°C, Events=%lu\r\n",
                     usLastTemp / 10.0f, pxAO->ulEventsProcessed);
            UART_SendString(cBuffer);
            break;
            
        default:
            break;
    }
}

/* ========================== TIMER TASK ==================================== */

/**
 * Periodic task to generate timer ticks for AOs
 */
static void vTickGeneratorTask(void *pvParameters)
{
    (void)pvParameters;
    
    TickType_t xLastWakeTime;
    AO_Event_t xTickEvent = { .eSignal = SIG_TIMER_TICK };
    uint32_t ulCycle = 0;
    char cBuffer[80];
    
    xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        ulCycle++;
        
        /* Send tick to Button AO (every second) */
        xAO_Post(&xButtonAO, &xTickEvent);
        
        /* Send tick to Temp AO (every 3 seconds) */
        if ((ulCycle % 3) == 0)
        {
            xAO_Post(&xTempAO, &xTickEvent);
        }
        
        /* Print statistics every 15 seconds */
        if ((ulCycle % 15) == 0)
        {
            UART_SendString("\r\n=== ACTIVE OBJECT STATISTICS ===\r\n");
            snprintf(cBuffer, sizeof(cBuffer), 
                     "LED_AO:  %lu events processed\r\n", xLedAO.ulEventsProcessed);
            UART_SendString(cBuffer);
            snprintf(cBuffer, sizeof(cBuffer), 
                     "BTN_AO:  %lu events processed\r\n", xButtonAO.ulEventsProcessed);
            UART_SendString(cBuffer);
            snprintf(cBuffer, sizeof(cBuffer), 
                     "TEMP_AO: %lu events processed\r\n", xTempAO.ulEventsProcessed);
            UART_SendString(cBuffer);
            
            /* Request status from each AO */
            AO_Event_t xStatusEvent = { .eSignal = SIG_GET_STATUS };
            xAO_Post(&xLedAO, &xStatusEvent);
            xAO_Post(&xButtonAO, &xStatusEvent);
            xAO_Post(&xTempAO, &xStatusEvent);
        }
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
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
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    UART1_Init();

    UART_SendString("\r\n");
    UART_SendString("============================================\r\n");
    UART_SendString("    PROGRAM 48: ACTIVE OBJECT PATTERN\r\n");
    UART_SendString("============================================\r\n");
    UART_SendString("Encapsulated concurrency with event-driven AOs\r\n\r\n");

    /* Initialize LED state */
    memset(&xLedState, 0, sizeof(xLedState));
    
    /* Create blink timer */
    xLedState.xBlinkTimer = xTimerCreate("BlinkTmr", 
                                          pdMS_TO_TICKS(LED_BLINK_PERIOD_MS),
                                          pdTRUE, /* Auto-reload */
                                          NULL,
                                          vBlinkTimerCallback);
    if (xLedState.xBlinkTimer == NULL)
    {
        UART_SendString("ERROR: Failed to create blink timer!\r\n");
        for (;;);
    }
    UART_SendString("Blink timer created\r\n");

    /* Create Active Objects */
    UART_SendString("\r\nCreating Active Objects...\r\n");
    
    /* LED Controller AO */
    if (xAO_Create(&xLedAO, "LED_AO", 0, vLedHandler, 2) != pdTRUE)
    {
        UART_SendString("ERROR: Failed to create LED AO!\r\n");
        for (;;);
    }
    xLedAO.pvState = &xLedState;
    
    /* Button Monitor AO */
    if (xAO_Create(&xButtonAO, "BTN_AO", 1, vButtonHandler, 2) != pdTRUE)
    {
        UART_SendString("ERROR: Failed to create Button AO!\r\n");
        for (;;);
    }
    
    /* Temperature Monitor AO */
    if (xAO_Create(&xTempAO, "TEMP_AO", 2, vTempHandler, 2) != pdTRUE)
    {
        UART_SendString("ERROR: Failed to create Temp AO!\r\n");
        for (;;);
    }

    /* Create tick generator task */
    xTaskCreate(vTickGeneratorTask, "TickGen", configMINIMAL_STACK_SIZE + 30, 
                NULL, 1, NULL);
    UART_SendString("Tick generator created\r\n");

    UART_SendString("\r\nStarting scheduler...\r\n");
    UART_SendString("----------------------------------------\r\n\r\n");

    vTaskStartScheduler();

    for (;;);
}
