/**
 * ============================================================================
 * PROJECT: 12-Recursive_Mutex_Locking
 * BOARD: STM32F103C8T6 (Blue Pill)
 * FRAMEWORK: STM32Cube HAL + FreeRTOS Native API
 * ============================================================================
 * 
 * KONSEP RECURSIVE MUTEX:
 * =======================
 * Recursive mutex memungkinkan satu task untuk mengambil mutex yang SUDAH
 * dia pegang, tanpa menyebabkan deadlock.
 * 
 * KAPAN RECURSIVE MUTEX DIBUTUHKAN?
 * =================================
 * 1. Fungsi yang memanggil fungsi lain yang juga butuh mutex
 * 2. Fungsi rekursif yang perlu proteksi
 * 3. API library yang dipanggil dari berbagai level
 * 
 * CONTOH KASUS:
 * =============
 * 
 *    void printHeader(void) {
 *        takeMutex();
 *        print("=== HEADER ===");
 *        giveMutex();
 *    }
 * 
 *    void printReport(void) {
 *        takeMutex();         // Ambil mutex
 *        printHeader();       // Fungsi ini juga butuh mutex!
 *                             // Dengan mutex biasa = DEADLOCK
 *                             // Dengan recursive mutex = OK
 *        print("Report data...");
 *        giveMutex();
 *    }
 * 
 * ILUSTRASI RECURSIVE MUTEX:
 * ==========================
 * 
 *    ┌─────────────────────────────────────────────────────────────┐
 *    │                    RECURSIVE MUTEX                          │
 *    │                                                             │
 *    │    Take #1         Take #2         Take #3                  │
 *    │    (count=1)       (count=2)       (count=3)                │
 *    │        │               │               │                    │
 *    │        ▼               ▼               ▼                    │
 *    │    ┌───────────────────────────────────────────┐            │
 *    │    │   Owner: Task A                          │            │
 *    │    │   Lock Count: 3                          │            │
 *    │    │   Status: LOCKED                         │            │
 *    │    └───────────────────────────────────────────┘            │
 *    │        │               │               │                    │
 *    │        ▼               ▼               ▼                    │
 *    │    Give #1         Give #2         Give #3                  │
 *    │    (count=2)       (count=1)       (count=0)                │
 *    │                                        │                    │
 *    │                                        ▼                    │
 *    │                              Mutex BEBAS!                   │
 *    │                              Task lain boleh ambil          │
 *    └─────────────────────────────────────────────────────────────┘
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
SemaphoreHandle_t uartMutex;        /* Recursive mutex untuk proteksi UART */
volatile uint32_t lockCount = 0;    /* Counter untuk demonstrasi */

/* ===========================================================================
 * PROTOTIPE FUNGSI
 * =========================================================================== */
static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);

static void WorkerTask(void *argument);
static void LoggerTask(void *argument);

/* Fungsi yang menggunakan recursive mutex */
static void safePrint(const char *msg);
static void nestedFunction(uint8_t depth);
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
     * LANGKAH 1: Buat Recursive Mutex
     * 
     * Gunakan xSemaphoreCreateRecursiveMutex() bukan xSemaphoreCreateMutex()
     * 
     * Perbedaan:
     * - Mutex biasa: Task A ambil, Task A ambil lagi = DEADLOCK
     * - Recursive: Task A ambil, Task A ambil lagi = OK (count bertambah)
     */
    uartMutex = xSemaphoreCreateRecursiveMutex();
    if (uartMutex == NULL)
    {
        Error_Handler();
    }
    
    /* Buat task-task */
    xTaskCreate(WorkerTask, "work", WORKER_STACK, NULL, WORKER_PRIO, NULL);
    xTaskCreate(LoggerTask, "log",  LOGGER_STACK, NULL, LOGGER_PRIO, NULL);
    
    vTaskStartScheduler();
    
    while (1) {}
}

/* ===========================================================================
 * WORKER TASK
 * ===========================================================================
 * 
 * Task ini mendemonstrasikan penggunaan recursive mutex dengan memanggil
 * fungsi bersarang (nested) yang masing-masing mengambil mutex.
 */
static void WorkerTask(void *argument)
{
    TickType_t lastWake = xTaskGetTickCount();
    char buf[80];
    
    /* Cetak header */
    safePrint("\r\n=== RECURSIVE MUTEX DEMO ===\r\n");
    safePrint("Task Worker: Demonstrasi fungsi nested\r\n\r\n");
    
    for (;;)
    {
        /*
         * nestedFunction akan mengambil mutex, lalu memanggil dirinya
         * sendiri (rekursif) yang juga mengambil mutex.
         * 
         * Tanpa recursive mutex, ini akan menyebabkan DEADLOCK!
         */
        snprintf(buf, sizeof(buf), "[WORKER] Mulai nested call (depth=%d)\r\n", 
                 MAX_RECURSION_DEPTH);
        safePrint(buf);
        
        nestedFunction(MAX_RECURSION_DEPTH);
        
        safePrint("[WORKER] Selesai nested call\r\n\r\n");
        
        toggleLed();
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(WORKER_PERIOD_MS));
    }
}

/* ===========================================================================
 * LOGGER TASK
 * ===========================================================================
 * 
 * Task ini mendemonstrasikan bahwa recursive mutex tetap melindungi
 * resource dari task LAIN (bukan pemilik).
 */
static void LoggerTask(void *argument)
{
    TickType_t lastWake = xTaskGetTickCount();
    char buf[80];
    
    vTaskDelay(pdMS_TO_TICKS(100)); /* Tunggu sebentar agar Worker duluan */
    
    for (;;)
    {
        snprintf(buf, sizeof(buf), "[LOGGER] Status report | Tick: %lu\r\n",
                 (unsigned long)xTaskGetTickCount());
        
        /*
         * Ini akan menunggu jika Worker sedang memegang mutex,
         * meskipun Worker sudah Take berkali-kali.
         * 
         * Logger BUKAN pemilik mutex, jadi harus menunggu sampai
         * Worker melepaskan SEMUA lock-nya (count = 0).
         */
        safePrint(buf);
        
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(LOGGER_PERIOD_MS));
    }
}

/* ===========================================================================
 * NESTED FUNCTION (REKURSIF)
 * ===========================================================================
 * 
 * Fungsi ini memanggil dirinya sendiri secara rekursif.
 * Setiap level mengambil mutex (recursive).
 * 
 * Alur dengan depth=3:
 * 
 *    nestedFunction(3)  ──► Take mutex (count=1)
 *         │
 *         └──► nestedFunction(2)  ──► Take mutex (count=2)
 *                   │
 *                   └──► nestedFunction(1)  ──► Take mutex (count=3)
 *                             │
 *                             └──► nestedFunction(0)  ──► Base case, return
 *                             │
 *                             └──► Give mutex (count=2)
 *                   │
 *                   └──► Give mutex (count=1)
 *         │
 *         └──► Give mutex (count=0) - BEBAS!
 */
static void nestedFunction(uint8_t depth)
{
    char buf[80];
    
    if (depth == 0)
    {
        /* Base case: tidak perlu rekursi lagi */
        safePrint("        [NESTED] Reached bottom, returning...\r\n");
        return;
    }
    
    /*
     * AMBIL RECURSIVE MUTEX
     * 
     * Gunakan xSemaphoreTakeRecursive() bukan xSemaphoreTake()
     * Ini akan:
     * - Berhasil jika kita sudah pemilik (count bertambah)
     * - Block jika task lain yang punya
     */
    if (xSemaphoreTakeRecursive(uartMutex, portMAX_DELAY) == pdTRUE)
    {
        lockCount++;
        
        snprintf(buf, sizeof(buf), 
                 "    [NESTED depth=%u] Mutex acquired (lock count=%lu)\r\n",
                 depth, (unsigned long)lockCount);
        
        /* Print langsung ke UART karena kita sudah pegang mutex */
        HAL_UART_Transmit(&huart1, (uint8_t *)buf, strlen(buf), 100);
        
        /* Panggil diri sendiri (rekursif) */
        nestedFunction(depth - 1);
        
        /*
         * LEPASKAN RECURSIVE MUTEX
         * 
         * Gunakan xSemaphoreGiveRecursive() bukan xSemaphoreGive()
         * Ini akan mengurangi count. Mutex baru benar-benar bebas
         * jika count = 0.
         */
        lockCount--;
        snprintf(buf, sizeof(buf), 
                 "    [NESTED depth=%u] Mutex released (lock count=%lu)\r\n",
                 depth, (unsigned long)lockCount);
        HAL_UART_Transmit(&huart1, (uint8_t *)buf, strlen(buf), 100);
        
        xSemaphoreGiveRecursive(uartMutex);
    }
}

/* ===========================================================================
 * SAFE PRINT FUNCTION
 * ===========================================================================
 * 
 * Fungsi wrapper untuk mencetak ke UART dengan proteksi recursive mutex.
 * Bisa dipanggil dari fungsi yang sudah memegang mutex (nested call).
 */
static void safePrint(const char *msg)
{
    if (xSemaphoreTakeRecursive(uartMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), 100);
        xSemaphoreGiveRecursive(uartMutex);
    }
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
