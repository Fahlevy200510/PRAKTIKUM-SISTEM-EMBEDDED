/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 09-Queue_Peek_Operations
 * 
 * JUDUL: Queue Peek - Membaca Tanpa Menghapus Data
 * 
 * DESKRIPSI:
 * Demo penggunaan xQueuePeek() untuk membaca data tanpa menghapus.
 * Pattern: Producer → Queue → Validator (peek) → Processor (receive)
 * 
 * ============================================================================
 * ARSITEKTUR PROGRAM
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                       PEEK OPERATIONS DEMO                         │
 *    ├─────────────────────────────────────────────────────────────────────┤
 *    │                                                                     │
 *    │   ┌──────────────────┐                                              │
 *    │   │  vProducerTask   │                                              │
 *    │   │                  │                                              │
 *    │   │  Buat data       │                                              │
 *    │   │  (some invalid)  │                                              │
 *    │   └────────┬─────────┘                                              │
 *    │            │ xQueueSend()                                           │
 *    │            ▼                                                        │
 *    │   ┌───────────────────────────────────────┐                         │
 *    │   │              QUEUE                    │                         │
 *    │   │   ┌─────┬─────┬─────┬─────┬─────┐    │                         │
 *    │   │   │ D1  │ D2  │ D3  │     │     │    │                         │
 *    │   │   │valid│ INV │valid│     │     │    │                         │
 *    │   │   └─────┴─────┴─────┴─────┴─────┘    │                         │
 *    │   └───────────────┬───────────────────────┘                         │
 *    │                   │                                                 │
 *    │         ┌─────────┴─────────┐                                       │
 *    │         │                   │                                       │
 *    │         ▼                   ▼                                       │
 *    │   ┌───────────────┐   ┌───────────────┐                            │
 *    │   │ vValidator    │   │ vProcessor    │                            │
 *    │   │ (PEEK only)   │   │ (RECEIVE)     │                            │
 *    │   │               │   │               │                            │
 *    │   │ Preview data  │   │ Ambil data    │                            │
 *    │   │ Cek validitas │   │ Proses        │                            │
 *    │   │ Log status    │   │ Hapus dr queue│                            │
 *    │   └───────────────┘   └───────────────┘                            │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * ALUR PEEK vs RECEIVE
 * ============================================================================
 * 
 *    SKENARIO:
 *    ─────────
 *    Queue: [Data1][Data2][Data3]
 *    
 *    t=0: Validator PEEK Data1
 *         Queue tetap: [Data1][Data2][Data3]
 *         Validator: "Data1 valid!"
 *    
 *    t=1: Validator PEEK lagi Data1 (masih sama!)
 *         Queue tetap: [Data1][Data2][Data3]
 *         Validator: "Data1 valid!"
 *    
 *    t=2: Processor RECEIVE Data1
 *         Queue jadi: [Data2][Data3]
 *         Processor: "Processing Data1..."
 *    
 *    t=3: Validator PEEK Data2 (sekarang di head)
 *         Queue tetap: [Data2][Data3]
 *         Validator: "Data2 INVALID!"
 *    
 *    ═══════════════════════════════════════════════════════════════════════
 * 
 *    ILUSTRASI VISUAL:
 *    
 *    xQueuePeek():
 *    ┌─────────────────────────────────────────┐
 *    │ Queue: [██][░░][░░]                     │
 *    │              ↑                          │
 *    │         Copy ke buffer                  │
 *    │         Data TETAP di queue             │
 *    └─────────────────────────────────────────┘
 *    
 *    xQueueReceive():
 *    ┌─────────────────────────────────────────┐
 *    │ Before: [██][░░][░░]                    │
 *    │               ↑                         │
 *    │          Copy ke buffer                 │
 *    │          Data DIHAPUS dari queue        │
 *    │ After:  [░░][░░][  ]                    │
 *    └─────────────────────────────────────────┘
 * 
 * ============================================================================
 * EXPECTED OUTPUT
 * ============================================================================
 * 
 *    === Queue Peek Operations Demo ===
 *    
 *    [PROD] Sent data #1 (type=VALID)
 *    [VAL] PEEK: Data #1 is VALID
 *    [PROC] RECEIVE: Processing data #1
 *    
 *    [PROD] Sent data #2 (type=INVALID)
 *    [VAL] PEEK: Data #2 is INVALID - flagging
 *    [PROC] RECEIVE: Skipping invalid data #2
 *    
 *    [PROD] Sent data #3 (type=VALID)
 *    [VAL] PEEK: Data #3 is VALID
 *    [PROC] RECEIVE: Processing data #3
 * 
 * ============================================================================
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * DEFINISI
 * ============================================================================ */

typedef enum {
    DATA_VALID,
    DATA_INVALID,
    DATA_CHECKSUM_ERROR,
    DATA_CORRUPTED
} DataStatus_t;

typedef struct {
    uint32_t     dataId;
    uint16_t     value;
    DataStatus_t status;       /* Status validitas */
    uint8_t      validated;    /* Flag sudah di-validate */
    TickType_t   timestamp;
} DataPacket_t;

/* ============================================================================
 * HANDLE GLOBAL
 * ============================================================================ */

static QueueHandle_t xDataQueue = NULL;
static SemaphoreHandle_t xValidationMutex = NULL;

static UART_HandleTypeDef huart1;

/* Statistik */
static volatile uint32_t ulValidCount = 0;
static volatile uint32_t ulInvalidCount = 0;

/* ============================================================================
 * PROTOTYPE
 * ============================================================================ */

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART_Init(void);
static void UART_SendString(const char *str);

static void vProducerTask(void *pvParameters);
static void vValidatorTask(void *pvParameters);
static void vProcessorTask(void *pvParameters);

static const char* GetStatusName(DataStatus_t status);

/* ============================================================================
 * HELPER
 * ============================================================================ */

static const char* GetStatusName(DataStatus_t status)
{
    switch(status)
    {
        case DATA_VALID:          return "VALID";
        case DATA_INVALID:        return "INVALID";
        case DATA_CHECKSUM_ERROR: return "CHECKSUM_ERROR";
        case DATA_CORRUPTED:      return "CORRUPTED";
        default:                  return "UNKNOWN";
    }
}

/* ============================================================================
 * IMPLEMENTASI TASK
 * ============================================================================ */

/**
 * @brief Producer Task - Membuat data dengan status bervariasi
 * 
 * Beberapa data sengaja dibuat invalid untuk demonstrasi
 * bagaimana Validator bisa peek dan menandai sebelum Processor mengambil.
 */
static void vProducerTask(void *pvParameters)
{
    (void)pvParameters;
    
    DataPacket_t packet;
    uint32_t dataCounter = 0;
    char buffer[80];
    
    UART_SendString("[PROD] Producer started\r\n\r\n");
    
    for(;;)
    {
        dataCounter++;
        
        packet.dataId = dataCounter;
        packet.value = (uint16_t)(dataCounter * 100);
        packet.timestamp = xTaskGetTickCount();
        packet.validated = 0;  /* Belum di-validate */
        
        /* Buat beberapa data invalid untuk demo */
        if(dataCounter % 4 == 0)
        {
            packet.status = DATA_INVALID;
        }
        else if(dataCounter % 7 == 0)
        {
            packet.status = DATA_CHECKSUM_ERROR;
        }
        else
        {
            packet.status = DATA_VALID;
        }
        
        snprintf(buffer, sizeof(buffer),
                "[PROD] Sent data #%lu (type=%s)\r\n",
                dataCounter, GetStatusName(packet.status));
        UART_SendString(buffer);
        
        if(xQueueSend(xDataQueue, &packet, pdMS_TO_TICKS(100)) != pdPASS)
        {
            UART_SendString("[PROD] Queue full!\r\n");
        }
        
        vTaskDelay(pdMS_TO_TICKS(800));
    }
}

/**
 * @brief Validator Task - PEEK untuk melihat data tanpa mengambil
 * 
 * ILUSTRASI PEEK:
 * ═══════════════════════════════════════════════════════════════════════════
 *    
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                                                                     │
 *    │   xQueuePeek(xQueue, &peekedData, timeout)                         │
 *    │                                                                     │
 *    │   Queue: [Data_A][Data_B][Data_C]                                  │
 *    │              │                                                      │
 *    │              ▼ COPY (bukan move!)                                  │
 *    │         peekedData = Data_A                                        │
 *    │                                                                     │
 *    │   Queue TETAP: [Data_A][Data_B][Data_C]                            │
 *    │                                                                     │
 *    │   PENTING:                                                          │
 *    │   - Data tidak dihapus                                             │
 *    │   - Task lain tetap bisa peek/receive                              │
 *    │   - Berguna untuk preview/validation                               │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 *    
 * ═══════════════════════════════════════════════════════════════════════════
 */
static void vValidatorTask(void *pvParameters)
{
    (void)pvParameters;
    
    DataPacket_t peekedData;
    char buffer[100];
    
    UART_SendString("[VAL] Validator started (uses PEEK)\r\n");
    
    for(;;)
    {
        /*
         * PEEK - lihat data tanpa mengambil!
         * 
         * Berbeda dengan xQueueReceive():
         * - Data tetap ada di queue
         * - Task lain masih bisa mengaksesnya
         * - Cocok untuk validasi sebelum proses
         */
        if(xQueuePeek(xDataQueue, &peekedData, pdMS_TO_TICKS(500)) == pdPASS)
        {
            /* Cek apakah data sudah pernah di-validate */
            if(!peekedData.validated)
            {
                /* Validasi data */
                if(peekedData.status == DATA_VALID)
                {
                    ulValidCount++;
                    snprintf(buffer, sizeof(buffer),
                            "[VAL] PEEK: Data #%lu is VALID ✓\r\n",
                            peekedData.dataId);
                }
                else
                {
                    ulInvalidCount++;
                    snprintf(buffer, sizeof(buffer),
                            "[VAL] PEEK: Data #%lu is %s ✗\r\n",
                            peekedData.dataId, GetStatusName(peekedData.status));
                }
                UART_SendString(buffer);
            }
            
            /* Kecil delay untuk tidak monopoli CPU */
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

/**
 * @brief Processor Task - RECEIVE untuk mengambil dan memproses data
 * 
 * Setelah Validator peek dan menandai, Processor akan mengambil
 * data dengan xQueueReceive() dan memprosesnya.
 */
static void vProcessorTask(void *pvParameters)
{
    (void)pvParameters;
    
    DataPacket_t rxData;
    char buffer[100];
    
    UART_SendString("[PROC] Processor started (uses RECEIVE)\r\n\r\n");
    
    for(;;)
    {
        /* 
         * RECEIVE - ambil dan hapus dari queue
         * Ini berbeda dari peek - data akan hilang dari queue
         */
        if(xQueueReceive(xDataQueue, &rxData, portMAX_DELAY) == pdPASS)
        {
            HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
            
            if(rxData.status == DATA_VALID)
            {
                snprintf(buffer, sizeof(buffer),
                        "[PROC] RECEIVE: Processing data #%lu (value=%u)\r\n",
                        rxData.dataId, rxData.value);
                UART_SendString(buffer);
                
                /* Simulasi proses */
                vTaskDelay(pdMS_TO_TICKS(200));
                
                UART_SendString("[PROC] Processing complete\r\n\r\n");
            }
            else
            {
                snprintf(buffer, sizeof(buffer),
                        "[PROC] RECEIVE: Skipping invalid data #%lu\r\n\r\n",
                        rxData.dataId);
                UART_SendString(buffer);
            }
        }
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
    UART_SendString("09-Queue_Peek_Operations\r\n");
    UART_SendString("STM32F103 + FreeRTOS\r\n");
    UART_SendString("================================\r\n");
    UART_SendString("\r\n=== Queue Peek Operations Demo ===\r\n\r\n");
    
    xDataQueue = xQueueCreate(QUEUE_LENGTH, sizeof(DataPacket_t));
    xValidationMutex = xSemaphoreCreateMutex();
    
    if(xDataQueue == NULL || xValidationMutex == NULL)
    {
        UART_SendString("ERROR: Failed to create queue/mutex!\r\n");
        while(1);
    }
    
    /* Validator punya prioritas tertinggi untuk peek duluan */
    xTaskCreate(vValidatorTask, "Validator", VALIDATOR_STACK, NULL,
                VALIDATOR_PRIO, NULL);
    
    xTaskCreate(vProcessorTask, "Processor", PROCESSOR_STACK, NULL,
                PROCESSOR_PRIO, NULL);
    
    xTaskCreate(vProducerTask, "Producer", PRODUCER_STACK, NULL,
                PRODUCER_PRIO, NULL);
    
    vTaskStartScheduler();
    
    for(;;);
}
