#include "ota_service.h"

#include <string.h>

#include <esp_err.h>
#include <esp_flash_partitions.h>
#include <esp_partition.h>

#include <services/logger_service.h>

static char TAG[] = __FILE__;

static inline int max ( int a, int b ) { return a > b ? a : b; }
static inline int min ( int a, int b ) { return a < b ? a : b; }

ota_service_err_t ota_service_initialize(ota_state_t* otaState)
{
    const esp_partition_t* configured = esp_ota_get_boot_partition();
    const esp_partition_t* running = esp_ota_get_running_partition();

    if (configured != running) 
    {
        LOG_W(TAG, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x", configured->address, running->address);
        LOG_W(TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
    }

    LOG_I(TAG, "Running partition %s", running->label);

    // Initialize otaState
    memset(otaState, 0, sizeof(ota_state_t));
    otaState->app_update_partition = esp_ota_get_next_update_partition(NULL);

    return OTA_SERVICE_OK;
}

ota_service_err_t ota_service_app_update_begin(ota_state_t* otaState)
{
    return OTA_SERVICE_OK;
}

ota_service_err_t ota_service_app_update_write(ota_state_t* otaState, const char* data, size_t length)
{
    esp_err_t err;
    
    // Check if we need to write the header
    if(otaState->nr_of_bytes_received < sizeof(otaState->app_header))
    {
        size_t nrOfHeaderBytesRemaining = sizeof(otaState->app_header) - otaState->nr_of_bytes_received;
        size_t nrOfHeaderBytesToWrite = min(length, nrOfHeaderBytesRemaining);    

        memcpy(otaState->app_header + otaState->nr_of_bytes_received, data, nrOfHeaderBytesToWrite);

        if(otaState->nr_of_bytes_received + length >= sizeof(otaState->app_header))
        {
            // All header bytes received, read header and begin update
            LOG_I(TAG, "Read header");
            esp_image_segment_header_t newAppImageSegmentHeader;
            esp_app_desc_t new_app_info;
            memcpy(&new_app_info, &otaState->app_header[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)], sizeof(esp_app_desc_t));
            LOG_I(TAG, "New firmware version: %s", new_app_info.version);
            LOG_I(TAG, "Length: %s", new_app_info.version);

            err = esp_ota_begin(otaState->app_update_partition, OTA_SIZE_UNKNOWN, &otaState->app_update_handle);
            if (err != ESP_OK) 
            {
                LOG_E(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
                return OTA_SERVICE_FAIL;
            }
            LOG_I(TAG, "esp_ota_begin succeeded");

            // Write the first piece of the ota
            ESP_ERROR_CHECK(esp_ota_write(otaState->app_update_handle, (const void *)otaState->app_header, sizeof(otaState->app_header)));
            ESP_ERROR_CHECK(esp_ota_write(otaState->app_update_handle, (const void *)data + nrOfHeaderBytesToWrite, length - nrOfHeaderBytesToWrite));
        }
    }
    else
    {
        ESP_ERROR_CHECK(esp_ota_write(otaState->app_update_handle, (const void *)data, length));
    }
    
    // Update the amount of bytes we have received
    otaState->nr_of_bytes_received += length;
    LOG_I(TAG, "Read %i", otaState->nr_of_bytes_received);

    return OTA_SERVICE_OK;
}

ota_service_err_t ota_service_app_update_end(ota_state_t* otaState)
{
    if (esp_ota_end(otaState->app_update_handle) != ESP_OK) 
    {
        LOG_E(TAG, "esp_ota_end failed!");
        return OTA_SERVICE_ERR_OTA_END_FAILED;
    }

    LOG_I(TAG, "App Done!");
    return OTA_SERVICE_OK;
}

ota_service_err_t ota_service_spiffs_update_begin(ota_state_t* otaState)
{
    return OTA_SERVICE_OK;
}

ota_service_err_t ota_service_spiffs_update_write(ota_state_t* otaState, const char* data, size_t length)
{
    return OTA_SERVICE_OK;
}

ota_service_err_t ota_service_spiffs_update_end(ota_state_t* otaState)
{
    return OTA_SERVICE_OK;
}

ota_service_err_t ota_service_finalize(ota_state_t* otaState)
{
    if (esp_ota_set_boot_partition(otaState->app_update_partition)!= ESP_OK) 
    {
        LOG_E(TAG, "esp_ota_set_boot_partition failed!");
        return OTA_SERVICE_ERR_SET_BOOT_PARTITON_FAILED;
    }

    LOG_I(TAG, "OTA Done!");
    return OTA_SERVICE_OK;
}