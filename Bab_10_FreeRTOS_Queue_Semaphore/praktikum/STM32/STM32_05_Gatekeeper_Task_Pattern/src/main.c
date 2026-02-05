/**
 * ============================================================================
 * PROJECT: 11-Gatekeeper_Task_Pattern
 * BOARD: STM32F103C8T6 (Blue Pill)
 * FRAMEWORK: STM32Cube HAL + FreeRTOS Native API
 * ============================================================================
 * 
 * KONSEP GATEKEEPER PATTERN:
 * ==========================
 * Gatekeeper adalah task khusus yang menjadi satu-satunya "pemilik" dari
 * suatu resource (dalam hal ini UART). Task-task lain TIDAK mengakses UART
 * secara langsung, melainkan mengirim pesan melalui queue ke gatekeeper.
 * 
 * Gatekeeper kemudian mengeluarkan pesan dari queue satu per satu dan
 * mengirimkannya ke UART. Dengan cara ini:
 * 
 * 1. TIDAK ADA RACE CONDITION - Hanya gatekeeper yang akses UART
 * 2. TIDAK PERLU MUTEX - Queue sudah thread-safe
 * 3. TIDAK ADA PRIORITY INVERSION - Task tidak saling block
 * 4. LOGGING TERURUT - Pesan dikirim sesuai urutan di queue
 * 
 * ILUSTRASI ALUR DATA:
 * ====================
 * 
 *    SensorTask ──┐
 *         │       │
 *    ButtonTask ──┼──► PrintQueue ──► GatekeeperTask ──► UART1 TX
 *         │       │
 *    StatusTask ──┘
 * 
 * CATATAN PENTING:
 * ================
 * - Gatekeeper harus prioritas TINGGI agar queue tidak overflow
 * - Ukuran queue disesuaikan dengan beban sistem
 * - Jika queue penuh, task pemanggil bisa:
 *   a) Block (menunggu) - xQueueSend dengan timeout
 *   b) Drop pesan - xQueueSend dengan timeout 0
 * 
 * ============================================================================
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include <string.h>

#include "FreeRTOSConfig.h"

/* ===========================================================================
 * STRUKTUR DATA
 * =========================================================================== */

/**
 * Struktur pesan yang dikirim ke gatekeeper.
 * Berisi buffer teks yang akan dicetak ke UART.
 */
typedef struct {
    char text[PRINT_MSG_MAX_LEN];
} PrintMessage_t;

/* ===========================================================================
 * VARIABEL GLOBAL
 * =========================================================================== */
UART_HandleTypeDef huart1;          /* Handle UART1 untuk debug output */
QueueHandle_t printQueue;           /* Queue untuk pesan ke gatekeeper */

/* ===========================================================================
 * PROTOTIPE FUNGSI
 * =========================================================================== */
static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);

/* Task functions */
static void GatekeeperTask(void *argument);
static void SensorTask(void *argument);
static void ButtonTask(void *argument);
static void StatusTask(void *argument);

/* Helper functions */
static void sendToPrint(const char *msg);
void Error_Handler(void);

/* ===========================================================================
 * FUNGSI UTAMA
 * =========================================================================== */
int main(void)
{
    /* Inisialisasi HAL dan sistem clock */
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    
    /*
     * LANGKAH 1: Buat queue untuk komunikasi ke gatekeeper
     * 
     * Queue ini menampung PRINT_QUEUE_LENGTH pesan.
     * Setiap pesan berukuran sizeof(PrintMessage_t).
     */
    printQueue = xQueueCreate(PRINT_QUEUE_LENGTH, sizeof(PrintMessage_t));
    if (printQueue == NULL)
    {
        /* Queue gagal dibuat, masuk error handler */
        Error_Handler();
    }
    
    /*
     * LANGKAH 2: Buat Gatekeeper Task
     * 
     * Task ini adalah SATU-SATUNYA yang boleh mengakses UART.
     * Prioritas TINGGI agar selalu siap mengambil pesan dari queue.
     */
    xTaskCreate(GatekeeperTask, "gate", GATEKEEPER_STACK, NULL, GATEKEEPER_PRIO, NULL);
    
    /*
     * LANGKAH 3: Buat task-task producer
     * 
     * Task-task ini mengirim pesan ke queue, BUKAN langsung ke UART.
     */
    xTaskCreate(SensorTask, "sens", SENSOR_STACK, NULL, SENSOR_PRIO, NULL);
    xTaskCreate(ButtonTask, "btn",  BUTTON_STACK, NULL, BUTTON_PRIO, NULL);
    xTaskCreate(StatusTask, "stat", STATUS_STACK, NULL, STATUS_PRIO, NULL);
    
    /* Mulai scheduler FreeRTOS */
    vTaskStartScheduler();
    
    /* Tidak akan sampai sini jika scheduler berjalan normal */
    while (1) {}
}

/* ===========================================================================
 * GATEKEEPER TASK
 * ===========================================================================
 * 
 * Task ini adalah satu-satunya yang mengakses UART.
 * Ia menunggu pesan dari queue dan mencetaknya.
 * 
 * Diagram alur:
 * 
 *    ┌───────────────────────────────────────┐
 *    │         GATEKEEPER TASK               │
 *    │                                       │
 *    │   ┌─────────────────────────────┐     │
 *    │   │  xQueueReceive(printQueue)  │◄────┼── Tunggu pesan dari queue
 *    │   └──────────────┬──────────────┘     │
 *    │                  │                    │
 *    │                  ▼                    │
 *    │   ┌─────────────────────────────┐     │
 *    │   │  HAL_UART_Transmit()        │────►│ Kirim ke UART
 *    │   └──────────────┬──────────────┘     │
 *    │                  │                    │
 *    │                  ▼                    │
 *    │            Toggle LED                 │
 *    │                  │                    │
 *    │                  └────────────────────┤ Loop kembali
 *    └───────────────────────────────────────┘
 */
static void GatekeeperTask(void *argument)
{
    PrintMessage_t rxMsg;
    
    /* Cetak header saat mulai */
    const char *header = "\r\n=== GATEKEEPER PATTERN DEMO ===\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t *)header, strlen(header), 100);
    
    for (;;)
    {
        /*
         * Tunggu pesan dari queue.
         * portMAX_DELAY = tunggu selamanya sampai ada pesan.
         * 
         * xQueueReceive akan:
         * - Block task ini jika queue kosong
         * - Mengembalikan pdPASS jika berhasil dapat pesan
         * - Menghapus pesan dari queue setelah dibaca
         */
        if (xQueueReceive(printQueue, &rxMsg, portMAX_DELAY) == pdPASS)
        {
            /* Kirim pesan ke UART */
            HAL_UART_Transmit(&huart1, (uint8_t *)rxMsg.text, strlen(rxMsg.text), 100);
            
            /* Toggle LED sebagai indikator aktivitas */
            HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
        }
    }
}

/* ===========================================================================
 * SENSOR TASK
 * ===========================================================================
 * 
 * Simulasi pembacaan sensor.
 * Mengirim hasil pembacaan ke gatekeeper via queue.
 */
static void SensorTask(void *argument)
{
    TickType_t lastWake = xTaskGetTickCount();
    uint16_t sensorValue = 0;
    char buf[PRINT_MSG_MAX_LEN];
    
    for (;;)
    {
        /* Simulasi pembacaan sensor (nilai naik terus) */
        sensorValue += 5;
        if (sensorValue > 100) sensorValue = 0;
        
        /* Format pesan */
        snprintf(buf, sizeof(buf), "[SENSOR] Value: %3u | Tick: %lu\r\n",
                 sensorValue, (unsigned long)xTaskGetTickCount());
        
        /*
         * Kirim ke gatekeeper via queue.
         * TIDAK langsung ke UART!
         */
        sendToPrint(buf);
        
        /* Tunggu periode berikutnya */
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(SENSOR_PERIOD_MS));
    }
}

/* ===========================================================================
 * BUTTON TASK
 * ===========================================================================
 * 
 * Simulasi penekanan tombol (setiap 3 detik).
 * Contoh task dengan prioritas lebih tinggi.
 */
static void ButtonTask(void *argument)
{
    TickType_t lastWake = xTaskGetTickCount();
    uint32_t pressCount = 0;
    char buf[PRINT_MSG_MAX_LEN];
    
    for (;;)
    {
        /* Simulasi: tombol "ditekan" setiap 30 iterasi (3 detik) */
        static uint8_t counter = 0;
        counter++;
        
        if (counter >= 30)  /* 30 x 100ms = 3 detik */
        {
            counter = 0;
            pressCount++;
            
            /* Format pesan */
            snprintf(buf, sizeof(buf), "[BUTTON] Press #%lu detected!\r\n",
                     (unsigned long)pressCount);
            
            /* Kirim ke gatekeeper */
            sendToPrint(buf);
        }
        
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(BUTTON_PERIOD_MS));
    }
}

/* ===========================================================================
 * STATUS TASK
 * ===========================================================================
 * 
 * Laporan status sistem secara periodik.
 * Contoh task dengan periode panjang.
 */
static void StatusTask(void *argument)
{
    TickType_t lastWake = xTaskGetTickCount();
    char buf[PRINT_MSG_MAX_LEN];
    
    for (;;)
    {
        /* Format status report */
        snprintf(buf, sizeof(buf), 
                 "[STATUS] Queue depth: %lu/%d | Uptime: %lu ms\r\n",
                 (unsigned long)uxQueueMessagesWaiting(printQueue),
                 PRINT_QUEUE_LENGTH,
                 (unsigned long)xTaskGetTickCount());
        
        /* Kirim ke gatekeeper */
        sendToPrint(buf);
        
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(STATUS_PERIOD_MS));
    }
}

/* ===========================================================================
 * HELPER FUNCTION: sendToPrint
 * ===========================================================================
 * 
 * Fungsi wrapper untuk mengirim pesan ke gatekeeper.
 * 
 * Keuntungan menggunakan wrapper:
 * - Konsisten di semua tempat
 * - Mudah diubah behavior-nya (misal: tambah timestamp, filter, dll)
 * - Bisa menangani queue full dengan cara yang konsisten
 */
static void sendToPrint(const char *msg)
{
    PrintMessage_t txMsg;
    
    /* Copy pesan ke struktur */
    strncpy(txMsg.text, msg, PRINT_MSG_MAX_LEN - 1);
    txMsg.text[PRINT_MSG_MAX_LEN - 1] = '\0';  /* Pastikan null-terminated */
    
    /*
     * Kirim ke queue dengan timeout singkat (10 tick = 10ms).
     * 
     * Jika queue penuh:
     * - pdMS_TO_TICKS(10) = tunggu 10ms, lalu drop jika masih penuh
     * - 0 = langsung drop tanpa menunggu
     * - portMAX_DELAY = tunggu selamanya (bisa block task lama)
     */
    xQueueSend(printQueue, &txMsg, pdMS_TO_TICKS(10));
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
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;  /* 8MHz x 9 = 72MHz */
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;  /* Max 36MHz */
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;  /* Max 72MHz */

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

    /* LED PC13 */
    GPIO_InitStruct.Pin = LED_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);
    
#if LED_ACTIVE_LOW
    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_SET);  /* LED OFF */
#else
    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_RESET);
#endif

    /* UART TX */
    GPIO_InitStruct.Pin = DEBUG_UART_TX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DEBUG_UART_TX_PORT, &GPIO_InitStruct);

    /* UART RX */
    GPIO_InitStruct.Pin = DEBUG_UART_RX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DEBUG_UART_RX_PORT, &GPIO_InitStruct);
}

/* ===========================================================================
 * CALLBACK DAN HOOK FUNCTIONS
 * =========================================================================== */

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
