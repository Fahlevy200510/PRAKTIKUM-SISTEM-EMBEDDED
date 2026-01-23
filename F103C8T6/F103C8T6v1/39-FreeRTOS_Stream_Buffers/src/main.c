/**
 ******************************************************************************
 * @file    main.c
 * @brief   FreeRTOS Stream Buffers & Message Buffers demo
 * @date    2026-01-22
 ******************************************************************************
 * Program 39: Variable-length data passing
 * - Stream Buffer: continuous byte stream (UART-like)
 * - Message Buffer: discrete messages with length prefix
 *
 * Hardware:
 * - PA9/PA10: USART1 TX/RX (115200)
 * - PC13: LED (data indicator, active LOW)
 ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include <stdio.h>
#include <string.h>

#define STREAM_BUF_SIZE    256
#define MSG_BUF_SIZE       256
#define STREAM_TRIGGER     16      /* Wake consumer after 16 bytes */

UART_HandleTypeDef huart1;
StreamBufferHandle_t xStreamBuf;
MessageBufferHandle_t xMsgBuf;

void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART_Init(void);

void vProducerTask(void *pvParameters);
void vConsumerTask(void *pvParameters);
void vMsgProducerTask(void *pvParameters);
void vMsgConsumerTask(void *pvParameters);

int _write(int file, char *ptr, int len) {
    HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    UART_Init();

    printf("\r\n=== FreeRTOS Stream & Message Buffers ===\r\n");
    printf("Program 39: Variable-length IPC\r\n\r\n");

    xStreamBuf = xStreamBufferCreate(STREAM_BUF_SIZE, STREAM_TRIGGER);
    xMsgBuf = xMessageBufferCreate(MSG_BUF_SIZE);
    if (xStreamBuf == NULL || xMsgBuf == NULL) {
        printf("ERROR: Buffer alloc failed\r\n");
        while (1) {}
    }

    xTaskCreate(vProducerTask, "StreamProd", 256, NULL, 2, NULL);
    xTaskCreate(vConsumerTask, "StreamCons", 256, NULL, 2, NULL);
    xTaskCreate(vMsgProducerTask, "MsgProd", 256, NULL, 1, NULL);
    xTaskCreate(vMsgConsumerTask, "MsgCons", 256, NULL, 1, NULL);

    vTaskStartScheduler();
    while (1) {}
}

/* Stream buffer producer: sends bytes with varying length */
void vProducerTask(void *pvParameters) {
    const char *payloads[] = {
        "ping\n", "sensor:23.4\n", "hello-world-stream\n",
        "short\n", "longer-payload-data-xyz\n"
    };
    size_t idx = 0;
    TickType_t last = xTaskGetTickCount();

    for (;;) {
        const char *p = payloads[idx];
        size_t len = strlen(p);
        size_t sent = xStreamBufferSend(xStreamBuf, p, len, pdMS_TO_TICKS(100));
        if (sent != len) {
            printf("[STREAM PROD] Buffer full, lost %u bytes\r\n", (unsigned)(len - sent));
        }
        idx = (idx + 1) % (sizeof(payloads) / sizeof(payloads[0]));
        vTaskDelayUntil(&last, pdMS_TO_TICKS(300));
    }
}

/* Stream buffer consumer */
void vConsumerTask(void *pvParameters) {
    uint8_t rx[64];
    for (;;) {
        size_t r = xStreamBufferReceive(xStreamBuf, rx, sizeof(rx) - 1, pdMS_TO_TICKS(1000));
        if (r > 0) {
            rx[r] = '\0';
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
            printf("[STREAM CONS] %s", rx);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
        }
    }
}

/* Message buffer producer: sends discrete messages */
void vMsgProducerTask(void *pvParameters) {
    uint32_t counter = 0;
    TickType_t last = xTaskGetTickCount();
    char msg[48];

    for (;;) {
        counter++;
        int len = snprintf(msg, sizeof(msg), "MSG #%lu @tick %lu", counter, (unsigned long)xTaskGetTickCount());
        size_t sent = xMessageBufferSend(xMsgBuf, msg, len + 1, pdMS_TO_TICKS(200));
        if (sent != (size_t)(len + 1)) {
            printf("[MSG PROD] Buffer full, msg dropped\r\n");
        }
        vTaskDelayUntil(&last, pdMS_TO_TICKS(700));
    }
}

/* Message buffer consumer */
void vMsgConsumerTask(void *pvParameters) {
    char rx[64];
    for (;;) {
        size_t r = xMessageBufferReceive(xMsgBuf, rx, sizeof(rx), pdMS_TO_TICKS(1500));
        if (r > 0) {
            printf("[MSG CONS] %s\r\n", rx);
        }
    }
}

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
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
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}

static void UART_Init(void) {
    __HAL_RCC_USART1_CLK_ENABLE();
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart1);

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void Error_Handler(void) {
    __disable_irq();
    while (1) {}
}
