/**
 * Program 25: Flash Write Protection
 * 
 * Deskripsi:
 * Program ini mendemonstrasikan cara mengaktifkan dan menonaktifkan 
 * write protection pada flash memory STM32F103C8T6 untuk keamanan aplikasi.
 * 
 * Fitur:
 * - Membaca status write protection untuk semua pages
 * - Mengaktifkan write protection untuk page tertentu
 * - Menonaktifkan write protection
 * - Option byte programming
 * - UART interface untuk kontrol dan monitoring
 * 
 * Option Bytes:
 * - Read Protection Level (RDP): LEVEL_0, LEVEL_1
 * - Write Protection: Per page/sector protection
 * - User options: WDG, RST_STOP, RST_STDBY
 * 
 * PERINGATAN:
 * - Read Protection Level 1 akan menghapus flash jika di-downgrade ke Level 0
 * - Hati-hati saat mengubah option bytes!
 * - Jika salah setting, chip bisa menjadi unusable
 * 
 * Hardware:
 * - STM32F103C8T6 Blue Pill
 * - ST-Link V2
 * - UART USB adapter (PA9/PA10)
 * - LED PC13 (onboard)
 * 
 * Pin Configuration:
 * - PA9: UART TX
 * - PA10: UART RX
 * - PC13: Status LED
 */

#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* UART handle */
UART_HandleTypeDef huart1;

/* Function prototypes */
void SystemClock_Config(void);
void GPIO_Init(void);
void UART_Init(void);
void Display_Flash_Protection_Status(void);
void Display_Option_Bytes(void);
void Enable_Write_Protection(uint32_t page_start, uint32_t page_end);
void Disable_Write_Protection(void);
void Display_Menu(void);
void Process_Command(char cmd);

/* Buffer */
char msg[200];
uint8_t rx_data;

/**
 * Main function
 */
int main(void)
{
    /* Initialize HAL */
    HAL_Init();

    /* Configure system clock */
    SystemClock_Config();

    /* Initialize peripherals */
    GPIO_Init();
    UART_Init();

    /* Welcome message */
    sprintf(msg, "\r\n\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "╔═══════════════════════════════════════════╗\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "║   STM32F103 Flash Protection Manager     ║\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "║   Write Protection Configuration Tool    ║\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "╚═══════════════════════════════════════════╝\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "\r\nPERINGATAN: Hati-hati saat mengubah option bytes!\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "Setting yang salah dapat membuat chip unusable.\r\n\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    /* Display current status */
    Display_Option_Bytes();
    Display_Flash_Protection_Status();
    Display_Menu();

    /* Main loop */
    while(1)
    {
        /* Wait for command from UART */
        if(HAL_UART_Receive(&huart1, &rx_data, 1, 100) == HAL_OK)
        {
            Process_Command((char)rx_data);
            Display_Menu();
        }

        /* LED heartbeat */
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_Delay(500);
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
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
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
 * Display Option Bytes
 */
void Display_Option_Bytes(void)
{
    FLASH_OBProgramInitTypeDef OBInit;
    HAL_FLASHEx_OBGetConfig(&OBInit);

    sprintf(msg, "\r\n=== Option Bytes Status ===\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    sprintf(msg, "Read Protection Level: ");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    if(OBInit.RDPLevel == OB_RDP_LEVEL_0)
        sprintf(msg, "Level 0 (No protection)\r\n");
    else
        sprintf(msg, "Level 1 (Read protected)\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    sprintf(msg, "User Options: 0x%02lX\r\n", OBInit.USERConfig);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    sprintf(msg, "- WDG_SW: %s\r\n", 
            (OBInit.USERConfig & OB_IWDG_SW) ? "Software" : "Hardware");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    sprintf(msg, "- nRST_STOP: %s\r\n",
            (OBInit.USERConfig & OB_STOP_NO_RST) ? "No reset" : "Reset");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    sprintf(msg, "- nRST_STDBY: %s\r\n",
            (OBInit.USERConfig & OB_STDBY_NO_RST) ? "No reset" : "Reset");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

/**
 * Display Flash Protection Status
 */
void Display_Flash_Protection_Status(void)
{
    FLASH_OBProgramInitTypeDef OBInit;
    HAL_FLASHEx_OBGetConfig(&OBInit);

    sprintf(msg, "\r\n=== Write Protection Status ===\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    sprintf(msg, "WRP Pages: 0x%08lX\r\n", OBInit.WRPPage);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    if(OBInit.WRPPage == 0xFFFFFFFF)
    {
        sprintf(msg, "Status: No pages are write-protected\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    }
    else if(OBInit.WRPPage == 0x00000000)
    {
        sprintf(msg, "Status: All pages are write-protected\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    }
    else
    {
        sprintf(msg, "Status: Some pages are write-protected\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

        /* Show which pages are protected */
        sprintf(msg, "Protected pages:\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        
        for(int i = 0; i < 32; i++)
        {
            if((OBInit.WRPPage & (1 << i)) == 0)
            {
                sprintf(msg, "  - Page %d-%d\r\n", i*4, i*4+3);
                HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
            }
        }
    }
}

/**
 * Enable Write Protection untuk pages tertentu
 */
void Enable_Write_Protection(uint32_t page_start, uint32_t page_end)
{
    sprintf(msg, "\r\nEnabling write protection for pages %lu-%lu...\r\n", 
            page_start, page_end);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    /* Unlock Flash dan Option Bytes */
    HAL_FLASH_Unlock();
    HAL_FLASH_OB_Unlock();

    /* Get current option bytes */
    FLASH_OBProgramInitTypeDef OBInit;
    HAL_FLASHEx_OBGetConfig(&OBInit);

    /* Calculate WRP page mask (STM32F103: 4 pages per bit) */
    uint32_t wrp_pages = 0;
    for(uint32_t page = page_start; page <= page_end; page++)
    {
        wrp_pages |= (1 << (page / 4));
    }

    /* Clear bits untuk pages yang ingin di-protect (0 = protected) */
    OBInit.OptionType = OPTIONBYTE_WRP;
    OBInit.WRPPage = OBInit.WRPPage & ~wrp_pages;

    /* Program option bytes */
    if(HAL_FLASHEx_OBProgram(&OBInit) == HAL_OK)
    {
        sprintf(msg, "Write protection enabled successfully!\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

        sprintf(msg, "System will reset to apply changes...\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        HAL_Delay(100);

        /* Launch option bytes dan reset */
        HAL_FLASH_OB_Launch();
    }
    else
    {
        sprintf(msg, "ERROR: Failed to enable write protection!\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    }

    /* Lock Flash */
    HAL_FLASH_OB_Lock();
    HAL_FLASH_Lock();
}

/**
 * Disable Write Protection (remove all protection)
 */
void Disable_Write_Protection(void)
{
    sprintf(msg, "\r\nDisabling all write protection...\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    /* Unlock Flash dan Option Bytes */
    HAL_FLASH_Unlock();
    HAL_FLASH_OB_Unlock();

    /* Set all bits to 1 (no protection) */
    FLASH_OBProgramInitTypeDef OBInit;
    OBInit.OptionType = OPTIONBYTE_WRP;
    OBInit.WRPPage = 0xFFFFFFFF; // All pages writable

    /* Program option bytes */
    if(HAL_FLASHEx_OBProgram(&OBInit) == HAL_OK)
    {
        sprintf(msg, "Write protection disabled successfully!\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

        sprintf(msg, "System will reset to apply changes...\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        HAL_Delay(100);

        /* Launch option bytes dan reset */
        HAL_FLASH_OB_Launch();
    }
    else
    {
        sprintf(msg, "ERROR: Failed to disable write protection!\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    }

    /* Lock Flash */
    HAL_FLASH_OB_Lock();
    HAL_FLASH_Lock();
}

/**
 * Display Menu
 */
void Display_Menu(void)
{
    sprintf(msg, "\r\n--- Command Menu ---\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "s - Show current protection status\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "e - Enable write protection (pages 60-63)\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "d - Disable all write protection\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "o - Show option bytes\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "\r\nEnter command: ");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

/**
 * Process Command
 */
void Process_Command(char cmd)
{
    sprintf(msg, "%c\r\n", cmd);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    switch(cmd)
    {
        case 's':
        case 'S':
            Display_Flash_Protection_Status();
            break;

        case 'e':
        case 'E':
            /* Protect pages 60-63 (last 4 pages) */
            Enable_Write_Protection(60, 63);
            break;

        case 'd':
        case 'D':
            Disable_Write_Protection();
            break;

        case 'o':
        case 'O':
            Display_Option_Bytes();
            break;

        default:
            sprintf(msg, "Unknown command: %c\r\n", cmd);
            HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
            break;
    }
}

/**
 * SysTick Handler
 */
void SysTick_Handler(void)
{
    HAL_IncTick();
}
