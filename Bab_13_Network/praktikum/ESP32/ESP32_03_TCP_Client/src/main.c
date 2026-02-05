/**
 * Program 41: TCP Client
 * Connect to TCP server and send data
 */

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "lwip/sockets.h"

#define TAG "Prog41"
#define HOST_IP "127.0.0.1"
#define PORT 8000

void tcp_client_task(void *pvParameters)
{
    char rx_buffer[128];
    char addr_str[128];
    int sock;

    while(1) {
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(HOST_IP);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);
        inet_ntoa_r(dest_addr.sin_addr, addr_str, sizeof(addr_str) - 1);

        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket");
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }
        ESP_LOGI(TAG, "Socket created");

        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err != 0) {
            ESP_LOGE(TAG, "Socket unable to connect");
            close(sock);
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }
        ESP_LOGI(TAG, "Connected to %s:%d", HOST_IP, PORT);

        while(1) {
            const char *payload = "Hello from ESP32\n";
            int err = send(sock, payload, strlen(payload), 0);
            if (err < 0) {
                ESP_LOGE(TAG, "Send failed");
                break;
            }
            ESP_LOGI(TAG, "Sent: %s", payload);

            int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            if (len < 0) {
                ESP_LOGE(TAG, "Receive failed");
                break;
            } else if (len == 0) {
                ESP_LOGW(TAG, "Connection closed");
                break;
            } else {
                rx_buffer[len] = 0;
                ESP_LOGI(TAG, "Received: %s", rx_buffer);
            }

            vTaskDelay(pdMS_TO_TICKS(2000));
        }

        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket");
            shutdown(sock, 0);
            close(sock);
        }
        
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== TCP Client ===");
    nvs_flash_init();
    
    // Assume WiFi already configured
    // In real app, initialize WiFi first
    
    xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, 5, NULL);
}
