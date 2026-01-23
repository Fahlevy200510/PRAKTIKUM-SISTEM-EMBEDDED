/**
 * ============================================================================
 * PRAKTIKUM 10: COOPERATIVE MULTITASKING
 * ============================================================================
 * 
 * Tujuan Pembelajaran:
 * - Memahami konsep multitasking tanpa RTOS
 * - Implementasi scheduler sederhana
 * - Non-blocking task execution
 * - State machine pattern untuk concurrent-like behavior
 * - Memahami timing dan prioritas task
 * 
 * Konsep:
 * - Cooperative multitasking tidak memerlukan RTOS
 * - Setiap task harus "yield" (memberikan kontrol) secara sukarela
 * - Menggunakan timer untuk schedule berbagai aktivitas
 * - Cocok untuk sistem sederhana dengan resource terbatas
 * 
 * Hardware:
 * - STM32F103C8T6 (Blue Pill)
 * - LED di PC13 (onboard)
 * - LED external di PA1 (opsional)
 * - Button di PB0 (opsional)
 * 
 * ============================================================================
 */

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

/* UART Handle */
UART_HandleTypeDef huart1;

/* ============================================
 * SIMPLE TASK SCHEDULER
 * ============================================ */

/* Task state */
typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_WAITING,
    TASK_SUSPENDED
} TaskState_t;

/* Task structure */
typedef struct {
    void (*taskFunction)(void);     // Pointer ke fungsi task
    const char* taskName;           // Nama task
    uint32_t period;                // Periode eksekusi (ms)
    uint32_t lastRun;               // Waktu eksekusi terakhir
    TaskState_t state;              // State task
    uint32_t runCount;              // Counter berapa kali task dijalankan
} Task_t;

/* Maximum number of tasks */
#define MAX_TASKS 8

/* Task list */
static Task_t taskList[MAX_TASKS];
static uint8_t taskCount = 0;

/* Scheduler statistics */
static uint32_t schedulerCycles = 0;
static uint32_t idleTime = 0;

/* Private function prototypes */
void SystemClock_Config(void);
void Error_Handler(void);
void GPIO_Init(void);
void UART1_Init(void);
void UART_SendString(const char* str);

/* Scheduler functions */
void Scheduler_Init(void);
uint8_t Scheduler_AddTask(void (*taskFunc)(void), const char* name, uint32_t period);
void Scheduler_Run(void);
void Scheduler_SuspendTask(uint8_t taskId);
void Scheduler_ResumeTask(uint8_t taskId);
void Scheduler_PrintStats(void);

/* Task function prototypes */
void Task_LED1_Toggle(void);
void Task_LED2_Pattern(void);
void Task_Button_Check(void);
void Task_UART_Heartbeat(void);
void Task_System_Monitor(void);
void Task_Counter(void);

/* Shared variables between tasks (simulasi) */
volatile uint32_t sharedCounter = 0;
volatile uint8_t buttonPressed = 0;
volatile uint8_t led1State = 0;
volatile uint8_t led2State = 0;

/**
 * @brief  Main program
 * @retval int
 */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    UART1_Init();
    
    UART_SendString("\r\n=========================================\r\n");
    UART_SendString("STM32F103C8T6 Cooperative Multitasking Demo\r\n");
    UART_SendString("=========================================\r\n");
    UART_SendString("Simple Task Scheduler without RTOS\r\n");
    UART_SendString("=========================================\r\n\r\n");
    
    /* Initialize scheduler */
    Scheduler_Init();
    
    /* Add tasks with different periods */
    UART_SendString("Adding tasks...\r\n");
    
    Scheduler_AddTask(Task_LED1_Toggle,     "LED1",     500);    // 500ms - LED blink
    Scheduler_AddTask(Task_LED2_Pattern,    "LED2",     100);    // 100ms - LED pattern
    Scheduler_AddTask(Task_Button_Check,    "Button",   50);     // 50ms  - Button polling
    Scheduler_AddTask(Task_UART_Heartbeat,  "UART",     1000);   // 1s    - Heartbeat
    Scheduler_AddTask(Task_System_Monitor,  "Monitor",  5000);   // 5s    - System stats
    Scheduler_AddTask(Task_Counter,         "Counter",  200);    // 200ms - Counter
    
    char msg[80];
    sprintf(msg, "\r\n%d tasks registered\r\n\r\n", taskCount);
    UART_SendString(msg);
    
    UART_SendString("Starting scheduler loop...\r\n\r\n");
    
    /* Run scheduler forever */
    Scheduler_Run();
    
    /* Should never reach here */
    while (1);
}

/**
 * @brief  Initialize the scheduler
 */
void Scheduler_Init(void)
{
    taskCount = 0;
    schedulerCycles = 0;
    
    for (int i = 0; i < MAX_TASKS; i++)
    {
        taskList[i].taskFunction = NULL;
        taskList[i].taskName = "";
        taskList[i].period = 0;
        taskList[i].lastRun = 0;
        taskList[i].state = TASK_SUSPENDED;
        taskList[i].runCount = 0;
    }
}

/**
 * @brief  Add a task to the scheduler
 */
uint8_t Scheduler_AddTask(void (*taskFunc)(void), const char* name, uint32_t period)
{
    if (taskCount >= MAX_TASKS)
    {
        return 0xFF;  // Error: too many tasks
    }
    
    uint8_t id = taskCount;
    
    taskList[id].taskFunction = taskFunc;
    taskList[id].taskName = name;
    taskList[id].period = period;
    taskList[id].lastRun = 0;
    taskList[id].state = TASK_READY;
    taskList[id].runCount = 0;
    
    taskCount++;
    
    char msg[60];
    sprintf(msg, "  Task[%d]: %s (period=%lums)\r\n", id, name, period);
    UART_SendString(msg);
    
    return id;
}

/**
 * @brief  Main scheduler loop - runs forever
 */
void Scheduler_Run(void)
{
    while (1)
    {
        uint32_t currentTime = HAL_GetTick();
        uint8_t taskExecuted = 0;
        
        /* Check each task */
        for (uint8_t i = 0; i < taskCount; i++)
        {
            /* Skip suspended or already running tasks */
            if (taskList[i].state != TASK_READY)
            {
                continue;
            }
            
            /* Check if it's time to run this task */
            if ((currentTime - taskList[i].lastRun) >= taskList[i].period)
            {
                /* Mark task as running */
                taskList[i].state = TASK_RUNNING;
                
                /* Execute the task */
                if (taskList[i].taskFunction != NULL)
                {
                    taskList[i].taskFunction();
                }
                
                /* Update timing */
                taskList[i].lastRun = currentTime;
                taskList[i].runCount++;
                
                /* Mark task as ready again */
                taskList[i].state = TASK_READY;
                
                taskExecuted = 1;
            }
        }
        
        /* Track idle time when no task was executed */
        if (!taskExecuted)
        {
            idleTime++;
        }
        
        schedulerCycles++;
    }
}

/**
 * @brief  Suspend a task by ID
 */
void Scheduler_SuspendTask(uint8_t taskId)
{
    if (taskId < taskCount)
    {
        taskList[taskId].state = TASK_SUSPENDED;
    }
}

/**
 * @brief  Resume a suspended task
 */
void Scheduler_ResumeTask(uint8_t taskId)
{
    if (taskId < taskCount && taskList[taskId].state == TASK_SUSPENDED)
    {
        taskList[taskId].state = TASK_READY;
        taskList[taskId].lastRun = HAL_GetTick();  // Prevent immediate execution
    }
}

/**
 * @brief  Print scheduler statistics
 */
void Scheduler_PrintStats(void)
{
    char msg[80];
    
    UART_SendString("\r\n--- Scheduler Statistics ---\r\n");
    
    sprintf(msg, "Uptime: %lu ms\r\n", HAL_GetTick());
    UART_SendString(msg);
    
    sprintf(msg, "Scheduler cycles: %lu\r\n", schedulerCycles);
    UART_SendString(msg);
    
    sprintf(msg, "Idle cycles: %lu\r\n", idleTime);
    UART_SendString(msg);
    
    UART_SendString("\r\nTask Statistics:\r\n");
    
    for (uint8_t i = 0; i < taskCount; i++)
    {
        sprintf(msg, "  [%d] %-10s: runs=%lu, state=%d\r\n",
                i, taskList[i].taskName, taskList[i].runCount, taskList[i].state);
        UART_SendString(msg);
    }
    
    UART_SendString("-----------------------------\r\n\r\n");
}

/* ============================================
 * TASK IMPLEMENTATIONS
 * ============================================ */

/**
 * @brief  Task: Toggle LED1 (onboard PC13)
 */
void Task_LED1_Toggle(void)
{
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    led1State = !led1State;
}

/**
 * @brief  Task: LED2 Pattern (external LED on PA1)
 */
void Task_LED2_Pattern(void)
{
    static uint8_t pattern_state = 0;
    
    /* Simple pattern: on for 300ms, off for 700ms */
    pattern_state++;
    
    if (pattern_state < 3)
    {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
        led2State = 1;
    }
    else
    {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
        led2State = 0;
    }
    
    if (pattern_state >= 10)
    {
        pattern_state = 0;
    }
}

/**
 * @brief  Task: Check button state
 */
void Task_Button_Check(void)
{
    static uint8_t lastButtonState = 1;
    uint8_t currentState = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);
    
    /* Detect button press (falling edge) */
    if (currentState == GPIO_PIN_RESET && lastButtonState == GPIO_PIN_SET)
    {
        buttonPressed++;
        
        char msg[40];
        sprintf(msg, "[Button] Pressed! Count: %d\r\n", buttonPressed);
        UART_SendString(msg);
    }
    
    lastButtonState = currentState;
}

/**
 * @brief  Task: Send UART heartbeat
 */
void Task_UART_Heartbeat(void)
{
    static uint32_t heartbeat = 0;
    char msg[60];
    
    heartbeat++;
    sprintf(msg, "[Heartbeat #%lu] LED1=%d, LED2=%d, Counter=%lu\r\n",
            heartbeat, led1State, led2State, sharedCounter);
    UART_SendString(msg);
}

/**
 * @brief  Task: System monitor - prints statistics
 */
void Task_System_Monitor(void)
{
    Scheduler_PrintStats();
}

/**
 * @brief  Task: Simple counter task
 */
void Task_Counter(void)
{
    sharedCounter++;
    
    /* Demo: milestone notification every 50 counts */
    if (sharedCounter % 50 == 0)
    {
        UART_SendString("[Counter] Milestone reached!\r\n");
    }
}

/* ============================================
 * INITIALIZATION FUNCTIONS
 * ============================================ */

/**
 * @brief  Inisialisasi GPIO
 */
void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    /* LED PC13 (onboard) */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    /* LED PA1 (external) */
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* Button PB0 */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
}

/**
 * @brief  Inisialisasi UART1
 */
void UART1_Init(void)
{
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* TX (PA9) */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* RX (PA10) */
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
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

void UART_SendString(const char* str)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
}

/**
 * @brief  System Clock Configuration - 72MHz
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

void Error_Handler(void)
{
    while (1)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_Delay(100);
    }
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}
