/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 08-Flow_Control_Backpressure
 * 
 * JUDUL: Flow Control dan Backpressure dengan Queue
 * 
 * DESKRIPSI:
 * Demo mekanisme flow control menggunakan uxQueueSpacesAvailable().
 * Producer memperlambat laju produksi ketika queue hampir penuh,
 * mencegah data loss dan memberikan kesempatan consumer mengejar.
 * 
 * ============================================================================
 * ARSITEKTUR PROGRAM
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                     FLOW CONTROL SYSTEM                            │
 *    ├─────────────────────────────────────────────────────────────────────┤
 *    │                                                                     │
 *    │   ┌──────────────────┐         ┌────────────────────────────┐      │
 *    │   │  vProducerTask   │         │         QUEUE              │      │
 *    │   │                  │         │  ┌──┬──┬──┬──┬──┬──┬──┬──┐ │      │
 *    │   │ 1. Cek space ────┼────────►│  │  │  │  │  │  │██│██│██│ │      │
 *    │   │                  │◄────────┼──│  │  │  │  │  │  │  │  │ │      │
 *    │   │ 2. Adjust delay  │ spaces  │  └──┴──┴──┴──┴──┴──┴──┴──┘ │      │
 *    │   │                  │         │                            │      │
 *    │   │ 3. Send data     │────────►│  uxQueueSpacesAvailable() │      │
 *    │   └──────────────────┘         └─────────────┬──────────────┘      │
 *    │                                              │                     │
 *    │                                              ▼                     │
 *    │                                ┌──────────────────┐                │
 *    │                                │  vConsumerTask   │                │
 *    │                                │  (proses lambat) │                │
 *    │                                │  300ms per item  │                │
 *    │                                └──────────────────┘                │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * ALGORITMA BACKPRESSURE
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                                                                     │
 *    │   spaces = uxQueueSpacesAvailable(queue);                          │
 *    │                                                                     │
 *    │   if (spaces >= 8)       → NORMAL,    delay = 100ms               │
 *    │   else if (spaces >= 5)  → SLOW,      delay = 200ms               │
 *    │   else if (spaces >= 2)  → VERY_SLOW, delay = 500ms               │
 *    │   else                   → CRITICAL,  delay = 1000ms              │
 *    │                                                                     │
 *    │   ILUSTRASI VISUAL:                                                 │
 *    │   ═══════════════════════════════════════════════════════════      │
 *    │                                                                     │
 *    │   Queue: 10 slots total                                            │
 *    │                                                                     │
 *    │   spaces = 9:  ░░░░░░░░░█  (10% full)  → NORMAL 🟢                │
 *    │   spaces = 6:  ░░░░░░████  (40% full)  → SLOW 🟡                  │
 *    │   spaces = 3:  ░░░███████  (70% full)  → VERY_SLOW 🟠             │
 *    │   spaces = 1:  ░█████████  (90% full)  → CRITICAL 🔴              │
 *    │                                                                     │
 *    │   ░ = kosong, █ = terisi                                           │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * EXPECTED OUTPUT (UART 115200 baud)
 * ============================================================================
 * 
 *    === Flow Control Backpressure Demo ===
 *    
 *    [PROD] Data #1, Space=10, Mode=NORMAL (100ms)
 *    [PROD] Data #2, Space=9, Mode=NORMAL (100ms)
 *    [CONS] Processing data #1...
 *    [PROD] Data #3, Space=8, Mode=NORMAL (100ms)
 *    [PROD] Data #4, Space=7, Mode=SLOW (200ms)
 *    [PROD] Data #5, Space=6, Mode=SLOW (200ms)
 *    [CONS] Processing data #2...
 *    [PROD] Data #6, Space=5, Mode=SLOW (200ms)
 *    [PROD] Data #7, Space=4, Mode=VERY_SLOW (500ms)
 *    [CONS] Processing data #3...
 *    [PROD] Data #8, Space=4, Mode=VERY_SLOW (500ms)
 *    ...
 * 
 * ============================================================================
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * DEFINISI
 * ============================================================================ */

typedef enum {
    MODE_NORMAL,
    MODE_SLOW,
    MODE_VERY_SLOW,
    MODE_CRITICAL
} FlowMode_t;

typedef struct {
    uint32_t   dataId;
    TickType_t timestamp;
} DataPacket_t;

/* ============================================================================
 * HANDLE GLOBAL
 * ============================================================================ */

static QueueHandle_t xDataQueue = NULL;

static UART_HandleTypeDef huart1;

/* Statistik */
static volatile uint32_t ulDataProduced = 0;
static volatile uint32_t ulDataConsumed = 0;
static volatile uint32_t ulDataDropped = 0;

/* ============================================================================
 * PROTOTYPE
 * ============================================================================ */

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART_Init(void);
static void UART_SendString(const char *str);

static void vProducerTask(void *pvParameters);
static void vConsumerTask(void *pvParameters);
static void vMonitorTask(void *pvParameters);

static const char* GetModeName(FlowMode_t mode);
static uint32_t GetModeDelay(FlowMode_t mode);
static FlowMode_t DetermineMode(UBaseType_t spacesAvailable);

/* ============================================================================
 * HELPER FUNCTIONS
 * ============================================================================ */

static const char* GetModeName(FlowMode_t mode)
{
    switch(mode)
    {
        case MODE_NORMAL:    return "NORMAL";
        case MODE_SLOW:      return "SLOW";
        case MODE_VERY_SLOW: return "VERY_SLOW";
        case MODE_CRITICAL:  return "CRITICAL";
        default:             return "UNKNOWN";
    }
}

static uint32_t GetModeDelay(FlowMode_t mode)
{
    switch(mode)
    {
        case MODE_NORMAL:    return DELAY_NORMAL_MS;
        case MODE_SLOW:      return DELAY_SLOW_MS;
        case MODE_VERY_SLOW: return DELAY_VERY_SLOW_MS;
        case MODE_CRITICAL:  return DELAY_CRITICAL_MS;
        default:             return DELAY_NORMAL_MS;
    }
}

/**
 * @brief Tentukan mode berdasarkan space tersedia
 * 
 * ILUSTRASI:
 * ─────────────────────────────────────────────────────────────────
 *    
 *    Space Available:    10  9  8  7  6  5  4  3  2  1  0
 *                        │   │  │  │  │  │  │  │  │  │  │
 *    NORMAL:             ████████───────────────────────│
 *    SLOW:               ────────███████────────────────│
 *    VERY_SLOW:          ───────────────██████──────────│
 *    CRITICAL:           ─────────────────────█████████│
 *    
 * ─────────────────────────────────────────────────────────────────
 */
static FlowMode_t DetermineMode(UBaseType_t spacesAvailable)
{
    if(spacesAvailable >= THRESHOLD_NORMAL)
        return MODE_NORMAL;
    else if(spacesAvailable >= THRESHOLD_SLOW)
        return MODE_SLOW;
    else if(spacesAvailable >= THRESHOLD_VERY_SLOW)
        return MODE_VERY_SLOW;
    else
        return MODE_CRITICAL;
}

/* ============================================================================
 * IMPLEMENTASI TASK
 * ============================================================================ */

/**
 * @brief Producer Task dengan Backpressure Mechanism
 * 
 * FLOW DIAGRAM:
 * ═══════════════════════════════════════════════════════════════════════════
 *    
 *    ┌─────────────┐
 *    │   START     │
 *    └──────┬──────┘
 *           │
 *           ▼
 *    ┌──────────────────────────┐
 *    │ spaces = uxQueueSpaces() │  ← Cek berapa slot kosong
 *    └──────────────┬───────────┘
 *                   │
 *                   ▼
 *    ┌──────────────────────────┐
 *    │ mode = DetermineMode()   │  ← Tentukan mode berdasarkan space
 *    └──────────────┬───────────┘
 *                   │
 *                   ▼
 *    ┌──────────────────────────┐
 *    │ delay = GetModeDelay()   │  ← Dapat delay sesuai mode
 *    └──────────────┬───────────┘
 *                   │
 *                   ▼
 *    ┌──────────────────────────┐
 *    │ xQueueSend(data, 0)      │  ← Kirim dengan timeout=0
 *    └──────────────┬───────────┘
 *          ┌────────┴────────┐
 *          │                 │
 *        pdPASS            pdFAIL
 *          │                 │
 *          ▼                 ▼
 *    ┌───────────┐    ┌────────────┐
 *    │ Success!  │    │ Data DROP! │
 *    └─────┬─────┘    └─────┬──────┘
 *          │                │
 *          └────────┬───────┘
 *                   │
 *                   ▼
 *    ┌──────────────────────────┐
 *    │ vTaskDelay(delay)        │  ← Delay adaptive!
 *    └──────────────┬───────────┘
 *                   │
 *                   └──────────► (loop)
 *    
 * ═══════════════════════════════════════════════════════════════════════════
 */
static void vProducerTask(void *pvParameters)
{
    (void)pvParameters;
    
    DataPacket_t packet;
    uint32_t dataCounter = 0;
    UBaseType_t spacesAvailable;
    FlowMode_t currentMode;
    uint32_t delayMs;
    char buffer[100];
    
    UART_SendString("[PROD] Producer started with backpressure\r\n\r\n");
    
    for(;;)
    {
        dataCounter++;
        
        /* 1. Cek space tersedia di queue */
        spacesAvailable = uxQueueSpacesAvailable(xDataQueue);
        
        /* 2. Tentukan mode berdasarkan space */
        currentMode = DetermineMode(spacesAvailable);
        delayMs = GetModeDelay(currentMode);
        
        /* 3. Siapkan data */
        packet.dataId = dataCounter;
        packet.timestamp = xTaskGetTickCount();
        
        /* 4. Log status */
        snprintf(buffer, sizeof(buffer),
                "[PROD] Data #%lu, Space=%u, Mode=%s (%lums)\r\n",
                dataCounter, spacesAvailable, 
                GetModeName(currentMode), delayMs);
        UART_SendString(buffer);
        
        /* 5. Kirim dengan timeout=0 (non-blocking) */
        if(xQueueSend(xDataQueue, &packet, 0) == pdPASS)
        {
            ulDataProduced++;
        }
        else
        {
            ulDataDropped++;
            UART_SendString("[PROD] WARNING: Queue full, data dropped!\r\n");
        }
        
        /* 6. Delay ADAPTIF berdasarkan mode */
        vTaskDelay(pdMS_TO_TICKS(delayMs));
    }
}

/**
 * @brief Consumer Task - Proses lambat untuk simulasi
 * 
 * Consumer sengaja dibuat lambat (300ms per item)
 * untuk mendemonstrasikan efek backpressure.
 */
static void vConsumerTask(void *pvParameters)
{
    (void)pvParameters;
    
    DataPacket_t rxPacket;
    char buffer[80];
    TickType_t latency;
    
    UART_SendString("[CONS] Consumer started (slow: 300ms per item)\r\n");
    
    for(;;)
    {
        if(xQueueReceive(xDataQueue, &rxPacket, portMAX_DELAY) == pdPASS)
        {
            latency = xTaskGetTickCount() - rxPacket.timestamp;
            
            snprintf(buffer, sizeof(buffer),
                    "[CONS] Processing data #%lu (latency: %lums)\r\n",
                    rxPacket.dataId, latency);
            UART_SendString(buffer);
            
            ulDataConsumed++;
            
            /* Toggle LED */
            HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
            
            /* Simulasi proses lambat */
            vTaskDelay(pdMS_TO_TICKS(CONSUMER_PROCESS_MS));
        }
    }
}

/**
 * @brief Monitor Task - Print statistik periodik
 */
static void vMonitorTask(void *pvParameters)
{
    (void)pvParameters;
    
    char buffer[120];
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for(;;)
    {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(5000));
        
        UBaseType_t spaces = uxQueueSpacesAvailable(xDataQueue);
        UBaseType_t waiting = uxQueueMessagesWaiting(xDataQueue);
        
        UART_SendString("\r\n========== STATISTICS ==========\r\n");
        
        snprintf(buffer, sizeof(buffer),
                "Produced: %lu, Consumed: %lu, Dropped: %lu\r\n",
                ulDataProduced, ulDataConsumed, ulDataDropped);
        UART_SendString(buffer);
        
        snprintf(buffer, sizeof(buffer),
                "Queue: %u waiting, %u spaces available\r\n",
                waiting, spaces);
        UART_SendString(buffer);
        
        UART_SendString("=================================\r\n\r\n");
    }
}

/* ============================================================================
 * KONFIGURASI HARDWARE
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
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = LED_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);
    
    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_SET);
}

static void UART_Init(void)
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

static void UART_SendString(const char *str)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
}

/* ============================================================================
 * HOOKS FreeRTOS
 * ============================================================================ */

void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();
    for(;;);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    taskDISABLE_INTERRUPTS();
    for(;;);
}

/* ============================================================================
 * MAIN
 * ============================================================================ */

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    UART_Init();
    
    UART_SendString("\r\n================================\r\n");
    UART_SendString("08-Flow_Control_Backpressure\r\n");
    UART_SendString("STM32F103 + FreeRTOS\r\n");
    UART_SendString("================================\r\n");
    UART_SendString("\r\n=== Flow Control Backpressure Demo ===\r\n\r\n");
    
    xDataQueue = xQueueCreate(QUEUE_DEPTH, sizeof(DataPacket_t));
    
    if(xDataQueue == NULL)
    {
        UART_SendString("ERROR: Failed to create queue!\r\n");
        while(1);
    }
    
    char buffer[50];
    snprintf(buffer, sizeof(buffer), "Queue created (depth=%d)\r\n\r\n", QUEUE_DEPTH);
    UART_SendString(buffer);
    
    xTaskCreate(vProducerTask, "Producer", PRODUCER_STACK, NULL,
                PRODUCER_PRIO, NULL);
    
    xTaskCreate(vConsumerTask, "Consumer", CONSUMER_STACK, NULL,
                CONSUMER_PRIO, NULL);
    
    xTaskCreate(vMonitorTask, "Monitor", 256, NULL,
                tskIDLE_PRIORITY + 1, NULL);
    
    vTaskStartScheduler();
    
    for(;;);
}
