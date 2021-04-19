
#include <sys/param.h>

#include "esp_netif.h"

#include "netconf.h"
#include <esp_http_server.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#define EXAMPLE_ESP_WIFI_SSID "M5GO"
#define EXAMPLE_ESP_WIFI_PASS "123456789"
#define EXAMPLE_ESP_WIFI_CHANNEL 1
#define EXAMPLE_MAX_STA_CONN 4

extern const uint8_t root_html_start[] asm("_binary_root_html_start");
static const char *const root_html = (char *)root_html_start;
static const char *TAG = "netconf";

static esp_err_t hello_get_handler(httpd_req_t *req)
{
    const char *resp_str = (const char *)req->user_ctx;
    httpd_resp_send(req, resp_str, strlen(resp_str));
    return ESP_OK;
}

void strSwap(char text[], char a[], char b[])
{
    char *str = text, back[20];
    while ((str = strstr(str, a)) != NULL)
    {
        strcpy(back, str + strlen(a));
        *str = 0;
        strcat(text, b);
        strcat(text, back);
        str += strlen(b);
    }
}

void strConversion(char *strBuff)
{
    strSwap(strBuff, "%3F", "?");
    strSwap(strBuff, "%20", " ");
    strSwap(strBuff, "%24", "$");
    strSwap(strBuff, "%25", "%");
    strSwap(strBuff, "%26", "&");
    strSwap(strBuff, "%27", "\\");
    strSwap(strBuff, "%2F", "/");
    strSwap(strBuff, "%3A", ":");
    strSwap(strBuff, "%3B", ";");
    strSwap(strBuff, "%2B", "+");
    strSwap(strBuff, "%3D", "=");
    strSwap(strBuff, "%40", "@");
    strSwap(strBuff, "%2A", "*");
}

static const httpd_uri_t root = {
    .uri = "/root",
    .method = HTTP_GET,
    .handler = hello_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx = root_html};

static esp_err_t form_handler(httpd_req_t *req)
{
    char *buf;
    size_t buf_len;
    nvs_handle_t nvs_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &nvs_handle));

    /* Get header value string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK)
        {
            ESP_LOGI(TAG, "Found URL query => %s", buf);
            char param[100];
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "ssid", param, sizeof(param)) == ESP_OK)
            {
                strConversion(param);
                ESP_LOGI(TAG, "ssid=%s", param);
                ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "ssid", param));
            }
            if (httpd_query_key_value(buf, "password", param, sizeof(param)) == ESP_OK)
            {
                strConversion(param);
                ESP_LOGI(TAG, "password=%s", param);
                ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "password", param));
            }
            if (httpd_query_key_value(buf, "id_scope", param, sizeof(param)) == ESP_OK)
            {
                strConversion(param);
                ESP_LOGI(TAG, "id_scope=%s", param);
                ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "id_scope", param));
            }
            if (httpd_query_key_value(buf, "device_id", param, sizeof(param)) == ESP_OK)
            {

                strConversion(param);
                ESP_LOGI(TAG, "device_id=%s", param);
                ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "device_id", param));
            }
            if (httpd_query_key_value(buf, "symmetric_key", param, sizeof(param)) == ESP_OK)
            {
                strConversion(param);
                ESP_LOGI(TAG, "symmetric_key=%s", param);
                ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "symmetric_key", param));
            }
        }
        free(buf);
        ESP_ERROR_CHECK(nvs_commit(nvs_handle));

        nvs_close(nvs_handle);
        esp_restart();
    }
    return ESP_OK;
}

static const httpd_uri_t connect = {
    .uri = "/connect",
    .method = HTTP_GET,
    .handler = form_handler,
    .user_ctx = NULL};

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &root);
        httpd_register_uri_handler(server, &connect);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}

static void disconnect_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)arg;
    if (*server)
    {
        ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}

static void connect_handler(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)arg;
    if (*server == NULL)
    {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK},
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
}
//Start the webserver after startup, and it will reset M5GO automatically after getting the data
void netconf(void)
{
    static httpd_handle_t server = NULL;
    wifi_init_softap();
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));

    server = start_webserver();
}