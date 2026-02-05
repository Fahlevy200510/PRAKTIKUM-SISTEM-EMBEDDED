/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 24-Deferred_ISR_Processing
 * 
 * JUDUL: Deferred Interrupt Processing Demo
 * 
 * DESKRIPSI:
 * Demo pattern deferred interrupt processing menggunakan:
 * 1. Queue untuk transfer data dari "ISR" ke handler task
 * 2. Semaphore untuk signaling
 * 3. xTimerPendFunctionCall() untuk deferred function call
 * 
 * Karena tidak ada hardware interrupt yang dikonfigurasi,
 * kita simulasikan dengan timer callback.
 * 
 * ============================================================================
 * ARSITEKTUR PROGRAM
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                 DEFERRED ISR PROCESSING DEMO                       │
 *    ├─────────────────────────────────────────────────────────────────────┤
 *    │                                                                     │
 *    │   ┌─────────────────────────────────────────────────────────────┐   │
 *    │   │              vISRSimulatorTimer (Callback)                  │   │
 *    │   │                                                             │   │
 *    │   │   // Simulasi ISR - harus CEPAT!                            │   │
 *    │   │   - Capture timestamp                                       │   │
 *    │   │   - Create event struct                                     │   │
 *    │   │   - Send to queue (non-blocking)                            │   │
 *    │   │   - Exit (~10μs)                                            │   │
 *    │   │                                                             │   │
 *    │   └────────────────────────────────┬────────────────────────────┘   │
 *    │                                    │                                │
 *    │                                    │ xQueueSend                     │
 *    │                                    ▼                                │
 *    │                           ┌────────────────┐                        │
 *    │                           │  EVENT QUEUE   │                        │
 *    │                           │  Depth: 10     │                        │
 *    │                           └───────┬────────┘                        │
 *    │                                   │                                 │
 *    │                                   │ xQueueReceive                   │
 *    │                                   ▼                                 │
 *    │   ┌─────────────────────────────────────────────────────────────┐   │
 *    │   │              vDeferredHandlerTask                           │   │
 *    │   │              Priority: HIGH                                 │   │
 *    │   │                                                             │   │
 *    │   │   while(1) {                                                │   │
 *    │   │       xQueueReceive(queue, &event, MAX_DELAY);              │   │
 *    │   │                                                             │   │
 *    │   │       // Heavy processing OK di sini                        │   │
 *    │   │       ProcessEvent(&event);                                 │   │
 *    │   │       ValidateData();                                       │   │
 *    │   │       UpdateStatistics();                                   │   │
 *    │   │       LogToUART();                                          │   │
 *    │   │   }                                                         │   │
 *    │   │                                                             │   │
 *    │   └─────────────────────────────────────────────────────────────┘   │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * PERBANDINGAN LATENCY
 * ============================================================================
 * 
 *    LANGSUNG DI ISR:
 *    ══════════════════════════════════════════════════════════════════════
 *    
 *    ISR Start ───────────────────────────────────────────► ISR End
 *              │                                          │
 *              │◄──────────── 10ms processing ───────────►│
 *              │                                          │
 *              Interrupt lain DIBLOK selama 10ms!
 *    
 *    
 *    DEFERRED PROCESSING:
 *    ══════════════════════════════════════════════════════════════════════
 *    
 *    ISR Start ──► ISR End    Handler Start ─────────────► Handler End
 *              │           │               │              │
 *              │◄── 10μs ──►│              │◄── 10ms ────►│
 *              │           │               │              │
 *              │           │               │              │
 *              Fast!       Context switch  Processing     Complete
 *                          to handler task (scheduler OK)
 * 
 * ============================================================================
 * EXPECTED OUTPUT
 * ============================================================================
 * 
 *    === Deferred ISR Processing Demo ===
 *    
 *    [0.500s] ISR: Event #1 captured (ADC=1234), queued in 5us
 *    [0.500s] HANDLER: Processing event #1...
 *    [0.520s] HANDLER: Event #1 complete (20ms processing)
 *    
 *    [1.000s] ISR: Event #2 captured (ADC=2345), queued in 4us
 *    [1.000s] HANDLER: Processing event #2...
 * 
 * ============================================================================
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * EVENT STRUCTURE
 * ============================================================================ */

typedef struct {
    uint32_t eventNumber;
    uint32_t adcValue;
    TickType_t timestamp;
    uint32_t isrDurationUs;  /* Waktu di "ISR" */
} Event_t;

/* ============================================================================
 * GLOBAL VARIABLES
 * ============================================================================ */

static TimerHandle_t xISRSimTimer = NULL;
static QueueHandle_t xEventQueue = NULL;
static SemaphoreHandle_t xProcessingSem = NULL;

static UART_HandleTypeDef huart1;
static ADC_HandleTypeDef hadc1;

static volatile uint32_t ulEventCount = 0;

/* Untuk mengukur waktu */
static volatile uint32_t ulDWT_Start;

/* ============================================================================
 * PROTOTYPE
 * ============================================================================ */

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART_Init(void);
static void ADC_Init(void);
static void DWT_Init(void);
static void UART_SendString(const char *str);
static uint16_t ADC_Read(void);

static void vISRSimulatorCallback(TimerHandle_t xTimer);
static void vDeferredHandlerTask(void *pvParameters);

/* ============================================================================
 * DWT CYCLE COUNTER (Untuk mengukur microseconds)
 * ============================================================================ */

/**
 * @brief Initialize DWT (Data Watchpoint and Trace) untuk cycle counting
 * 
 * DWT_CYCCNT adalah 32-bit counter yang increment setiap CPU cycle.
 * Pada 72MHz: 1 cycle = 13.89ns
 */
static void DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static inline uint32_t DWT_GetCycles(void)
{
    return DWT->CYCCNT;
}

static inline uint32_t CyclesToMicroseconds(uint32_t cycles)
{
    /* 72 cycles = 1 microsecond at 72MHz */
    return cycles / 72;
}

/* ============================================================================
 * ISR SIMULATOR (Timer Callback)
 * ============================================================================ */

/**
 * @brief Simulasi ISR - harus CEPAT!
 * 
 * DALAM IMPLEMENTASI NYATA:
 * ═══════════════════════════════════════════════════════════════════════════
 * 
 *    Ini akan menjadi actual interrupt handler, contoh:
 *    
 *    void USART1_IRQHandler(void)
 *    {
 *        if(USART1->SR & USART_SR_RXNE)
 *        {
 *            uint8_t data = USART1->DR;
 *            
 *            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
 *            
 *            // Kirim ke queue - CEPAT!
 *            xQueueSendFromISR(xEventQueue, &data, &xHigherPriorityTaskWoken);
 *            
 *            // Yield jika task dengan prioritas lebih tinggi ready
 *            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
 *        }
 *    }
 *    
 *    Karena ini demo tanpa hardware interrupt, kita pakai timer callback.
 *    Prinsipnya SAMA: minimize waktu, kirim ke queue!
 * 
 * ═══════════════════════════════════════════════════════════════════════════
 */
static void vISRSimulatorCallback(TimerHandle_t xTimer)
{
    (void)xTimer;
    
    /* Start timing */
    uint32_t startCycles = DWT_GetCycles();
    
    ulEventCount++;
    
    /* Baca ADC (simulasi capture data di ISR) */
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 1);
    uint16_t adcValue = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    
    /* Buat event struct */
    Event_t event = {
        .eventNumber = ulEventCount,
        .adcValue = adcValue,
        .timestamp = xTaskGetTickCount(),
        .isrDurationUs = 0  /* Akan diisi nanti */
    };
    
    /* Kirim ke queue - NON-BLOCKING! */
    BaseType_t result = xQueueSend(xEventQueue, &event, 0);
    
    /* End timing */
    uint32_t endCycles = DWT_GetCycles();
    uint32_t durationUs = CyclesToMicroseconds(endCycles - startCycles);
    
    /* Print dari ISR context (OK untuk demo, hindari di real ISR) */
    char buffer[80];
    float seconds = (float)xTaskGetTickCount() / 1000.0f;
    
    if(result == pdPASS)
    {
        snprintf(buffer, sizeof(buffer),
                 "[%.3fs] ISR: Event #%lu captured (ADC=%u), queued in %luus\r\n",
                 seconds, ulEventCount, adcValue, durationUs);
    }
    else
    {
        snprintf(buffer, sizeof(buffer),
                 "[%.3fs] ISR: Event #%lu - QUEUE FULL! Data lost!\r\n",
                 seconds, ulEventCount);
    }
    UART_SendString(buffer);
    
    /* Toggle LED untuk indikasi ISR */
    HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
}

/* ============================================================================
 * DEFERRED HANDLER TASK
 * ============================================================================ */

/**
 * @brief Task yang melakukan processing berat
 * 
 * KEUNTUNGAN:
 * - Processing time tidak block interrupt lain
 * - Scheduler tetap berjalan
 * - Bisa melakukan operasi blocking (UART, Flash, dll)
 */
static void vDeferredHandlerTask(void *pvParameters)
{
    (void)pvParameters;
    
    Event_t event;
    char buffer[100];
    
    UART_SendString("[HANDLER] Deferred handler task started...\r\n\r\n");
    
    for(;;)
    {
        /* Block sampai ada event di queue */
        if(xQueueReceive(xEventQueue, &event, portMAX_DELAY) == pdPASS)
        {
            float seconds = (float)xTaskGetTickCount() / 1000.0f;
            TickType_t startTick = xTaskGetTickCount();
            
            snprintf(buffer, sizeof(buffer),
                     "[%.3fs] HANDLER: Processing event #%lu...\r\n",
                     seconds, event.eventNumber);
            UART_SendString(buffer);
            
            /*
             * Simulasi HEAVY PROCESSING
             * Di real app, ini bisa:
             * - Parse protocol
             * - Validate CRC
             * - Store to flash
             * - Send response
             */
            vTaskDelay(pdMS_TO_TICKS(20));  /* Simulasi 20ms processing */
            
            /* Setelah processing selesai */
            TickType_t endTick = xTaskGetTickCount();
            uint32_t processingTime = endTick - startTick;
            
            snprintf(buffer, sizeof(buffer),
                     "[%.3fs] HANDLER: Event #%lu complete (%lums processing)\r\n\r\n",
                     (float)endTick / 1000.0f,
                     event.eventNumber,
                     processingTime);
            UART_SendString(buffer);
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
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
    
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
    
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
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

static void ADC_Init(void)
{
    __HAL_RCC_ADC1_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    HAL_ADC_Init(&hadc1);
    
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_7CYCLES_5;  /* Fast untuk simulasi ISR */
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
}

static void UART_SendString(const char *str)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
}

static uint16_t ADC_Read(void)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    uint16_t value = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    return value;
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
    ADC_Init();
    DWT_Init();
    
    UART_SendString("\r\n================================\r\n");
    UART_SendString("24-Deferred_ISR_Processing\r\n");
    UART_SendString("STM32F103 + FreeRTOS\r\n");
    UART_SendString("================================\r\n\r\n");
    UART_SendString("=== Deferred ISR Processing Demo ===\r\n\r\n");
    
    /* Buat event queue */
    xEventQueue = xQueueCreate(EVENT_QUEUE_LENGTH, sizeof(Event_t));
    
    if(xEventQueue == NULL)
    {
        UART_SendString("ERROR: Failed to create queue!\r\n");
        while(1);
    }
    
    /* Buat timer untuk simulasi ISR */
    xISRSimTimer = xTimerCreate(
        "ISRSim",
        pdMS_TO_TICKS(SIMULATE_ISR_PERIOD_MS),
        pdTRUE,   /* Auto-reload */
        NULL,
        vISRSimulatorCallback
    );
    
    if(xISRSimTimer == NULL)
    {
        UART_SendString("ERROR: Failed to create timer!\r\n");
        while(1);
    }
    
    /* Buat deferred handler task dengan prioritas TINGGI */
    xTaskCreate(vDeferredHandlerTask, "Handler", TASK_STACK, NULL,
                configMAX_PRIORITIES - 2, NULL);  /* High priority */
    
    /* Start ISR simulator timer */
    xTimerStart(xISRSimTimer, 0);
    
    UART_SendString("System ready. Simulated ISR every 500ms...\r\n\r\n");
    
    /* Start scheduler */
    vTaskStartScheduler();
    
    for(;;);
}
