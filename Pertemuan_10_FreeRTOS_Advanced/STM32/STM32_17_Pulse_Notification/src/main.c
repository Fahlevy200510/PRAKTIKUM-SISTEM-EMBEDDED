/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 19-Pulse_Notification
 * 
 * JUDUL: Pulse Notification - Lightweight Task Synchronization
 * 
 * DESKRIPSI:
 * Demo penggunaan xTaskNotifyGive() dan ulTaskNotifyTake() untuk
 * sinkronisasi task. Ini adalah cara paling ringan untuk membangunkan
 * task lain - seperti "menepuk bahu" untuk memberitahu ada pekerjaan.
 * 
 * ============================================================================
 * KONSEP PULSE NOTIFICATION
 * ============================================================================
 * 
 *    Pulse = sinyal singkat tanpa membawa data
 *    
 *    Sender Task:           Receiver Task:
 *    ─────────────          ─────────────────────────────────────
 *    │ Kirim pulse │  ──►   │ Terbangun dari blocked state      │
 *    │ (Give)      │        │ Lakukan action                     │
 *    └─────────────┘        │ Kembali blocked menunggu pulse    │
 *                           └─────────────────────────────────────┘
 * 
 * ============================================================================
 * PERBANDINGAN DENGAN SEMAPHORE
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────┐
 *    │            BINARY SEMAPHORE                                    │
 *    ├─────────────────────────────────────────────────────────────────┤
 *    │                                                                │
 *    │  Task A ──────► ┌────────────────┐ ◄────── Task B             │
 *    │  (Give)         │   Semaphore    │         (Take)             │
 *    │                 │    Object      │                            │
 *    │                 │   (80 bytes)   │                            │
 *    │                 └────────────────┘                            │
 *    │                                                                │
 *    │  - Butuh membuat semaphore object terlebih dahulu             │
 *    │  - Multiple tasks bisa wait di semaphore yang sama            │
 *    │  - Lebih fleksibel tapi overhead lebih besar                   │
 *    │                                                                │
 *    └─────────────────────────────────────────────────────────────────┘
 *    
 *    ┌─────────────────────────────────────────────────────────────────┐
 *    │            PULSE NOTIFICATION (Task Notification)              │
 *    ├─────────────────────────────────────────────────────────────────┤
 *    │                                                                │
 *    │  Task A ─────────────────────────────────►  Task B             │
 *    │  xTaskNotifyGive(TaskB)        ulTaskNotifyTake()             │
 *    │                                                                │
 *    │  - TIDAK perlu membuat object apapun                          │
 *    │  - Notification langsung ke task target                        │
 *    │  - RAM: 0 bytes tambahan (sudah di TCB)                        │
 *    │  - Lebih cepat ~45% dibanding semaphore                        │
 *    │  - Hanya SATU task yang bisa menjadi receiver                  │
 *    │                                                                │
 *    └─────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * NOTIFICATION COUNT
 * ============================================================================
 * 
 *    Setiap xTaskNotifyGive() menambah notification count:
 *    
 *    ┌───────────────────────────────────────────────────────────────┐
 *    │ Notification Count dalam TCB                                  │
 *    ├───────────────────────────────────────────────────────────────┤
 *    │                                                               │
 *    │   Initial:  count = 0                                         │
 *    │                                                               │
 *    │   Give: count++    │   Take (pdTRUE):  │   Take (pdFALSE):   │
 *    │   0 → 1            │   count = 0       │   count--           │
 *    │   1 → 2            │   (clear to 0)    │   (decrement)       │
 *    │   2 → 3            │                   │                     │
 *    │                                                               │
 *    └───────────────────────────────────────────────────────────────┘
 *    
 *    Contoh:
 *    - Sender Give 3x saat receiver sibuk
 *    - count = 3
 *    - Receiver Take dengan pdTRUE → terima 3, count = 0
 *    - Receiver Take dengan pdFALSE → terima 3, count = 2
 * 
 * ============================================================================
 * ARSITEKTUR DEMO
 * ============================================================================
 * 
 *    ┌────────────────────┐                 ┌────────────────────┐
 *    │  vFastPulseTask    │                 │  vSlowPulseTask    │
 *    │  (Period: 200ms)   │                 │  (Period: 800ms)   │
 *    └─────────┬──────────┘                 └─────────┬──────────┘
 *              │                                      │
 *              │ xTaskNotifyGive()                    │ xTaskNotifyGive()
 *              │                                      │
 *              └──────────────┬───────────────────────┘
 *                             │
 *                             ▼
 *              ┌──────────────────────────────┐
 *              │       vReceiverTask          │
 *              │  ulTaskNotifyTake(pdTRUE,...)│
 *              │                              │
 *              │  Menerima count dan action   │
 *              └──────────────────────────────┘
 * 
 * ============================================================================
 * API YANG DIGUNAKAN
 * ============================================================================
 * 
 *    // Mengirim pulse (increment notification count)
 *    xTaskNotifyGive(xReceiverTaskHandle);
 *    
 *    // Menerima pulse dengan blocking
 *    uint32_t count = ulTaskNotifyTake(
 *        pdTRUE,           // Clear to zero on exit (TRUE) atau decrement (FALSE)
 *        portMAX_DELAY     // Timeout
 *    );
 *    // count = jumlah Give yang diterima
 * 
 * ============================================================================
 * EXPECTED OUTPUT (UART 115200 baud)
 * ============================================================================
 * 
 *    === Pulse Notification Demo ===
 *    
 *    [FAST] Sending pulse...
 *    [SLOW] Sending pulse...
 *    [RX] Woken! Received 2 pulse(s)
 *    
 *    [FAST] Sending pulse...
 *    [RX] Woken! Received 1 pulse(s)
 *    
 *    [FAST] Sending pulse...
 *    [FAST] Sending pulse...
 *    [RX] Woken! Received 2 pulse(s)
 * 
 * ============================================================================
 */

#include "FreeRTOS.h"
#include "task.h"
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * KONFIGURASI
 * ============================================================================ */

#define FAST_PULSE_PERIOD_MS    200     /* Sender cepat: 200ms */
#define SLOW_PULSE_PERIOD_MS    800     /* Sender lambat: 800ms */
#define RECEIVER_PROCESS_MS     500     /* Waktu proses receiver: 500ms */

/* ============================================================================
 * HANDLE
 * ============================================================================ */

static TaskHandle_t xReceiverTaskHandle = NULL;

static UART_HandleTypeDef huart1;

/* ============================================================================
 * STATISTIK
 * ============================================================================ */

static volatile uint32_t ulTotalPulsesReceived = 0;
static volatile uint32_t ulFastPulsesSent = 0;
static volatile uint32_t ulSlowPulsesSent = 0;

/* ============================================================================
 * PROTOTYPE
 * ============================================================================ */

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART_Init(void);
static void UART_SendString(const char *str);

static void vFastPulseTask(void *pvParameters);
static void vSlowPulseTask(void *pvParameters);
static void vReceiverTask(void *pvParameters);
static void vStatsTask(void *pvParameters);

/* ============================================================================
 * IMPLEMENTASI TASK
 * ============================================================================ */

/**
 * @brief Task pengirim pulse cepat (200ms interval)
 * 
 * ILUSTRASI TIMING:
 * ─────────────────────────────────────────────────────────────
 *    Time:    0    200   400   600   800   1000  1200
 *             │     │     │     │     │     │     │
 *    Fast:    █     █     █     █     █     █     █
 *             ↓     ↓     ↓     ↓     ↓     ↓     ↓
 *           Give  Give  Give  Give  Give  Give  Give
 * ─────────────────────────────────────────────────────────────
 */
static void vFastPulseTask(void *pvParameters)
{
    (void)pvParameters;
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for(;;)
    {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(FAST_PULSE_PERIOD_MS));
        
        UART_SendString("[FAST] Sending pulse...\r\n");
        
        /*
         * xTaskNotifyGive() - increment notification count
         * 
         * Ini equivalent dengan:
         * xTaskNotify(xReceiverTaskHandle, 0, eIncrement);
         * 
         * Tapi xTaskNotifyGive lebih mudah dibaca untuk use case
         * "lightweight semaphore".
         */
        xTaskNotifyGive(xReceiverTaskHandle);
        
        ulFastPulsesSent++;
    }
}

/**
 * @brief Task pengirim pulse lambat (800ms interval)
 * 
 * ILUSTRASI TIMING:
 * ─────────────────────────────────────────────────────────────
 *    Time:    0         800        1600       2400
 *             │          │          │          │
 *    Slow:    █          █          █          █
 *             ↓          ↓          ↓          ↓
 *           Give       Give       Give       Give
 * ─────────────────────────────────────────────────────────────
 */
static void vSlowPulseTask(void *pvParameters)
{
    (void)pvParameters;
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for(;;)
    {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(SLOW_PULSE_PERIOD_MS));
        
        UART_SendString("[SLOW] Sending pulse...\r\n");
        
        xTaskNotifyGive(xReceiverTaskHandle);
        
        ulSlowPulsesSent++;
    }
}

/**
 * @brief Task penerima pulse
 * 
 * ILUSTRASI ulTaskNotifyTake:
 * ═══════════════════════════════════════════════════════════════════════
 *    
 *    ulTaskNotifyTake(xClearCountOnExit, xTicksToWait)
 *    
 *    Parameter 1: xClearCountOnExit
 *    ───────────────────────────────
 *    
 *    pdTRUE:  Setelah Take, count di-reset ke 0
 *    ┌────────────────────────────────────────────┐
 *    │ Before: count = 5                          │
 *    │ Take returns: 5                            │
 *    │ After: count = 0                           │
 *    │                                            │
 *    │ Use case: "Ada berapa event sejak terakhir │
 *    │           saya cek?" (batch processing)    │
 *    └────────────────────────────────────────────┘
 *    
 *    pdFALSE: Setelah Take, count di-decrement
 *    ┌────────────────────────────────────────────┐
 *    │ Before: count = 5                          │
 *    │ Take returns: 5                            │
 *    │ After: count = 4                           │
 *    │                                            │
 *    │ Use case: "Process satu event per satu"    │
 *    │           (counting semaphore behavior)    │
 *    └────────────────────────────────────────────┘
 *    
 *    ILUSTRASI BLOCKING:
 *    ───────────────────────────────────────────────────────────
 *    
 *    count = 0 saat Take dipanggil:
 *    
 *    Receiver:  ░░░░░░░░░░░░░░░░░░░░█████████████
 *               [BLOCKED waiting]   [woken by Give]
 *               
 *    Sender:              │
 *               xTaskNotifyGive()
 *    
 *    count > 0 saat Take dipanggil:
 *    
 *    Receiver:  █████████████████████████████████
 *               [return immediately dengan count]
 * 
 * ═══════════════════════════════════════════════════════════════════════
 */
static void vReceiverTask(void *pvParameters)
{
    (void)pvParameters;
    
    uint32_t ulReceivedCount;
    char buffer[80];
    
    UART_SendString("\r\n=== Pulse Notification Demo ===\r\n");
    UART_SendString("Fast sender: 200ms, Slow sender: 800ms\r\n");
    UART_SendString("Receiver process time: 500ms\r\n\r\n");
    
    for(;;)
    {
        /*
         * Block sampai ada notification.
         * 
         * pdTRUE = clear count ke 0 setelah Take
         * Ini artinya kita batch process semua pulse yang terkumpul.
         */
        ulReceivedCount = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        
        if(ulReceivedCount > 0)
        {
            /* Toggle LED */
            HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
            
            snprintf(buffer, sizeof(buffer), 
                    "[RX] Woken! Received %lu pulse(s)\r\n\r\n", 
                    ulReceivedCount);
            UART_SendString(buffer);
            
            ulTotalPulsesReceived += ulReceivedCount;
            
            /*
             * Simulasi processing time.
             * Selama delay ini, pulse dari sender akan terakumulasi.
             */
            vTaskDelay(pdMS_TO_TICKS(RECEIVER_PROCESS_MS));
        }
    }
}

/**
 * @brief Task statistik (cetak summary tiap 10 detik)
 * 
 * Memantau efisiensi sistem pulse notification.
 */
static void vStatsTask(void *pvParameters)
{
    (void)pvParameters;
    
    char buffer[120];
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for(;;)
    {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10000));
        
        UART_SendString("\r\n========== STATS ==========\r\n");
        
        snprintf(buffer, sizeof(buffer),
                "Fast pulses sent: %lu\r\n", ulFastPulsesSent);
        UART_SendString(buffer);
        
        snprintf(buffer, sizeof(buffer),
                "Slow pulses sent: %lu\r\n", ulSlowPulsesSent);
        UART_SendString(buffer);
        
        snprintf(buffer, sizeof(buffer),
                "Total pulses received: %lu\r\n", ulTotalPulsesReceived);
        UART_SendString(buffer);
        
        snprintf(buffer, sizeof(buffer),
                "Total sent: %lu (should match received)\r\n",
                ulFastPulsesSent + ulSlowPulsesSent);
        UART_SendString(buffer);
        
        UART_SendString("===========================\r\n\r\n");
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
    
    /* LED PC13 */
    GPIO_InitStruct.Pin = LED_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);
    
    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_SET); /* LED off */
}

static void UART_Init(void)
{
    __HAL_RCC_USART1_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* TX - PA9 */
    GPIO_InitStruct.Pin = DEBUG_UART_TX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DEBUG_UART_TX_PORT, &GPIO_InitStruct);
    
    /* RX - PA10 */
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
    UART_SendString("19-Pulse_Notification\r\n");
    UART_SendString("STM32F103 + FreeRTOS\r\n");
    UART_SendString("================================\r\n");
    
    /*
     * Buat receiver task lebih dulu agar handle-nya valid
     * saat sender tasks mulai mengirim pulse.
     * 
     * Prioritas receiver lebih rendah agar sender bisa
     * mengirim beberapa pulse sebelum receiver dijalankan.
     */
    xTaskCreate(vReceiverTask, "Receiver", PULSE_RECEIVER_STACK, NULL,
                PULSE_RECEIVER_PRIO, &xReceiverTaskHandle);
    
    /* Buat sender tasks dengan prioritas lebih tinggi */
    xTaskCreate(vFastPulseTask, "FastPulse", PULSE_SENDER_STACK, NULL,
                PULSE_SENDER_PRIO, NULL);
    
    xTaskCreate(vSlowPulseTask, "SlowPulse", PULSE_SENDER_STACK, NULL,
                PULSE_SENDER_PRIO, NULL);
    
    /* Task statistik */
    xTaskCreate(vStatsTask, "Stats", 256, NULL,
                tskIDLE_PRIORITY + 1, NULL);
    
    vTaskStartScheduler();
    
    for(;;);
}
