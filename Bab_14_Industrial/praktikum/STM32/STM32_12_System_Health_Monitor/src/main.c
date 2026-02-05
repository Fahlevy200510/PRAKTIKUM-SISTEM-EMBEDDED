/**
 * ===========================================================================
 * PROGRAM 50: SYSTEM HEALTH MONITOR - COMPREHENSIVE SYSTEM MONITORING
 * ===========================================================================
 * 
 * Demo ini mengimplementasikan sistem monitoring kesehatan lengkap yang
 * mengawasi memory, tasks, resources, dan peripherals. Memberikan health
 * score agregat dan detailed diagnostic reports.
 * 
 * KOMPONEN YANG DIMONITOR:
 * - Memory: Heap usage, minimum free, allocation failures
 * - Tasks: Stack high water mark, state, responsiveness
 * - Resources: Queue fill levels, uptime
 * - Peripherals: UART status
 * 
 * FITUR:
 * - Agregasi health score dengan weighted components
 * - LED status indicator berdasarkan health level
 * - Detailed periodic reports via UART
 * - Event logging untuk anomali
 * 
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/* ========== HARDWARE CONFIGURATION ========== */
#define LED_PIN         GPIO_PIN_13
#define LED_PORT        GPIOC

#define UART_TX_PIN     GPIO_PIN_9
#define UART_RX_PIN     GPIO_PIN_10
#define UART_PORT       GPIOA

/* ========== DATA STRUCTURES ========== */

/* Peripheral status */
typedef enum {
    PERIPH_OK = 0,
    PERIPH_DEGRADED,
    PERIPH_FAILED,
    PERIPH_NOT_AVAILABLE
} PeripheralStatus_t;

/* Health status */
typedef enum {
    STATUS_HEALTHY = 0,   /* 80-100% */
    STATUS_DEGRADED,      /* 60-79%  */
    STATUS_WARNING,       /* 40-59%  */
    STATUS_CRITICAL       /* 0-39%   */
} HealthStatus_t;

/* Event types */
typedef enum {
    EVENT_MEMORY_WARNING = 0,
    EVENT_MEMORY_CRITICAL,
    EVENT_STACK_WARNING,
    EVENT_QUEUE_WARNING,
    EVENT_PERIPHERAL_FAIL,
    EVENT_HEALTH_CHANGE
} EventType_t;

/* Health event record */
typedef struct {
    TickType_t timestamp;
    EventType_t type;
    char message[40];
} HealthEvent_t;

/* Queue tracking */
typedef struct {
    QueueHandle_t handle;
    const char *name;
    UBaseType_t capacity;
} TrackedQueue_t;

/* Component health scores */
typedef struct {
    uint8_t memoryScore;
    uint8_t taskScore;
    uint8_t resourceScore;
    uint8_t peripheralScore;
    uint8_t overallScore;
    HealthStatus_t status;
} HealthScores_t;

/* Memory stats */
typedef struct {
    size_t heapFree;
    size_t heapMinFree;
    size_t heapTotal;
    uint32_t allocFailures;
} MemoryStats_t;

/* Task stats */
typedef struct {
    uint8_t numTasks;
    uint8_t numBlocked;
    uint8_t numStackWarnings;
} TaskStats_t;

/* ========== GLOBAL VARIABLES ========== */
static UART_HandleTypeDef huart1;
static SemaphoreHandle_t uartMutex;

/* Tracked queues */
static TrackedQueue_t trackedQueues[HEALTH_MAX_TRACKED_QUEUES];
static uint8_t numTrackedQueues = 0;

/* Health data */
static HealthScores_t currentHealth;
static MemoryStats_t memoryStats;
static TaskStats_t taskStats;

/* Event log */
static HealthEvent_t eventLog[HEALTH_MAX_EVENTS];
static uint8_t eventIndex = 0;

/* Counters */
static volatile uint32_t mallocFailCount = 0;
static volatile uint32_t idleCounter = 0;
static TickType_t startTime;

/* Simulated peripherals */
static PeripheralStatus_t uartStatus = PERIPH_OK;
static PeripheralStatus_t gpioStatus = PERIPH_OK;
static PeripheralStatus_t adcStatus = PERIPH_OK;

/* Demo queues for monitoring */
static QueueHandle_t sensorQueue;
static QueueHandle_t logQueue;
static QueueHandle_t commandQueue;

/* Task handles */
static TaskHandle_t sensorTaskHandle;
static TaskHandle_t loggerTaskHandle;
static TaskHandle_t displayTaskHandle;
static TaskHandle_t healthMonitorHandle;
static TaskHandle_t stressTestHandle;

/* ========== FUNCTION PROTOTYPES ========== */
static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART1_Init(void);
static void UART_Print(const char *msg);
static void UART_Printf(const char *fmt, ...);

static void Health_RegisterQueue(QueueHandle_t queue, const char *name, UBaseType_t capacity);
static void Health_LogEvent(EventType_t type, const char *message);
static void Health_CollectMemoryStats(void);
static void Health_CollectTaskStats(void);
static uint8_t Health_CalculateMemoryScore(void);
static uint8_t Health_CalculateTaskScore(void);
static uint8_t Health_CalculateResourceScore(void);
static uint8_t Health_CalculatePeripheralScore(void);
static void Health_CalculateOverallHealth(void);
static HealthStatus_t Health_GetStatusFromScore(uint8_t score);
static const char* Health_GetStatusString(HealthStatus_t status);
static void Health_PrintReport(void);
static void Health_UpdateLED(void);
static void Health_FormatUptime(char *buffer, size_t size);

/* Tasks */
static void SensorTask(void *pvParameters);
static void LoggerTask(void *pvParameters);
static void DisplayTask(void *pvParameters);
static void HealthMonitorTask(void *pvParameters);
static void StressTestTask(void *pvParameters);

/* ========== SYSTEM INITIALIZATION ========== */

static void SystemClock_Config(void) {
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

static void GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    /* LED PC13 */
    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);
    
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET); /* LED OFF initially */
}

static void UART1_Init(void) {
    __HAL_RCC_USART1_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* TX PA9 */
    GPIO_InitStruct.Pin = UART_TX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(UART_PORT, &GPIO_InitStruct);
    
    /* RX PA10 */
    GPIO_InitStruct.Pin = UART_RX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(UART_PORT, &GPIO_InitStruct);
    
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

/* ========== UART FUNCTIONS ========== */

static void UART_Print(const char *msg) {
    if (xSemaphoreTake(uartMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        xSemaphoreGive(uartMutex);
    }
}

static void UART_Printf(const char *fmt, ...) {
    char buffer[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    UART_Print(buffer);
}

/* ========== HEALTH MONITORING FUNCTIONS ========== */

static void Health_RegisterQueue(QueueHandle_t queue, const char *name, UBaseType_t capacity) {
    if (numTrackedQueues < HEALTH_MAX_TRACKED_QUEUES) {
        trackedQueues[numTrackedQueues].handle = queue;
        trackedQueues[numTrackedQueues].name = name;
        trackedQueues[numTrackedQueues].capacity = capacity;
        numTrackedQueues++;
    }
}

static void Health_LogEvent(EventType_t type, const char *message) {
    eventLog[eventIndex].timestamp = xTaskGetTickCount();
    eventLog[eventIndex].type = type;
    strncpy(eventLog[eventIndex].message, message, sizeof(eventLog[0].message) - 1);
    eventLog[eventIndex].message[sizeof(eventLog[0].message) - 1] = '\0';
    
    eventIndex = (eventIndex + 1) % HEALTH_MAX_EVENTS;
}

static void Health_CollectMemoryStats(void) {
    memoryStats.heapFree = xPortGetFreeHeapSize();
    memoryStats.heapMinFree = xPortGetMinimumEverFreeHeapSize();
    memoryStats.heapTotal = configTOTAL_HEAP_SIZE;
    memoryStats.allocFailures = mallocFailCount;
}

static void Health_CollectTaskStats(void) {
    /* Count tasks and check stacks */
    UBaseType_t numTasks = uxTaskGetNumberOfTasks();
    taskStats.numTasks = (uint8_t)numTasks;
    taskStats.numBlocked = 0;
    taskStats.numStackWarnings = 0;
    
    /* Check stack high water marks */
    TaskHandle_t handles[] = {sensorTaskHandle, loggerTaskHandle, displayTaskHandle, healthMonitorHandle};
    const char *names[] = {"Sensor", "Logger", "Display", "Health"};
    
    for (int i = 0; i < 4; i++) {
        if (handles[i] != NULL) {
            UBaseType_t hwm = uxTaskGetStackHighWaterMark(handles[i]);
            /* Stack warning if less than 25% remaining */
            if (hwm < (configMINIMAL_STACK_SIZE / 4)) {
                taskStats.numStackWarnings++;
                char msg[40];
                snprintf(msg, sizeof(msg), "Stack low: %s", names[i]);
                Health_LogEvent(EVENT_STACK_WARNING, msg);
            }
        }
    }
}

static uint8_t Health_CalculateMemoryScore(void) {
    Health_CollectMemoryStats();
    
    /* Calculate usage percentage */
    uint32_t usedPercent = ((memoryStats.heapTotal - memoryStats.heapFree) * 100) / memoryStats.heapTotal;
    
    /* Base score is inverse of usage */
    uint8_t score = 100 - usedPercent;
    
    /* Apply penalties */
    if (usedPercent >= HEALTH_MEMORY_CRITICAL_PERCENT) {
        Health_LogEvent(EVENT_MEMORY_CRITICAL, "Heap critical!");
        score = (score > 25) ? score - 25 : 0;
    } else if (usedPercent >= HEALTH_MEMORY_WARNING_PERCENT) {
        Health_LogEvent(EVENT_MEMORY_WARNING, "Heap usage high");
        score = (score > 10) ? score - 10 : 0;
    }
    
    /* Penalty for allocation failures */
    if (memoryStats.allocFailures > 0) {
        score = (score > 20) ? score - 20 : 0;
    }
    
    return score;
}

static uint8_t Health_CalculateTaskScore(void) {
    Health_CollectTaskStats();
    
    /* Start with 100% */
    uint8_t score = 100;
    
    /* Penalty for stack warnings */
    if (taskStats.numStackWarnings > 0) {
        score -= (taskStats.numStackWarnings * 15);
    }
    
    /* Ensure score doesn't underflow */
    if (score > 100) score = 0;
    
    return score;
}

static uint8_t Health_CalculateResourceScore(void) {
    uint32_t totalScore = 0;
    uint8_t numChecked = 0;
    
    for (uint8_t i = 0; i < numTrackedQueues; i++) {
        if (trackedQueues[i].handle != NULL) {
            UBaseType_t count = uxQueueMessagesWaiting(trackedQueues[i].handle);
            UBaseType_t capacity = trackedQueues[i].capacity;
            
            /* Score is inverse of fill percentage */
            uint8_t fillPercent = (count * 100) / capacity;
            uint8_t queueScore = 100 - fillPercent;
            
            /* Log warning if near full */
            if (fillPercent >= HEALTH_QUEUE_WARNING_PERCENT) {
                char msg[40];
                snprintf(msg, sizeof(msg), "%s: %u%%", trackedQueues[i].name, fillPercent);
                Health_LogEvent(EVENT_QUEUE_WARNING, msg);
            }
            
            totalScore += queueScore;
            numChecked++;
        }
    }
    
    if (numChecked == 0) return 100;
    
    return (uint8_t)(totalScore / numChecked);
}

static uint8_t Health_CalculatePeripheralScore(void) {
    uint32_t totalScore = 0;
    uint8_t numPeripherals = 0;
    
    /* Check each peripheral */
    PeripheralStatus_t peripherals[] = {uartStatus, gpioStatus, adcStatus};
    
    for (int i = 0; i < 3; i++) {
        switch (peripherals[i]) {
            case PERIPH_OK:
                totalScore += 100;
                break;
            case PERIPH_DEGRADED:
                totalScore += 50;
                break;
            case PERIPH_FAILED:
                totalScore += 0;
                Health_LogEvent(EVENT_PERIPHERAL_FAIL, "Peripheral failed");
                break;
            case PERIPH_NOT_AVAILABLE:
                continue; /* Don't count N/A peripherals */
        }
        numPeripherals++;
    }
    
    if (numPeripherals == 0) return 100;
    
    return (uint8_t)(totalScore / numPeripherals);
}

static void Health_CalculateOverallHealth(void) {
    HealthStatus_t previousStatus = currentHealth.status;
    
    /* Calculate component scores */
    currentHealth.memoryScore = Health_CalculateMemoryScore();
    currentHealth.taskScore = Health_CalculateTaskScore();
    currentHealth.resourceScore = Health_CalculateResourceScore();
    currentHealth.peripheralScore = Health_CalculatePeripheralScore();
    
    /* Weighted average: Memory 25%, Tasks 30%, Resources 25%, Peripherals 20% */
    uint32_t overall = (currentHealth.memoryScore * 25) +
                       (currentHealth.taskScore * 30) +
                       (currentHealth.resourceScore * 25) +
                       (currentHealth.peripheralScore * 20);
    
    currentHealth.overallScore = (uint8_t)(overall / 100);
    currentHealth.status = Health_GetStatusFromScore(currentHealth.overallScore);
    
    /* Log status changes */
    if (currentHealth.status != previousStatus) {
        char msg[40];
        snprintf(msg, sizeof(msg), "Status: %s->%s", 
                Health_GetStatusString(previousStatus),
                Health_GetStatusString(currentHealth.status));
        Health_LogEvent(EVENT_HEALTH_CHANGE, msg);
    }
}

static HealthStatus_t Health_GetStatusFromScore(uint8_t score) {
    if (score >= 80) return STATUS_HEALTHY;
    if (score >= 60) return STATUS_DEGRADED;
    if (score >= 40) return STATUS_WARNING;
    return STATUS_CRITICAL;
}

static const char* Health_GetStatusString(HealthStatus_t status) {
    switch (status) {
        case STATUS_HEALTHY:  return "HEALTHY";
        case STATUS_DEGRADED: return "DEGRADED";
        case STATUS_WARNING:  return "WARNING";
        case STATUS_CRITICAL: return "CRITICAL";
        default:              return "UNKNOWN";
    }
}

static void Health_FormatUptime(char *buffer, size_t size) {
    TickType_t now = xTaskGetTickCount();
    uint32_t totalSec = (now - startTime) / configTICK_RATE_HZ;
    
    uint32_t hours = totalSec / 3600;
    uint32_t mins = (totalSec % 3600) / 60;
    uint32_t secs = totalSec % 60;
    
    snprintf(buffer, size, "%02lu:%02lu:%02lu", hours, mins, secs);
}

static void Health_PrintReport(void) {
    char uptimeStr[12];
    Health_FormatUptime(uptimeStr, sizeof(uptimeStr));
    
    UART_Print("\r\n");
    UART_Print("==================================================\r\n");
    UART_Printf("   SYSTEM HEALTH REPORT @ %s\r\n", uptimeStr);
    UART_Print("==================================================\r\n\r\n");
    
    /* Overall health with ASCII bar */
    UART_Print("OVERALL HEALTH: [");
    uint8_t barLen = currentHealth.overallScore / 5; /* 20 chars = 100% */
    for (int i = 0; i < 20; i++) {
        if (i < barLen) {
            UART_Print("#");
        } else {
            UART_Print("-");
        }
    }
    UART_Printf("] %u%% [%s]\r\n\r\n", currentHealth.overallScore, 
               Health_GetStatusString(currentHealth.status));
    
    /* Memory section */
    UART_Print("[MEMORY]\r\n");
    uint32_t heapUsed = memoryStats.heapTotal - memoryStats.heapFree;
    uint32_t usedPercent = (heapUsed * 100) / memoryStats.heapTotal;
    UART_Printf("  Heap: %lu/%lu used (%lu%%)\r\n", heapUsed, 
               (unsigned long)memoryStats.heapTotal, usedPercent);
    UART_Printf("  Free: %u bytes\r\n", (unsigned)memoryStats.heapFree);
    UART_Printf("  Min Free: %u bytes\r\n", (unsigned)memoryStats.heapMinFree);
    UART_Printf("  Alloc Fails: %lu\r\n", memoryStats.allocFailures);
    UART_Printf("  Score: %u%%\r\n\r\n", currentHealth.memoryScore);
    
    /* Tasks section */
    UART_Print("[TASKS]\r\n");
    UART_Printf("  Total: %u tasks\r\n", taskStats.numTasks);
    UART_Printf("  Stack warnings: %u\r\n", taskStats.numStackWarnings);
    UART_Printf("  Score: %u%%\r\n\r\n", currentHealth.taskScore);
    
    /* Resources section */
    UART_Print("[RESOURCES]\r\n");
    for (uint8_t i = 0; i < numTrackedQueues; i++) {
        if (trackedQueues[i].handle != NULL) {
            UBaseType_t count = uxQueueMessagesWaiting(trackedQueues[i].handle);
            UBaseType_t capacity = trackedQueues[i].capacity;
            uint8_t fillPercent = (count * 100) / capacity;
            const char *status = (fillPercent >= 80) ? " [!]" : "";
            UART_Printf("  %s: %u/%u (%u%%)%s\r\n", 
                       trackedQueues[i].name, (unsigned)count, (unsigned)capacity, 
                       fillPercent, status);
        }
    }
    UART_Printf("  Uptime: %s\r\n", uptimeStr);
    UART_Printf("  Score: %u%%\r\n\r\n", currentHealth.resourceScore);
    
    /* Peripherals section */
    UART_Print("[PERIPHERALS]\r\n");
    const char* periphNames[] = {"UART1", "GPIO", "ADC"};
    PeripheralStatus_t periphStats[] = {uartStatus, gpioStatus, adcStatus};
    const char* statusStrs[] = {"OK", "DEGRADED", "FAILED", "N/A"};
    
    for (int i = 0; i < 3; i++) {
        UART_Printf("  %s: %s\r\n", periphNames[i], statusStrs[periphStats[i]]);
    }
    UART_Printf("  Score: %u%%\r\n\r\n", currentHealth.peripheralScore);
    
    /* Recent events */
    UART_Print("[RECENT EVENTS]\r\n");
    uint8_t eventsShown = 0;
    for (int i = 0; i < HEALTH_MAX_EVENTS && eventsShown < 3; i++) {
        int idx = (eventIndex - 1 - i + HEALTH_MAX_EVENTS) % HEALTH_MAX_EVENTS;
        if (eventLog[idx].timestamp != 0) {
            uint32_t sec = eventLog[idx].timestamp / 1000;
            UART_Printf("  [%05lu] %s\r\n", sec, eventLog[idx].message);
            eventsShown++;
        }
    }
    if (eventsShown == 0) {
        UART_Print("  (no events)\r\n");
    }
    
    UART_Print("\r\n==================================================\r\n");
}

static void Health_UpdateLED(void) {
    static TickType_t lastToggle = 0;
    TickType_t now = xTaskGetTickCount();
    uint32_t blinkPeriod;
    
    switch (currentHealth.status) {
        case STATUS_HEALTHY:
            /* Solid ON */
            HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
            return;
            
        case STATUS_DEGRADED:
            blinkPeriod = 1000; /* Slow blink */
            break;
            
        case STATUS_WARNING:
            blinkPeriod = 250;  /* Fast blink */
            break;
            
        case STATUS_CRITICAL:
            blinkPeriod = 100;  /* Very fast blink */
            break;
            
        default:
            blinkPeriod = 500;
            break;
    }
    
    if (now - lastToggle >= pdMS_TO_TICKS(blinkPeriod)) {
        HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
        lastToggle = now;
    }
}

/* ========== APPLICATION TASKS ========== */

/* Simulated sensor task - produces data */
static void SensorTask(void *pvParameters) {
    (void)pvParameters;
    uint32_t sensorValue = 0;
    
    for (;;) {
        sensorValue++;
        
        /* Send to queue */
        if (xQueueSend(sensorQueue, &sensorValue, pdMS_TO_TICKS(100)) != pdPASS) {
            /* Queue full */
        }
        
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

/* Simulated logger task - consumes and logs data */
static void LoggerTask(void *pvParameters) {
    (void)pvParameters;
    uint32_t value;
    
    for (;;) {
        /* Receive from sensor queue */
        if (xQueueReceive(sensorQueue, &value, pdMS_TO_TICKS(500)) == pdPASS) {
            /* Log entry - add to log queue */
            if (xQueueSend(logQueue, &value, 0) != pdPASS) {
                /* Log queue full - drop entry */
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(300)); /* Slower than producer = backpressure */
    }
}

/* Display task - shows status periodically */
static void DisplayTask(void *pvParameters) {
    (void)pvParameters;
    
    for (;;) {
        /* Display mini status */
        UART_Printf("\r[Health: %u%% | Heap: %u | Up: %lu s]  ",
                   currentHealth.overallScore,
                   (unsigned)xPortGetFreeHeapSize(),
                   (unsigned long)(xTaskGetTickCount() / configTICK_RATE_HZ));
        
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

/* Health monitor task - main monitoring loop */
static void HealthMonitorTask(void *pvParameters) {
    (void)pvParameters;
    TickType_t lastReportTime = xTaskGetTickCount();
    
    /* Initial delay to let system stabilize */
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    for (;;) {
        /* Calculate health scores */
        Health_CalculateOverallHealth();
        
        /* Update LED based on status */
        Health_UpdateLED();
        
        /* Print full report periodically */
        if (xTaskGetTickCount() - lastReportTime >= pdMS_TO_TICKS(HEALTH_REPORT_PERIOD_MS)) {
            Health_PrintReport();
            lastReportTime = xTaskGetTickCount();
        }
        
        vTaskDelay(pdMS_TO_TICKS(100)); /* Monitor at 10Hz */
    }
}

/* Stress test task - intentionally causes load variations */
static void StressTestTask(void *pvParameters) {
    (void)pvParameters;
    uint32_t cycle = 0;
    
    for (;;) {
        cycle++;
        
        /* Every 20 seconds, fill log queue to simulate backpressure */
        if ((cycle % 40) == 0) {
            UART_Print("\r\n[STRESS] Filling log queue...\r\n");
            uint32_t dummy = 999;
            for (int i = 0; i < 8; i++) {
                xQueueSend(logQueue, &dummy, 0);
            }
        }
        
        /* Every 30 seconds, simulate peripheral degradation */
        if ((cycle % 60) == 0) {
            UART_Print("\r\n[STRESS] Simulating ADC degradation...\r\n");
            adcStatus = PERIPH_DEGRADED;
        }
        
        /* Recover after 10 seconds */
        if ((cycle % 60) == 20) {
            adcStatus = PERIPH_OK;
        }
        
        /* Drain log queue occasionally to recover */
        if ((cycle % 10) == 5) {
            uint32_t drain;
            for (int i = 0; i < 3; i++) {
                xQueueReceive(logQueue, &drain, 0);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* ========== FREERTOS HOOKS ========== */

void vApplicationIdleHook(void) {
    idleCounter++;
}

void vApplicationMallocFailedHook(void) {
    mallocFailCount++;
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void)xTask;
    (void)pcTaskName;
    
    /* Log and halt */
    for (;;) {
        HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
        HAL_Delay(50);
    }
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    /* Initialize hardware */
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    UART1_Init();
    
    /* Record start time */
    startTime = 0;
    
    /* Initialize health data */
    memset(&currentHealth, 0, sizeof(currentHealth));
    memset(&memoryStats, 0, sizeof(memoryStats));
    memset(&taskStats, 0, sizeof(taskStats));
    memset(eventLog, 0, sizeof(eventLog));
    currentHealth.status = STATUS_HEALTHY;
    
    /* Print banner */
    UART_Print("\r\n");
    UART_Print("============================================\r\n");
    UART_Print("  PROGRAM 50: SYSTEM HEALTH MONITOR\r\n");
    UART_Print("  Comprehensive System Monitoring Demo\r\n");
    UART_Print("============================================\r\n\r\n");
    
    UART_Print("This demo monitors:\r\n");
    UART_Print("  - Memory usage (heap, allocations)\r\n");
    UART_Print("  - Task health (stack, states)\r\n");
    UART_Print("  - Resources (queues, uptime)\r\n");
    UART_Print("  - Peripherals (UART, GPIO, ADC)\r\n\r\n");
    
    UART_Print("Health status levels:\r\n");
    UART_Print("  HEALTHY (80-100%): LED solid ON\r\n");
    UART_Print("  DEGRADED (60-79%): LED slow blink\r\n");
    UART_Print("  WARNING (40-59%):  LED fast blink\r\n");
    UART_Print("  CRITICAL (0-39%):  LED very fast blink\r\n\r\n");
    
    UART_Print("Starting system health monitoring...\r\n\r\n");
    
    /* Create mutex */
    uartMutex = xSemaphoreCreateMutex();
    
    /* Create demo queues */
    sensorQueue = xQueueCreate(10, sizeof(uint32_t));
    logQueue = xQueueCreate(10, sizeof(uint32_t));
    commandQueue = xQueueCreate(5, sizeof(uint32_t));
    
    /* Register queues for monitoring */
    Health_RegisterQueue(sensorQueue, "SensorQ", 10);
    Health_RegisterQueue(logQueue, "LogQ", 10);
    Health_RegisterQueue(commandQueue, "CmdQ", 5);
    
    /* Create tasks */
    xTaskCreate(SensorTask, "Sensor", 128, NULL, 2, &sensorTaskHandle);
    xTaskCreate(LoggerTask, "Logger", 128, NULL, 2, &loggerTaskHandle);
    xTaskCreate(DisplayTask, "Display", 128, NULL, 1, &displayTaskHandle);
    xTaskCreate(HealthMonitorTask, "HealthMon", 256, NULL, 3, &healthMonitorHandle);
    xTaskCreate(StressTestTask, "Stress", 128, NULL, 1, &stressTestHandle);
    
    /* Start scheduler */
    vTaskStartScheduler();
    
    /* Should never reach here */
    for (;;);
}
