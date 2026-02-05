/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 18-Notification_Event_Bits
 * 
 * JUDUL: Task Notification sebagai Event Bits (Event Flags)
 * 
 * DESKRIPSI:
 * Demo penggunaan Task Notification untuk menggantikan Event Group.
 * Setiap bit dalam notification value mewakili satu event berbeda.
 * Pendekatan ini lebih ringan daripada menggunakan Event Group object.
 * 
 * ============================================================================
 * PERBANDINGAN: EVENT GROUP vs NOTIFICATION EVENT BITS
 * ============================================================================
 * 
 *    FITUR                    EVENT GROUP       NOTIFICATION BITS
 *    ─────────────────────────────────────────────────────────────
 *    Multiple waiters         ✓ Ya              ✗ Tidak (1 saja)
 *    Memory overhead          24+ bytes         0 (built-in)
 *    Speed                    Medium            Sangat Cepat
 *    Max bits                 24 bits           32 bits (native)
 *    Barrier sync             ✓ Ya              ✗ Tidak
 *    ISR safe                 ✓ Ya (special)    ✓ Ya
 * 
 * ============================================================================
 * ARSITEKTUR PROGRAM
 * ============================================================================
 * 
 *    ┌─────────────────┐        ┌─────────────────┐
 *    │ vButtonTask     │        │ vSensorTask     │
 *    │ (Simulasi)      │        │ (Simulasi)      │
 *    └────────┬────────┘        └────────┬────────┘
 *             │                          │
 *             │ xTaskNotify              │ xTaskNotify
 *             │ EVENT_BUTTON_PRESSED     │ EVENT_DATA_READY
 *             │                          │
 *             └──────────┬───────────────┘
 *                        ▼
 *         ┌──────────────────────────────┐
 *         │       vEventHandlerTask      │
 *         │ xTaskNotifyWait(EVENT_ALL)   │
 *         │                              │
 *         │ Notification Value:          │
 *         │ ┌──┬──┬──┬──┬──┬──┬──┬──┐    │
 *         │ │..|..|..|..|E3│E2│E1│E0│    │
 *         │ └──┴──┴──┴──┴──┴──┴──┴──┘    │
 *         │               │  │  │  │     │
 *         │     ERROR ────┘  │  │  │     │
 *         │     TIMER ───────┘  │  │     │
 *         │     DATA ───────────┘  │     │
 *         │     BUTTON ────────────┘     │
 *         └──────────────────────────────┘
 * 
 * ============================================================================
 * MEKANISME SET BITS
 * ============================================================================
 * 
 *    Initial Value: 0000 0000
 *    
 *    Step 1: Button pressed → xTaskNotify(..., 0x01, eSetBits)
 *            0000 0000 OR 0000 0001 = 0000 0001
 *    
 *    Step 2: Data ready → xTaskNotify(..., 0x02, eSetBits)
 *            0000 0001 OR 0000 0010 = 0000 0011
 *    
 *    Step 3: xTaskNotifyWait() menerima: 0000 0011
 *            Artinya: Button DAN Data event keduanya aktif!
 * 
 * ============================================================================
 * ALUR TIMING
 * ============================================================================
 * 
 *    Button: ╔═══╗         ╔═══╗         ╔═══╗
 *    (1s)    ║SET║         ║SET║         ║SET║
 *            ╚═══╝         ╚═══╝         ╚═══╝
 *    
 *    Sensor:       ╔═══╗         ╔═══╗
 *    (1.5s)        ║SET║         ║SET║
 *                  ╚═══╝         ╚═══╝
 *    
 *    Handler: ░░░░░░░░░█░░░░░░░░░█░░░░░░░░░█
 *             (block) (handle) (block) ...
 * 
 * ============================================================================
 * PENGGUNAAN API
 * ============================================================================
 * 
 *    // Mengirim event bit
 *    xTaskNotify(xHandlerTask, EVENT_BUTTON_PRESSED, eSetBits);
 *    
 *    // Menunggu dan menerima event bits
 *    xTaskNotifyWait(
 *        0,                    // Bits to clear on entry (tidak clear)
 *        ULONG_MAX,            // Bits to clear on exit (clear semua)
 *        &ulNotificationValue, // Menerima nilai sebelum di-clear
 *        portMAX_DELAY         // Tunggu selamanya
 *    );
 * 
 * ============================================================================
 * EXPECTED OUTPUT (UART 115200 baud)
 * ============================================================================
 * 
 *    === Notification Event Bits Demo ===
 *    
 *    [BTN] Sending BUTTON_PRESSED event
 *    [SEN] Sending DATA_READY event
 *    [HDL] Received events: 0x03
 *          + BUTTON_PRESSED
 *          + DATA_READY
 *    
 *    [BTN] Sending BUTTON_PRESSED event
 *    [HDL] Received events: 0x01
 *          + BUTTON_PRESSED
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

#define BUTTON_PERIOD_MS    1000    /* Simulasi button press tiap 1 detik */
#define SENSOR_PERIOD_MS    1500    /* Simulasi sensor ready tiap 1.5 detik */
#define ERROR_PERIOD_MS     5000    /* Simulasi error tiap 5 detik */

/* ============================================================================
 * HANDLE
 * ============================================================================ */

static TaskHandle_t xEventHandlerTask = NULL;

static UART_HandleTypeDef huart1;

/* ============================================================================
 * PROTOTYPE
 * ============================================================================ */

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART_Init(void);
static void UART_SendString(const char *str);

static void vButtonTask(void *pvParameters);
static void vSensorTask(void *pvParameters);
static void vErrorSimTask(void *pvParameters);
static void vEventHandlerTask(void *pvParameters);

/* ============================================================================
 * IMPLEMENTASI
 * ============================================================================ */

/**
 * @brief Task simulasi tombol ditekan
 * 
 * ILUSTRASI:
 * ─────────────────────────────────────────────────
 *    ┌─────────────────────────────────────────┐
 *    │ vButtonTask                             │
 *    ├─────────────────────────────────────────┤
 *    │ while(1) {                              │
 *    │   vTaskDelay(1000ms);                   │
 *    │   ╔═══════════════════════════════════╗ │
 *    │   ║ xTaskNotify(Handler,               ║ │
 *    │   ║             EVENT_BUTTON_PRESSED,  ║ │
 *    │   ║             eSetBits);             ║ │
 *    │   ╚═══════════════════════════════════╝ │
 *    │ }                                       │
 *    └─────────────────────────────────────────┘
 * ─────────────────────────────────────────────────
 */
static void vButtonTask(void *pvParameters)
{
    (void)pvParameters;
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for(;;)
    {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(BUTTON_PERIOD_MS));
        
        UART_SendString("[BTN] Sending BUTTON_PRESSED event\r\n");
        
        /* Set bit EVENT_BUTTON_PRESSED pada notification value handler */
        xTaskNotify(xEventHandlerTask, EVENT_BUTTON_PRESSED, eSetBits);
    }
}

/**
 * @brief Task simulasi sensor data ready
 * 
 * ILUSTRASI:
 * ─────────────────────────────────────────────────
 *    ┌─────────────────────────────────────────┐
 *    │ vSensorTask                             │
 *    ├─────────────────────────────────────────┤
 *    │ Period: 1.5 detik                       │
 *    │                                         │
 *    │   xTaskNotify(Handler,                  │
 *    │               EVENT_DATA_READY,         │
 *    │               eSetBits);                │
 *    │                                         │
 *    │   Bit Pattern: 0x02 = 0000 0010         │
 *    └─────────────────────────────────────────┘
 * ─────────────────────────────────────────────────
 */
static void vSensorTask(void *pvParameters)
{
    (void)pvParameters;
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for(;;)
    {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(SENSOR_PERIOD_MS));
        
        UART_SendString("[SEN] Sending DATA_READY event\r\n");
        
        /* Set bit EVENT_DATA_READY */
        xTaskNotify(xEventHandlerTask, EVENT_DATA_READY, eSetBits);
    }
}

/**
 * @brief Task simulasi error occurrence
 * 
 * Event error yang jarang terjadi (tiap 5 detik).
 * Mendemonstrasikan bahwa multiple bits bisa di-set bersamaan.
 */
static void vErrorSimTask(void *pvParameters)
{
    (void)pvParameters;
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for(;;)
    {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(ERROR_PERIOD_MS));
        
        UART_SendString("[ERR] Simulating ERROR_OCCURRED event\r\n");
        
        /* Set bit EVENT_ERROR_OCCURRED */
        xTaskNotify(xEventHandlerTask, EVENT_ERROR_OCCURRED, eSetBits);
    }
}

/**
 * @brief Task handler yang menunggu semua event
 * 
 * ILUSTRASI XASKNOTIFYWAIT:
 * ═════════════════════════════════════════════════════════════════════
 *    
 *    xTaskNotifyWait(ulBitsToClearOnEntry,  // Clear bits saat masuk wait
 *                    ulBitsToClearOnExit,   // Clear bits setelah terima
 *                    &pulNotificationValue, // Pointer untuk menerima value
 *                    xTicksToWait);         // Timeout
 *    
 *    ┌───────────────────────────────────────────────────────────┐
 *    │                      FLOW DIAGRAM                         │
 *    ├───────────────────────────────────────────────────────────┤
 *    │                                                           │
 *    │   ┌─────────┐                                             │
 *    │   │ ENTRY   │                                             │
 *    │   └────┬────┘                                             │
 *    │        │                                                  │
 *    │        ▼                                                  │
 *    │   ┌─────────────────────────────────────┐                 │
 *    │   │ Clear bits: ulBitsToClearOnEntry    │                 │
 *    │   │ notif_value &= ~ulBitsToClearOnEntry│                 │
 *    │   └─────────────────┬───────────────────┘                 │
 *    │                     │                                     │
 *    │                     ▼                                     │
 *    │   ┌─────────────────────────────────────┐                 │
 *    │   │ Notification pending?               │                 │
 *    │   └─────────────────┬───────────────────┘                 │
 *    │          ┌──────────┴───────────┐                         │
 *    │          │YES                   │NO                       │
 *    │          ▼                      ▼                         │
 *    │   ┌─────────────┐       ┌───────────────┐                 │
 *    │   │ Return now  │       │ BLOCK & WAIT  │                 │
 *    │   └──────┬──────┘       │ for timeout   │                 │
 *    │          │              │ or notify     │                 │
 *    │          │              └───────┬───────┘                 │
 *    │          └──────────┬───────────┘                         │
 *    │                     │                                     │
 *    │                     ▼                                     │
 *    │   ┌─────────────────────────────────────┐                 │
 *    │   │ *pulNotificationValue = current     │                 │
 *    │   │ (copy sebelum di-clear)             │                 │
 *    │   └─────────────────┬───────────────────┘                 │
 *    │                     │                                     │
 *    │                     ▼                                     │
 *    │   ┌─────────────────────────────────────┐                 │
 *    │   │ Clear bits: ulBitsToClearOnExit     │                 │
 *    │   │ notif_value &= ~ulBitsToClearOnExit │                 │
 *    │   └─────────────────┬───────────────────┘                 │
 *    │                     │                                     │
 *    │                     ▼                                     │
 *    │               ┌──────────┐                                │
 *    │               │  RETURN  │                                │
 *    │               └──────────┘                                │
 *    │                                                           │
 *    └───────────────────────────────────────────────────────────┘
 * 
 * ═════════════════════════════════════════════════════════════════════
 */
static void vEventHandlerTask(void *pvParameters)
{
    (void)pvParameters;
    
    uint32_t ulNotificationValue;
    BaseType_t xResult;
    char buffer[100];
    
    UART_SendString("\r\n=== Notification Event Bits Demo ===\r\n\r\n");
    
    for(;;)
    {
        /*
         * Tunggu notification dengan timeout tak terbatas.
         * - ulBitsToClearOnEntry = 0 (tidak clear saat entry)
         * - ulBitsToClearOnExit = EVENT_ALL_BITS (clear semua setelah baca)
         * - pulNotificationValue = &ulNotificationValue (terima nilai)
         * - xTicksToWait = portMAX_DELAY (tunggu selamanya)
         */
        xResult = xTaskNotifyWait(
            0,                      /* Bits to clear on entry */
            EVENT_ALL_BITS,         /* Bits to clear on exit */
            &ulNotificationValue,   /* Menerima notification value */
            portMAX_DELAY           /* Tunggu selamanya */
        );
        
        if(xResult == pdPASS)
        {
            /* Toggle LED untuk indikasi event diterima */
            HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
            
            /* Tampilkan nilai hex dari events */
            snprintf(buffer, sizeof(buffer), 
                    "[HDL] Received events: 0x%02lX\r\n", ulNotificationValue);
            UART_SendString(buffer);
            
            /* Decode dan tampilkan setiap event yang aktif */
            if(ulNotificationValue & EVENT_BUTTON_PRESSED)
            {
                UART_SendString("      + BUTTON_PRESSED\r\n");
            }
            
            if(ulNotificationValue & EVENT_DATA_READY)
            {
                UART_SendString("      + DATA_READY\r\n");
            }
            
            if(ulNotificationValue & EVENT_TIMER_EXPIRED)
            {
                UART_SendString("      + TIMER_EXPIRED\r\n");
            }
            
            if(ulNotificationValue & EVENT_ERROR_OCCURRED)
            {
                UART_SendString("      + ERROR_OCCURRED\r\n");
            }
            
            UART_SendString("\r\n");
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
    UART_SendString("18-Notification_Event_Bits\r\n");
    UART_SendString("STM32F103 + FreeRTOS\r\n");
    UART_SendString("================================\r\n");
    
    /*
     * Buat handler task lebih dulu agar handle-nya valid
     * saat producer tasks mengirim notification.
     */
    xTaskCreate(vEventHandlerTask, "Handler", HANDLER_STACK, NULL, 
                HANDLER_PRIO, &xEventHandlerTask);
    
    /* Buat producer tasks */
    xTaskCreate(vButtonTask, "Button", PRODUCER_STACK, NULL, 
                PRODUCER_PRIO, NULL);
    
    xTaskCreate(vSensorTask, "Sensor", PRODUCER_STACK, NULL, 
                PRODUCER_PRIO, NULL);
    
    xTaskCreate(vErrorSimTask, "Error", PRODUCER_STACK, NULL, 
                PRODUCER_PRIO, NULL);
    
    vTaskStartScheduler();
    
    for(;;);
}
