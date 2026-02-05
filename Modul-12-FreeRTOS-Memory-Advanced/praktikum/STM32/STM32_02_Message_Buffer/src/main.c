/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 30-Message_Buffer
 * 
 * DESKRIPSI:
 * Demo Message Buffer untuk discrete message communication. Berbeda dengan
 * Stream Buffer, Message Buffer menjaga message boundaries sehingga setiap
 * receive mendapat COMPLETE message.
 * 
 * ============================================================================
 * ARSITEKTUR SISTEM
 * ============================================================================
 * 
 *    ┌──────────────────────────────────────────────────────────────────────┐
 *    │                      MESSAGE BUFFER DEMO                             │
 *    │                                                                      │
 *    │   Command Task                            Handler Task               │
 *    │   ┌─────────────────┐                     ┌─────────────────┐        │
 *    │   │  Generate       │                     │  Process        │        │
 *    │   │  commands:      │                     │  commands:      │        │
 *    │   │  - LED_ON       │                     │  - Parse        │        │
 *    │   │  - LED_OFF      │                     │  - Execute      │        │
 *    │   │  - STATUS       │                     │  - Respond      │        │
 *    │   └────────┬────────┘                     └────────▲────────┘        │
 *    │            │                                       │                 │
 *    │            │ xMessageBufferSend()    xMessageBufferReceive()         │
 *    │            │                                       │                 │
 *    │            ▼                                       │                 │
 *    │   ┌────────────────────────────────────────────────┴────────┐        │
 *    │   │                    MESSAGE BUFFER                       │        │
 *    │   │  ┌───┬────────────┬───┬──────────┬───┬──────────────┐   │        │
 *    │   │  │ 6 │ LED_ON │ 7 │LED_OFF│10 │STATUS:OK │   │        │
 *    │   │  └───┴────────────┴───┴──────────┴───┴──────────────┘   │        │
 *    │   │                                                         │        │
 *    │   │    Messages with length prefix                          │        │
 *    │   └─────────────────────────────────────────────────────────┘        │
 *    │                                                                      │
 *    └──────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * COMMAND PROTOCOL
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │  Command Message Format:                                           │
 *    │                                                                     │
 *    │  ┌──────────┬──────────┬─────────────────────────────┐             │
 *    │  │   Type   │   Cmd    │          Payload            │             │
 *    │  │  1 byte  │  1 byte  │        variable             │             │
 *    │  └──────────┴──────────┴─────────────────────────────┘             │
 *    │                                                                     │
 *    │  Types:                                                             │
 *    │    0x01 = LED Control (payload: state 0/1)                         │
 *    │    0x02 = Sensor Read (payload: sensor ID)                         │
 *    │    0x03 = System Command (payload: command string)                 │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * CONTOH OUTPUT SERIAL
 * ============================================================================
 * 
 *    === MESSAGE BUFFER DEMO ===
 *    Buffer size: 200 bytes
 *    
 *    [Sender] Sending: {Type:LED, Cmd:ON, Len:3}
 *    [Handler] Received message (3 bytes)
 *    [Handler] LED Command: ON
 *    [Handler] LED turned ON
 *    
 *    [Sender] Sending: {Type:SENSOR, Cmd:READ, Payload:TEMP}
 *    [Handler] Received message (6 bytes)
 *    [Handler] Sensor Read: ID=TEMP, Value=25.3
 *    
 *    [Sender] Sending: {Type:SYSTEM, Cmd:STATUS}
 *    [Handler] Received message (4 bytes)
 *    [Handler] System Status: OK, Free heap: 5120
 * 
 * ============================================================================
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * MESSAGE TYPES
 * ============================================================================ */

/* Message types */
typedef enum {
    MSG_TYPE_LED     = 0x01,
    MSG_TYPE_SENSOR  = 0x02,
    MSG_TYPE_SYSTEM  = 0x03,
} MessageType_t;

/* LED commands */
typedef enum {
    LED_CMD_OFF = 0,
    LED_CMD_ON  = 1,
    LED_CMD_TOGGLE = 2,
} LedCommand_t;

/* Sensor IDs */
typedef enum {
    SENSOR_TEMP   = 0,
    SENSOR_HUMID  = 1,
    SENSOR_LIGHT  = 2,
} SensorId_t;

/* System commands */
typedef enum {
    SYS_CMD_STATUS  = 0,
    SYS_CMD_RESET   = 1,
    SYS_CMD_HEAP    = 2,
} SystemCommand_t;

/* Generic message header */
typedef struct {
    uint8_t type;
    uint8_t cmd;
    uint8_t payload[MAX_MESSAGE_SIZE - 2];  /* Variable payload */
} Message_t;

/* ============================================================================
 * GLOBAL VARIABLES
 * ============================================================================ */
UART_HandleTypeDef huart1;
MessageBufferHandle_t xMessageBuffer = NULL;

/* Statistics */
static uint32_t ulMessagesSent = 0;
static uint32_t ulMessagesReceived = 0;

/* ============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================ */
static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART1_Init(void);
static void prvSenderTask(void *pvParameters);
static void prvHandlerTask(void *pvParameters);
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
    
    char msg[150];
    
    UART_SendString("\r\n\r\n");
    UART_SendString("╔══════════════════════════════════════════════════════════╗\r\n");
    UART_SendString("║         30. MESSAGE BUFFER - FRAMED MESSAGES             ║\r\n");
    UART_SendString("╚══════════════════════════════════════════════════════════╝\r\n\r\n");
    
    snprintf(msg, sizeof(msg), "[Config] Message Buffer size: %d bytes\r\n", MESSAGE_BUFFER_SIZE);
    UART_SendString(msg);
    snprintf(msg, sizeof(msg), "[Config] Max message size: %d bytes\r\n\r\n", MAX_MESSAGE_SIZE);
    UART_SendString(msg);
    
    /*
     * CREATE MESSAGE BUFFER
     * 
     * xMessageBufferCreate(size):
     *   - size: Total buffer size in bytes
     *   - Messages automatically prefixed with length
     *   - Overhead: sizeof(size_t) = 4 bytes per message
     */
    xMessageBuffer = xMessageBufferCreate(MESSAGE_BUFFER_SIZE);
    
    if (xMessageBuffer == NULL)
    {
        UART_SendString("[ERROR] Failed to create Message Buffer!\r\n");
        while (1);
    }
    UART_SendString("[OK] Message Buffer created\r\n\r\n");
    
    /* Create tasks */
    xTaskCreate(prvSenderTask, "Sender", TASK_STACK, NULL, 2, NULL);
    xTaskCreate(prvHandlerTask, "Handler", TASK_STACK, NULL, 3, NULL);
    
    UART_SendString("[INFO] Tasks created. Starting scheduler...\r\n\r\n");
    UART_SendString("────────────────────────────────────────────────────────────\r\n");
    
    vTaskStartScheduler();
    
    while (1);
}

/* ============================================================================
 * SENDER TASK
 * Generates various command messages
 * ============================================================================ */
static void prvSenderTask(void *pvParameters)
{
    (void)pvParameters;
    
    Message_t txMsg;
    char msg[120];
    size_t xBytesSent;
    uint32_t ulMsgNum = 0;
    
    const TickType_t xSendDelay = pdMS_TO_TICKS(1500);
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        ulMsgNum++;
        
        /* Cycle through different message types */
        uint8_t msgType = (ulMsgNum % 6);
        size_t msgLen = 2;  /* Header only by default */
        
        switch (msgType)
        {
            case 0:
                /* LED ON */
                txMsg.type = MSG_TYPE_LED;
                txMsg.cmd = LED_CMD_ON;
                UART_SendString("\r\n[Sender] Creating: LED_ON command\r\n");
                break;
                
            case 1:
                /* LED OFF */
                txMsg.type = MSG_TYPE_LED;
                txMsg.cmd = LED_CMD_OFF;
                UART_SendString("\r\n[Sender] Creating: LED_OFF command\r\n");
                break;
                
            case 2:
                /* LED TOGGLE */
                txMsg.type = MSG_TYPE_LED;
                txMsg.cmd = LED_CMD_TOGGLE;
                UART_SendString("\r\n[Sender] Creating: LED_TOGGLE command\r\n");
                break;
                
            case 3:
                /* Read Temperature sensor */
                txMsg.type = MSG_TYPE_SENSOR;
                txMsg.cmd = SENSOR_TEMP;
                UART_SendString("\r\n[Sender] Creating: SENSOR_READ(TEMP) command\r\n");
                break;
                
            case 4:
                /* Read Humidity sensor */
                txMsg.type = MSG_TYPE_SENSOR;
                txMsg.cmd = SENSOR_HUMID;
                UART_SendString("\r\n[Sender] Creating: SENSOR_READ(HUMID) command\r\n");
                break;
                
            case 5:
                /* System status with string payload */
                txMsg.type = MSG_TYPE_SYSTEM;
                txMsg.cmd = SYS_CMD_STATUS;
                strcpy((char *)txMsg.payload, "CHECK");
                msgLen = 2 + strlen("CHECK");
                UART_SendString("\r\n[Sender] Creating: SYSTEM_STATUS command\r\n");
                break;
        }
        
        snprintf(msg, sizeof(msg), "          Message: Type=0x%02X, Cmd=0x%02X, Len=%u\r\n",
                 txMsg.type, txMsg.cmd, (unsigned)msgLen);
        UART_SendString(msg);
        
        /*
         * SEND MESSAGE
         * 
         * xMessageBufferSend():
         *   - Sends COMPLETE message atomically
         *   - Length prefix added automatically
         *   - Returns bytes sent (0 if failed/timeout)
         */
        xBytesSent = xMessageBufferSend(xMessageBuffer,
                                         &txMsg,
                                         msgLen,
                                         pdMS_TO_TICKS(100));
        
        if (xBytesSent == msgLen)
        {
            ulMessagesSent++;
            snprintf(msg, sizeof(msg), "          Sent successfully (total: %lu)\r\n",
                     ulMessagesSent);
            UART_SendString(msg);
        }
        else
        {
            UART_SendString("          [WARN] Send failed - buffer full?\r\n");
        }
        
        vTaskDelayUntil(&xLastWakeTime, xSendDelay);
    }
}

/* ============================================================================
 * HANDLER TASK
 * Receives and processes command messages
 * ============================================================================ */
static void prvHandlerTask(void *pvParameters)
{
    (void)pvParameters;
    
    Message_t rxMsg;
    char msg[150];
    size_t xBytesReceived;
    
    /* Simulated sensor values */
    float fTemperature = 25.0f;
    float fHumidity = 60.0f;
    
    for (;;)
    {
        /*
         * RECEIVE MESSAGE
         * 
         * xMessageBufferReceive():
         *   - Receives COMPLETE message
         *   - Blocks until full message available
         *   - Returns actual message size
         *   - Will NOT return partial message!
         */
        xBytesReceived = xMessageBufferReceive(xMessageBuffer,
                                                &rxMsg,
                                                sizeof(rxMsg),
                                                portMAX_DELAY);
        
        if (xBytesReceived >= 2)  /* Minimum: type + cmd */
        {
            ulMessagesReceived++;
            
            snprintf(msg, sizeof(msg), "[Handler] Received #%lu (%u bytes): Type=0x%02X, Cmd=0x%02X\r\n",
                     ulMessagesReceived, (unsigned)xBytesReceived, rxMsg.type, rxMsg.cmd);
            UART_SendString(msg);
            
            /* Process based on message type */
            switch (rxMsg.type)
            {
                case MSG_TYPE_LED:
                    switch (rxMsg.cmd)
                    {
                        case LED_CMD_ON:
                            HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_RESET);
                            UART_SendString("          → LED turned ON\r\n");
                            break;
                        case LED_CMD_OFF:
                            HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_SET);
                            UART_SendString("          → LED turned OFF\r\n");
                            break;
                        case LED_CMD_TOGGLE:
                            HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
                            UART_SendString("          → LED toggled\r\n");
                            break;
                    }
                    break;
                    
                case MSG_TYPE_SENSOR:
                    /* Simulate sensor readings */
                    fTemperature += 0.1f;
                    if (fTemperature > 35.0f) fTemperature = 20.0f;
                    
                    fHumidity += 1.0f;
                    if (fHumidity > 90.0f) fHumidity = 40.0f;
                    
                    switch (rxMsg.cmd)
                    {
                        case SENSOR_TEMP:
                            snprintf(msg, sizeof(msg), "          → Temperature: %.1f°C\r\n", fTemperature);
                            UART_SendString(msg);
                            break;
                        case SENSOR_HUMID:
                            snprintf(msg, sizeof(msg), "          → Humidity: %.1f%%\r\n", fHumidity);
                            UART_SendString(msg);
                            break;
                        case SENSOR_LIGHT:
                            UART_SendString("          → Light: 1024 lux\r\n");
                            break;
                    }
                    break;
                    
                case MSG_TYPE_SYSTEM:
                    switch (rxMsg.cmd)
                    {
                        case SYS_CMD_STATUS:
                            snprintf(msg, sizeof(msg), 
                                     "          → System OK, Heap free: %u bytes\r\n",
                                     (unsigned)xPortGetFreeHeapSize());
                            UART_SendString(msg);
                            break;
                        case SYS_CMD_RESET:
                            UART_SendString("          → Reset requested (ignored)\r\n");
                            break;
                        case SYS_CMD_HEAP:
                            snprintf(msg, sizeof(msg), 
                                     "          → Min free heap: %u bytes\r\n",
                                     (unsigned)xPortGetMinimumEverFreeHeapSize());
                            UART_SendString(msg);
                            break;
                    }
                    break;
                    
                default:
                    UART_SendString("          → Unknown message type!\r\n");
                    break;
            }
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
    
    /* LED PC13 */
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
