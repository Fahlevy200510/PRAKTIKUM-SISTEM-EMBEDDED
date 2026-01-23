/**
 * ============================================================================
 * PRAKTIKUM 3: UART SERIAL COMMUNICATION
 * ============================================================================
 * 
 * Tujuan Pembelajaran:
 * - Memahami konfigurasi UART untuk komunikasi serial
 * - Memahami transmisi dan penerimaan data via UART
 * - Mengimplementasikan printf untuk debugging
 * - Memahami penggunaan interrupt untuk UART
 * 
 * Hardware:
 * - STM32F103C8T6 (Blue Pill)
 * - USB-to-TTL Serial Adapter (CP2102, CH340, FTDI)
 * - LED di PC13
 * 
 * Wiring UART1:
 *   PA9  (TX)  -----> RX pada USB-TTL
 *   PA10 (RX)  <----- TX pada USB-TTL
 *   GND        -----> GND pada USB-TTL
 * 
 * ============================================================================
 */

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

/* UART Handle */
UART_HandleTypeDef huart1;

/* Buffer untuk receive */
#define RX_BUFFER_SIZE 64
uint8_t rx_buffer[RX_BUFFER_SIZE];
uint8_t rx_data;
volatile uint8_t rx_index = 0;
volatile uint8_t rx_complete = 0;

/* Private function prototypes */
void SystemClock_Config(void);
void Error_Handler(void);
void GPIO_Init(void);
void UART1_Init(void);
void UART_SendString(const char* str);
void Process_Command(char* cmd);

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
    
    /* Kirim pesan selamat datang */
    UART_SendString("\r\n===================================\r\n");
    UART_SendString("STM32F103C8T6 UART Demo\r\n");
    UART_SendString("===================================\r\n");
    UART_SendString("Perintah tersedia:\r\n");
    UART_SendString("  LED ON   - Nyalakan LED\r\n");
    UART_SendString("  LED OFF  - Matikan LED\r\n");
    UART_SendString("  TOGGLE   - Toggle LED\r\n");
    UART_SendString("  STATUS   - Tampilkan status\r\n");
    UART_SendString("===================================\r\n");
    UART_SendString("> ");
    
    /* Start receive interrupt */
    HAL_UART_Receive_IT(&huart1, &rx_data, 1);
    
    uint32_t counter = 0;
    
    while (1)
    {
        /* Cek apakah ada command yang diterima */
        if (rx_complete)
        {
            rx_buffer[rx_index] = '\0';  // Null terminate
            Process_Command((char*)rx_buffer);
            rx_index = 0;
            rx_complete = 0;
            UART_SendString("> ");
        }
        
        /* Kirim heartbeat setiap 5 detik */
        HAL_Delay(1000);
        counter++;
        if (counter % 5 == 0)
        {
            char msg[50];
            sprintf(msg, "[Heartbeat: %lu detik]\r\n> ", counter);
            UART_SendString(msg);
        }
    }
}

/**
 * @brief  Proses command yang diterima
 * @param  cmd: Command string
 * @retval None
 */
void Process_Command(char* cmd)
{
    UART_SendString("\r\n");
    
    if (strcmp(cmd, "LED ON") == 0 || strcmp(cmd, "led on") == 0)
    {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
        UART_SendString("LED menyala\r\n");
    }
    else if (strcmp(cmd, "LED OFF") == 0 || strcmp(cmd, "led off") == 0)
    {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
        UART_SendString("LED mati\r\n");
    }
    else if (strcmp(cmd, "TOGGLE") == 0 || strcmp(cmd, "toggle") == 0)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        UART_SendString("LED di-toggle\r\n");
    }
    else if (strcmp(cmd, "STATUS") == 0 || strcmp(cmd, "status") == 0)
    {
        char status[100];
        uint8_t led_state = !HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);
        sprintf(status, "Status LED: %s\r\n", led_state ? "ON" : "OFF");
        UART_SendString(status);
        
        sprintf(status, "System Clock: %lu MHz\r\n", HAL_RCC_GetSysClockFreq() / 1000000);
        UART_SendString(status);
        
        sprintf(status, "Uptime: %lu ms\r\n", HAL_GetTick());
        UART_SendString(status);
    }
    else if (strlen(cmd) > 0)
    {
        UART_SendString("Command tidak dikenal: ");
        UART_SendString(cmd);
        UART_SendString("\r\n");
    }
}

/**
 * @brief  Kirim string via UART
 * @param  str: String yang akan dikirim
 * @retval None
 */
void UART_SendString(const char* str)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
}

/**
 * @brief  UART Receive Complete Callback
 * @param  huart: UART handle
 * @retval None
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        /* Echo karakter yang diterima */
        HAL_UART_Transmit(&huart1, &rx_data, 1, 10);
        
        if (rx_data == '\r' || rx_data == '\n')
        {
            /* Command selesai */
            rx_complete = 1;
        }
        else if (rx_data == 0x7F || rx_data == 0x08)  // Backspace
        {
            if (rx_index > 0)
            {
                rx_index--;
                UART_SendString(" \b");  // Hapus karakter di terminal
            }
        }
        else if (rx_index < RX_BUFFER_SIZE - 1)
        {
            rx_buffer[rx_index++] = rx_data;
        }
        
        /* Aktifkan receive interrupt lagi */
        HAL_UART_Receive_IT(&huart1, &rx_data, 1);
    }
}

/**
 * @brief  Inisialisasi UART1
 * @retval None
 */
void UART1_Init(void)
{
    /* Enable clock untuk USART1 dan GPIOA */
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    /* Konfigurasi GPIO untuk UART1 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* PA9 = TX (Alternate Push-Pull) */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* PA10 = RX (Input Floating) */
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* Konfigurasi UART1 */
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
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
    
    /* Enable UART interrupt */
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
}

/**
 * @brief  USART1 Interrupt Handler
 * @retval None
 */
void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);
}

/**
 * @brief  Inisialisasi GPIO untuk LED
 * @retval None
 */
void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}

/**
 * @brief  System Clock Configuration - 72MHz
 * @retval None
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
