/**
 * ============================================================================
 * PROJECT: 13-Priority_Inversion_Fix
 * BOARD: STM32F103C8T6 (Blue Pill)
 * FRAMEWORK: STM32Cube HAL + FreeRTOS Native API
 * ============================================================================
 * 
 * DEMO PRIORITY INVERSION & PRIORITY INHERITANCE:
 * ================================================
 * 
 * Project ini mendemonstrasikan:
 * 1. Bagaimana priority inversion BISA terjadi
 * 2. Bagaimana FreeRTOS mutex OTOMATIS memperbaikinya dengan priority inheritance
 * 
 * SETUP:
 * ======
 * - Task L (Low priority): Mengambil mutex, bekerja lama
 * - Task M (Medium priority): Tidak butuh mutex, selalu siap jalan
 * - Task H (High priority): Butuh mutex yang dipegang Task L
 * 
 * TANPA PRIORITY INHERITANCE (Binary Semaphore):
 * ==============================================
 * 
 *    1. Task L ambil mutex, mulai bekerja
 *    2. Task H bangun, butuh mutex, ter-block
 *    3. Task M bangun, preempt Task L!
 *    4. Task H HARUS MENUNGGU Task M selesai + Task L selesai
 *    5. Ini BURUK! Task H prioritas tertinggi tapi paling lama menunggu
 * 
 * DENGAN PRIORITY INHERITANCE (Mutex):
 * ====================================
 * 
 *    1. Task L ambil mutex, mulai bekerja
 *    2. Task H bangun, butuh mutex, ter-block
 *    3. FreeRTOS NAIKKAN prioritas Task L = Task H
 *    4. Task M TIDAK BISA preempt Task L
 *    5. Task L selesai, lepas mutex, prioritas kembali normal
 *    6. Task H LANGSUNG jalan
 * 
 * OUTPUT YANG DIHARAPKAN:
 * =======================
 * Perhatikan log "Priority inherited" dan "Priority restored"
 * yang menunjukkan priority inheritance bekerja.
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
SemaphoreHandle_t sharedMutex;      /* Mutex dengan priority inheritance */
volatile uint32_t demoRound = 0;    /* Penghitung putaran demo */

/* Task handles untuk monitoring prioritas */
TaskHandle_t hTaskLow = NULL;
TaskHandle_t hTaskMed = NULL;
TaskHandle_t hTaskHigh = NULL;

/* ===========================================================================
 * PROTOTIPE FUNGSI
 * =========================================================================== */
static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);

static void TaskLow(void *argument);
static void TaskMedium(void *argument);
static void TaskHigh(void *argument);

static void safePrint(const char *msg);
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
     * BUAT MUTEX (bukan binary semaphore!)
     * 
     * Perbedaan penting:
     * - xSemaphoreCreateBinary() - TIDAK ada priority inheritance
     * - xSemaphoreCreateMutex()  - ADA priority inheritance (OTOMATIS!)
     * 
     * FreeRTOS mutex secara otomatis mengimplementasi priority inheritance.
     */
    sharedMutex = xSemaphoreCreateMutex();
    if (sharedMutex == NULL)
    {
        Error_Handler();
    }
    
    /*
     * Buat task dengan prioritas berbeda.
     * Simpan handle untuk monitoring prioritas.
     */
    xTaskCreate(TaskLow,    "Low",  TASK_LOW_STACK,  NULL, TASK_LOW_PRIO,  &hTaskLow);
    xTaskCreate(TaskMedium, "Med",  TASK_MED_STACK,  NULL, TASK_MED_PRIO,  &hTaskMed);
    xTaskCreate(TaskHigh,   "High", TASK_HIGH_STACK, NULL, TASK_HIGH_PRIO, &hTaskHigh);
    
    vTaskStartScheduler();
    
    while (1) {}
}

/* ===========================================================================
 * TASK LOW PRIORITY
 * ===========================================================================
 * 
 * Task ini:
 * 1. Mengambil mutex
 * 2. Bekerja "lama" (simulasi dengan delay)
 * 3. Melepas mutex
 * 
 * Saat Task High menunggu mutex yang dipegang Task Low:
 * - Priority Task Low akan DINAIKKAN oleh FreeRTOS
 * - Ini terlihat dari log prioritas
 */
static void TaskLow(void *argument)
{
    char buf[100];
    UBaseType_t originalPrio, currentPrio;
    
    vTaskDelay(pdMS_TO_TICKS(100)); /* Tunggu semua task siap */
    
    safePrint("\r\n=== PRIORITY INVERSION DEMO ===\r\n");
    safePrint("Mutex FreeRTOS memiliki Priority Inheritance OTOMATIS\r\n\r\n");
    
    for (;;)
    {
        demoRound++;
        snprintf(buf, sizeof(buf), "--- ROUND %lu ---\r\n", (unsigned long)demoRound);
        safePrint(buf);
        
        /* Simpan prioritas asli */
        originalPrio = uxTaskPriorityGet(NULL);
        
        snprintf(buf, sizeof(buf), "[LOW] Mengambil mutex (prio=%lu)\r\n",
                 (unsigned long)originalPrio);
        safePrint(buf);
        
        /*
         * AMBIL MUTEX
         * 
         * Setelah ini, jika Task High menunggu mutex:
         * - FreeRTOS akan menaikkan prioritas Task Low
         * - Task Medium TIDAK bisa preempt
         */
        if (xSemaphoreTake(sharedMutex, portMAX_DELAY) == pdTRUE)
        {
            safePrint("[LOW] Mutex diperoleh, mulai bekerja...\r\n");
            
            /*
             * SIMULASI KERJA LAMA
             * 
             * Selama delay ini:
             * - Task High akan bangun dan butuh mutex
             * - Task Medium akan bangun dan siap jalan
             * 
             * TANPA priority inheritance: Task M preempt Task L
             * DENGAN priority inheritance: Task L "naik pangkat"
             */
            vTaskDelay(pdMS_TO_TICKS(RESOURCE_HOLD_TIME_MS / 2));
            
            /* Cek apakah prioritas sudah berubah (inheritance) */
            currentPrio = uxTaskPriorityGet(NULL);
            if (currentPrio != originalPrio)
            {
                snprintf(buf, sizeof(buf), 
                         "[LOW] *** PRIORITY INHERITED! %lu -> %lu ***\r\n",
                         (unsigned long)originalPrio, (unsigned long)currentPrio);
                safePrint(buf);
            }
            
            vTaskDelay(pdMS_TO_TICKS(RESOURCE_HOLD_TIME_MS / 2));
            
            /* Cek prioritas lagi sebelum lepas mutex */
            currentPrio = uxTaskPriorityGet(NULL);
            snprintf(buf, sizeof(buf), "[LOW] Selesai bekerja (prio=%lu)\r\n",
                     (unsigned long)currentPrio);
            safePrint(buf);
            
            /*
             * LEPAS MUTEX
             * 
             * Setelah ini:
             * - Prioritas Task Low kembali ke original
             * - Task High langsung dijalankan (preempt)
             */
            xSemaphoreGive(sharedMutex);
            
            /* Cek prioritas setelah lepas mutex */
            currentPrio = uxTaskPriorityGet(NULL);
            if (currentPrio == originalPrio)
            {
                snprintf(buf, sizeof(buf), 
                         "[LOW] Priority restored ke %lu\r\n",
                         (unsigned long)currentPrio);
                safePrint(buf);
            }
        }
        
        /* Tunggu untuk putaran berikutnya */
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* ===========================================================================
 * TASK MEDIUM PRIORITY
 * ===========================================================================
 * 
 * Task ini TIDAK menggunakan mutex.
 * 
 * TANPA priority inheritance:
 * - Task M bisa preempt Task L yang sedang pegang mutex
 * - Task H harus menunggu Task M selesai + Task L selesai
 * 
 * DENGAN priority inheritance:
 * - Task L sudah "naik pangkat" = Task H
 * - Task M TIDAK bisa preempt Task L
 * - Task H langsung jalan setelah Task L selesai
 */
static void TaskMedium(void *argument)
{
    char buf[80];
    
    vTaskDelay(pdMS_TO_TICKS(150)); /* Mulai setelah Task Low ambil mutex */
    
    for (;;)
    {
        safePrint("[MED] Siap jalan (tidak butuh mutex)\r\n");
        
        /*
         * Simulasi kerja CPU-bound.
         * 
         * TANPA priority inheritance: Ini akan delay Task H
         * DENGAN priority inheritance: Task L selesai dulu, Task H jalan
         */
        safePrint("[MED] Bekerja sebentar...\r\n");
        vTaskDelay(pdMS_TO_TICKS(50));
        safePrint("[MED] Selesai\r\n");
        
        vTaskDelay(pdMS_TO_TICKS(800));
    }
}

/* ===========================================================================
 * TASK HIGH PRIORITY
 * ===========================================================================
 * 
 * Task ini membutuhkan mutex yang sama dengan Task Low.
 * 
 * Ketika Task High mencoba mengambil mutex yang dipegang Task Low:
 * - FreeRTOS mendeteksi priority inversion
 * - Prioritas Task Low DINAIKKAN = Task High
 * - Ini mencegah Task Medium preempt Task Low
 */
static void TaskHigh(void *argument)
{
    char buf[80];
    TickType_t startWait, endWait;
    
    vTaskDelay(pdMS_TO_TICKS(120)); /* Mulai setelah Task Low ambil mutex */
    
    for (;;)
    {
        safePrint("[HIGH] Butuh mutex, menunggu...\r\n");
        
        startWait = xTaskGetTickCount();
        
        /*
         * TUNGGU MUTEX
         * 
         * Saat xSemaphoreTake ini dipanggil dan mutex dipegang Task Low:
         * - FreeRTOS OTOMATIS menaikkan prioritas Task Low
         * - Ini terjadi di dalam kernel, transparan bagi kita
         */
        if (xSemaphoreTake(sharedMutex, portMAX_DELAY) == pdTRUE)
        {
            endWait = xTaskGetTickCount();
            
            snprintf(buf, sizeof(buf), 
                     "[HIGH] Mutex diperoleh! (tunggu %lu ms)\r\n",
                     (unsigned long)(endWait - startWait));
            safePrint(buf);
            
            toggleLed();
            
            /* Gunakan resource sebentar */
            vTaskDelay(pdMS_TO_TICKS(20));
            
            safePrint("[HIGH] Selesai, lepas mutex\r\n\r\n");
            xSemaphoreGive(sharedMutex);
        }
        
        vTaskDelay(pdMS_TO_TICKS(900));
    }
}

/* ===========================================================================
 * HELPER FUNCTIONS
 * =========================================================================== */

static void safePrint(const char *msg)
{
    /* Langsung print (untuk kesederhanaan demo) */
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
