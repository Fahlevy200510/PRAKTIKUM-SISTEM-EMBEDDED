/**
 * ============================================================================
 * PROJECT: 14-Counting_Semaphore_Events
 * BOARD: STM32F103C8T6 (Blue Pill)
 * FRAMEWORK: STM32Cube HAL + FreeRTOS Native API
 * ============================================================================
 * 
 * DEMO COUNTING SEMAPHORE:
 * ========================
 * 
 * Counting semaphore menghitung "event" yang belum diproses.
 * 
 * SKENARIO DEMO:
 * ==============
 * - Producer: Generate event CEPAT (100ms)
 * - Consumer: Proses event LAMBAT (300ms)
 * - Monitor: Lapor status semaphore count
 * 
 * YANG TERJADI:
 * =============
 * 
 *    Waktu   │ Producer          │ Consumer        │ Semaphore Count
 *    ────────┼───────────────────┼─────────────────┼─────────────────
 *    0ms     │ Give() event #1   │                 │ 1
 *    100ms   │ Give() event #2   │ Take() process  │ 1
 *    200ms   │ Give() event #3   │ (processing...) │ 2
 *    300ms   │ Give() event #4   │                 │ 3
 *    400ms   │ Give() event #5   │ Take() process  │ 3
 *    ...     │                   │                 │ (naik terus!)
 * 
 * TANPA COUNTING SEMAPHORE (binary):
 * - Event #2, #3, #4 HILANG saat consumer masih proses event #1
 * 
 * DENGAN COUNTING SEMAPHORE:
 * - Semua event TERCATAT
 * - Consumer akan proses semuanya, satu per satu
 * 
 * CATATAN:
 * ========
 * - Jika producer lebih cepat dari consumer, count naik terus
 * - Jika count mencapai MAX, xSemaphoreGive gagal (event hilang)
 * - Sesuaikan MAX_COUNT dengan kebutuhan sistem
 * 
 * ============================================================================
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdio.h>
#include <string.h>

#include "FreeRTOSConfig.h"

/* ===========================================================================
 * VARIABEL GLOBAL
 * =========================================================================== */
UART_HandleTypeDef huart1;
SemaphoreHandle_t eventSemaphore;       /* Counting semaphore */
volatile uint32_t eventsGenerated = 0;  /* Counter event dibuat */
volatile uint32_t eventsProcessed = 0;  /* Counter event diproses */
volatile uint32_t eventsLost = 0;       /* Counter event hilang */

/* ===========================================================================
 * PROTOTIPE FUNGSI
 * =========================================================================== */
static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);

static void EventProducerTask(void *argument);
static void EventConsumerTask(void *argument);
static void MonitorTask(void *argument);

static void printLine(const char *msg);
static void toggleLed(void);
void Error_Handler(void);

/* ===========================================================================
 * FUNGSI UTAMA
 * =========================================================================== */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    
    /*
     * BUAT COUNTING SEMAPHORE
     * 
     * xSemaphoreCreateCounting(maxCount, initialCount)
     * 
     * - maxCount: Nilai maksimal semaphore
     * - initialCount: Nilai awal (0 = tidak ada event pending)
     * 
     * Berbeda dengan:
     * - xSemaphoreCreateBinary(): max=1, initial=0
     * - xSemaphoreCreateMutex(): max=1, initial=1 (+ priority inheritance)
     */
    eventSemaphore = xSemaphoreCreateCounting(EVENT_MAX_COUNT, EVENT_INITIAL_COUNT);
    if (eventSemaphore == NULL)
    {
        Error_Handler();
    }
    
    /* Buat task-task */
    xTaskCreate(EventProducerTask, "prod", PRODUCER_STACK, NULL, PRODUCER_PRIO, NULL);
    xTaskCreate(EventConsumerTask, "cons", CONSUMER_STACK, NULL, CONSUMER_PRIO, NULL);
    xTaskCreate(MonitorTask,       "mon",  MONITOR_STACK,  NULL, MONITOR_PRIO,  NULL);
    
    vTaskStartScheduler();
    
    while (1) {}
}

/* ===========================================================================
 * EVENT PRODUCER TASK
 * ===========================================================================
 * 
 * Generate event lebih CEPAT dari consumer.
 * Simulasi sensor/interrupt yang menghasilkan data.
 * 
 * Setiap Give():
 * - Jika count < MAX: count++ dan berhasil
 * - Jika count = MAX: GAGAL, event hilang!
 */
static void EventProducerTask(void *argument)
{
    TickType_t lastWake = xTaskGetTickCount();
    char buf[80];
    
    printLine("\r\n=== COUNTING SEMAPHORE DEMO ===\r\n");
    printLine("Producer CEPAT (100ms), Consumer LAMBAT (300ms)\r\n\r\n");
    
    for (;;)
    {
        eventsGenerated++;
        
        /*
         * GIVE SEMAPHORE (menambah count)
         * 
         * Return pdTRUE jika berhasil.
         * Return pdFALSE jika count sudah mencapai MAX.
         */
        if (xSemaphoreGive(eventSemaphore) == pdTRUE)
        {
            snprintf(buf, sizeof(buf), 
                     "[PROD] Event #%lu generated | pending: %lu\r\n",
                     (unsigned long)eventsGenerated,
                     (unsigned long)uxSemaphoreGetCount(eventSemaphore));
            printLine(buf);
        }
        else
        {
            /* Semaphore sudah penuh (count = MAX) */
            eventsLost++;
            snprintf(buf, sizeof(buf), 
                     "[PROD] Event #%lu LOST! (semaphore full) | lost total: %lu\r\n",
                     (unsigned long)eventsGenerated,
                     (unsigned long)eventsLost);
            printLine(buf);
        }
        
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(PRODUCER_PERIOD_MS));
    }
}

/* ===========================================================================
 * EVENT CONSUMER TASK
 * ===========================================================================
 * 
 * Memproses event lebih LAMBAT dari producer.
 * 
 * Setiap Take():
 * - Jika count > 0: count-- dan proses
 * - Jika count = 0: Block sampai ada event baru
 */
static void EventConsumerTask(void *argument)
{
    char buf[80];
    
    vTaskDelay(pdMS_TO_TICKS(50)); /* Tunggu producer mulai */
    
    for (;;)
    {
        /*
         * TAKE SEMAPHORE (mengurangi count)
         * 
         * Akan block jika count = 0.
         * portMAX_DELAY = tunggu selamanya.
         */
        if (xSemaphoreTake(eventSemaphore, portMAX_DELAY) == pdTRUE)
        {
            eventsProcessed++;
            
            snprintf(buf, sizeof(buf), 
                     "    [CONS] Processing event #%lu | remaining: %lu\r\n",
                     (unsigned long)eventsProcessed,
                     (unsigned long)uxSemaphoreGetCount(eventSemaphore));
            printLine(buf);
            
            toggleLed();
            
            /*
             * SIMULASI PROSES LAMA
             * 
             * Selama delay ini, producer terus generate event.
             * Counting semaphore akan menampung event-event tersebut.
             */
            vTaskDelay(pdMS_TO_TICKS(CONSUMER_PROCESS_MS));
        }
    }
}

/* ===========================================================================
 * MONITOR TASK
 * ===========================================================================
 * 
 * Melaporkan status counting semaphore secara periodik.
 */
static void MonitorTask(void *argument)
{
    TickType_t lastWake = xTaskGetTickCount();
    char buf[100];
    
    vTaskDelay(pdMS_TO_TICKS(200)); /* Tunggu sistem mulai */
    
    for (;;)
    {
        UBaseType_t pending = uxSemaphoreGetCount(eventSemaphore);
        
        snprintf(buf, sizeof(buf), 
                 "\r\n>>> STATUS: gen=%lu proc=%lu lost=%lu pending=%lu <<<\r\n\r\n",
                 (unsigned long)eventsGenerated,
                 (unsigned long)eventsProcessed,
                 (unsigned long)eventsLost,
                 (unsigned long)pending);
        printLine(buf);
        
        /* Peringatan jika semaphore hampir penuh */
        if (pending > (EVENT_MAX_COUNT * 8 / 10))
        {
            printLine(">>> WARNING: Semaphore hampir penuh! <<<\r\n");
        }
        
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(MONITOR_PERIOD_MS));
    }
}

/* ===========================================================================
 * HELPER FUNCTIONS
 * =========================================================================== */

static void printLine(const char *msg)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), 100);
}

static void toggleLed(void)
{
    HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
}

/* ===========================================================================
 * KONFIGURASI SISTEM
 * =========================================================================== */

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
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

static void MX_USART1_UART_Init(void)
{
    huart1.Instance = DEBUG_UART_INSTANCE;
    huart1.Init.BaudRate = DEBUG_UART_BAUDRATE;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }
}

static void MX_GPIO_Init(void)
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

    GPIO_InitStruct.Pin = DEBUG_UART_TX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DEBUG_UART_TX_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = DEBUG_UART_RX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DEBUG_UART_RX_PORT, &GPIO_InitStruct);
}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
    if (uartHandle->Instance == USART1)
    {
        __HAL_RCC_USART1_CLK_ENABLE();
    }
}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
        HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
        HAL_Delay(100);
    }
}

void vApplicationMallocFailedHook(void)
{
    Error_Handler();
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    Error_Handler();
}
