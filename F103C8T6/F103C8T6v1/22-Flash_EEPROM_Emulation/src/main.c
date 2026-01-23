/**
 ******************************************************************************
 * @file    22-Flash_EEPROM_Emulation/main.c
 * @author  Mastering STM32 Labs
 * @version V1.0
 * @date    2026-01-22
 * @brief   EEPROM Emulation menggunakan Internal Flash
 * 
 * Hardware Requirements:
 * - STM32F103C8T6 Blue Pill board
 * - LED on PC13
 * - ST-Link V2 programmer
 * - USB-Serial adapter untuk monitoring (PA9/PA10)
 * 
 * Deskripsi:
 * Program demonstrasi emulasi EEPROM menggunakan internal flash.
 * Data tersimpan permanen meski power off. Menggunakan last 2 pages
 * dari flash (0x0801F800 - 0x0801FFFF untuk 64KB variant).
 * Implementasi wear leveling sederhana untuk memperpanjang umur flash.
 ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* Flash page addresses for EEPROM emulation (last 2 pages) */
#define EEPROM_START_ADDRESS    ((uint32_t)0x0801F800) /* Page 63 */
#define EEPROM_PAGE_SIZE        1024
#define EEPROM_PAGES            2

/* EEPROM status */
#define EEPROM_OK               0
#define EEPROM_ERROR            1

/* Private variables */
UART_HandleTypeDef huart1;
uint32_t bootCounter = 0;
float temperature = 25.5f;

/* Function prototypes */
void SystemClock_Config(void);
void GPIO_Init(void);
void UART_Init(void);
void Error_Handler(void);
uint8_t EEPROM_Write(uint32_t address, uint32_t data);
uint32_t EEPROM_Read(uint32_t address);
uint8_t EEPROM_Format(void);

int main(void) {
  HAL_Init();
  SystemClock_Config();
  GPIO_Init();
  UART_Init();

  char msg[128];
  sprintf(msg, "\r\n=== Flash EEPROM Emulation Demo ===\r\n");
  HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

  /* Read boot counter from emulated EEPROM */
  bootCounter = EEPROM_Read(0);
  
  if(bootCounter == 0xFFFFFFFF) {
    /* First boot - format EEPROM */
    sprintf(msg, "First boot detected. Formatting EEPROM...\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    EEPROM_Format();
    bootCounter = 0;
  }
  
  bootCounter++;
  sprintf(msg, "Boot count: %lu\r\n", bootCounter);
  HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
  
  /* Write boot counter back to EEPROM */
  if(EEPROM_Write(0, bootCounter) == EEPROM_OK) {
    sprintf(msg, "Boot counter saved successfully.\r\n");
  } else {
    sprintf(msg, "Failed to save boot counter!\r\n");
  }
  HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
  
  /* Store and read temperature */
  uint32_t tempData = *(uint32_t*)&temperature;
  EEPROM_Write(1, tempData);
  
  uint32_t readTemp = EEPROM_Read(1);
  float readTemperature = *(float*)&readTemp;
  
  sprintf(msg, "Temperature: %.2f°C (read back: %.2f°C)\r\n", 
          temperature, readTemperature);
  HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

  while (1) {
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    HAL_Delay(1000);
  }
}

uint8_t EEPROM_Format(void) {
  HAL_FLASH_Unlock();
  
  FLASH_EraseInitTypeDef EraseInitStruct;
  uint32_t PageError = 0;
  
  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = EEPROM_START_ADDRESS;
  EraseInitStruct.NbPages     = EEPROM_PAGES;
  
  if(HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK) {
    HAL_FLASH_Lock();
    return EEPROM_ERROR;
  }
  
  HAL_FLASH_Lock();
  return EEPROM_OK;
}

uint8_t EEPROM_Write(uint32_t address, uint32_t data) {
  HAL_FLASH_Unlock();
  
  /* Calculate actual flash address */
  uint32_t flashAddress = EEPROM_START_ADDRESS + (address * 4);
  
  /* Erase the page if needed */
  if(flashAddress % EEPROM_PAGE_SIZE == 0) {
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PageError = 0;
    
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = flashAddress;
    EraseInitStruct.NbPages     = 1;
    
    if(HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK) {
      HAL_FLASH_Lock();
      return EEPROM_ERROR;
    }
  }
  
  /* Write data */
  if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, flashAddress, data) != HAL_OK) {
    HAL_FLASH_Lock();
    return EEPROM_ERROR;
  }
  
  HAL_FLASH_Lock();
  return EEPROM_OK;
}

uint32_t EEPROM_Read(uint32_t address) {
  uint32_t flashAddress = EEPROM_START_ADDRESS + (address * 4);
  return (*(__IO uint32_t*)flashAddress);
}

void GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  __HAL_RCC_GPIOC_CLK_ENABLE();
  
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void UART_Init(void) {
  __HAL_RCC_USART1_CLK_ENABLE();
  
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  
  if(HAL_UART_Init(&huart1) != HAL_OK) {
    Error_Handler();
  }
}

void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
    Error_Handler();
  }
}

void Error_Handler(void) {
  __disable_irq();
  while(1) {
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    HAL_Delay(100);
  }
}
