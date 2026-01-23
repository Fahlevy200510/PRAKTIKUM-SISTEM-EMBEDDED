/**
 * Program 30: SysTick Precision Delay and Timing
 * 
 * Deskripsi:
 * Program ini mendemonstrasikan penggunaan SysTick timer untuk 
 * delay presisi tinggi (microseconds) dan pengukuran waktu eksekusi.
 * Cocok untuk aplikasi yang membutuhkan timing presisi.
 * 
 * Fitur:
 * - Microsecond delay (1µs resolution)
 * - Code execution time measurement
 * - Millisecond delay (standard HAL_Delay)
 * - DWT cycle counter untuk profiling
 * - Multiple timer comparison (SysTick vs DWT vs TIM)
 * - Timing accuracy benchmark
 * 
 * SysTick Configuration:
 * - Clock: 72 MHz (HCLK)
 * - Resolution: 1/72 MHz = 13.89 ns per tick
 * - Max delay: 2^24 / 72MHz ≈ 233ms (24-bit counter)
 * 
 * DWT Cycle Counter:
 * - 32-bit counter
 * - Clock: 72 MHz
 * - Free-running, no interrupt
 * - Wrap-around setiap 59.65 seconds @ 72MHz
 * 
 * Aplikasi:
 * - Sensor timing (DHT22, DHT11, DS18B20)
 * - Protocol timing (1-Wire, WS2812B)
 * - Code profiling dan optimization
 * - Precise PWM/signal generation
 * 
 * Hardware:
 * - STM32F103C8T6 Blue Pill
 * - UART USB adapter (PA9/PA10)
 * - LED PC13 (onboard)
 * - Oscilloscope (optional, untuk verifikasi timing)
 * 
 * Pin Configuration:
 * - PA9: UART TX
 * - PA10: UART RX
 * - PC13: LED (timing test)
 * - PB0: Test pin (timing benchmark)
 */

#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* UART handle */
UART_HandleTypeDef huart1;

/* DWT cycle counter enable */
#define DWT_ENABLE()    do { \
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; \
    DWT->CYCCNT = 0; \
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk; \
} while(0)

/* Function prototypes */
void SystemClock_Config(void);
void GPIO_Init(void);
void UART_Init(void);
void Delay_us(uint32_t us);
void Delay_us_DWT(uint32_t us);
uint32_t Get_Cycles(void);
void Benchmark_Delays(void);
void Benchmark_GPIO(void);
void Benchmark_Math(void);
void Test_Timing_Accuracy(void);

/* Buffer */
char msg[150];

/**
 * Main function
 */
int main(void)
{
    /* Initialize HAL */
    HAL_Init();

    /* Configure system clock */
    SystemClock_Config();

    /* Enable DWT cycle counter */
    DWT_ENABLE();

    /* Initialize peripherals */
    GPIO_Init();
    UART_Init();

    sprintf(msg, "\r\n\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "╔═══════════════════════════════════════╗\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "║  SysTick Precision Timing System     ║\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "║  STM32F103C8T6 @ 72MHz               ║\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "╚═══════════════════════════════════════╝\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    sprintf(msg, "\r\nSystem Configuration:\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "- SYSCLK: %lu MHz\r\n", HAL_RCC_GetSysClockFreq() / 1000000);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "- HCLK: %lu MHz\r\n", HAL_RCC_GetHCLKFreq() / 1000000);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "- SysTick: 1 ms per tick\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "- DWT Cycle Counter: Enabled\r\n\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    /* Run benchmarks */
    sprintf(msg, "═══════════════════════════════════════\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "  BENCHMARK 1: Delay Functions\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "═══════════════════════════════════════\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    Benchmark_Delays();

    HAL_Delay(1000);

    sprintf(msg, "\r\n═══════════════════════════════════════\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "  BENCHMARK 2: GPIO Operations\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "═══════════════════════════════════════\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    Benchmark_GPIO();

    HAL_Delay(1000);

    sprintf(msg, "\r\n═══════════════════════════════════════\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "  BENCHMARK 3: Math Operations\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "═══════════════════════════════════════\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    Benchmark_Math();

    HAL_Delay(1000);

    sprintf(msg, "\r\n═══════════════════════════════════════\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "  BENCHMARK 4: Timing Accuracy Test\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "═══════════════════════════════════════\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    Test_Timing_Accuracy();

    sprintf(msg, "\r\n\r\nBenchmark selesai!\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "LED akan blink dengan timing presisi...\r\n\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    /* Main loop - LED blink dengan timing presisi */
    while(1)
    {
        /* Blink dengan delay microsecond */
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
        Delay_us(500000); // 500ms
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
        Delay_us(500000); // 500ms

        /* Print uptime setiap 10 detik */
        static uint32_t last_print = 0;
        uint32_t now = HAL_GetTick();
        if((now - last_print) >= 10000)
        {
            last_print = now;
            sprintf(msg, "Uptime: %lu seconds\r\n", now / 1000);
            HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);
        }
    }
}

/**
 * System Clock Configuration
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
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

/**
 * GPIO Initialization
 */
void GPIO_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* PC13 - LED */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* PB0 - Test pin */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/**
 * UART Initialization
 */
void UART_Init(void)
{
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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

/**
 * Microsecond delay menggunakan SysTick
 */
void Delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t cycles = us * (SystemCoreClock / 1000000);
    
    while((DWT->CYCCNT - start) < cycles)
    {
        // Wait
    }
}

/**
 * Alternative microsecond delay menggunakan pure DWT
 */
void Delay_us_DWT(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * 72; // 72 cycles per microsecond @ 72MHz
    
    while((DWT->CYCCNT - start) < ticks);
}

/**
 * Get current cycle count
 */
uint32_t Get_Cycles(void)
{
    return DWT->CYCCNT;
}

/**
 * Benchmark Delay Functions
 */
void Benchmark_Delays(void)
{
    uint32_t start, end, cycles;

    /* Test 1: HAL_Delay(1) */
    start = Get_Cycles();
    HAL_Delay(1);
    end = Get_Cycles();
    cycles = end - start;
    sprintf(msg, "HAL_Delay(1ms):     %lu cycles (%.2f µs)\r\n", 
            cycles, cycles / 72.0f);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    /* Test 2: Delay_us(1000) */
    start = Get_Cycles();
    Delay_us(1000);
    end = Get_Cycles();
    cycles = end - start;
    sprintf(msg, "Delay_us(1000µs):   %lu cycles (%.2f µs)\r\n", 
            cycles, cycles / 72.0f);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    /* Test 3: Delay_us(100) */
    start = Get_Cycles();
    Delay_us(100);
    end = Get_Cycles();
    cycles = end - start;
    sprintf(msg, "Delay_us(100µs):    %lu cycles (%.2f µs)\r\n", 
            cycles, cycles / 72.0f);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    /* Test 4: Delay_us(10) */
    start = Get_Cycles();
    Delay_us(10);
    end = Get_Cycles();
    cycles = end - start;
    sprintf(msg, "Delay_us(10µs):     %lu cycles (%.2f µs)\r\n", 
            cycles, cycles / 72.0f);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    /* Test 5: Delay_us(1) */
    start = Get_Cycles();
    Delay_us(1);
    end = Get_Cycles();
    cycles = end - start;
    sprintf(msg, "Delay_us(1µs):      %lu cycles (%.2f µs)\r\n", 
            cycles, cycles / 72.0f);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

/**
 * Benchmark GPIO Operations
 */
void Benchmark_GPIO(void)
{
    uint32_t start, end, cycles;

    /* Test 1: Single GPIO toggle */
    start = Get_Cycles();
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
    end = Get_Cycles();
    cycles = end - start;
    sprintf(msg, "GPIO Toggle:        %lu cycles (%.2f ns)\r\n", 
            cycles, (cycles * 1000.0f) / 72.0f);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    /* Test 2: GPIO Write (direct register) */
    start = Get_Cycles();
    GPIOB->BSRR = GPIO_PIN_0;
    end = Get_Cycles();
    cycles = end - start;
    sprintf(msg, "GPIO Write (BSRR):  %lu cycles (%.2f ns)\r\n", 
            cycles, (cycles * 1000.0f) / 72.0f);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    /* Test 3: 1000 GPIO toggles */
    start = Get_Cycles();
    for(int i = 0; i < 1000; i++)
    {
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
    }
    end = Get_Cycles();
    cycles = (end - start) / 1000;
    sprintf(msg, "1000x Toggle avg:   %lu cycles (%.2f ns)\r\n", 
            cycles, (cycles * 1000.0f) / 72.0f);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

/**
 * Benchmark Math Operations
 */
void Benchmark_Math(void)
{
    uint32_t start, end, cycles;
    volatile uint32_t result; // volatile to prevent optimization

    /* Test 1: Integer addition */
    start = Get_Cycles();
    result = 12345 + 67890;
    end = Get_Cycles();
    cycles = end - start;
    sprintf(msg, "Integer ADD:        %lu cycles\r\n", cycles);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    /* Test 2: Integer multiplication */
    start = Get_Cycles();
    result = 12345 * 67890;
    end = Get_Cycles();
    cycles = end - start;
    sprintf(msg, "Integer MUL:        %lu cycles\r\n", cycles);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    /* Test 3: Integer division */
    start = Get_Cycles();
    result = 12345678 / 12345;
    end = Get_Cycles();
    cycles = end - start;
    sprintf(msg, "Integer DIV:        %lu cycles\r\n", cycles);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    /* Test 4: Float multiplication */
    volatile float f_result;
    start = Get_Cycles();
    f_result = 123.45f * 678.90f;
    end = Get_Cycles();
    cycles = end - start;
    sprintf(msg, "Float MUL:          %lu cycles\r\n", cycles);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    /* Test 5: Float division */
    start = Get_Cycles();
    f_result = 12345.678f / 123.45f;
    end = Get_Cycles();
    cycles = end - start;
    sprintf(msg, "Float DIV:          %lu cycles\r\n", cycles);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

/**
 * Test Timing Accuracy
 */
void Test_Timing_Accuracy(void)
{
    uint32_t delays_us[] = {1, 5, 10, 50, 100, 500, 1000};
    
    for(int i = 0; i < 7; i++)
    {
        uint32_t target_us = delays_us[i];
        uint32_t start = Get_Cycles();
        Delay_us(target_us);
        uint32_t end = Get_Cycles();
        
        uint32_t actual_cycles = end - start;
        float actual_us = actual_cycles / 72.0f;
        float error_percent = ((actual_us - target_us) / target_us) * 100.0f;
        
        sprintf(msg, "Target: %4lu µs | Actual: %.2f µs | Error: %+.2f%%\r\n",
                target_us, actual_us, error_percent);
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    }
}

/**
 * SysTick Handler
 */
void SysTick_Handler(void)
{
    HAL_IncTick();
}
