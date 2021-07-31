
#include "shared.h"
#include "pins.h"

#include <stdio.h>
#include <time.h>

#include <soc/soc.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <esp_ota_ops.h>
#include <esp_vfs_dev.h>
#include <esp_vfs_fat.h>

#include <driver/i2c.h>
#include <driver/sdmmc_host.h>
#include <driver/sdspi_host.h>
#include <sdmmc_cmd.h>

#include <esp_netif.h>
#include <mdns.h>
#include <esp_wifi.h>
#include <esp_sntp.h>

#include <ds1307.h>
#include <logger.h>

#include <sdkconfig.h>
#include <services/lift_service.h>
#include <services/spiffs_service.h>
#include <tasks/blink/blink_task.h>
#include <tasks/webserver/webserver_task.h>

const char hostname[] = "tvlift";

static const char* TAG = "APP";
static bool shuttingDown = false;
static esp_netif_t* espNetifInstance;

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
        LOG_I(TAG, "WIFI_EVENT: WIFI_EVENT_STA_START");
        
        // Connect wifi to access point
        ESP_ERROR_CHECK(esp_wifi_connect());
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        LOG_I(TAG, "WIFI_EVENT: WIFI_EVENT_STA_DISCONNECTED");

        // Re-connect wifi to access point
        ESP_ERROR_CHECK(esp_wifi_connect());
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        LOG_I(TAG, "IP_EVENT: IP_EVENT_STA_GOT_IP");

        // Print out assigned ip address
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        LOG_I(TAG, "got ip: " IPSTR, IP2STR(&event->ip_info.ip));

        // Set hostname
        ESP_ERROR_CHECK(esp_netif_set_hostname(espNetifInstance, hostname));
    }
}

static int logger_service_vprintf(const char * format, va_list list)
{
    // Get level from first character of format
    logger_service_loglevel_t level;
    switch(format[0])
    {
        case 'E':
            level = LOGGER_SERVICE_LOGLEVEL_ERROR;
            break;
        case 'W':
            level = LOGGER_SERVICE_LOGLEVEL_WARN;
            break;
        case 'I':
            level = LOGGER_SERVICE_LOGLEVEL_INFO;
            break;
        case 'D':
            level = LOGGER_SERVICE_LOGLEVEL_DEBUG;
            break;
        case 'V':
            level = LOGGER_SERVICE_LOGLEVEL_VERBOSE;
            break;
        default:
            level = LOGGER_SERVICE_LOGLEVEL_NONE;
            break;
    }

    return logger_service_vlog(level, format, list);
}

static void serial_logger_sink(const char* message, const size_t len, void* user_data)
{
    printf(message);
}

void time_sync_notification_cb(struct timeval* tv)
{
    time_t sntpNow = tv->tv_sec;
    LOG_I(TAG, "SNTP Synchronisation notification recieved. SNTP Time: %s", asctime(gmtime(&sntpNow)));

    // Resync RTC
    ds1307_device_handle_t handle;
    ds1307_err_t err = ds1307_add_device(I2C_PORT, 0x68, &handle);
    if(err != DS1307_OK)
    {
        LOG_W(TAG, "Can not connect to RTC device for synchronization");
        return;
    }

    err = ds1307_set_time(handle, sntpNow);
    if(err != DS1307_OK)
    {
        LOG_W(TAG, "Can not connect to RTC device for synchronization");
        return;
    }

    ds1307_remove_device(handle);
}

static void initialize_i2c(void)
{
    i2c_config_t cfg = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = PIN_NUM_SDA,
        .sda_pullup_en = GPIO_PULLUP_DISABLE,
        .scl_io_num =  PIN_NUM_SCL,
        .scl_pullup_en = GPIO_PULLUP_DISABLE,

        .master.clk_speed = 100000
    };

    i2c_param_config(I2C_PORT, &cfg);
    i2c_driver_install(I2C_PORT, I2C_MODE_MASTER, 0, 0, 0);
}

static void initialise_sdcard(void)
{    
    LOG_I(TAG, "Using SPI peripheral");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
    slot_config.gpio_miso = PIN_NUM_MISO;
    slot_config.gpio_mosi = PIN_NUM_MOSI;
    slot_config.gpio_sck  = PIN_NUM_CLK;
    slot_config.gpio_cs   = PIN_NUM_CS;
    
    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc_mount is an all-in-one convenience function.
    // Please check its source code and implement error recovery when developing
    // production applications.
    sdmmc_card_t* card;
    esp_err_t err = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

    if (err != ESP_OK)
    {
        if (err == ESP_FAIL)
        {
            LOG_E(TAG, "Failed to mount filesystem. "
                "If you want the card to be formatted, set format_if_mount_failed = true.");
        } else
        {
            LOG_E(TAG, "Failed to initialize the card (%s). "
                "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(err));
        }
        return;
    }

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);
}

static void initialise_mdns(void)
{
    // Initialize mDNS
    ESP_ERROR_CHECK(mdns_init());

    // Set mDNS hostname
    ESP_ERROR_CHECK(mdns_hostname_set(hostname));
    LOG_I(TAG, "mDNS hostname set to: %s", hostname);

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

    // Initialise ip adapter
    ESP_ERROR_CHECK(esp_netif_init());

    // Initialise wifi
    espNetifInstance = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
        },
    };

    // Configure wifi
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

    // Start wifi
    ESP_ERROR_CHECK(esp_wifi_start());

    LOG_I(TAG, "Setting WiFi configuration SSID: %s", wifi_config.sta.ssid);
}

static void initialize_sntp(void)
{
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();
}

void initialize_app(void)
{
    // TODO: How to "automatically" call this when logger service is defined?
    logger_service_init();
    logger_service_register_sink(serial_logger_sink, NULL);
    esp_log_set_vprintf(logger_service_vprintf);

    LOG_I(TAG, "Starting App");

    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    
    // Register shutdown handler
    ESP_ERROR_CHECK(esp_register_shutdown_handler(&shutdown_handler));
    
    // Create the event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Do partition checks
    const esp_partition_t* currentPartition = esp_ota_get_running_partition();

    // TODO: Do this a bit better
    assert(currentPartition != NULL);
    LOG_I(TAG, "Current partition: %s", currentPartition->label);

    esp_ota_img_states_t state;
    err = esp_ota_get_state_partition(currentPartition, &state);
    ESP_ERROR_CHECK(err);

    if(state == ESP_OTA_IMG_PENDING_VERIFY)
    {
        // Do image validity checks
        LOG_I(TAG, "Current partition is pending verification");
    }

    const char* spiffsPartitionLabel = spiffs_service_get_spiffs_partition_label_for_app_partition(currentPartition->label);
    spiffs_service_mount(spiffsPartitionLabel, "/data");

    // I2C conflicts with Lift Pul and Lift Dir on current board
    // initialize_i2c();

    // Set time from RTC
    ds1307_device_handle_t handle;
    ds1307_err_t rtcErr = ds1307_add_device(I2C_PORT, 0x68, &handle);
    if(rtcErr == DS1307_OK)
    {
        time_t now;
        rtcErr = ds1307_get_time(handle, &now);
        if(rtcErr == DS1307_OK)
        {
            ds1307_remove_device(handle);

            struct timeval rtcTimeval = {
                .tv_sec = now,
                .tv_usec = 0
            };

            settimeofday(&rtcTimeval, NULL);

            LOG_I(TAG, "RTC Time: %s", asctime(gmtime(&now)));
        }
    }
    
    if(rtcErr != DS1307_OK)
    {
        LOG_W(TAG, "Could not get current time from RTC");
    }

    initialise_sdcard();
    initialise_mdns();
    initialise_wifi();
    initialize_sntp();

    lift_service_init();

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

    // If we get this far, assume app is functioning
    // TODO: Check workings of tasks
    esp_ota_mark_app_valid_cancel_rollback();
}

void app_main(void)
{
    initialize_app();

    for(;;)
    {
        vTaskSuspend(NULL);
    }

    vTaskDelete(NULL);
}

