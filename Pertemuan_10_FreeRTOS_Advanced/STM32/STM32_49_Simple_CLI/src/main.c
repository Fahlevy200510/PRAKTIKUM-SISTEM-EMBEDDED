/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 43-Simple_CLI
 * 
 * DESKRIPSI:
 * Demonstrasi Command Line Interface sederhana. User dapat mengirim perintah
 * via serial terminal untuk berinteraksi dengan sistem.
 * 
 * ============================================================================
 * PERINTAH YANG TERSEDIA
 * ============================================================================
 * 
 *    help    - Tampilkan daftar perintah
 *    status  - Tampilkan status sistem
 *    led on  - Nyalakan LED
 *    led off - Matikan LED
 *    tasks   - Daftar semua task
 *    heap    - Tampilkan penggunaan heap
 *    uptime  - Tampilkan waktu sejak boot
 *    reset   - Software reset
 * 
 * ============================================================================
 * CARA PENGGUNAAN
 * ============================================================================
 * 
 *    1. Connect serial terminal (115200 baud)
 *    2. Ketik perintah dan tekan Enter
 *    3. Sistem akan merespon dengan output
 * 
 * ============================================================================
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/* ============================================================================
 * CLI COMMAND STRUCTURE
 * ============================================================================ */
typedef void (*CommandHandler_t)(const char *args);

typedef struct {
    const char *pcCommand;
    const char *pcHelp;
    CommandHandler_t pxHandler;
} CLICommand_t;

/* ============================================================================
 * GLOBAL VARIABLES
 * ============================================================================ */
UART_HandleTypeDef huart1;
QueueHandle_t xCmdQueue = NULL;

/* Command buffer */
static char pcCmdBuffer[CLI_CMD_MAX_LEN];
static uint8_t ucCmdIndex = 0;

/* ============================================================================
 * COMMAND HANDLERS - FORWARD DECLARATIONS
 * ============================================================================ */
static void CMD_Help(const char *args);
static void CMD_Status(const char *args);
static void CMD_Led(const char *args);
static void CMD_Tasks(const char *args);
static void CMD_Heap(const char *args);
static void CMD_Uptime(const char *args);
static void CMD_Reset(const char *args);

/* ============================================================================
 * COMMAND TABLE
 * ============================================================================ */
static const CLICommand_t xCommands[] = {
    {"help",    "Show available commands",      CMD_Help},
    {"status",  "Show system status",           CMD_Status},
    {"led",     "Control LED (on/off)",         CMD_Led},
    {"tasks",   "List running tasks",           CMD_Tasks},
    {"heap",    "Show heap usage",              CMD_Heap},
    {"uptime",  "Show time since boot",         CMD_Uptime},
    {"reset",   "Software reset",               CMD_Reset},
    {NULL, NULL, NULL}  /* Terminator */
};

/* ============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================ */
static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART1_Init(void);
static void prvCLITask(void *pvParameters);
static void prvUartRxTask(void *pvParameters);
static void UART_SendString(const char *str);
static void ProcessCommand(const char *pcCmd);

/* ============================================================================
 * UART HELPER
 * ============================================================================ */
static void UART_SendString(const char *str)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}

static void UART_SendChar(char c)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&c, 1, HAL_MAX_DELAY);
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
    UART_SendString("║               43. SIMPLE CLI DEMO                        ║\r\n");
    UART_SendString("╚══════════════════════════════════════════════════════════╝\r\n\r\n");
    
    UART_SendString("Type 'help' for available commands\r\n\r\n");
    
    /* Create command queue */
    xCmdQueue = xQueueCreate(3, CLI_CMD_MAX_LEN);
    if (xCmdQueue == NULL)
    {
        UART_SendString("[ERROR] Queue creation failed!\r\n");
        while (1);
    }
    
    /* Create tasks */
    xTaskCreate(prvCLITask, "CLI", TASK_STACK + 128, NULL, 2, NULL);
    xTaskCreate(prvUartRxTask, "UART_RX", TASK_STACK, NULL, 3, NULL);
    
    /* Print first prompt */
    UART_SendString(CLI_PROMPT);
    
    vTaskStartScheduler();
    
    while (1);
}

/* ============================================================================
 * UART RX TASK
 * Receives characters and builds command buffer
 * ============================================================================ */
static void prvUartRxTask(void *pvParameters)
{
    (void)pvParameters;
    
    uint8_t ucRxChar;
    
    for (;;)
    {
        /* Poll for character (simple approach) */
        if (HAL_UART_Receive(&huart1, &ucRxChar, 1, pdMS_TO_TICKS(50)) == HAL_OK)
        {
            /* Handle backspace */
            if (ucRxChar == '\b' || ucRxChar == 127)
            {
                if (ucCmdIndex > 0)
                {
                    ucCmdIndex--;
                    pcCmdBuffer[ucCmdIndex] = '\0';
                    /* Echo backspace */
                    UART_SendString("\b \b");
                }
            }
            /* Handle Enter */
            else if (ucRxChar == '\r' || ucRxChar == '\n')
            {
                UART_SendString("\r\n");
                
                /* Null terminate and send to CLI task */
                pcCmdBuffer[ucCmdIndex] = '\0';
                
                if (ucCmdIndex > 0)
                {
                    xQueueSend(xCmdQueue, pcCmdBuffer, pdMS_TO_TICKS(100));
                }
                else
                {
                    /* Empty command - just show prompt */
                    UART_SendString(CLI_PROMPT);
                }
                
                /* Reset buffer */
                ucCmdIndex = 0;
                memset(pcCmdBuffer, 0, CLI_CMD_MAX_LEN);
            }
            /* Normal character */
            else if (ucRxChar >= 32 && ucRxChar < 127)
            {
                if (ucCmdIndex < CLI_CMD_MAX_LEN - 1)
                {
                    pcCmdBuffer[ucCmdIndex++] = ucRxChar;
                    /* Echo character */
                    UART_SendChar(ucRxChar);
                }
            }
        }
        
        /* LED indicates activity */
        HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
    }
}

/* ============================================================================
 * CLI TASK
 * Processes commands from queue
 * ============================================================================ */
static void prvCLITask(void *pvParameters)
{
    (void)pvParameters;
    
    char pcReceivedCmd[CLI_CMD_MAX_LEN];
    
    for (;;)
    {
        /* Wait for command */
        if (xQueueReceive(xCmdQueue, pcReceivedCmd, portMAX_DELAY) == pdTRUE)
        {
            ProcessCommand(pcReceivedCmd);
            UART_SendString(CLI_PROMPT);
        }
    }
}

/* ============================================================================
 * PROCESS COMMAND
 * Parses and executes command
 * ============================================================================ */
static void ProcessCommand(const char *pcCmd)
{
    char pcCmdCopy[CLI_CMD_MAX_LEN];
    strncpy(pcCmdCopy, pcCmd, CLI_CMD_MAX_LEN - 1);
    pcCmdCopy[CLI_CMD_MAX_LEN - 1] = '\0';
    
    /* Convert to lowercase for matching */
    char *p = pcCmdCopy;
    while (*p)
    {
        *p = tolower(*p);
        p++;
    }
    
    /* Extract command (first word) */
    char *pcCommand = strtok(pcCmdCopy, " ");
    char *pcArgs = strtok(NULL, "");  /* Rest of string */
    
    if (pcCommand == NULL)
    {
        return;
    }
    
    /* Search command table */
    for (int i = 0; xCommands[i].pcCommand != NULL; i++)
    {
        if (strcmp(pcCommand, xCommands[i].pcCommand) == 0)
        {
            xCommands[i].pxHandler(pcArgs);
            return;
        }
    }
    
    /* Command not found */
    char msg[80];
    snprintf(msg, sizeof(msg), "Unknown command: '%s'\r\n", pcCommand);
    UART_SendString(msg);
    UART_SendString("Type 'help' for available commands\r\n\r\n");
}

/* ============================================================================
 * COMMAND HANDLERS
 * ============================================================================ */

static void CMD_Help(const char *args)
{
    (void)args;
    
    UART_SendString("\r\n");
    UART_SendString("┌─── AVAILABLE COMMANDS ────────────────────────────────────┐\r\n");
    
    for (int i = 0; xCommands[i].pcCommand != NULL; i++)
    {
        char line[60];
        snprintf(line, sizeof(line), "│  %-10s - %-40s│\r\n",
                 xCommands[i].pcCommand, xCommands[i].pcHelp);
        UART_SendString(line);
    }
    
    UART_SendString("└────────────────────────────────────────────────────────────┘\r\n\r\n");
}

static void CMD_Status(const char *args)
{
    (void)args;
    
    char msg[80];
    
    UART_SendString("\r\n");
    UART_SendString("┌─── SYSTEM STATUS ─────────────────────────────────────────┐\r\n");
    
    snprintf(msg, sizeof(msg), "│  CPU Clock: %lu MHz                                       │\r\n",
             HAL_RCC_GetSysClockFreq() / 1000000);
    UART_SendString(msg);
    
    snprintf(msg, sizeof(msg), "│  Tick Rate: %lu Hz                                        │\r\n",
             (unsigned long)configTICK_RATE_HZ);
    UART_SendString(msg);
    
    snprintf(msg, sizeof(msg), "│  Uptime: %lu ms                                           │\r\n",
             (unsigned long)xTaskGetTickCount());
    UART_SendString(msg);
    
    snprintf(msg, sizeof(msg), "│  Tasks: %lu                                               │\r\n",
             (unsigned long)uxTaskGetNumberOfTasks());
    UART_SendString(msg);
    
    snprintf(msg, sizeof(msg), "│  LED: %s                                                 │\r\n",
             (HAL_GPIO_ReadPin(LED_GPIO_PORT, LED_GPIO_PIN) == GPIO_PIN_RESET) ? "ON " : "OFF");
    UART_SendString(msg);
    
    UART_SendString("└────────────────────────────────────────────────────────────┘\r\n\r\n");
}

static void CMD_Led(const char *args)
{
    if (args == NULL)
    {
        UART_SendString("Usage: led on | led off\r\n\r\n");
        return;
    }
    
    if (strstr(args, "on") != NULL)
    {
        HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_RESET);
        UART_SendString("LED turned ON\r\n\r\n");
    }
    else if (strstr(args, "off") != NULL)
    {
        HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_SET);
        UART_SendString("LED turned OFF\r\n\r\n");
    }
    else
    {
        UART_SendString("Usage: led on | led off\r\n\r\n");
    }
}

static void CMD_Tasks(const char *args)
{
    (void)args;
    
    char msg[120];
    TaskStatus_t xTaskDetails[10];
    UBaseType_t uxArraySize;
    
    UART_SendString("\r\n");
    UART_SendString("┌─── TASK LIST ─────────────────────────────────────────────┐\r\n");
    UART_SendString("│  Name            State   Prio    Stack Free               │\r\n");
    UART_SendString("│  ──────────────────────────────────────────               │\r\n");
    
    uxArraySize = uxTaskGetSystemState(xTaskDetails, 10, NULL);
    
    for (UBaseType_t i = 0; i < uxArraySize; i++)
    {
        char cState;
        switch (xTaskDetails[i].eCurrentState)
        {
            case eRunning:   cState = 'R'; break;
            case eReady:     cState = 'r'; break;
            case eBlocked:   cState = 'B'; break;
            case eSuspended: cState = 'S'; break;
            case eDeleted:   cState = 'D'; break;
            default:         cState = '?'; break;
        }
        
        snprintf(msg, sizeof(msg), "│  %-16s  %c      %lu       %lu                        │\r\n",
                 xTaskDetails[i].pcTaskName,
                 cState,
                 (unsigned long)xTaskDetails[i].uxCurrentPriority,
                 (unsigned long)xTaskDetails[i].usStackHighWaterMark);
        UART_SendString(msg);
    }
    
    UART_SendString("└────────────────────────────────────────────────────────────┘\r\n");
    UART_SendString("States: R=Running, r=Ready, B=Blocked, S=Suspended\r\n\r\n");
}

static void CMD_Heap(const char *args)
{
    (void)args;
    
    char msg[80];
    size_t xFree = xPortGetFreeHeapSize();
    size_t xMinFree = xPortGetMinimumEverFreeHeapSize();
    
    UART_SendString("\r\n");
    UART_SendString("┌─── HEAP USAGE ─────────────────────────────────────────────┐\r\n");
    
    snprintf(msg, sizeof(msg), "│  Total heap: %lu bytes                                    │\r\n",
             (unsigned long)configTOTAL_HEAP_SIZE);
    UART_SendString(msg);
    
    snprintf(msg, sizeof(msg), "│  Free now: %lu bytes (%lu%%)                               │\r\n",
             (unsigned long)xFree, (unsigned long)(xFree * 100 / configTOTAL_HEAP_SIZE));
    UART_SendString(msg);
    
    snprintf(msg, sizeof(msg), "│  Min free ever: %lu bytes                                 │\r\n",
             (unsigned long)xMinFree);
    UART_SendString(msg);
    
    snprintf(msg, sizeof(msg), "│  Used: %lu bytes                                          │\r\n",
             (unsigned long)(configTOTAL_HEAP_SIZE - xFree));
    UART_SendString(msg);
    
    UART_SendString("└────────────────────────────────────────────────────────────┘\r\n\r\n");
}

static void CMD_Uptime(const char *args)
{
    (void)args;
    
    char msg[80];
    uint32_t ulTicks = xTaskGetTickCount();
    uint32_t ulSeconds = ulTicks / 1000;
    uint32_t ulMinutes = ulSeconds / 60;
    uint32_t ulHours = ulMinutes / 60;
    
    snprintf(msg, sizeof(msg), "Uptime: %lu:%02lu:%02lu (%lu ms)\r\n\r\n",
             ulHours, ulMinutes % 60, ulSeconds % 60, ulTicks);
    UART_SendString(msg);
}

static void CMD_Reset(const char *args)
{
    (void)args;
    
    UART_SendString("\r\n");
    UART_SendString("Resetting system in 2 seconds...\r\n");
    
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    NVIC_SystemReset();
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
