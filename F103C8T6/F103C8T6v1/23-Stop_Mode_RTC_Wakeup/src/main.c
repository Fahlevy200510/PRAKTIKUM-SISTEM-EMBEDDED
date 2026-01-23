/**
 ******************************************************************************
 * @file    23-Stop_Mode_RTC_Wakeup/main.c
 * @author  Mastering STM32 Labs
 * @version V1.0
 * @date    2026-01-22
 * @brief   Stop Mode dengan RTC Periodic Wakeup - Ultra Low Power
 * 
 * Hardware Requirements:
 * - STM32F103C8T6 Blue Pill board
 * - 32.768kHz crystal untuk LSE (opsional, bisa gunakan LSI)
 * - 2x Capacitor 20pF untuk LSE crystal
 * - LED on PC13
 * - Ammeter untuk power measurement (opsional)
 * - ST-Link V2 programmer
 * - USB-Serial adapter (opsional, untuk monitoring sebelum sleep)
 * 
 * Deskripsi:
 * Program demonstrasi STOP mode (deep sleep) dengan RTC periodic wakeup.
 * MCU wake up setiap 5 detik, blink LED, lalu kembali ke STOP mode.
 * Power consumption di STOP mode: ~2-3µA dengan LSE, ~3-5µA dengan LSI.
 * 
 * Cara mengukur power:
 * 1. Lepas ST-Link setelah upload
 * 2. Pasang ammeter di jalur VDD
 * 3. Power dari external 3.3V
 * 4. Ukur current: should be <10µA in STOP mode
 ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* Private variables */
RTC_HandleTypeDef hrtc;
uint32_t wakeupCount = 0;

/* Function prototypes */
void SystemClock_Config(void);
void GPIO_Init(void);
void RTC_Init(void);
void Enter_Stop_Mode(void);
void Error_Handler(void);

int main(void) {
  HAL_Init();
  SystemClock_Config();
  GPIO_Init();
  RTC_Init();

  /* Blink LED 3x at startup */
  for(int i = 0; i < 3; i++) {
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    HAL_Delay(200);
  }
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET); // LED off

  while (1) {
    /* Increment wake-up counter */
    wakeupCount++;
    
    /* Blink LED to indicate wake-up */
    for(int i = 0; i < 5; i++) {
      HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
      HAL_Delay(100);
    }
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET); // LED off
    
    /* Short delay before entering stop mode */
    HAL_Delay(500);
    
    /* Enter STOP mode - wake up after 5 seconds by RTC */
    Enter_Stop_Mode();
    
    /* After wake-up, reconfigure system clock */
    SystemClock_Config();
  }
}

void Enter_Stop_Mode(void) {
  /* Enable PWR clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* Enter Stop Mode */
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
  
  /* After wake-up from STOP mode, system clocks need reconfiguration */
}

void RTC_Init(void) {
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_RCC_BKP_CLK_ENABLE();
  
  /* Enable access to backup domain */
  HAL_PWR_EnableBkUpAccess();
  
  /* Check if RTC is already configured */
  if(HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) != 0x32F2) {
    /* Reset backup domain */
    __HAL_RCC_BACKUPRESET_FORCE();
    __HAL_RCC_BACKUPRESET_RELEASE();
    
    /* Configure LSI as RTC clock source */
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
      Error_Handler();
    }
    
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
    if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
      Error_Handler();
    }
    
    /* Enable RTC clock */
    __HAL_RCC_RTC_ENABLE();
    
    /* RTC configuration */
    hrtc.Instance = RTC;
    hrtc.Init.AsynchPrediv = 39999; // 40kHz LSI / 40000 = 1Hz
    hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
    
    if(HAL_RTC_Init(&hrtc) != HAL_OK) {
      Error_Handler();
    }
    
    /* Write magic number to backup register */
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x32F2);
  } else {
    /* RTC already configured */
    hrtc.Instance = RTC;
    hrtc.Init.AsynchPrediv = 39999;
    hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
    if(HAL_RTC_Init(&hrtc) != HAL_OK) {
      Error_Handler();
    }
  }
  
  /* Configure RTC alarm for periodic wakeup (every 5 seconds) */
  /* Note: F103 doesn't have native wake-up timer, we use alarm */
  uint32_t currentCounter = HAL_RTCEx_GetSecond(&hrtc);
  
  /* Set alarm to current time + 5 seconds */
  RTC_AlarmTypeDef sAlarm = {0};
  uint32_t alarmTime = currentCounter + 5;
  
  /* Write alarm to RTC */
  WRITE_REG(hrtc.Instance->ALRH, (alarmTime >> 16) & 0xFFFF);
  WRITE_REG(hrtc.Instance->ALRL, alarmTime & 0xFFFF);
  
  /* Enable alarm interrupt */
  __HAL_RTC_ALARM_ENABLE_IT(&hrtc, RTC_IT_ALRA);
  
  /* Clear alarm flag */
  __HAL_RTC_ALARM_CLEAR_FLAG(&hrtc, RTC_FLAG_ALRAF);
  
  /* Enable EXTI line 17 for RTC Alarm */
  __HAL_RTC_ALARM_EXTI_ENABLE_IT();
  __HAL_RTC_ALARM_EXTI_ENABLE_RISING_EDGE();
  
  /* Enable RTC Alarm interrupt */
  HAL_NVIC_SetPriority(RTCAlarm_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(RTCAlarm_IRQn);
}

void RTCAlarm_IRQHandler(void) {
  HAL_RTC_AlarmIRQHandler(&hrtc);
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc) {
  /* Clear EXTI line flag */
  __HAL_RTC_ALARM_EXTI_CLEAR_FLAG();
  
  /* Set next alarm (current + 5 seconds) */
  uint32_t currentCounter = HAL_RTCEx_GetSecond(hrtc);
  uint32_t alarmTime = currentCounter + 5;
  
  WRITE_REG(hrtc->Instance->ALRH, (alarmTime >> 16) & 0xFFFF);
  WRITE_REG(hrtc->Instance->ALRL, alarmTime & 0xFFFF);
}

void GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  __HAL_RCC_GPIOC_CLK_ENABLE();
  
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET); // LED off initially
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
