
#include "shared.h"

#include <stdio.h>

#include <soc/soc.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_vfs_dev.h"

#include "nvs_flash.h"

#include "esp_spiffs.h"

#include "mdns.h"

#include "esp_wifi.h"

#include "sdkconfig.h"
#include "tasks/blink/blink_task.h"
#include "tasks/webserver/webserver_task.h"

const char hostname[] = "tvlift";

static const char* TAG = "APP";

static bool shuttingDown = false;

static void shutdown_handler(void)
{
    shuttingDown = true;
}

static void system_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if(shuttingDown)
    {
        // Ignore every event if we are shutting down
        return;
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI(TAG, "WIFI_EVENT: WIFI_EVENT_STA_START");
        
        // Set hostname
        ESP_ERROR_CHECK(tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, hostname));

        // Connect wifi to access point
        ESP_ERROR_CHECK(esp_wifi_connect());
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(TAG, "WIFI_EVENT: WIFI_EVENT_STA_DISCONNECTED");

        // Re-connect wifi to access point
        ESP_ERROR_CHECK(esp_wifi_connect());
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ESP_LOGI(TAG, "IP_EVENT: IP_EVENT_STA_GOT_IP");
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP:%s", ip4addr_ntoa(&event->ip_info.ip));
    }
}

static void initialise_spiffs(void)
{    
    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };
    
    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) 
    {
        if (ret == ESP_FAIL) 
        {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND) 
        {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        }
        else 
        {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    }
    else 
    {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
}

static void initialise_mdns(void)
{
    // Initialize mDNS
    ESP_ERROR_CHECK(mdns_init());

    // Set mDNS hostname
    ESP_ERROR_CHECK(mdns_hostname_set(hostname));
    ESP_LOGI(TAG, "mDNS hostname set to: %s", hostname);

    // Set default mDNS instance name
    ESP_ERROR_CHECK(mdns_instance_name_set("TV Lift"));

    // Structure with TXT records
    // mdns_txt_item_t serviceTxtData[3] = {
    //     {"board", "esp32"},
    //     {"u", "user"},
    //     {"p", "password"}};

    // Initialize service
    ESP_ERROR_CHECK(mdns_service_add("TV Lift", "_http", "_tcp", 80, NULL, 0));
}

static void initialise_wifi(void)
{
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &system_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &system_event_handler, NULL));

    // Initialise tcpip adapter
    tcpip_adapter_init();

    // Initialise wifi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
        },
    };

    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);

    // Configure wifi
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

    // Start wifi
    ESP_ERROR_CHECK(esp_wifi_start());
}

void app_main()
{
    ESP_LOGI(TAG, "Starting App");

    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) 
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    
    // Register shutdown handler
    ESP_ERROR_CHECK(esp_register_shutdown_handler(&shutdown_handler));
    
    // Create the event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    initialise_spiffs();
    initialise_mdns();

    TaskHandle_t blinkTaskHandle = NULL; 
    xTaskCreatePinnedToCore(
        blink_task_main,
        BLINK_TASK_TAG,
        BLINK_TASK_STACK_SIZE_KB * STACK_KB,
        NULL,
        tskIDLE_PRIORITY+12,
        &blinkTaskHandle,
        PRO_CPU_NUM);

    TaskHandle_t webserverTaskHandle = NULL; 
    xTaskCreatePinnedToCore(
        webserver_task_main,
        WEBSERVER_TASK_TAG,
        WEBSERVER_TASK_STACK_SIZE_KB * STACK_KB,
        NULL,
        tskIDLE_PRIORITY+12,
        &webserverTaskHandle,
        PRO_CPU_NUM);

    // Initialise wifi after all tasks are created since some might depend on ceratin events
    initialise_wifi();

    for(;;)
    {
        vTaskSuspend(NULL);
    }

    vTaskDelete(NULL);
}

