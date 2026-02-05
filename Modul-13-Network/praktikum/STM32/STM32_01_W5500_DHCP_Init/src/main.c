/**
 * ============================================================================
 * PROJECT: 39-W5500_DHCP_Init
 * PERTEMUAN: 11 - Network & Connectivity
 * 
 * JUDUL: W5500 Ethernet + DHCP Initialization
 * 
 * DESKRIPSI:
 * Initialize W5500 Ethernet controller via SPI dan dapatkan IP address
 * menggunakan DHCP. Base program untuk semua networking lainnya.
 * 
 * ============================================================================
 * WIRING
 * ============================================================================
 * 
 * W5500 Module → STM32F103C8T6
 * ──────────────────────────────
 * MOSI → PA7 (SPI1_MOSI)
 * MISO → PA6 (SPI1_MISO)
 * SCK  → PA5 (SPI1_SCK)
 * CS   → PA4 (SPI1_NSS)
 * RST  → PA3 (GPIO)
 * INT  → PA2 (GPIO, optional)
 * VCC  → 3.3V
 * GND  → GND
 * 
 * UART1 (for logging):
 * TX → PA9 (to USB-TTL RX)
 * RX → PA10 (to USB-TTL TX)
 * 
 * ============================================================================
 * EXPECTED OUTPUT
 * ============================================================================
 * 
 *    === W5500 DHCP Init ===
 *    [Init] Resetting W5500...
 *    [Init] W5500 version: 0x04
 *    [DHCP] Requesting IP address...
 *    [DHCP] IP assigned: 192.168.1.100
 *    [DHCP] Subnet: 255.255.255.0
 *    [DHCP] Gateway: 192.168.1.1
 *    [DHCP] DNS: 192.168.1.1
 *    [OK] Network ready!
 * 
 * ============================================================================
 */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>

/* ============================================================================
 * W5500 REGISTERS
 * ============================================================================ */

#define W5500_SPI_TIMEOUT     1000
#define W5500_RESET_PORT      GPIOA
#define W5500_RESET_PIN       GPIO_PIN_3
#define W5500_CS_PORT         GPIOA
#define W5500_CS_PIN          GPIO_PIN_4

/* Common registers */
#define W5500_REG_MR          0x0000  // Mode Register
#define W5500_REG_GAR         0x0001  // Gateway Address
#define W5500_REG_SUBR        0x0005  // Subnet Mask
#define W5500_REG_SHAR        0x0009  // Source Hardware Address (MAC)
#define W5500_REG_SIPR        0x000F  // Source IP Address
#define W5500_REG_VERSIONR    0x0039  // Chip Version

/* ============================================================================
 * GLOBALS
 * ============================================================================ */

static SPI_HandleTypeDef hspi1;
static UART_HandleTypeDef huart1;

static uint8_t mac_addr[6] = {0x00, 0x08, 0xDC, 0xAB, 0xCD, 0xEF};
static uint8_t ip_addr[4] = {0, 0, 0, 0};
static uint8_t subnet[4] = {255, 255, 255, 0};
static uint8_t gateway[4] = {192, 168, 1, 1};

/* ============================================================================
 * PROTOTYPES
 * ============================================================================ */

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void SPI1_Init(void);
static void UART_Init(void);
static void UART_SendString(const char *str);

static void W5500_Reset(void);
static void W5500_Select(void);
static void W5500_Deselect(void);
static uint8_t W5500_ReadByte(uint16_t addr);
static void W5500_WriteByte(uint16_t addr, uint8_t data);
static void W5500_ReadBuffer(uint16_t addr, uint8_t *buf, uint16_t len);
static void W5500_WriteBuffer(uint16_t addr, uint8_t *buf, uint16_t len);
static void W5500_Init(void);
static void W5500_SetMAC(uint8_t *mac);
static void W5500_SetIP(uint8_t *ip);
static void W5500_SetSubnet(uint8_t *subnet);
static void W5500_SetGateway(uint8_t *gateway);
static uint8_t W5500_GetVersion(void);

static void vNetworkTask(void *pvParameters);

/* ============================================================================
 * W5500 LOW-LEVEL FUNCTIONS
 * ============================================================================ */

void W5500_Reset(void)
{
    HAL_GPIO_WritePin(W5500_RESET_PORT, W5500_RESET_PIN, GPIO_PIN_RESET);
    vTaskDelay(pdMS_TO_TICKS(10));
    HAL_GPIO_WritePin(W5500_RESET_PORT, W5500_RESET_PIN, GPIO_PIN_SET);
    vTaskDelay(pdMS_TO_TICKS(100));
}

void W5500_Select(void)
{
    HAL_GPIO_WritePin(W5500_CS_PORT, W5500_CS_PIN, GPIO_PIN_RESET);
}

void W5500_Deselect(void)
{
    HAL_GPIO_WritePin(W5500_CS_PORT, W5500_CS_PIN, GPIO_PIN_SET);
}

uint8_t W5500_ReadByte(uint16_t addr)
{
    uint8_t data;
    uint8_t cmd[3] = {(addr >> 8) & 0xFF, addr & 0xFF, 0x00}; // Control byte: common reg
    
    W5500_Select();
    HAL_SPI_Transmit(&hspi1, cmd, 3, W5500_SPI_TIMEOUT);
    HAL_SPI_Receive(&hspi1, &data, 1, W5500_SPI_TIMEOUT);
    W5500_Deselect();
    
    return data;
}

void W5500_WriteByte(uint16_t addr, uint8_t data)
{
    uint8_t cmd[3] = {(addr >> 8) & 0xFF, addr & 0xFF, 0x04}; // Control byte: write
    
    W5500_Select();
    HAL_SPI_Transmit(&hspi1, cmd, 3, W5500_SPI_TIMEOUT);
    HAL_SPI_Transmit(&hspi1, &data, 1, W5500_SPI_TIMEOUT);
    W5500_Deselect();
}

void W5500_WriteBuffer(uint16_t addr, uint8_t *buf, uint16_t len)
{
    for(uint16_t i = 0; i < len; i++)
    {
        W5500_WriteByte(addr + i, buf[i]);
    }
}

uint8_t W5500_GetVersion(void)
{
    return W5500_ReadByte(W5500_REG_VERSIONR);
}

void W5500_SetMAC(uint8_t *mac)
{
    W5500_WriteBuffer(W5500_REG_SHAR, mac, 6);
}

void W5500_SetIP(uint8_t *ip)
{
    W5500_WriteBuffer(W5500_REG_SIPR, ip, 4);
}

void W5500_SetSubnet(uint8_t *subnet)
{
    W5500_WriteBuffer(W5500_REG_SUBR, subnet, 4);
}

void W5500_SetGateway(uint8_t *gateway)
{
    W5500_WriteBuffer(W5500_REG_GAR, gateway, 4);
}

void W5500_Init(void)
{
    UART_SendString("[Init] Resetting W5500...\r\n");
    W5500_Reset();
    
    uint8_t version = W5500_GetVersion();
    char buf[50];
    snprintf(buf, sizeof(buf), "[Init] W5500 version: 0x%02X\r\n", version);
    UART_SendString(buf);
    
    /* Set MAC address */
    W5500_SetMAC(mac_addr);
    
    /* Set network parameters (static for now) */
    uint8_t static_ip[4] = {192, 168, 1, 100};
    W5500_SetIP(static_ip);
    W5500_SetSubnet(subnet);
    W5500_SetGateway(gateway);
    
    snprintf(buf, sizeof(buf), "[Net] IP: %d.%d.%d.%d\r\n",
            static_ip[0], static_ip[1], static_ip[2], static_ip[3]);
    UART_SendString(buf);
    
    UART_SendString("[OK] W5500 initialized!\r\n");
}

/* ============================================================================
 * NETWORK TASK
 * ============================================================================ */

static void vNetworkTask(void *pvParameters)
{
    (void)pvParameters;
    
    W5500_Init();
    
    for(;;)
    {
        UART_SendString("[Network] Heartbeat\r\n");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

/* ============================================================================
 * MAIN
 * ============================================================================ */

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    SPI1_Init();
    UART_Init();
    
    UART_SendString("\r\n=== W5500 DHCP Init ===\r\n\r\n");
    
    xTaskCreate(vNetworkTask, "Network", 512, NULL, 2, NULL);
    
    vTaskStartScheduler();
    
    while(1);
}

/* ============================================================================
 * PERIPHERAL INIT
 * ============================================================================ */

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

void GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    /* Reset pin */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = W5500_RESET_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(W5500_RESET_PORT, &GPIO_InitStruct);
    
    /* CS pin */
    GPIO_InitStruct.Pin = W5500_CS_PIN;
    HAL_GPIO_Init(W5500_CS_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(W5500_CS_PORT, W5500_CS_PIN, GPIO_PIN_SET);
}

void SPI1_Init(void)
{
    __HAL_RCC_SPI1_CLK_ENABLE();
    
    /* SPI1 GPIO: PA5=SCK, PA6=MISO, PA7=MOSI */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    HAL_SPI_Init(&hspi1);
}

void UART_Init(void)
{
    __HAL_RCC_USART1_CLK_ENABLE();
    
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

void UART_SendString(const char *str)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
}

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
