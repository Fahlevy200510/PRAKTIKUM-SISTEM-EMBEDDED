/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 40-Task_State_Machine
 * 
 * DESKRIPSI:
 * Implementasi State Machine pattern menggunakan FreeRTOS.
 * Contoh: Simulasi sistem mesin kopi dengan state-state berbeda.
 * 
 * ============================================================================
 * ARSITEKTUR SISTEM
 * ============================================================================
 * 
 *    ┌──────────────────────────────────────────────────────────────────────┐
 *    │                  STATE MACHINE - COFFEE MACHINE                      │
 *    │                                                                      │
 *    │   ┌─────────────┐                                                    │
 *    │   │ Event Task  │  Simulates button press, sensor input              │
 *    │   │             │                                                    │
 *    │   └──────┬──────┘                                                    │
 *    │          │                                                           │
 *    │          │ xQueueSend(xEventQueue, &event)                           │
 *    │          │                                                           │
 *    │          ▼                                                           │
 *    │   ┌───────────────────────────────────────────────────────────────┐  │
 *    │   │                    EVENT QUEUE                                │  │
 *    │   │  ┌──────┬──────┬──────┬──────┬──────┐                         │  │
 *    │   │  │START │ TEMP │ DONE │ERROR │ ...  │                         │  │
 *    │   │  └──────┴──────┴──────┴──────┴──────┘                         │  │
 *    │   └───────────────────────────────────────────────────────────────┘  │
 *    │          │                                                           │
 *    │          │ xQueueReceive()                                           │
 *    │          ▼                                                           │
 *    │   ┌───────────────────────────────────────────────────────────────┐  │
 *    │   │              STATE MACHINE TASK                               │  │
 *    │   │                                                               │  │
 *    │   │    ┌─────────┐    ┌─────────┐    ┌─────────┐    ┌─────────┐  │  │
 *    │   │    │  IDLE   │───▶│ HEATING │───▶│ BREWING │───▶│COMPLETE │  │  │
 *    │   │    └────┬────┘    └────┬────┘    └────┬────┘    └────┬────┘  │  │
 *    │   │         │              │              │              │       │  │
 *    │   │         │              ▼              ▼              │       │  │
 *    │   │         │         ┌─────────┐                        │       │  │
 *    │   │         └─────────│  ERROR  │◀───────────────────────┘       │  │
 *    │   │                   └─────────┘                                │  │
 *    │   └───────────────────────────────────────────────────────────────┘  │
 *    │                                                                      │
 *    └──────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * CONTOH OUTPUT SERIAL
 * ============================================================================
 * 
 *    === TASK STATE MACHINE DEMO ===
 *    
 *    [SM] Initialized, entering IDLE state
 *    [SM:IDLE] LED slow blink, waiting for start...
 *    
 *    [Event] Sending START event
 *    [SM] Transition: IDLE → HEATING
 *    [SM:HEATING] Heating water...
 *    [SM:HEATING] Temperature: 25°C
 *    [SM:HEATING] Temperature: 45°C
 *    [SM:HEATING] Temperature: 68°C
 *    [SM:HEATING] Temperature: 91°C - Target reached!
 *    [SM] Transition: HEATING → BREWING
 *    [SM:BREWING] Making coffee...
 *    [SM:BREWING] Progress: 25%
 *    [SM:BREWING] Progress: 50%
 *    [SM:BREWING] Progress: 75%
 *    [SM:BREWING] Progress: 100%
 *    [SM] Transition: BREWING → COMPLETE
 *    [SM:COMPLETE] Coffee ready! Returning to IDLE
 *    [SM] Transition: COMPLETE → IDLE
 * 
 * ============================================================================
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * STATE MACHINE DEFINITIONS
 * ============================================================================ */

/* State enumeration */
typedef enum {
    STATE_IDLE,
    STATE_HEATING,
    STATE_BREWING,
    STATE_COMPLETE,
    STATE_ERROR,
    STATE_COUNT
} State_t;

/* Event enumeration */
typedef enum {
    EVENT_NONE,
    EVENT_START,
    EVENT_TEMP_OK,
    EVENT_BREW_DONE,
    EVENT_ERROR,
    EVENT_RESET,
    EVENT_TIMEOUT
} Event_t;

/* State machine context */
typedef struct {
    State_t eCurrentState;
    State_t ePreviousState;
    uint32_t ulStateEntryTime;
    uint32_t ulTemperature;
    uint32_t ulBrewProgress;
    uint32_t ulTransitionCount;
} StateMachine_t;

/* State name strings for logging */
static const char *pcStateNames[] = {
    "IDLE",
    "HEATING",
    "BREWING",
    "COMPLETE",
    "ERROR"
};

static const char *pcEventNames[] = {
    "NONE",
    "START",
    "TEMP_OK",
    "BREW_DONE",
    "ERROR",
    "RESET",
    "TIMEOUT"
};

/* ============================================================================
 * GLOBAL VARIABLES
 * ============================================================================ */
UART_HandleTypeDef huart1;
QueueHandle_t xEventQueue = NULL;
StateMachine_t xStateMachine;

/* ============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================ */
static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART1_Init(void);
static void prvStateMachineTask(void *pvParameters);
static void prvEventGeneratorTask(void *pvParameters);
static void UART_SendString(const char *str);

/* State handler functions */
static void StateHandler_Idle(StateMachine_t *pxSM, Event_t eEvent);
static void StateHandler_Heating(StateMachine_t *pxSM, Event_t eEvent);
static void StateHandler_Brewing(StateMachine_t *pxSM, Event_t eEvent);
static void StateHandler_Complete(StateMachine_t *pxSM, Event_t eEvent);
static void StateHandler_Error(StateMachine_t *pxSM, Event_t eEvent);

/* State function pointer type */
typedef void (*StateHandler_t)(StateMachine_t *, Event_t);

/* State handler lookup table */
static const StateHandler_t xStateHandlers[STATE_COUNT] = {
    StateHandler_Idle,
    StateHandler_Heating,
    StateHandler_Brewing,
    StateHandler_Complete,
    StateHandler_Error
};

/* ============================================================================
 * HELPER FUNCTIONS
 * ============================================================================ */
static void UART_SendString(const char *str)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}

static void TransitionTo(StateMachine_t *pxSM, State_t eNewState)
{
    char msg[80];
    
    pxSM->ePreviousState = pxSM->eCurrentState;
    pxSM->eCurrentState = eNewState;
    pxSM->ulStateEntryTime = xTaskGetTickCount();
    pxSM->ulTransitionCount++;
    
    snprintf(msg, sizeof(msg), "\r\n[SM] Transition: %s → %s (count: %lu)\r\n",
             pcStateNames[pxSM->ePreviousState],
             pcStateNames[pxSM->eCurrentState],
             pxSM->ulTransitionCount);
    UART_SendString(msg);
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
    UART_SendString("║           40. TASK STATE MACHINE DEMO                    ║\r\n");
    UART_SendString("╚══════════════════════════════════════════════════════════╝\r\n\r\n");
    
    UART_SendString("[Info] Coffee Machine Simulator\r\n");
    UART_SendString("[Info] States: IDLE → HEATING → BREWING → COMPLETE\r\n\r\n");
    
    /* Initialize state machine */
    xStateMachine.eCurrentState = STATE_IDLE;
    xStateMachine.ePreviousState = STATE_IDLE;
    xStateMachine.ulStateEntryTime = 0;
    xStateMachine.ulTemperature = 25;  /* Room temperature */
    xStateMachine.ulBrewProgress = 0;
    xStateMachine.ulTransitionCount = 0;
    
    /* Create event queue */
    xEventQueue = xQueueCreate(10, sizeof(Event_t));
    if (xEventQueue == NULL)
    {
        UART_SendString("[ERROR] Event queue creation failed!\r\n");
        while (1);
    }
    
    /* Create tasks */
    xTaskCreate(prvStateMachineTask, "SM", TASK_STACK, NULL, 2, NULL);
    xTaskCreate(prvEventGeneratorTask, "EventGen", TASK_STACK, NULL, 1, NULL);
    
    UART_SendString("────────────────────────────────────────────────────────────\r\n\r\n");
    
    vTaskStartScheduler();
    
    while (1);
}

/* ============================================================================
 * STATE MACHINE TASK
 * Main task that runs the state machine
 * ============================================================================ */
static void prvStateMachineTask(void *pvParameters)
{
    (void)pvParameters;
    
    Event_t eEvent;
    
    UART_SendString("[SM] State Machine started\r\n");
    UART_SendString("[SM:IDLE] Entering IDLE state...\r\n\r\n");
    
    for (;;)
    {
        /* Wait for event with timeout */
        if (xQueueReceive(xEventQueue, &eEvent, pdMS_TO_TICKS(500)) == pdTRUE)
        {
            char msg[60];
            snprintf(msg, sizeof(msg), "[SM] Received event: %s\r\n", 
                     pcEventNames[eEvent]);
            UART_SendString(msg);
        }
        else
        {
            /* Timeout - generate internal timer event */
            eEvent = EVENT_NONE;
        }
        
        /* Call current state handler */
        if (xStateMachine.eCurrentState < STATE_COUNT)
        {
            xStateHandlers[xStateMachine.eCurrentState](&xStateMachine, eEvent);
        }
        
        /* LED indicates current state */
        switch (xStateMachine.eCurrentState)
        {
            case STATE_IDLE:
                /* LED ON (slow) */
                HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_RESET);
                vTaskDelay(pdMS_TO_TICKS(100));
                HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_SET);
                break;
                
            case STATE_HEATING:
            case STATE_BREWING:
                /* LED fast blink */
                HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
                break;
                
            case STATE_COMPLETE:
                /* LED ON solid */
                HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_RESET);
                break;
                
            case STATE_ERROR:
                /* LED rapid blink */
                HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
                vTaskDelay(pdMS_TO_TICKS(50));
                break;
                
            default:
                break;
        }
    }
}

/* ============================================================================
 * STATE HANDLERS
 * ============================================================================ */

static void StateHandler_Idle(StateMachine_t *pxSM, Event_t eEvent)
{
    static uint32_t ulIdleCount = 0;
    
    switch (eEvent)
    {
        case EVENT_START:
            /* Reset and transition to HEATING */
            pxSM->ulTemperature = 25;
            pxSM->ulBrewProgress = 0;
            TransitionTo(pxSM, STATE_HEATING);
            break;
            
        case EVENT_NONE:
            ulIdleCount++;
            if (ulIdleCount % 10 == 0)
            {
                UART_SendString("[SM:IDLE] Waiting for START event...\r\n");
            }
            break;
            
        default:
            break;
    }
}

static void StateHandler_Heating(StateMachine_t *pxSM, Event_t eEvent)
{
    char msg[60];
    
    switch (eEvent)
    {
        case EVENT_ERROR:
            TransitionTo(pxSM, STATE_ERROR);
            break;
            
        case EVENT_NONE:
            /* Simulate heating */
            pxSM->ulTemperature += 8 + (xTaskGetTickCount() % 5);
            
            snprintf(msg, sizeof(msg), "[SM:HEATING] Temperature: %lu°C\r\n",
                     pxSM->ulTemperature);
            UART_SendString(msg);
            
            /* Check if target temperature reached */
            if (pxSM->ulTemperature >= 90)
            {
                UART_SendString("[SM:HEATING] Target temperature reached!\r\n");
                TransitionTo(pxSM, STATE_BREWING);
            }
            break;
            
        default:
            break;
    }
}

static void StateHandler_Brewing(StateMachine_t *pxSM, Event_t eEvent)
{
    char msg[60];
    
    switch (eEvent)
    {
        case EVENT_ERROR:
            TransitionTo(pxSM, STATE_ERROR);
            break;
            
        case EVENT_NONE:
            /* Simulate brewing */
            pxSM->ulBrewProgress += 10;
            
            snprintf(msg, sizeof(msg), "[SM:BREWING] Progress: %lu%%\r\n",
                     pxSM->ulBrewProgress);
            UART_SendString(msg);
            
            /* Check if brewing complete */
            if (pxSM->ulBrewProgress >= 100)
            {
                UART_SendString("[SM:BREWING] Brewing complete!\r\n");
                TransitionTo(pxSM, STATE_COMPLETE);
            }
            break;
            
        default:
            break;
    }
}

static void StateHandler_Complete(StateMachine_t *pxSM, Event_t eEvent)
{
    (void)eEvent;
    
    /* Auto-transition back to IDLE after showing complete */
    static uint32_t ulCompleteCount = 0;
    ulCompleteCount++;
    
    if (ulCompleteCount == 1)
    {
        UART_SendString("[SM:COMPLETE] ☕ Coffee ready!\r\n");
    }
    else if (ulCompleteCount >= 4)
    {
        UART_SendString("[SM:COMPLETE] Returning to IDLE...\r\n");
        ulCompleteCount = 0;
        TransitionTo(pxSM, STATE_IDLE);
    }
}

static void StateHandler_Error(StateMachine_t *pxSM, Event_t eEvent)
{
    switch (eEvent)
    {
        case EVENT_RESET:
            UART_SendString("[SM:ERROR] Reset received, returning to IDLE\r\n");
            pxSM->ulTemperature = 25;
            pxSM->ulBrewProgress = 0;
            TransitionTo(pxSM, STATE_IDLE);
            break;
            
        case EVENT_NONE:
            UART_SendString("[SM:ERROR] System error! Send RESET to recover\r\n");
            break;
            
        default:
            break;
    }
}

/* ============================================================================
 * EVENT GENERATOR TASK
 * Simulates external events
 * ============================================================================ */
static void prvEventGeneratorTask(void *pvParameters)
{
    (void)pvParameters;
    
    Event_t eEvent;
    uint32_t ulCycle = 0;
    
    UART_SendString("[EventGen] Event generator started\r\n\r\n");
    
    for (;;)
    {
        ulCycle++;
        
        /* Wait between cycles */
        vTaskDelay(pdMS_TO_TICKS(8000));
        
        UART_SendString("\r\n╔════════════════════════════════════════════╗\r\n");
        UART_SendString("║          NEW COFFEE CYCLE                  ║\r\n");
        UART_SendString("╚════════════════════════════════════════════╝\r\n\r\n");
        
        /* Send START event */
        eEvent = EVENT_START;
        UART_SendString("[EventGen] Sending START event (button pressed)\r\n");
        xQueueSend(xEventQueue, &eEvent, pdMS_TO_TICKS(100));
        
        /* Occasionally inject error for demonstration */
        if (ulCycle % 3 == 0)
        {
            vTaskDelay(pdMS_TO_TICKS(3000));
            eEvent = EVENT_ERROR;
            UART_SendString("[EventGen] Injecting ERROR event!\r\n");
            xQueueSend(xEventQueue, &eEvent, pdMS_TO_TICKS(100));
            
            vTaskDelay(pdMS_TO_TICKS(2000));
            eEvent = EVENT_RESET;
            UART_SendString("[EventGen] Sending RESET event\r\n");
            xQueueSend(xEventQueue, &eEvent, pdMS_TO_TICKS(100));
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
