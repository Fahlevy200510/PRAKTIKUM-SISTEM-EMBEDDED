/**
 * =============================================================================
 * PROGRAM 44: SENSOR FUSION - Multi-Sensor Data Fusion dengan FreeRTOS
 * =============================================================================
 * 
 * Demo sensor fusion dengan 3 sensor temperature simulasi.
 * Menggabungkan data menggunakan weighted average dengan outlier detection.
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* ========================== HARDWARE CONFIGURATION ======================== */
#define LED_PIN         GPIO_PIN_13
#define LED_PORT        GPIOC

/* ========================== DATA STRUCTURES =============================== */

/* Sensor reading structure */
typedef struct {
    uint8_t  ucSensorId;      /* Sensor identifier (0-2) */
    float    fValue;          /* Temperature reading */
    uint32_t ulTimestamp;     /* Tick count when read */
    uint8_t  ucConfidence;    /* Confidence level 0-100% */
} SensorReading_t;

/* Fused result structure */
typedef struct {
    float    fFusedValue;     /* Fused temperature */
    uint8_t  ucConfidence;    /* Overall confidence */
    uint8_t  ucValidSensors;  /* Bitmask of valid sensors */
    uint32_t ulTimestamp;     /* Fusion timestamp */
    float    fMin;            /* Minimum reading */
    float    fMax;            /* Maximum reading */
    float    fStdDev;         /* Standard deviation */
} FusedResult_t;

/* Sensor health tracking */
typedef struct {
    uint32_t ulReadings;      /* Total readings */
    uint32_t ulOutliers;      /* Outlier count */
    float    fLastValue;      /* Last reading */
    uint8_t  ucHealth;        /* Health percentage 0-100 */
    uint8_t  ucOnline;        /* Sensor online flag */
} SensorHealth_t;

/* ========================== GLOBALS ======================================= */

UART_HandleTypeDef huart1;

/* FreeRTOS handles */
static QueueHandle_t xSensorQueue;
static SemaphoreHandle_t xHealthMutex;
static TaskHandle_t xSensorTaskHandles[NUM_SENSORS];
static TaskHandle_t xFusionTaskHandle;

/* Sensor health status */
static volatile SensorHealth_t xSensorHealth[NUM_SENSORS];

/* Latest fused result */
static volatile FusedResult_t xLatestFusedResult;

/* Random seed for simulation */
static uint32_t ulRandomSeed = 12345;

/* ========================== FUNCTION PROTOTYPES =========================== */

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART1_Init(void);
static void UART_SendString(const char *str);

static void vSensorTask(void *pvParameters);
static void vFusionTask(void *pvParameters);
static void vOutputTask(void *pvParameters);

static float fSimulateTemperature(uint8_t ucSensorId);
static float fCalculateStdDev(float *pfValues, uint8_t ucCount, float fMean);
static uint8_t ucDetectOutliers(SensorReading_t *pxReadings, uint8_t ucCount, 
                                 uint8_t *pucValid);

/* ========================== RANDOM NUMBER GENERATOR ======================= */

/* Simple LCG pseudo-random number generator */
static uint32_t ulRandom(void)
{
    ulRandomSeed = ulRandomSeed * 1103515245 + 12345;
    return (ulRandomSeed >> 16) & 0x7FFF;
}

/* Get random float in range [fMin, fMax] */
static float fRandomFloat(float fMin, float fMax)
{
    return fMin + ((float)ulRandom() / 32767.0f) * (fMax - fMin);
}

/* ========================== SIMULATION ==================================== */

/**
 * Simulate temperature reading from a sensor
 * Each sensor has slightly different characteristics
 */
static float fSimulateTemperature(uint8_t ucSensorId)
{
    float fBase = TEMP_NOMINAL;
    float fNoise;
    float fDrift;
    
    /* Each sensor has different noise and drift characteristics */
    switch (ucSensorId)
    {
        case 0:
            /* Sensor 0: Low noise, stable */
            fNoise = fRandomFloat(-TEMP_NOISE_MAX * 0.5f, TEMP_NOISE_MAX * 0.5f);
            fDrift = 0.0f;
            break;
            
        case 1:
            /* Sensor 1: Medium noise */
            fNoise = fRandomFloat(-TEMP_NOISE_MAX, TEMP_NOISE_MAX);
            fDrift = 0.1f;
            break;
            
        case 2:
            /* Sensor 2: Higher noise, occasional outliers */
            fNoise = fRandomFloat(-TEMP_NOISE_MAX * 1.5f, TEMP_NOISE_MAX * 1.5f);
            /* 10% chance of outlier */
            if ((ulRandom() % 10) == 0)
            {
                fNoise = fRandomFloat(5.0f, 10.0f); /* Large positive deviation */
            }
            fDrift = -0.05f;
            break;
            
        default:
            fNoise = 0.0f;
            fDrift = 0.0f;
            break;
    }
    
    return fBase + fNoise + fDrift;
}

/* ========================== STATISTICAL FUNCTIONS ========================= */

/**
 * Calculate standard deviation
 */
static float fCalculateStdDev(float *pfValues, uint8_t ucCount, float fMean)
{
    float fSum = 0.0f;
    uint8_t i;
    
    if (ucCount < 2)
    {
        return 0.0f;
    }
    
    for (i = 0; i < ucCount; i++)
    {
        float fDiff = pfValues[i] - fMean;
        fSum += fDiff * fDiff;
    }
    
    return sqrtf(fSum / (float)(ucCount - 1));
}

/**
 * Detect outliers using standard deviation threshold
 * Returns number of valid sensors, fills pucValid bitmask
 */
static uint8_t ucDetectOutliers(SensorReading_t *pxReadings, uint8_t ucCount, 
                                 uint8_t *pucValid)
{
    float fValues[NUM_SENSORS];
    float fSum = 0.0f;
    float fMean;
    float fStdDev;
    float fThreshold;
    uint8_t ucValidCount = 0;
    uint8_t i;
    
    *pucValid = 0;
    
    if (ucCount == 0)
    {
        return 0;
    }
    
    /* Calculate mean */
    for (i = 0; i < ucCount; i++)
    {
        fValues[i] = pxReadings[i].fValue;
        fSum += fValues[i];
    }
    fMean = fSum / (float)ucCount;
    
    /* Calculate standard deviation */
    fStdDev = fCalculateStdDev(fValues, ucCount, fMean);
    
    /* Set threshold (handle case where all values are same) */
    if (fStdDev < 0.1f)
    {
        fStdDev = 0.1f;
    }
    fThreshold = OUTLIER_THRESHOLD_SIGMA * fStdDev;
    
    /* Mark valid sensors (within threshold of mean) */
    for (i = 0; i < ucCount; i++)
    {
        float fDeviation = fabsf(pxReadings[i].fValue - fMean);
        
        if (fDeviation <= fThreshold)
        {
            *pucValid |= (1 << pxReadings[i].ucSensorId);
            ucValidCount++;
        }
        else
        {
            /* Mark as outlier in health tracking */
            if (xSemaphoreTake(xHealthMutex, pdMS_TO_TICKS(10)) == pdTRUE)
            {
                xSensorHealth[pxReadings[i].ucSensorId].ulOutliers++;
                xSemaphoreGive(xHealthMutex);
            }
        }
    }
    
    return ucValidCount;
}

/* ========================== TASKS ========================================= */

/**
 * Sensor Task - One instance per sensor
 * Reads sensor and sends data to fusion queue
 */
static void vSensorTask(void *pvParameters)
{
    uint8_t ucSensorId = (uint8_t)(uint32_t)pvParameters;
    TickType_t xLastWakeTime;
    SensorReading_t xReading;
    char cBuffer[80];
    
    /* Stagger sensor readings to distribute load */
    vTaskDelay(pdMS_TO_TICKS(ucSensorId * 30));
    
    snprintf(cBuffer, sizeof(cBuffer), 
             "[Sensor %d] Task started\r\n", ucSensorId);
    UART_SendString(cBuffer);
    
    xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        /* Read simulated sensor */
        xReading.ucSensorId = ucSensorId;
        xReading.fValue = fSimulateTemperature(ucSensorId);
        xReading.ulTimestamp = xTaskGetTickCount();
        
        /* Calculate confidence based on sensor characteristics */
        xReading.ucConfidence = 95 - (ucSensorId * 5); /* S0=95%, S1=90%, S2=85% */
        
        /* Reduce confidence if value is near edge of range */
        if (xReading.fValue < TEMP_MIN + 2.0f || xReading.fValue > TEMP_MAX - 2.0f)
        {
            xReading.ucConfidence -= 10;
        }
        
        /* Update health tracking */
        if (xSemaphoreTake(xHealthMutex, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            xSensorHealth[ucSensorId].ulReadings++;
            xSensorHealth[ucSensorId].fLastValue = xReading.fValue;
            xSensorHealth[ucSensorId].ucOnline = 1;
            xSemaphoreGive(xHealthMutex);
        }
        
        /* Send to fusion queue */
        if (xQueueSend(xSensorQueue, &xReading, pdMS_TO_TICKS(50)) != pdPASS)
        {
            UART_SendString("[Sensor] Queue full!\r\n");
        }
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(SENSOR_SAMPLE_PERIOD_MS));
    }
}

/**
 * Fusion Task - Collects readings and performs sensor fusion
 */
static void vFusionTask(void *pvParameters)
{
    (void)pvParameters;
    
    SensorReading_t xReadings[NUM_SENSORS];
    SensorReading_t xReceived;
    uint8_t ucReadingCount;
    uint8_t ucValidMask;
    uint8_t ucValidCount;
    TickType_t xLastWakeTime;
    char cBuffer[100];
    uint8_t i;
    
    UART_SendString("[Fusion] Task started\r\n");
    
    xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        /* Collect all available readings from queue */
        ucReadingCount = 0;
        memset(xReadings, 0, sizeof(xReadings));
        
        /* Receive up to NUM_SENSORS readings */
        while (xQueueReceive(xSensorQueue, &xReceived, 0) == pdPASS && 
               ucReadingCount < NUM_SENSORS)
        {
            /* Store reading, overwrite if same sensor */
            xReadings[xReceived.ucSensorId] = xReceived;
            if (xReadings[xReceived.ucSensorId].ulTimestamp == 0)
            {
                ucReadingCount++;
            }
            xReadings[xReceived.ucSensorId] = xReceived;
        }
        
        /* Count actual readings */
        ucReadingCount = 0;
        for (i = 0; i < NUM_SENSORS; i++)
        {
            if (xReadings[i].ulTimestamp > 0)
            {
                ucReadingCount++;
            }
        }
        
        if (ucReadingCount >= MIN_VALID_SENSORS)
        {
            /* Detect outliers */
            SensorReading_t xValidReadings[NUM_SENSORS];
            uint8_t ucIdx = 0;
            
            for (i = 0; i < NUM_SENSORS; i++)
            {
                if (xReadings[i].ulTimestamp > 0)
                {
                    xValidReadings[ucIdx++] = xReadings[i];
                }
            }
            
            ucValidCount = ucDetectOutliers(xValidReadings, ucIdx, &ucValidMask);
            
            if (ucValidCount >= MIN_VALID_SENSORS)
            {
                /* Perform weighted average fusion */
                float fWeightedSum = 0.0f;
                float fWeightSum = 0.0f;
                float fMin = 1000.0f;
                float fMax = -1000.0f;
                float fMean;
                float fValues[NUM_SENSORS];
                uint8_t ucValIdx = 0;
                uint8_t ucTotalConf = 0;
                
                for (i = 0; i < NUM_SENSORS; i++)
                {
                    if ((ucValidMask & (1 << i)) && xReadings[i].ulTimestamp > 0)
                    {
                        float fWeight = (float)xReadings[i].ucConfidence;
                        fWeightedSum += xReadings[i].fValue * fWeight;
                        fWeightSum += fWeight;
                        
                        if (xReadings[i].fValue < fMin) fMin = xReadings[i].fValue;
                        if (xReadings[i].fValue > fMax) fMax = xReadings[i].fValue;
                        
                        fValues[ucValIdx++] = xReadings[i].fValue;
                        ucTotalConf += xReadings[i].ucConfidence;
                    }
                }
                
                if (fWeightSum > 0.0f)
                {
                    fMean = fWeightedSum / fWeightSum;
                    
                    /* Update fused result */
                    xLatestFusedResult.fFusedValue = fMean;
                    xLatestFusedResult.ucConfidence = ucTotalConf / ucValidCount;
                    xLatestFusedResult.ucValidSensors = ucValidMask;
                    xLatestFusedResult.ulTimestamp = xTaskGetTickCount();
                    xLatestFusedResult.fMin = fMin;
                    xLatestFusedResult.fMax = fMax;
                    xLatestFusedResult.fStdDev = fCalculateStdDev(fValues, ucValIdx, fMean);
                    
                    /* Print fusion result */
                    UART_SendString("\r\n--- Fusion Result ---\r\n");
                    snprintf(cBuffer, sizeof(cBuffer), 
                             "Fused Temp: %.2f°C\r\n", xLatestFusedResult.fFusedValue);
                    UART_SendString(cBuffer);
                    snprintf(cBuffer, sizeof(cBuffer), 
                             "Confidence: %d%%\r\n", xLatestFusedResult.ucConfidence);
                    UART_SendString(cBuffer);
                    snprintf(cBuffer, sizeof(cBuffer), 
                             "Valid: %d/%d (mask: 0x%02X)\r\n", 
                             ucValidCount, NUM_SENSORS, ucValidMask);
                    UART_SendString(cBuffer);
                    snprintf(cBuffer, sizeof(cBuffer), 
                             "Range: %.2f - %.2f°C, StdDev: %.2f\r\n",
                             fMin, fMax, xLatestFusedResult.fStdDev);
                    UART_SendString(cBuffer);
                    
                    /* LED feedback: blink based on confidence */
                    if (xLatestFusedResult.ucConfidence > 80)
                    {
                        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET); /* LED ON */
                    }
                    else
                    {
                        HAL_GPIO_TogglePin(LED_PORT, LED_PIN); /* Blink */
                    }
                }
            }
            else
            {
                UART_SendString("[Fusion] Not enough valid sensors!\r\n");
                HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET); /* LED OFF */
            }
        }
        
        /* Print individual sensor readings */
        UART_SendString("\r\n--- Sensor Readings ---\r\n");
        for (i = 0; i < NUM_SENSORS; i++)
        {
            if (xReadings[i].ulTimestamp > 0)
            {
                const char *pcStatus = "";
                if ((ucValidMask & (1 << i)) == 0)
                {
                    pcStatus = " ← OUTLIER";
                }
                snprintf(cBuffer, sizeof(cBuffer), 
                         "[S%d] Temp: %.2f°C (Conf: %d%%)%s\r\n",
                         i, xReadings[i].fValue, xReadings[i].ucConfidence, pcStatus);
                UART_SendString(cBuffer);
            }
            else
            {
                snprintf(cBuffer, sizeof(cBuffer), 
                         "[S%d] NO DATA\r\n", i);
                UART_SendString(cBuffer);
            }
        }
        
        /* Update sensor health periodically */
        if ((xTaskGetTickCount() % 5000) < FUSION_PERIOD_MS)
        {
            UART_SendString("\r\n=== Sensor Health Report ===\r\n");
            for (i = 0; i < NUM_SENSORS; i++)
            {
                if (xSemaphoreTake(xHealthMutex, pdMS_TO_TICKS(10)) == pdTRUE)
                {
                    uint32_t ulReadings = xSensorHealth[i].ulReadings;
                    uint32_t ulOutliers = xSensorHealth[i].ulOutliers;
                    uint8_t ucHealth;
                    const char *pcStatus;
                    
                    if (ulReadings > 0)
                    {
                        ucHealth = 100 - (uint8_t)((ulOutliers * 100) / ulReadings);
                        if (ucHealth > 100) ucHealth = 100;
                    }
                    else
                    {
                        ucHealth = 0;
                    }
                    
                    xSensorHealth[i].ucHealth = ucHealth;
                    
                    if (ucHealth >= 80) pcStatus = "HEALTHY";
                    else if (ucHealth >= 50) pcStatus = "DEGRADED";
                    else pcStatus = "FAILING";
                    
                    snprintf(cBuffer, sizeof(cBuffer), 
                             "Sensor %d: %s (%d%%) - Readings: %lu, Outliers: %lu\r\n",
                             i, pcStatus, ucHealth, ulReadings, ulOutliers);
                    UART_SendString(cBuffer);
                    
                    xSemaphoreGive(xHealthMutex);
                }
            }
        }
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(FUSION_PERIOD_MS));
    }
}

/* ========================== UART FUNCTIONS ================================ */

static void UART_SendString(const char *str)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}

/* ========================== HOOK FUNCTIONS ================================ */

void vApplicationMallocFailedHook(void)
{
    UART_SendString("MALLOC FAILED!\r\n");
    for (;;);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    UART_SendString("STACK OVERFLOW: ");
    UART_SendString(pcTaskName);
    UART_SendString("\r\n");
    for (;;);
}

/* ========================== HARDWARE INIT ================================= */

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
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* LED PC13 */
    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);
    
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET); /* LED OFF */
}

static void UART1_Init(void)
{
    __HAL_RCC_USART1_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* PA9 = TX, PA10 = RX */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart1);
}

/* ========================== MAIN ========================================== */

int main(void)
{
    uint8_t i;
    char cBuffer[50];
    
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    UART1_Init();

    UART_SendString("\r\n");
    UART_SendString("============================================\r\n");
    UART_SendString("    PROGRAM 44: SENSOR FUSION DEMO\r\n");
    UART_SendString("============================================\r\n");
    UART_SendString("Multi-sensor temperature fusion with outlier detection\r\n\r\n");

    /* Create sensor data queue */
    xSensorQueue = xQueueCreate(SENSOR_QUEUE_LENGTH, sizeof(SensorReading_t));
    if (xSensorQueue == NULL)
    {
        UART_SendString("ERROR: Failed to create sensor queue!\r\n");
        for (;;);
    }
    UART_SendString("Sensor queue created\r\n");

    /* Create health mutex */
    xHealthMutex = xSemaphoreCreateMutex();
    if (xHealthMutex == NULL)
    {
        UART_SendString("ERROR: Failed to create health mutex!\r\n");
        for (;;);
    }
    UART_SendString("Health mutex created\r\n");

    /* Initialize sensor health tracking */
    for (i = 0; i < NUM_SENSORS; i++)
    {
        xSensorHealth[i].ulReadings = 0;
        xSensorHealth[i].ulOutliers = 0;
        xSensorHealth[i].fLastValue = 0.0f;
        xSensorHealth[i].ucHealth = 100;
        xSensorHealth[i].ucOnline = 0;
    }

    /* Create sensor tasks */
    UART_SendString("Creating sensor tasks...\r\n");
    for (i = 0; i < NUM_SENSORS; i++)
    {
        snprintf(cBuffer, sizeof(cBuffer), "Sensor%d", i);
        BaseType_t xResult = xTaskCreate(
            vSensorTask,
            cBuffer,
            configMINIMAL_STACK_SIZE + 50,
            (void *)(uint32_t)i,
            2,
            &xSensorTaskHandles[i]
        );
        
        if (xResult != pdPASS)
        {
            snprintf(cBuffer, sizeof(cBuffer), 
                     "ERROR: Failed to create sensor %d task!\r\n", i);
            UART_SendString(cBuffer);
            for (;;);
        }
        
        snprintf(cBuffer, sizeof(cBuffer), 
                 "  Sensor %d task created\r\n", i);
        UART_SendString(cBuffer);
    }

    /* Create fusion task (higher priority) */
    if (xTaskCreate(vFusionTask, "Fusion", configMINIMAL_STACK_SIZE + 100, 
                    NULL, 3, &xFusionTaskHandle) != pdPASS)
    {
        UART_SendString("ERROR: Failed to create fusion task!\r\n");
        for (;;);
    }
    UART_SendString("Fusion task created\r\n");

    UART_SendString("\r\nStarting scheduler...\r\n");
    UART_SendString("----------------------------------------\r\n\r\n");

    vTaskStartScheduler();

    /* Should never reach here */
    for (;;);
}
