/**
 * ============================================================================
 * FILE: main.c
 * PROJECT: 10-Queue_Sets_Implementation
 * 
 * JUDUL: Queue Sets - Menunggu Multiple Queues Sekaligus
 * 
 * DESKRIPSI:
 * Demo penggunaan Queue Sets untuk menunggu data dari beberapa
 * queue sekaligus. Handler task akan terbangun ketika SALAH SATU
 * queue menerima data.
 * 
 * ============================================================================
 * ARSITEKTUR PROGRAM
 * ============================================================================
 * 
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                       QUEUE SETS DEMO                              │
 *    ├─────────────────────────────────────────────────────────────────────┤
 *    │                                                                     │
 *    │   ┌─────────────┐    ┌─────────────┐    ┌─────────────┐            │
 *    │   │ vTempTask   │    │ vHumidTask  │    │ vPressTask  │            │
 *    │   │ (Temp Sensor)│    │(Humid Sensor)│   │(Press Sensor)│           │
 *    │   │ Period:700ms│    │ Period:1.1s │    │ Period:1.5s │            │
 *    │   └──────┬──────┘    └──────┬──────┘    └──────┬──────┘            │
 *    │          │                  │                  │                    │
 *    │          ▼                  ▼                  ▼                    │
 *    │   ┌─────────────┐    ┌─────────────┐    ┌─────────────┐            │
 *    │   │  Queue A    │    │  Queue B    │    │  Queue C    │            │
 *    │   │ (TempData)  │    │ (HumidData) │    │ (PressData) │            │
 *    │   └──────┬──────┘    └──────┬──────┘    └──────┬──────┘            │
 *    │          │                  │                  │                    │
 *    │          └─────────────┬────┴────┬─────────────┘                    │
 *    │                        │         │                                  │
 *    │                        ▼         ▼                                  │
 *    │               ┌────────────────────────────┐                        │
 *    │               │        QUEUE SET           │                        │
 *    │               │                            │                        │
 *    │               │ Monitors: Queue A,B,C      │                        │
 *    │               └─────────────┬──────────────┘                        │
 *    │                             │                                       │
 *    │                             ▼                                       │
 *    │               ┌────────────────────────────┐                        │
 *    │               │      vHandlerTask          │                        │
 *    │               │                            │                        │
 *    │               │ xQueueSelectFromSet()      │                        │
 *    │               │ → Return handle yg ready   │                        │
 *    │               │ → xQueueReceive(handle)    │                        │
 *    │               │ → Process data             │                        │
 *    │               └────────────────────────────┘                        │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 * 
 * ============================================================================
 * API QUEUE SETS
 * ============================================================================
 * 
 *    // 1. Buat Queue Set (ukuran = total semua queue)
 *    xQueueSet = xQueueCreateSet(QUEUE_A_SIZE + QUEUE_B_SIZE + ...);
 *    
 *    // 2. Tambahkan queue ke set
 *    xQueueAddToSet(xQueueA, xQueueSet);
 *    xQueueAddToSet(xQueueB, xQueueSet);
 *    xQueueAddToSet(xQueueC, xQueueSet);
 *    
 *    // 3. Tunggu salah satu queue ada data
 *    QueueSetMemberHandle_t xActivatedMember;
 *    xActivatedMember = xQueueSelectFromSet(xQueueSet, timeout);
 *    
 *    // 4. Terima data dari queue yang aktif
 *    if(xActivatedMember == xQueueA) {
 *        xQueueReceive(xQueueA, &dataA, 0);
 *    } else if(xActivatedMember == xQueueB) {
 *        xQueueReceive(xQueueB, &dataB, 0);
 *    }
 * 
 * ============================================================================
 * EXPECTED OUTPUT
 * ============================================================================
 * 
 *    === Queue Sets Demo ===
 *    
 *    [TEMP] Sent: 25.3C
 *    [HANDLER] Got TEMPERATURE: 25.3C
 *    
 *    [TEMP] Sent: 25.5C
 *    [HANDLER] Got TEMPERATURE: 25.5C
 *    
 *    [HUMID] Sent: 65.2%
 *    [HANDLER] Got HUMIDITY: 65.2%
 *    
 *    [PRESS] Sent: 1013.2hPa
 *    [HANDLER] Got PRESSURE: 1013.2hPa
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
 * DEFINISI DATA TYPES
 * ============================================================================ */

typedef enum {
    SENSOR_TEMPERATURE,
    SENSOR_HUMIDITY,
    SENSOR_PRESSURE
} SensorType_t;

typedef struct {
    SensorType_t type;
    float        value;
    TickType_t   timestamp;
} SensorData_t;

/* ============================================================================
 * HANDLE GLOBAL
 * ============================================================================ */

static QueueHandle_t xTempQueue = NULL;
static QueueHandle_t xHumidQueue = NULL;
static QueueHandle_t xPressQueue = NULL;

static QueueSetHandle_t xSensorQueueSet = NULL;

static UART_HandleTypeDef huart1;

/* ============================================================================
 * PROTOTYPE
 * ============================================================================ */

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART_Init(void);
static void UART_SendString(const char *str);

static void vTempSensorTask(void *pvParameters);
static void vHumidSensorTask(void *pvParameters);
static void vPressSensorTask(void *pvParameters);
static void vHandlerTask(void *pvParameters);

/* ============================================================================
 * IMPLEMENTASI TASK
 * ============================================================================ */

/**
 * @brief Temperature Sensor Task
 * 
 * Mengirim data suhu ke Queue A dengan periode 700ms.
 * Simulasi pembacaan sensor dengan nilai random.
 */
static void vTempSensorTask(void *pvParameters)
{
    (void)pvParameters;
    
    SensorData_t data;
    char buffer[50];
    float tempBase = 25.0f;
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for(;;)
    {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(700));
        
        /* Simulasi pembacaan suhu */
        data.type = SENSOR_TEMPERATURE;
        data.value = tempBase + ((float)(xTaskGetTickCount() % 30) / 10.0f);
        data.timestamp = xTaskGetTickCount();
        
        snprintf(buffer, sizeof(buffer), "[TEMP] Sent: %.1fC\r\n", data.value);
        UART_SendString(buffer);
        
        xQueueSend(xTempQueue, &data, 0);
    }
}

/**
 * @brief Humidity Sensor Task
 * 
 * Mengirim data kelembaban ke Queue B dengan periode 1100ms.
 */
static void vHumidSensorTask(void *pvParameters)
{
    (void)pvParameters;
    
    SensorData_t data;
    char buffer[50];
    float humidBase = 60.0f;
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for(;;)
    {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1100));
        
        data.type = SENSOR_HUMIDITY;
        data.value = humidBase + ((float)(xTaskGetTickCount() % 20) / 2.0f);
        data.timestamp = xTaskGetTickCount();
        
        snprintf(buffer, sizeof(buffer), "[HUMID] Sent: %.1f%%\r\n", data.value);
        UART_SendString(buffer);
        
        xQueueSend(xHumidQueue, &data, 0);
    }
}

/**
 * @brief Pressure Sensor Task
 * 
 * Mengirim data tekanan ke Queue C dengan periode 1500ms.
 */
static void vPressSensorTask(void *pvParameters)
{
    (void)pvParameters;
    
    SensorData_t data;
    char buffer[50];
    float pressBase = 1013.0f;
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for(;;)
    {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1500));
        
        data.type = SENSOR_PRESSURE;
        data.value = pressBase + ((float)(xTaskGetTickCount() % 10) / 5.0f);
        data.timestamp = xTaskGetTickCount();
        
        snprintf(buffer, sizeof(buffer), "[PRESS] Sent: %.1fhPa\r\n", data.value);
        UART_SendString(buffer);
        
        xQueueSend(xPressQueue, &data, 0);
    }
}

/**
 * @brief Handler Task - Menunggu data dari Queue Set
 * 
 * ILUSTRASI xQueueSelectFromSet:
 * ═══════════════════════════════════════════════════════════════════════════
 *    
 *    ┌─────────────────────────────────────────────────────────────────────┐
 *    │                                                                     │
 *    │   xQueueSelectFromSet(xQueueSet, portMAX_DELAY)                    │
 *    │                                                                     │
 *    │   ┌───────────────────────────────────────────────────────────────┐ │
 *    │   │                                                               │ │
 *    │   │   Queue A: [ ] [ ] [ ]  ─┐                                    │ │
 *    │   │                          │                                    │ │
 *    │   │   Queue B: [█] [ ] [ ]  ─┼──► QUEUE SET ──► Select returns B │ │
 *    │   │            ↑             │                                    │ │
 *    │   │         data!            │                                    │ │
 *    │   │   Queue C: [ ] [ ] [ ]  ─┘                                    │ │
 *    │   │                                                               │ │
 *    │   └───────────────────────────────────────────────────────────────┘ │
 *    │                                                                     │
 *    │   Handler:                                                          │
 *    │   ─────────                                                         │
 *    │   1. xActivatedMember = xQueueSelectFromSet(set, MAX_DELAY);       │
 *    │      // BLOCK sampai ada data di salah satu queue                  │
 *    │      // Return: handle ke queue yang punya data                    │
 *    │                                                                     │
 *    │   2. if(xActivatedMember == xQueueB) {                             │
 *    │          xQueueReceive(xQueueB, &data, 0);  // timeout=0, pasti ada│
 *    │          ProcessHumidityData(&data);                                │
 *    │      }                                                              │
 *    │                                                                     │
 *    └─────────────────────────────────────────────────────────────────────┘
 *    
 * ═══════════════════════════════════════════════════════════════════════════
 */
static void vHandlerTask(void *pvParameters)
{
    (void)pvParameters;
    
    QueueSetMemberHandle_t xActivatedMember;
    SensorData_t rxData;
    char buffer[80];
    
    UART_SendString("[HANDLER] Handler started, waiting on Queue Set...\r\n\r\n");
    
    for(;;)
    {
        /*
         * Block sampai SALAH SATU queue dalam set memiliki data.
         * Ini sangat efisien - tidak ada polling!
         * 
         * Return value adalah HANDLE ke queue yang memiliki data.
         */
        xActivatedMember = xQueueSelectFromSet(xSensorQueueSet, portMAX_DELAY);
        
        if(xActivatedMember != NULL)
        {
            /* Toggle LED untuk indikasi */
            HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
            
            /*
             * Tentukan queue mana yang aktif dan receive dari situ.
             * Timeout = 0 karena kita TAHU pasti ada data.
             */
            if(xActivatedMember == (QueueSetMemberHandle_t)xTempQueue)
            {
                xQueueReceive(xTempQueue, &rxData, 0);
                snprintf(buffer, sizeof(buffer),
                        "[HANDLER] Got TEMPERATURE: %.1fC\r\n\r\n",
                        rxData.value);
            }
            else if(xActivatedMember == (QueueSetMemberHandle_t)xHumidQueue)
            {
                xQueueReceive(xHumidQueue, &rxData, 0);
                snprintf(buffer, sizeof(buffer),
                        "[HANDLER] Got HUMIDITY: %.1f%%\r\n\r\n",
                        rxData.value);
            }
            else if(xActivatedMember == (QueueSetMemberHandle_t)xPressQueue)
            {
                xQueueReceive(xPressQueue, &rxData, 0);
                snprintf(buffer, sizeof(buffer),
                        "[HANDLER] Got PRESSURE: %.1fhPa\r\n\r\n",
                        rxData.value);
            }
            else
            {
                snprintf(buffer, sizeof(buffer),
                        "[HANDLER] Unknown queue activated!\r\n");
            }
            
            UART_SendString(buffer);
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
    UART_SendString("10-Queue_Sets_Implementation\r\n");
    UART_SendString("STM32F103 + FreeRTOS\r\n");
    UART_SendString("================================\r\n");
    UART_SendString("\r\n=== Queue Sets Demo ===\r\n\r\n");
    
    /*
     * 1. Buat Queue Set terlebih dahulu
     *    Ukuran = total semua queue yang akan ditambahkan
     */
    xSensorQueueSet = xQueueCreateSet(QUEUE_SET_SIZE);
    
    if(xSensorQueueSet == NULL)
    {
        UART_SendString("ERROR: Failed to create queue set!\r\n");
        while(1);
    }
    
    /*
     * 2. Buat individual queues
     */
    xTempQueue = xQueueCreate(QUEUE_A_LENGTH, sizeof(SensorData_t));
    xHumidQueue = xQueueCreate(QUEUE_B_LENGTH, sizeof(SensorData_t));
    xPressQueue = xQueueCreate(QUEUE_C_LENGTH, sizeof(SensorData_t));
    
    if(xTempQueue == NULL || xHumidQueue == NULL || xPressQueue == NULL)
    {
        UART_SendString("ERROR: Failed to create queues!\r\n");
        while(1);
    }
    
    /*
     * 3. Tambahkan queues ke set
     *    PENTING: Harus dilakukan SEBELUM ada data masuk!
     */
    xQueueAddToSet(xTempQueue, xSensorQueueSet);
    xQueueAddToSet(xHumidQueue, xSensorQueueSet);
    xQueueAddToSet(xPressQueue, xSensorQueueSet);
    
    UART_SendString("Queue Set created with 3 queues:\r\n");
    UART_SendString("  - Temperature (700ms period)\r\n");
    UART_SendString("  - Humidity (1100ms period)\r\n");
    UART_SendString("  - Pressure (1500ms period)\r\n\r\n");
    
    /* Buat tasks */
    xTaskCreate(vHandlerTask, "Handler", TASK_STACK, NULL,
                tskIDLE_PRIORITY + 3, NULL);
    
    xTaskCreate(vTempSensorTask, "Temp", TASK_STACK, NULL,
                tskIDLE_PRIORITY + 2, NULL);
    
    xTaskCreate(vHumidSensorTask, "Humid", TASK_STACK, NULL,
                tskIDLE_PRIORITY + 2, NULL);
    
    xTaskCreate(vPressSensorTask, "Press", TASK_STACK, NULL,
                tskIDLE_PRIORITY + 2, NULL);
    
    vTaskStartScheduler();
    
    for(;;);
}
