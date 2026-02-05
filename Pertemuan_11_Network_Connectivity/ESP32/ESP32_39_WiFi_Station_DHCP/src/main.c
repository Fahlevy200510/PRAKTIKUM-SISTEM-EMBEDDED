/**
 * ============================================================================
 * PROJECT: 39-WiFi_Station_DHCP
 * PLATFORM: ESP32
 * PERTEMUAN: 11 - Network & Connectivity
 * 
 * JUDUL: WiFi Station Mode + DHCP
 * 
 * DESKRIPSI:
 * Connect ESP32 ke WiFi access point menggunakan station mode dan dapatkan
 * IP address via DHCP.
 * 
 * ============================================================================
 * EXPECTED OUTPUT
 * ============================================================================
 * 
 *    I (320) Prog39: === WiFi Station Mode + DHCP ===
 *    I (330) wifi:wifi driver task: 3ffc0abc
 *    I (340) Prog39: WiFi init finished
 *    I (350) Prog39: Connecting to SSID: YourWiFi...
 *    I (1540) wifi:new:<6,0>, old:<1,0>, ap:<255,255>, sta:<6,0>, prof:1
 *    I (2530) wifi:state: init -> auth (b0)
 *    I (2540) wifi:state: auth -> assoc (0)
 *    I (2550) wifi:state: assoc -> run (10)
 *    I (2590) Prog39: WiFi connected!
 *    I (2600) Prog39: IP Address: 192.168.1.105
 *    I (2610) Prog39: Netmask: 255.255.255.0
 *    I (2620) Prog39: Gateway: 192.168.1.1
 * 
 * ============================================================================
 */

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"

/* ============================================================================
 * CONFIGURATION
 * ============================================================================ */

#define WIFI_SSID      "YourWiFi"
#define WIFI_PASS      "YourPassword"
#define MAX_RETRY      5

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#define TAG "Prog39"

/* ============================================================================
 * GLOBALS
 * ============================================================================ */

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;

/* ============================================================================
 * EVENT HANDLER
 * ============================================================================ */

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < MAX_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retry connecting (%d/%d)", s_retry_num, MAX_RETRY);
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "WiFi connected!");
        ESP_LOGI(TAG, "IP Address: " IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "Netmask: " IPSTR, IP2STR(&event->ip_info.netmask));
        ESP_LOGI(TAG, "Gateway: " IPSTR, IP2STR(&event->ip_info.gw));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/* ============================================================================
 * WiFi INITIALIZATION
 * ============================================================================ */

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi init finished");
    ESP_LOGI(TAG, "Connecting to SSID: %s...", WIFI_SSID);

    /* Wait for connection */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected successfully!");
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect");
    }
}

/* ============================================================================
 * MAIN
 * ============================================================================ */

void app_main(void)
{
    ESP_LOGI(TAG, "=== WiFi Station Mode + DHCP ===");
    
    /* Initialize NVS */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /* Initialize WiFi */
    wifi_init_sta();
    
    /* Main loop */
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(5000));
        ESP_LOGI(TAG, "Heartbeat - WiFi connected");
    }
}

