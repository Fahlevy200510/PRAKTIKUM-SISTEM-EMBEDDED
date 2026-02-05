/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 07-Mailbox_Pattern
 * 
 * JUDUL: Mailbox Pattern - Queue Depth=1 untuk Data Terbaru
 * 
 * DESKRIPSI:
 * Demo penggunaan Mailbox pattern dengan xQueueOverwrite().
 * Sensor task update nilai terbaru ke mailbox, Display task
 * membaca dengan xQueuePeek() tanpa menghapus data.
 * 
 * ============================================================================
 * ARSITEKTUR PROGRAM
 * ============================================================================
 * 
 *    ┌────────────────────────────────────────────────────────────────────┐
 *    │                     MAILBOX PATTERN DEMO                           │
 *    ├────────────────────────────────────────────────────────────────────┤
 *    │                                                                    │
 *    │   ┌──────────────────┐                                             │
 *    │   │  vSensorTask     │                                             │
 *    │   │  (Update 200ms)  │                                             │
 *    │   │                  │                                             │
 *    │   │  Baca ADC ───────┼──► xQueueOverwrite() ──┐                    │
 *    │   │  Buat Packet     │                        │                    │
 *    │   └──────────────────┘                        │                    │
 *    │                                               ▼                    │
 *    │                                    ┌──────────────────┐            │
 *    │                                    │     MAILBOX      │            │
 *    │                                    │   (depth = 1)    │            │
 *    │                                    │                  │            │
 *    │                                    │ ┌──────────────┐ │            │
 *    │                                    │ │ SensorData   │ │            │
 *    │                                    │ │ (terbaru)    │ │            │
 *    │                                    │ └──────────────┘ │            │
 *    │                                    └────────┬─────────┘            │
 *    │                                             │                      │
 *    │   ┌──────────────────┐                      │                      │
 *    │   │  vDisplayTask    │◄──── xQueuePeek() ───┘                      │
 *    │   │  (Refresh 500ms) │                                             │
 *    │   │                  │                                             │
 *    │   │  Print ke UART   │                                             │
 *    │   └──────────────────┘                                             │
 *    │                                                                    │
 *    └────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * XQUEUEOVERWRITE vs XQUEUESEND
 * ============================================================================
 * 
 *    xQueueSend():
 *    ─────────────
 *    - Gagal (return pdFAIL) jika queue penuh
 *    - Data lama tetap ada, data baru ditolak
 *    - Cocok untuk: semua data penting
 *    
 *    xQueueOverwrite():
 *    ──────────────────
 *    - SELALU berhasil (tidak ada return value yang perlu dicek)
 *    - Data lama ditimpa dengan data baru
 *    - HANYA untuk queue dengan depth=1 (mailbox)
 *    - Cocok untuk: sensor reading, status terbaru
 *    
 *    ILUSTRASI:
 *    
 *    Mailbox: [Data_v1]
 *             
 *    xQueueOverwrite(&Data_v2)
 *             
 *    Mailbox: [Data_v2]  ← v1 hilang, diganti v2
 * 
 * ============================================================================
 * XQUEUEPEEK vs XQUEUERECEIVE
 * ============================================================================
 * 
 *    xQueueReceive():
 *    ────────────────
 *    - Membaca DAN MENGHAPUS data dari queue
 *    - Setelah baca, slot menjadi kosong
 *    - Pembaca berikutnya harus menunggu data baru
 *    
 *    xQueuePeek():
 *    ─────────────
 *    - Membaca TANPA MENGHAPUS data
 *    - Data tetap ada di queue
 *    - Multiple task bisa peek data yang sama
 *    - Cocok untuk: shared sensor reading
 *    
 *    ILUSTRASI:
 *    
 *                     xQueueReceive()         xQueuePeek()
 *    Before:  [Data]       [Data]              [Data]
 *    After:   [    ]       [    ]              [Data] ← masih ada!
 * 
 * ============================================================================
 * EXPECTED OUTPUT (UART 115200 baud)
 * ============================================================================
 * 
 *    === Mailbox Pattern Demo ===
 *    
 *    [SENSOR] Update #1: ADC=2048, Temp=25.0C
 *    [SENSOR] Update #2: ADC=2100, Temp=25.6C
 *    [DISPLAY] Reading: ADC=2100, Temp=25.6C (update #2)
 *    [SENSOR] Update #3: ADC=2050, Temp=25.1C
 *    [SENSOR] Update #4: ADC=2080, Temp=25.4C
 *    [DISPLAY] Reading: ADC=2080, Temp=25.4C (update #4)
 *    
 *    Note: Display skip update #2 dan #3 karena lebih lambat.
 *          Ini normal untuk mailbox pattern!
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
 * DEFINISI STRUCT
 * ============================================================================ */

/**
 * @brief Data sensor yang disimpan di mailbox
 * 
 * Struct ini menyimpan pembacaan sensor terbaru.
 * Hanya nilai TERBARU yang penting, nilai lama bisa di-overwrite.
 */
typedef struct {
    uint16_t   adcRaw;        /* Nilai ADC mentah */
    float      temperature;   /* Suhu dalam Celsius (dihitung dari ADC) */
    uint32_t   updateCount;   /* Counter berapa kali update */
    TickType_t timestamp;     /* Waktu update terakhir */
} SensorData_t;

/* ============================================================================
 * HANDLE GLOBAL
 * ============================================================================ */

/* Mailbox = Queue dengan depth 1 */
static QueueHandle_t xSensorMailbox = NULL;

static UART_HandleTypeDef huart1;
static ADC_HandleTypeDef  hadc1;

/* ============================================================================
 * PROTOTYPE
 * ============================================================================ */

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART_Init(void);
static void ADC_Init(void);
static void UART_SendString(const char *str);

static void vSensorTask(void *pvParameters);
static void vDisplayTask(void *pvParameters);

/* ============================================================================
 * KONVERSI ADC KE SUHU
 * ============================================================================
 * 
 * Simulasi konversi ADC ke suhu.
 * Asumsi: 
 * - ADC 0 = 0°C
 * - ADC 4095 = 100°C
 * - Linear interpolation
 */
static float ADC_ToTemperature(uint16_t adcValue)
{
    return (float)adcValue * 100.0f / 4095.0f;
}

/* ============================================================================
 * IMPLEMENTASI TASK
 * ============================================================================ */

/**
 * @brief Sensor Task - Update mailbox dengan data terbaru
 * 
 * ILUSTRASI:
 * ═══════════════════════════════════════════════════════════════════════════
 *    
 *    Setiap 200ms:
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                                                                     │
 *    │   ┌─────────────────┐                                               │
 *    │   │ 1. Baca ADC     │  → adcRaw = 2048                             │
 *    │   └────────┬────────┘                                               │
 *    │            ▼                                                        │
 *    │   ┌─────────────────┐                                               │
 *    │   │ 2. Konversi     │  → temperature = 50.0°C                      │
 *    │   └────────┬────────┘                                               │
 *    │            ▼                                                        │
 *    │   ┌─────────────────┐                                               │
 *    │   │ 3. Isi Struct   │  data.adcRaw = 2048                          │
 *    │   │                 │  data.temperature = 50.0                     │
 *    │   │                 │  data.updateCount++                          │
 *    │   │                 │  data.timestamp = now                        │
 *    │   └────────┬────────┘                                               │
 *    │            ▼                                                        │
 *    │   ╔═════════════════╗                                               │
 *    │   ║ 4. OVERWRITE!   ║  xQueueOverwrite(xMailbox, &data)            │
 *    │   ║                 ║                                               │
 *    │   ║   Mailbox:      ║                                               │
 *    │   ║   [old] → [new] ║  Data lama hilang, diganti baru              │
 *    │   ╚═════════════════╝                                               │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 *    
 * ═══════════════════════════════════════════════════════════════════════════
 */
static void vSensorTask(void *pvParameters)
{
    (void)pvParameters;
    
    SensorData_t sensorData;
    uint32_t updateCounter = 0;
    char buffer[80];
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    UART_SendString("[SENSOR] Sensor task started (200ms interval)\r\n");
    
    for(;;)
    {
        /* Delay dengan timing presisi */
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(SENSOR_UPDATE_MS));
        
        updateCounter++;
        
        /* 1. Baca ADC */
        HAL_ADC_Start(&hadc1);
        if(HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
        {
            sensorData.adcRaw = HAL_ADC_GetValue(&hadc1);
        }
        else
        {
            sensorData.adcRaw = 0;
        }
        HAL_ADC_Stop(&hadc1);
        
        /* 2. Konversi ke suhu */
        sensorData.temperature = ADC_ToTemperature(sensorData.adcRaw);
        
        /* 3. Isi metadata */
        sensorData.updateCount = updateCounter;
        sensorData.timestamp = xTaskGetTickCount();
        
        /* 4. Log update */
        snprintf(buffer, sizeof(buffer),
                "[SENSOR] Update #%lu: ADC=%u, Temp=%.1fC\r\n",
                sensorData.updateCount,
                sensorData.adcRaw,
                sensorData.temperature);
        UART_SendString(buffer);
        
        /* 
         * 5. OVERWRITE mailbox dengan data baru!
         * 
         * xQueueOverwrite() SELALU berhasil untuk mailbox (queue depth=1).
         * Data lama akan ditimpa dengan data baru.
         * Tidak perlu check return value.
         */
        xQueueOverwrite(xSensorMailbox, &sensorData);
        
        /* Toggle LED sebagai heartbeat */
        HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
    }
}

/**
 * @brief Display Task - Baca mailbox dan tampilkan
 * 
 * ILUSTRASI:
 * ═══════════════════════════════════════════════════════════════════════════
 *    
 *    Setiap 500ms:
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                                                                     │
 *    │   ╔═════════════════╗                                               │
 *    │   ║ 1. PEEK mailbox ║  xQueuePeek(xMailbox, &data, timeout)        │
 *    │   ║                 ║                                               │
 *    │   ║   Mailbox:      ║                                               │
 *    │   ║   [data] ──────►║──► data (copy)                               │
 *    │   ║   [data] tetap! ║  Data masih ada di mailbox!                  │
 *    │   ╚═════════════════╝                                               │
 *    │            │                                                        │
 *    │            ▼                                                        │
 *    │   ┌─────────────────┐                                               │
 *    │   │ 2. Format       │  Buat string untuk display                   │
 *    │   └────────┬────────┘                                               │
 *    │            ▼                                                        │
 *    │   ┌─────────────────┐                                               │
 *    │   │ 3. Print UART   │  Tampilkan ke terminal                       │
 *    │   └─────────────────┘                                               │
 *    │                                                                     │
 *    │   CATATAN: Karena Peek, Sensor task bisa update                    │
 *    │            tanpa menunggu Display selesai baca!                    │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 *    
 * ═══════════════════════════════════════════════════════════════════════════
 */
static void vDisplayTask(void *pvParameters)
{
    (void)pvParameters;
    
    SensorData_t displayData;
    char buffer[100];
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    UART_SendString("[DISPLAY] Display task started (500ms interval)\r\n\r\n");
    
    for(;;)
    {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(DISPLAY_REFRESH_MS));
        
        /*
         * PEEK mailbox - baca tanpa menghapus!
         * 
         * Keuntungan:
         * - Sensor task bisa terus update
         * - Data tetap ada untuk task lain yang mau baca
         * - Tidak ada blocking pada sensor
         */
        if(xQueuePeek(xSensorMailbox, &displayData, pdMS_TO_TICKS(100)) == pdPASS)
        {
            snprintf(buffer, sizeof(buffer),
                    "[DISPLAY] Reading: ADC=%u, Temp=%.1fC (update #%lu)\r\n",
                    displayData.adcRaw,
                    displayData.temperature,
                    displayData.updateCount);
            UART_SendString(buffer);
        }
        else
        {
            UART_SendString("[DISPLAY] No data in mailbox yet\r\n");
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
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    
    HAL_ADCEx_Calibration_Start(&hadc1);
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
    ADC_Init();
    
    UART_SendString("\r\n================================\r\n");
    UART_SendString("07-Mailbox_Pattern\r\n");
    UART_SendString("STM32F103 + FreeRTOS\r\n");
    UART_SendString("================================\r\n");
    UART_SendString("\r\n=== Mailbox Pattern Demo ===\r\n\r\n");
    
    /*
     * Buat MAILBOX = Queue dengan depth 1
     * 
     * Karakteristik:
     * - Hanya menyimpan 1 item (data terbaru)
     * - Gunakan xQueueOverwrite() untuk selalu update
     * - Gunakan xQueuePeek() untuk baca tanpa hapus
     */
    xSensorMailbox = xQueueCreate(1, sizeof(SensorData_t));
    
    if(xSensorMailbox == NULL)
    {
        UART_SendString("ERROR: Failed to create mailbox!\r\n");
        while(1);
    }
    
    UART_SendString("Mailbox created (depth=1)\r\n\r\n");
    
    xTaskCreate(vSensorTask, "Sensor", SENSOR_TASK_STACK, NULL,
                SENSOR_TASK_PRIO, NULL);
    
    xTaskCreate(vDisplayTask, "Display", DISPLAY_TASK_STACK, NULL,
                DISPLAY_TASK_PRIO, NULL);
    
    vTaskStartScheduler();
    
    for(;;);
}
