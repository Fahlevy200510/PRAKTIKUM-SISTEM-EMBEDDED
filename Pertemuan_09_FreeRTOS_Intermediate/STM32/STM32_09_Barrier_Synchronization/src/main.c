/**
 * ============================================================================
 * PROJECT: 15-Barrier_Synchronization
 * BOARD: STM32F103C8T6 (Blue Pill)
 * FRAMEWORK: STM32Cube HAL + FreeRTOS Native API
 * ============================================================================
 * 
 * DEMO BARRIER SYNCHRONIZATION:
 * =============================
 * 
 * Barrier adalah titik sinkronisasi dimana semua task harus tiba
 * sebelum salah satu bisa melanjutkan.
 * 
 * IMPLEMENTASI:
 * =============
 * Menggunakan xEventGroupSync() yang:
 * 1. Set bit milik task ini
 * 2. Tunggu sampai SEMUA bit ter-set
 * 3. Clear semua bit (siap untuk barrier berikutnya)
 * 
 * SKENARIO DEMO:
 * ==============
 * 
 *    Round 1:
 *    ────────
 *    Task A: Kerja 100ms ──► Tiba di barrier ──► Tunggu...
 *    Task B: Kerja 200ms ──► Tiba di barrier ──► Tunggu...
 *    Task C: Kerja 300ms ──► Tiba di barrier ──► SEMUA LEPAS!
 *                                                   │
 *    ◄──────────────────────────────────────────────┘
 *    
 *    Round 2: (waktu kerja berubah untuk variasi)
 *    ────────
 *    ...dan seterusnya...
 * 
 * OUTPUT YANG DIHARAPKAN:
 * =======================
 * - Task selesai kerja dalam waktu berbeda
 * - Task menunggu di barrier
 * - Semua task lepas BERSAMAAN setelah task terakhir tiba
 * 
 * ============================================================================
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include <stdio.h>
#include <string.h>

#include "FreeRTOSConfig.h"

/* ===========================================================================
 * VARIABEL GLOBAL
 * =========================================================================== */
UART_HandleTypeDef huart1;
EventGroupHandle_t barrierEventGroup;   /* Event group untuk barrier */
volatile uint32_t roundCounter = 0;      /* Counter putaran */

/* ===========================================================================
 * PROTOTIPE FUNGSI
 * =========================================================================== */
static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);

static void WorkerTaskA(void *argument);
static void WorkerTaskB(void *argument);
static void WorkerTaskC(void *argument);

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
     * BUAT EVENT GROUP
     * 
     * Event group adalah kumpulan bit yang bisa di-set/clear oleh task.
     * Digunakan untuk barrier dengan cara:
     * - Setiap task punya 1 bit
     * - Task set bit-nya saat tiba di barrier
     * - Task tunggu sampai SEMUA bit = 1
     */
    barrierEventGroup = xEventGroupCreate();
    if (barrierEventGroup == NULL)
    {
        Error_Handler();
    }
    
    /* Buat 3 worker task dengan prioritas sama */
    xTaskCreate(WorkerTaskA, "wrkA", WORKER_STACK, NULL, WORKER_PRIO, NULL);
    xTaskCreate(WorkerTaskB, "wrkB", WORKER_STACK, NULL, WORKER_PRIO, NULL);
    xTaskCreate(WorkerTaskC, "wrkC", WORKER_STACK, NULL, WORKER_PRIO, NULL);
    
    vTaskStartScheduler();
    
    while (1) {}
}

/* ===========================================================================
 * WORKER TASK A
 * ===========================================================================
 * 
 * Task dengan waktu kerja TERCEPAT.
 * Akan tiba di barrier duluan dan menunggu yang lain.
 */
static void WorkerTaskA(void *argument)
{
    char buf[80];
    EventBits_t result;
    uint16_t workTime = 100;    /* Waktu kerja awal */
    
    vTaskDelay(pdMS_TO_TICKS(50)); /* Tunggu semua task siap */
    printLine("\r\n=== BARRIER SYNCHRONIZATION DEMO ===\r\n\r\n");
    
    for (;;)
    {
        roundCounter++;
        snprintf(buf, sizeof(buf), "\r\n--- ROUND %lu ---\r\n",
                 (unsigned long)roundCounter);
        printLine(buf);
        
        /* Simulasi kerja */
        snprintf(buf, sizeof(buf), "[A] Mulai kerja (%u ms)...\r\n", workTime);
        printLine(buf);
        vTaskDelay(pdMS_TO_TICKS(workTime));
        
        printLine("[A] Selesai! Menunggu di barrier...\r\n");
        
        /*
         * BARRIER: xEventGroupSync()
         * 
         * Parameter:
         * - barrierEventGroup: Event group handle
         * - BARRIER_BIT_TASK_A: Bit yang akan di-SET oleh task ini
         * - BARRIER_ALL_BITS: Bit yang harus di-tunggu (semua task)
         * - portMAX_DELAY: Tunggu selamanya
         * 
         * Fungsi ini akan:
         * 1. Set bit BARRIER_BIT_TASK_A
         * 2. Tunggu sampai BARRIER_ALL_BITS ter-set
         * 3. OTOMATIS clear BARRIER_ALL_BITS setelah semua tiba
         */
        result = xEventGroupSync(barrierEventGroup,
                                 BARRIER_BIT_TASK_A,
                                 BARRIER_ALL_BITS,
                                 portMAX_DELAY);
        
        if ((result & BARRIER_ALL_BITS) == BARRIER_ALL_BITS)
        {
            printLine("[A] >>> BARRIER RELEASED! Melanjutkan... <<<\r\n");
            toggleLed();
        }
        
        /* Variasikan waktu kerja untuk putaran berikutnya */
        workTime = 150 + (roundCounter * 50) % 200;
        
        vTaskDelay(pdMS_TO_TICKS(500)); /* Jeda antar putaran */
    }
}

/* ===========================================================================
 * WORKER TASK B
 * ===========================================================================
 * 
 * Task dengan waktu kerja MENENGAH.
 */
static void WorkerTaskB(void *argument)
{
    char buf[80];
    EventBits_t result;
    uint16_t workTime = 200;    /* Waktu kerja awal */
    
    vTaskDelay(pdMS_TO_TICKS(50));
    
    for (;;)
    {
        /* Simulasi kerja */
        snprintf(buf, sizeof(buf), "[B] Mulai kerja (%u ms)...\r\n", workTime);
        printLine(buf);
        vTaskDelay(pdMS_TO_TICKS(workTime));
        
        printLine("[B] Selesai! Menunggu di barrier...\r\n");
        
        /* BARRIER */
        result = xEventGroupSync(barrierEventGroup,
                                 BARRIER_BIT_TASK_B,
                                 BARRIER_ALL_BITS,
                                 portMAX_DELAY);
        
        if ((result & BARRIER_ALL_BITS) == BARRIER_ALL_BITS)
        {
            printLine("[B] >>> BARRIER RELEASED! Melanjutkan... <<<\r\n");
        }
        
        /* Variasikan waktu kerja */
        workTime = 100 + (roundCounter * 30) % 250;
        
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* ===========================================================================
 * WORKER TASK C
 * ===========================================================================
 * 
 * Task dengan waktu kerja TERLAMA.
 * Biasanya task terakhir yang tiba di barrier.
 */
static void WorkerTaskC(void *argument)
{
    char buf[80];
    EventBits_t result;
    uint16_t workTime = 300;    /* Waktu kerja awal */
    
    vTaskDelay(pdMS_TO_TICKS(50));
    
    for (;;)
    {
        /* Simulasi kerja */
        snprintf(buf, sizeof(buf), "[C] Mulai kerja (%u ms)...\r\n", workTime);
        printLine(buf);
        vTaskDelay(pdMS_TO_TICKS(workTime));
        
        printLine("[C] Selesai! Menunggu di barrier...\r\n");
        
        /* BARRIER */
        result = xEventGroupSync(barrierEventGroup,
                                 BARRIER_BIT_TASK_C,
                                 BARRIER_ALL_BITS,
                                 portMAX_DELAY);
        
        if ((result & BARRIER_ALL_BITS) == BARRIER_ALL_BITS)
        {
            printLine("[C] >>> BARRIER RELEASED! Melanjutkan... <<<\r\n");
        }
        
        /* Variasikan waktu kerja */
        workTime = 200 + (roundCounter * 40) % 300;
        
        vTaskDelay(pdMS_TO_TICKS(500));
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
