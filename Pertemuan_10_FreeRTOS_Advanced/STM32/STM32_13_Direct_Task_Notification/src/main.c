/**
 * ============================================================================
 * PROJECT: 16-Direct_Task_Notification
 * BOARD: STM32F103C8T6 (Blue Pill)
 * FRAMEWORK: STM32Cube HAL + FreeRTOS Native API
 * ============================================================================
 * 
 * DEMO TASK NOTIFICATION:
 * =======================
 * 
 * Task notification adalah cara paling EFISIEN untuk sinkronisasi task.
 * Lebih cepat 45% dari binary semaphore dan tidak butuh RAM tambahan.
 * 
 * API YANG DIGUNAKAN:
 * ===================
 * 
 *    xTaskNotifyGive(taskHandle)
 *    ───────────────────────────
 *    - Mengirim notification ke task tertentu
 *    - Menaikkan notification value (seperti Give pada counting semaphore)
 *    - Membangunkan task jika sedang menunggu
 * 
 *    ulTaskNotifyTake(xClearOnExit, timeout)
 *    ───────────────────────────────────────
 *    - Menunggu notification
 *    - xClearOnExit = pdTRUE: Reset value ke 0 setelah return
 *    - xClearOnExit = pdFALSE: Decrement value (seperti counting semaphore)
 * 
 * ILUSTRASI ALUR:
 * ===============
 * 
 *    Producer Task                     Consumer Task
 *    ─────────────                     ─────────────
 *         │                                 │
 *         │                      ulTaskNotifyTake() ◄─┐
 *         │                            BLOCK         │
 *         │                              │           │
 *    xTaskNotifyGive() ─────────────►  WAKE UP!     │
 *         │                              │           │
 *         │                         Process data    │
 *         │                              │           │
 *         │                              └───────────┘
 *         ▼
 * 
 * ============================================================================
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>

#include "FreeRTOSConfig.h"

/* ===========================================================================
 * VARIABEL GLOBAL
 * =========================================================================== */
UART_HandleTypeDef huart1;
TaskHandle_t consumerTaskHandle = NULL;     /* Handle untuk target notification */
volatile uint32_t notificationsSent = 0;
volatile uint32_t notificationsReceived = 0;

/* ===========================================================================
 * PROTOTIPE FUNGSI
 * =========================================================================== */
static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);

static void ProducerTask(void *argument);
static void ConsumerTask(void *argument);

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
     * TIDAK PERLU membuat semaphore!
     * Task notification sudah bawaan setiap task.
     * 
     * Kita hanya perlu menyimpan handle task yang akan di-notify.
     */
    
    /* Buat Consumer dulu, simpan handle-nya */
    xTaskCreate(ConsumerTask, "cons", CONSUMER_STACK, NULL, CONSUMER_PRIO, &consumerTaskHandle);
    
    /* Buat Producer */
    xTaskCreate(ProducerTask, "prod", PRODUCER_STACK, NULL, PRODUCER_PRIO, NULL);
    
    vTaskStartScheduler();
    
    while (1) {}
}

/* ===========================================================================
 * PRODUCER TASK
 * ===========================================================================
 * 
 * Mengirim notification ke Consumer secara periodik.
 */
static void ProducerTask(void *argument)
{
    TickType_t lastWake = xTaskGetTickCount();
    char buf[80];
    
    printLine("\r\n=== TASK NOTIFICATION DEMO ===\r\n");
    printLine("Lebih efisien dari Binary Semaphore!\r\n\r\n");
    
    for (;;)
    {
        notificationsSent++;
        
        snprintf(buf, sizeof(buf), 
                 "[PROD] Sending notification #%lu\r\n",
                 (unsigned long)notificationsSent);
        printLine(buf);
        
        /*
         * KIRIM NOTIFICATION
         * 
         * xTaskNotifyGive() melakukan:
         * 1. Increment notification value pada consumerTaskHandle
         * 2. Jika Consumer sedang menunggu, bangunkan dia
         * 
         * Ini LEBIH CEPAT dari xSemaphoreGive() karena:
         * - Tidak perlu lookup ke semaphore object
         * - Langsung akses ke TCB task
         */
        xTaskNotifyGive(consumerTaskHandle);
        
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(PRODUCER_PERIOD_MS));
    }
}

/* ===========================================================================
 * CONSUMER TASK
 * ===========================================================================
 * 
 * Menunggu notification dari Producer.
 */
static void ConsumerTask(void *argument)
{
    char buf[100];
    uint32_t notifyCount;
    
    vTaskDelay(pdMS_TO_TICKS(50)); /* Tunggu producer siap */
    
    for (;;)
    {
        /*
         * TUNGGU NOTIFICATION
         * 
         * ulTaskNotifyTake() parameter:
         * 
         * 1. xClearCountOnExit:
         *    - pdTRUE: Reset value ke 0 (seperti binary semaphore)
         *    - pdFALSE: Decrement value (seperti counting semaphore)
         * 
         * 2. xTicksToWait:
         *    - portMAX_DELAY: Tunggu selamanya
         *    - pdMS_TO_TICKS(x): Tunggu x milidetik
         * 
         * Return value:
         *    - Nilai notification value sebelum di-clear/decrement
         *    - 0 jika timeout
         */
        notifyCount = ulTaskNotifyTake(pdTRUE,      /* Clear on exit */
                                       portMAX_DELAY);
        
        if (notifyCount > 0)
        {
            notificationsReceived++;
            
            toggleLed();
            
            snprintf(buf, sizeof(buf), 
                     "    [CONS] Received! count=%lu | sent=%lu recv=%lu\r\n",
                     (unsigned long)notifyCount,
                     (unsigned long)notificationsSent,
                     (unsigned long)notificationsReceived);
            printLine(buf);
            
            /*
             * CATATAN:
             * - Jika producer lebih cepat, notifyCount bisa > 1
             * - Dengan pdTRUE, semua notification "dikonsumsi" sekaligus
             * - Dengan pdFALSE, hanya 1 notification per Take
             */
            if (notifyCount > 1)
            {
                snprintf(buf, sizeof(buf), 
                         "    [CONS] *** Multiple notifications! (%lu) ***\r\n",
                         (unsigned long)notifyCount);
                printLine(buf);
            }
        }
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
