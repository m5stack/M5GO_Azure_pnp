/* esp-azure example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "azure_c_shared_utility/http_proxy_io.h"

#include "nvs_flash.h"
#include "pnp_m5stack.h"
#include "m5go.h"
#include "netconf.h"
#define LGFX_M5STACK
#include <LovyanGFX.hpp>

LGFX lcd;

#define EXAMPLE_WIFI_SSID CONFIG_WIFI_SSID
#define EXAMPLE_WIFI_PASS CONFIG_WIFI_PASSWORD

char *ssid = NULL;
char *password = NULL;
char *id_scope = NULL;
char *device_id = NULL;
char *symmetric_key = NULL;
EventGroupHandle_t wifi_event_group;

#ifndef BIT0
#define BIT0 (0x1 << 0)
#endif
/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;

static const char *TAG = "azure";

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id)
    {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        /* This is a workaround as ESP platform WiFi libs don't currently
           auto-reassociate. */
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

static void initialise_wifi(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config_t));
    strcpy((char *)wifi_config.sta.ssid, ssid);
    strcpy((char *)wifi_config.sta.password, password);

    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

extern "C" const IO_INTERFACE_DESCRIPTION *socketio_get_interface_description(void)
{
    return NULL;
}

void azure_task(void *pvParameter)
{
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                        false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "Connected to AP success!");
    lcd.printf("wifi init!\r\n");
    lcd.printf("azure initializing....\r\n");
    pnp_temperature_controller();

    vTaskDelete(NULL);
}

void keyAInit(void)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;

    io_conf.pin_bit_mask = (1ULL << 39);

    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;

    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
}
int8_t getConf(void)
{
    nvs_handle_t nvs_handle;
    size_t required_size;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &nvs_handle));

    if (nvs_get_str(nvs_handle, "ssid", NULL, &required_size) != ESP_OK)
        return -1;
    ssid = (char *)malloc(required_size);
    nvs_get_str(nvs_handle, "ssid", ssid, &required_size);
    printf("ssid = %s\r\n", ssid);

    if (nvs_get_str(nvs_handle, "password", NULL, &required_size) != ESP_OK)
        return -1;
    password = (char *)malloc(required_size);
    nvs_get_str(nvs_handle, "password", password, &required_size);
    printf("password = %s\r\n", password);

    if (nvs_get_str(nvs_handle, "id_scope", NULL, &required_size) != ESP_OK)
        return -1;
    id_scope = (char *)malloc(required_size);
    nvs_get_str(nvs_handle, "id_scope", id_scope, &required_size);
    printf("id_scope = %s\r\n", id_scope);

    if (nvs_get_str(nvs_handle, "device_id", NULL, &required_size) != ESP_OK)
        return -1;
    device_id = (char *)malloc(required_size);
    nvs_get_str(nvs_handle, "device_id", device_id, &required_size);
    printf("device-id = %s\r\n", device_id);

    if (nvs_get_str(nvs_handle, "symmetric_key", NULL, &required_size) != ESP_OK)
        return -1;
    symmetric_key = (char *)malloc(required_size);
    nvs_get_str(nvs_handle, "symmetric_key", symmetric_key, &required_size);
    printf("symmetric_key = %s\r\n", symmetric_key);
    return 0;
}
extern "C" void app_main()
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    lcd.init();
    lcd.setRotation(1);
    lcd.setBrightness(128);
    lcd.setColorDepth(24);
    lcd.setCursor(0, 0, lgfx::fontdata[2]);

    keyAInit();
    vTaskDelay(100 / portTICK_PERIOD_MS);
    if (gpio_get_level(GPIO_NUM_39))
    {
        if (getConf() < 0)
        {
            lcd.setCursor(0, 0, lgfx::fontdata[4]);
            lcd.printf("Please link to WiFi configuration information\r\nssid: M5GO \r\npass: 123456789 ");
            netconf();
        }
        else
        {
            lcd.setCursor(0, 0, lgfx::fontdata[2]);
            lcd.printf("Initialize wifi...\r\n");
            initialise_wifi();
            lcd.printf("Initialize sensor...\r\n");

            m5go_Sk6812_Init();
            m5go_Angle_Init();
            MPU6886_Init();
            m5go_Motion_Init();
            bmp280_init();
            bmp280_set_work_mode(BMP280_FORCED_MODE);
            SHT30_Init();
            lcd.printf("Initialize sensor successfully!\r\n");
            vTaskDelay(100 / portTICK_PERIOD_MS);

            if (xTaskCreate(&azure_task, "azure_task", 1024 * 5, NULL, 5, NULL) != pdPASS)
            {
                printf("create azure task failed\r\n");
            }
        }
    }
    else
    {
        lcd.setCursor(0, 0, lgfx::fontdata[4]);
        lcd.printf("Please link to WiFi \r\nconfiguration information\r\nssid: M5GO \r\npass: 123456789 \r\nOpen Website:\r\n192.168.4.1/root");
        netconf();
    }
}
