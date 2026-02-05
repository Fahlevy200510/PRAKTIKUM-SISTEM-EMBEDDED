/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 20-Software_Watchdog
 * 
 * JUDUL: Software Watchdog untuk Monitoring Task Health
 * 
 * DESKRIPSI:
 * Implementasi sistem Software Watchdog yang memantau kesehatan setiap task.
 * Jika ada task yang tidak "check in" dalam waktu yang ditentukan,
 * sistem akan mendeteksi dan mengambil tindakan recovery.
 * 
 * ============================================================================
 * MENGAPA PERLU SOFTWARE WATCHDOG?
 * ============================================================================
 * 
 *    Hardware Watchdog Limitations:
 *    ──────────────────────────────
 *    - Hanya 1 watchdog untuk seluruh sistem
 *    - Jika main loop berjalan tapi 1 task hang, tidak terdeteksi
 *    - Reset seluruh sistem (tidak granular)
 *    
 *    Software Watchdog Benefits:
 *    ───────────────────────────
 *    - Monitor setiap task secara individual
 *    - Bisa me-restart hanya task yang bermasalah
 *    - Logging detail untuk debugging
 *    - Gradual recovery (coba restart dulu, baru reset)
 * 
 * ============================================================================
 * ARSITEKTUR SOFTWARE WATCHDOG
 * ============================================================================
 * 
 *    ┌────────────────────────────────────────────────────────────────────┐
 *    │                     SOFTWARE WATCHDOG SYSTEM                       │
 *    ├────────────────────────────────────────────────────────────────────┤
 *    │                                                                    │
 *    │   ┌─────────────────────────────────────────────────────────────┐  │
 *    │   │              Watchdog Registry (Array)                     │  │
 *    │   ├───────────────────┬───────────────────┬────────────────────┤  │
 *    │   │ Task A            │ Task B            │ Task C             │  │
 *    │   │ Handle: 0x2001000 │ Handle: 0x2001200 │ Handle: 0x2001400  │  │
 *    │   │ Timeout: 3000ms   │ Timeout: 5000ms   │ Timeout: 2000ms    │  │
 *    │   │ LastCheck: 1234   │ LastCheck: 1235   │ LastCheck: 890     │  │
 *    │   │ Status: HEALTHY   │ Status: HEALTHY   │ Status: TIMEOUT!   │  │
 *    │   └───────────────────┴───────────────────┴────────────────────┘  │
 *    │                                ▲                                  │
 *    │                                │                                  │
 *    │   ┌─────────────────────────────────────────────────────────────┐  │
 *    │   │              Watchdog Timer (1 second period)              │  │
 *    │   │                                                             │  │
 *    │   │  for each registered task:                                  │  │
 *    │   │    if (now - lastCheckIn > timeout)                         │  │
 *    │   │      → Mark as TIMEOUT                                      │  │
 *    │   │      → Call recovery handler                                │  │
 *    │   └─────────────────────────────────────────────────────────────┘  │
 *    │                                                                    │
 *    └────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * ALUR KERJA
 * ============================================================================
 * 
 *    INITIALIZATION:
 *    ───────────────
 *    1. Buat watchdog timer (auto-reload)
 *    2. Register task yang mau dimonitor
 *    3. Start watchdog timer
 *    
 *    NORMAL OPERATION:
 *    ─────────────────
 *    
 *    Task Loop:                    WDG Timer:
 *    ┌───────────────────┐         ┌──────────────────────┐
 *    │ while(1) {        │         │ Callback:            │
 *    │   // Do work      │         │   for each task:     │
 *    │   ...             │         │     check lastTime   │
 *    │   WDG_CheckIn();  │────────►│     if OK: continue  │
 *    │   ...             │         │     if FAIL: recover │
 *    │ }                 │         │                      │
 *    └───────────────────┘         └──────────────────────┘
 *    
 *    FAULT SCENARIO:
 *    ────────────────
 *    
 *    Time:   0      1s      2s      3s      4s      5s
 *            │       │       │       │       │       │
 *    Task:   ████████████████▓▓▓▓▓▓▓▓ (HANG di sini)
 *            check   check   │       │       │
 *            in      in      └──── Tidak check in lagi!
 *            
 *    WDG:         check   check   check
 *                 OK      OK      TIMEOUT!
 *                                 │
 *                                 └──► Recovery action
 * 
 * ============================================================================
 * RECOVERY STRATEGIES
 * ============================================================================
 * 
 *    Level 1: Log & Notify
 *    ─────────────────────
 *    - Print warning ke UART
 *    - Set flag untuk monitoring external
 *    
 *    Level 2: Task Restart
 *    ─────────────────────
 *    - Delete task yang hang
 *    - Re-create task baru
 *    - Reset state lokal
 *    
 *    Level 3: System Reset
 *    ─────────────────────
 *    - Trigger Hardware Watchdog
 *    - Atau NVIC_SystemReset()
 *    - Last resort!
 * 
 * ============================================================================
 * EXPECTED OUTPUT
 * ============================================================================
 * 
 *    === Software Watchdog Demo ===
 *    
 *    [Task1] Working... Check in to WDG
 *    [Task2] Working... Check in to WDG
 *    [Task3] Working... Check in to WDG
 *    [WDG] All tasks healthy (3/3)
 *    
 *    [Task1] Working... Check in to WDG
 *    [Task2] Working... Check in to WDG
 *    [Task3] Simulating HANG - no more check-ins!
 *    
 *    [WDG] All tasks healthy (3/3)
 *    [Task1] Working... Check in to WDG
 *    [Task2] Working... Check in to WDG
 *    
 *    [WDG] WARNING: Task 'Task3' TIMEOUT! Last seen 3500ms ago
 *    [WDG] Attempting task restart...
 *    [WDG] Task 'Task3' restarted successfully
 * 
 * ============================================================================
 */

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * SOFTWARE WATCHDOG DATA STRUCTURES
 * ============================================================================ */

/**
 * Status kesehatan task yang dimonitor
 */
typedef enum {
    WDG_STATUS_UNKNOWN = 0,     /* Belum pernah check in */
    WDG_STATUS_HEALTHY,         /* Task aktif dan sehat */
    WDG_STATUS_WARNING,         /* Mendekati timeout */
    WDG_STATUS_TIMEOUT,         /* Task hang! */
    WDG_STATUS_RESTARTING       /* Sedang di-restart */
} WDG_Status_t;

/**
 * Entry untuk setiap task yang dimonitor
 */
typedef struct {
    TaskHandle_t    xTaskHandle;        /* Handle task */
    TaskFunction_t  pxTaskCode;         /* Pointer ke function task (untuk restart) */
    const char      *pcTaskName;        /* Nama task */
    void            *pvParameters;      /* Parameter task */
    uint16_t        usStackDepth;       /* Stack size */
    UBaseType_t     uxPriority;         /* Priority */
    TickType_t      xTimeoutTicks;      /* Timeout dalam ticks */
    TickType_t      xLastCheckInTime;   /* Waktu check in terakhir */
    WDG_Status_t    eStatus;            /* Status saat ini */
    uint32_t        ulTimeoutCount;     /* Counter timeout (untuk statistik) */
} WDG_TaskEntry_t;

/**
 * Registry untuk semua task yang dimonitor
 */
typedef struct {
    WDG_TaskEntry_t xTasks[WDG_MAX_MONITORED_TASKS];
    uint8_t         ucTaskCount;
    TimerHandle_t   xWdgTimer;
} WDG_Registry_t;

/* ============================================================================
 * GLOBAL VARIABLES
 * ============================================================================ */

static WDG_Registry_t xWdgRegistry = {0};

static UART_HandleTypeDef huart1;

/* Flag untuk simulasi hang */
static volatile BaseType_t xSimulateHang = pdFALSE;
static volatile uint32_t ulHangAfterSeconds = 8;

/* ============================================================================
 * PROTOTYPE
 * ============================================================================ */

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART_Init(void);
static void UART_SendString(const char *str);

/* Watchdog API */
static BaseType_t WDG_Init(void);
static BaseType_t WDG_RegisterTask(TaskHandle_t xTaskHandle, 
                                   TaskFunction_t pxTaskCode,
                                   const char *pcName,
                                   void *pvParams,
                                   uint16_t usStack,
                                   UBaseType_t uxPrio,
                                   TickType_t xTimeoutMs);
static void WDG_CheckIn(TaskHandle_t xTaskHandle);
static void WDG_Start(void);
static void prvWdgTimerCallback(TimerHandle_t xTimer);
static void prvRecoverTask(WDG_TaskEntry_t *pxEntry);

/* Worker tasks */
static void vTask1(void *pvParameters);
static void vTask2(void *pvParameters);
static void vTask3_Unreliable(void *pvParameters);

/* ============================================================================
 * WATCHDOG API IMPLEMENTATION
 * ============================================================================ */

/**
 * @brief Inisialisasi sistem watchdog
 * 
 * ILUSTRASI:
 * ─────────────────────────────────────────────────────────────
 *    
 *    WDG_Init():
 *    ┌─────────────────────────────────────────────────────────┐
 *    │ 1. Clear registry                                       │
 *    │ 2. Create timer with WDG_CHECK_PERIOD_MS               │
 *    │ 3. Return handle (timer belum start)                    │
 *    └─────────────────────────────────────────────────────────┘
 *    
 * ─────────────────────────────────────────────────────────────
 */
static BaseType_t WDG_Init(void)
{
    memset(&xWdgRegistry, 0, sizeof(xWdgRegistry));
    
    /* Buat timer auto-reload untuk periodic check */
    xWdgRegistry.xWdgTimer = xTimerCreate(
        "WDG_Timer",
        pdMS_TO_TICKS(WDG_CHECK_PERIOD_MS),
        pdTRUE,                     /* Auto-reload */
        NULL,                       /* Timer ID */
        prvWdgTimerCallback         /* Callback */
    );
    
    if(xWdgRegistry.xWdgTimer == NULL)
    {
        UART_SendString("[WDG] ERROR: Failed to create watchdog timer!\r\n");
        return pdFAIL;
    }
    
    UART_SendString("[WDG] Watchdog system initialized\r\n");
    return pdPASS;
}

/**
 * @brief Mendaftarkan task ke watchdog monitor
 * 
 * ILUSTRASI:
 * ─────────────────────────────────────────────────────────────
 *    
 *    Before Register:
 *    ┌─────────┬─────────┬─────────┬─────────┐
 *    │ (empty) │ (empty) │ (empty) │ (empty) │
 *    └─────────┴─────────┴─────────┴─────────┘
 *    
 *    After WDG_RegisterTask(TaskA, ...):
 *    ┌─────────┬─────────┬─────────┬─────────┐
 *    │ Task A  │ (empty) │ (empty) │ (empty) │
 *    │ 3000ms  │         │         │         │
 *    └─────────┴─────────┴─────────┴─────────┘
 *    
 *    After WDG_RegisterTask(TaskB, ...):
 *    ┌─────────┬─────────┬─────────┬─────────┐
 *    │ Task A  │ Task B  │ (empty) │ (empty) │
 *    │ 3000ms  │ 5000ms  │         │         │
 *    └─────────┴─────────┴─────────┴─────────┘
 *    
 * ─────────────────────────────────────────────────────────────
 */
static BaseType_t WDG_RegisterTask(TaskHandle_t xTaskHandle,
                                   TaskFunction_t pxTaskCode,
                                   const char *pcName,
                                   void *pvParams,
                                   uint16_t usStack,
                                   UBaseType_t uxPrio,
                                   TickType_t xTimeoutMs)
{
    if(xWdgRegistry.ucTaskCount >= WDG_MAX_MONITORED_TASKS)
    {
        UART_SendString("[WDG] ERROR: Registry full!\r\n");
        return pdFAIL;
    }
    
    WDG_TaskEntry_t *pxEntry = &xWdgRegistry.xTasks[xWdgRegistry.ucTaskCount];
    
    pxEntry->xTaskHandle = xTaskHandle;
    pxEntry->pxTaskCode = pxTaskCode;
    pxEntry->pcTaskName = pcName;
    pxEntry->pvParameters = pvParams;
    pxEntry->usStackDepth = usStack;
    pxEntry->uxPriority = uxPrio;
    pxEntry->xTimeoutTicks = pdMS_TO_TICKS(xTimeoutMs);
    pxEntry->xLastCheckInTime = xTaskGetTickCount();
    pxEntry->eStatus = WDG_STATUS_UNKNOWN;
    pxEntry->ulTimeoutCount = 0;
    
    xWdgRegistry.ucTaskCount++;
    
    char buffer[80];
    snprintf(buffer, sizeof(buffer), 
            "[WDG] Registered '%s' (timeout: %lums)\r\n", 
            pcName, xTimeoutMs);
    UART_SendString(buffer);
    
    return pdPASS;
}

/**
 * @brief Task check in ke watchdog (tanda masih hidup)
 * 
 * PENTING: Setiap task yang dimonitor HARUS memanggil ini
 *          secara periodik, minimal sekali per timeout period.
 * 
 * ILUSTRASI:
 * ─────────────────────────────────────────────────────────────
 *    
 *    Task Loop:
 *    ┌──────────────────────────────────────────────────────────┐
 *    │ while(1) {                                               │
 *    │     // Kerjakan sesuatu...                               │
 *    │     process_data();                                      │
 *    │                                                          │
 *    │     // CHECK IN ke watchdog!                             │
 *    │     WDG_CheckIn(xTaskGetCurrentTaskHandle());            │
 *    │                                                          │
 *    │     // Delay atau wait sebelum loop berikutnya           │
 *    │     vTaskDelay(pdMS_TO_TICKS(1000));                    │
 *    │ }                                                        │
 *    └──────────────────────────────────────────────────────────┘
 *    
 * ─────────────────────────────────────────────────────────────
 */
static void WDG_CheckIn(TaskHandle_t xTaskHandle)
{
    taskENTER_CRITICAL();
    
    for(uint8_t i = 0; i < xWdgRegistry.ucTaskCount; i++)
    {
        if(xWdgRegistry.xTasks[i].xTaskHandle == xTaskHandle)
        {
            xWdgRegistry.xTasks[i].xLastCheckInTime = xTaskGetTickCount();
            xWdgRegistry.xTasks[i].eStatus = WDG_STATUS_HEALTHY;
            break;
        }
    }
    
    taskEXIT_CRITICAL();
}

/**
 * @brief Start watchdog timer
 */
static void WDG_Start(void)
{
    if(xWdgRegistry.xWdgTimer != NULL)
    {
        xTimerStart(xWdgRegistry.xWdgTimer, 0);
        UART_SendString("[WDG] Watchdog monitoring STARTED\r\n\r\n");
    }
}

/**
 * @brief Callback timer watchdog - dipanggil periodik
 * 
 * TUGAS:
 * 1. Cek waktu sejak terakhir check in untuk setiap task
 * 2. Jika melebihi timeout → tandai TIMEOUT dan recover
 * 3. Update status setiap task
 */
static void prvWdgTimerCallback(TimerHandle_t xTimer)
{
    (void)xTimer;
    
    TickType_t xNow = xTaskGetTickCount();
    uint8_t ucHealthyCount = 0;
    char buffer[120];
    
    for(uint8_t i = 0; i < xWdgRegistry.ucTaskCount; i++)
    {
        WDG_TaskEntry_t *pxEntry = &xWdgRegistry.xTasks[i];
        TickType_t xElapsed = xNow - pxEntry->xLastCheckInTime;
        
        if(pxEntry->eStatus == WDG_STATUS_RESTARTING)
        {
            /* Masih dalam proses restart, skip */
            continue;
        }
        
        if(xElapsed > pxEntry->xTimeoutTicks)
        {
            /* TIMEOUT detected! */
            pxEntry->eStatus = WDG_STATUS_TIMEOUT;
            pxEntry->ulTimeoutCount++;
            
            uint32_t elapsedMs = (xElapsed * 1000) / configTICK_RATE_HZ;
            snprintf(buffer, sizeof(buffer),
                    "[WDG] WARNING: Task '%s' TIMEOUT! Last seen %lums ago\r\n",
                    pxEntry->pcTaskName, elapsedMs);
            UART_SendString(buffer);
            
            /* Attempt recovery */
            prvRecoverTask(pxEntry);
        }
        else if(xElapsed > (pxEntry->xTimeoutTicks / 2))
        {
            /* Warning - approaching timeout */
            pxEntry->eStatus = WDG_STATUS_WARNING;
        }
        else
        {
            pxEntry->eStatus = WDG_STATUS_HEALTHY;
            ucHealthyCount++;
        }
    }
    
    /* Toggle LED setiap check (heartbeat) */
    HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
    
    /* Periodic status report */
    static uint8_t ucReportCounter = 0;
    ucReportCounter++;
    if(ucReportCounter >= 5) /* Tiap 5 detik */
    {
        ucReportCounter = 0;
        snprintf(buffer, sizeof(buffer),
                "[WDG] Status: %u/%u tasks healthy\r\n",
                ucHealthyCount, xWdgRegistry.ucTaskCount);
        UART_SendString(buffer);
    }
}

/**
 * @brief Recovery handler untuk task yang timeout
 * 
 * STRATEGI RECOVERY:
 * ─────────────────────────────────────────────────────────────
 *    
 *    ┌──────────────────────────────────────────────────────────┐
 *    │ 1. Log error detail                                      │
 *    │ 2. Mark task sebagai RESTARTING                          │
 *    │ 3. Delete task yang hang                                 │
 *    │ 4. Re-create task dengan parameter yang sama            │
 *    │ 5. Update registry dengan handle baru                    │
 *    │ 6. Mark sebagai HEALTHY lagi                            │
 *    └──────────────────────────────────────────────────────────┘
 *    
 *    CATATAN: Jika timeout count terlalu tinggi,
 *             pertimbangkan system reset!
 * 
 * ─────────────────────────────────────────────────────────────
 */
static void prvRecoverTask(WDG_TaskEntry_t *pxEntry)
{
    char buffer[100];
    
    /* Log recovery attempt */
    snprintf(buffer, sizeof(buffer),
            "[WDG] Attempting to restart '%s'... (timeout #%lu)\r\n",
            pxEntry->pcTaskName, pxEntry->ulTimeoutCount);
    UART_SendString(buffer);
    
    /* Check jika sudah terlalu banyak timeout */
    if(pxEntry->ulTimeoutCount >= 3)
    {
        UART_SendString("[WDG] CRITICAL: Too many timeouts! Consider system reset.\r\n");
        /* Di production, bisa trigger NVIC_SystemReset() di sini */
    }
    
    pxEntry->eStatus = WDG_STATUS_RESTARTING;
    
    /* Delete task yang hang */
    if(pxEntry->xTaskHandle != NULL)
    {
        vTaskDelete(pxEntry->xTaskHandle);
        pxEntry->xTaskHandle = NULL;
    }
    
    /* Re-create task */
    TaskHandle_t xNewHandle;
    BaseType_t xResult = xTaskCreate(
        pxEntry->pxTaskCode,
        pxEntry->pcTaskName,
        pxEntry->usStackDepth,
        pxEntry->pvParameters,
        pxEntry->uxPriority,
        &xNewHandle
    );
    
    if(xResult == pdPASS)
    {
        pxEntry->xTaskHandle = xNewHandle;
        pxEntry->xLastCheckInTime = xTaskGetTickCount();
        pxEntry->eStatus = WDG_STATUS_HEALTHY;
        
        snprintf(buffer, sizeof(buffer),
                "[WDG] Task '%s' restarted successfully!\r\n\r\n",
                pxEntry->pcTaskName);
        UART_SendString(buffer);
    }
    else
    {
        snprintf(buffer, sizeof(buffer),
                "[WDG] ERROR: Failed to restart '%s'!\r\n",
                pxEntry->pcTaskName);
        UART_SendString(buffer);
    }
}

/* ============================================================================
 * WORKER TASKS
 * ============================================================================ */

/**
 * @brief Task 1 - Worker normal yang selalu check in
 */
static void vTask1(void *pvParameters)
{
    (void)pvParameters;
    
    TaskHandle_t xMyHandle = xTaskGetCurrentTaskHandle();
    
    for(;;)
    {
        UART_SendString("[Task1] Working... Check in to WDG\r\n");
        
        /* DO WORK HERE */
        
        /* CHECK IN ke watchdog */
        WDG_CheckIn(xMyHandle);
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
 * @brief Task 2 - Worker normal dengan periode berbeda
 */
static void vTask2(void *pvParameters)
{
    (void)pvParameters;
    
    TaskHandle_t xMyHandle = xTaskGetCurrentTaskHandle();
    
    for(;;)
    {
        UART_SendString("[Task2] Working... Check in to WDG\r\n");
        
        /* DO WORK HERE */
        
        /* CHECK IN ke watchdog */
        WDG_CheckIn(xMyHandle);
        
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

/**
 * @brief Task 3 - Unreliable task yang akan HANG!
 * 
 * Setelah beberapa detik, task ini berhenti check in
 * untuk simulasi hang/deadlock.
 */
static void vTask3_Unreliable(void *pvParameters)
{
    (void)pvParameters;
    
    TaskHandle_t xMyHandle = xTaskGetCurrentTaskHandle();
    TickType_t xStartTime = xTaskGetTickCount();
    
    for(;;)
    {
        TickType_t xElapsed = xTaskGetTickCount() - xStartTime;
        
        if(xElapsed < pdMS_TO_TICKS(ulHangAfterSeconds * 1000))
        {
            /* Masih normal - check in */
            UART_SendString("[Task3] Working... Check in to WDG\r\n");
            WDG_CheckIn(xMyHandle);
        }
        else
        {
            /* SIMULASI HANG - tidak check in! */
            if(!xSimulateHang)
            {
                xSimulateHang = pdTRUE;
                UART_SendString("\r\n[Task3] >>> SIMULATING HANG - NO MORE CHECK-INS! <<<\r\n\r\n");
            }
            
            /* 
             * Di sini kita delay lama tanpa check in.
             * Watchdog akan mendeteksi dan me-restart task.
             * 
             * Setelah restart, xStartTime akan di-reset
             * karena task dimulai dari awal.
             */
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
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
    UART_SendString("20-Software_Watchdog\r\n");
    UART_SendString("STM32F103 + FreeRTOS\r\n");
    UART_SendString("================================\r\n\r\n");
    
    /* Initialize watchdog system */
    WDG_Init();
    
    /* Create worker tasks */
    TaskHandle_t xTask1Handle, xTask2Handle, xTask3Handle;
    
    xTaskCreate(vTask1, "Task1", WORKER_TASK_STACK, NULL, 
                WORKER_TASK_PRIO, &xTask1Handle);
    
    xTaskCreate(vTask2, "Task2", WORKER_TASK_STACK, NULL, 
                WORKER_TASK_PRIO, &xTask2Handle);
    
    xTaskCreate(vTask3_Unreliable, "Task3", WORKER_TASK_STACK, NULL, 
                WORKER_TASK_PRIO, &xTask3Handle);
    
    /* Register tasks ke watchdog dengan timeout berbeda */
    WDG_RegisterTask(xTask1Handle, vTask1, "Task1", NULL, 
                     WORKER_TASK_STACK, WORKER_TASK_PRIO, 3000);
    
    WDG_RegisterTask(xTask2Handle, vTask2, "Task2", NULL,
                     WORKER_TASK_STACK, WORKER_TASK_PRIO, 4000);
    
    WDG_RegisterTask(xTask3Handle, vTask3_Unreliable, "Task3", NULL,
                     WORKER_TASK_STACK, WORKER_TASK_PRIO, 3000);
    
    /* Start watchdog monitoring */
    WDG_Start();
    
    vTaskStartScheduler();
    
    for(;;);
}
