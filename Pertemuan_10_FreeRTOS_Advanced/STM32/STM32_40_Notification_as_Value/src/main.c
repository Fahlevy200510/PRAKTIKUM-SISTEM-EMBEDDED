/**
 * ============================================================================
 * PROJECT: 17-Notification_as_Value
 * BOARD: STM32F103C8T6 (Blue Pill)
 * FRAMEWORK: STM32Cube HAL + FreeRTOS Native API
 * ============================================================================
 * 
 * DEMO TASK NOTIFICATION DENGAN VALUE:
 * ====================================
 * 
 * Tidak hanya signal, tapi bisa kirim data 32-bit!
 * Seperti queue 1 item tetapi lebih cepat dan hemat RAM.
 * 
 * API YANG DIGUNAKAN:
 * ===================
 * 
 *    xTaskNotify(taskHandle, value, action)
 *    ──────────────────────────────────────
 *    action:
 *    - eSetValueWithOverwrite: Set value, timpa yang lama
 *    - eSetValueWithoutOverwrite: Set value hanya jika kosong
 *    - eSetBits: OR value dengan yang ada
 *    - eIncrement: Increment (sama seperti xTaskNotifyGive)
 *    - eNoAction: Hanya set notification state
 * 
 *    xTaskNotifyWait(ulBitsToClearOnEntry, ulBitsToClearOnExit, 
 *                    pulNotificationValue, xTicksToWait)
 *    ─────────────────────────────────────────────────────────
 *    - ulBitsToClearOnEntry: Bit yang di-clear sebelum wait
 *    - ulBitsToClearOnExit: Bit yang di-clear setelah receive
 *    - pulNotificationValue: Pointer untuk menerima value
 *    - xTicksToWait: Timeout
 * 
 * CONTOH PENGGUNAAN:
 * ==================
 * 
 *    // Kirim sensor reading
 *    xTaskNotify(displayTask, sensorValue, eSetValueWithOverwrite);
 * 
 *    // Terima di display task
 *    xTaskNotifyWait(0, 0xFFFFFFFF, &receivedValue, portMAX_DELAY);
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
 * STRUKTUR DATA - Simulasi sensor data dalam 32-bit
 * =========================================================================== */

/**
 * Pack sensor data ke 32-bit value:
 * Bit 31-24: Sensor ID (8-bit)
 * Bit 23-16: Status flags (8-bit)
 * Bit 15-0:  Reading value (16-bit)
 */
#define PACK_SENSOR_DATA(id, status, reading) \
    (((uint32_t)(id) << 24) | ((uint32_t)(status) << 16) | ((uint16_t)(reading)))

#define UNPACK_SENSOR_ID(data)      (((data) >> 24) & 0xFF)
#define UNPACK_STATUS(data)         (((data) >> 16) & 0xFF)
#define UNPACK_READING(data)        ((data) & 0xFFFF)

/* Status flags */
#define STATUS_OK                   0x01
#define STATUS_WARNING              0x02
#define STATUS_ERROR                0x04

/* ===========================================================================
 * VARIABEL GLOBAL
 * =========================================================================== */
UART_HandleTypeDef huart1;
TaskHandle_t receiverTaskHandle = NULL;

/* ===========================================================================
 * PROTOTIPE FUNGSI
 * =========================================================================== */
static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);

static void SenderTask(void *argument);
static void ReceiverTask(void *argument);

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
    
    /* Buat Receiver dulu, simpan handle-nya */
    xTaskCreate(ReceiverTask, "recv", RECEIVER_STACK, NULL, RECEIVER_PRIO, &receiverTaskHandle);
    
    /* Buat Sender */
    xTaskCreate(SenderTask, "send", SENDER_STACK, NULL, SENDER_PRIO, NULL);
    
    vTaskStartScheduler();
    
    while (1) {}
}

/* ===========================================================================
 * SENDER TASK
 * ===========================================================================
 * 
 * Mengirim sensor data yang di-pack dalam 32-bit notification value.
 */
static void SenderTask(void *argument)
{
    TickType_t lastWake = xTaskGetTickCount();
    char buf[100];
    uint8_t sensorId = 1;
    uint16_t reading = 100;
    uint8_t status;
    uint32_t packedData;
    
    printLine("\r\n=== NOTIFICATION VALUE DEMO ===\r\n");
    printLine("Mengirim sensor data sebagai 32-bit value\r\n\r\n");
    
    for (;;)
    {
        /* Simulasi pembacaan sensor */
        reading += 50;
        if (reading > 4000) reading = 100;
        
        /* Tentukan status berdasarkan reading */
        if (reading > 3000)
            status = STATUS_ERROR;
        else if (reading > 2000)
            status = STATUS_WARNING;
        else
            status = STATUS_OK;
        
        /* Pack data ke 32-bit */
        packedData = PACK_SENSOR_DATA(sensorId, status, reading);
        
        snprintf(buf, sizeof(buf), 
                 "[SEND] ID=%u status=0x%02X reading=%u -> packed=0x%08lX\r\n",
                 sensorId, status, reading, (unsigned long)packedData);
        printLine(buf);
        
        /*
         * KIRIM NOTIFICATION DENGAN VALUE
         * 
         * eSetValueWithOverwrite:
         * - Set notification value = packedData
         * - Timpa value lama jika belum dibaca
         * - Gunakan ini untuk data "latest value" (sensor terbaru)
         * 
         * Alternatif: eSetValueWithoutOverwrite
         * - Hanya set jika value belum dibaca
         * - Return pdFAIL jika value lama masih pending
         * - Gunakan ini untuk data yang tidak boleh hilang
         */
        xTaskNotify(receiverTaskHandle, 
                    packedData, 
                    eSetValueWithOverwrite);
        
        /* Rotasi sensor ID */
        sensorId = (sensorId % 3) + 1;
        
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(SENDER_PERIOD_MS));
    }
}

/* ===========================================================================
 * RECEIVER TASK
 * ===========================================================================
 * 
 * Menunggu dan meng-unpack data yang diterima.
 */
static void ReceiverTask(void *argument)
{
    char buf[120];
    uint32_t receivedData;
    uint8_t id, status;
    uint16_t reading;
    const char *statusStr;
    
    vTaskDelay(pdMS_TO_TICKS(50));
    
    for (;;)
    {
        /*
         * TUNGGU NOTIFICATION DENGAN VALUE
         * 
         * Parameter:
         * 1. ulBitsToClearOnEntry (0):
         *    - Bit yang di-clear SEBELUM cek notification
         *    - 0 = tidak clear apa-apa
         * 
         * 2. ulBitsToClearOnExit (0xFFFFFFFF):
         *    - Bit yang di-clear SETELAH dapat value
         *    - 0xFFFFFFFF = clear semua (reset ke 0)
         * 
         * 3. &receivedData:
         *    - Pointer untuk menerima value
         * 
         * 4. portMAX_DELAY:
         *    - Tunggu selamanya
         */
        if (xTaskNotifyWait(0,              /* Bits to clear on entry */
                            0xFFFFFFFF,     /* Bits to clear on exit */
                            &receivedData,  /* Where to store value */
                            portMAX_DELAY) == pdTRUE)
        {
            /* Unpack data */
            id = UNPACK_SENSOR_ID(receivedData);
            status = UNPACK_STATUS(receivedData);
            reading = UNPACK_READING(receivedData);
            
            /* Decode status */
            if (status & STATUS_ERROR)
                statusStr = "ERROR!";
            else if (status & STATUS_WARNING)
                statusStr = "WARNING";
            else
                statusStr = "OK";
            
            toggleLed();
            
            snprintf(buf, sizeof(buf), 
                     "    [RECV] Sensor %u: %s | reading=%u (raw=0x%08lX)\r\n",
                     id, statusStr, reading, (unsigned long)receivedData);
            printLine(buf);
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
