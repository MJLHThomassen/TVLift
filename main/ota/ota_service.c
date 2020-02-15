#include "ota_service.h"

#include <string.h>

#include <esp_err.h>
#include <esp_ota_ops.h>
#include <esp_flash_partitions.h>
#include <esp_partition.h>

#include <services/logger_service.h>

static char TAG[] = __FILE__;

static inline int max ( int a, int b ) { return a > b ? a : b; }
static inline int min ( int a, int b ) { return a < b ? a : b; }

struct ota_state_s
{
    size_t nr_of_bytes_received;

    // App
    uint8_t app_header[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t)];  
    esp_ota_handle_t app_update_handle;
    const esp_partition_t* app_update_partition;

    // SPIFFS
};

ota_service_err_t ota_service_initialize(ota_state_handle_t* handle)
{
    ota_state_handle_t newHandle = (ota_state_handle_t)malloc(sizeof(*newHandle));

    if(newHandle == NULL)
    {
        LOG_E(TAG, "Can not allocate memory for ota state");
        return OTA_SERVICE_FAIL;
    }

    const esp_partition_t* configured = esp_ota_get_boot_partition();
    const esp_partition_t* running = esp_ota_get_running_partition();

    if (configured != running) 
    {
        LOG_W(TAG, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x", configured->address, running->address);
        LOG_W(TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow)");
    }

    LOG_I(TAG, "Running partition %s", running->label);

    // Initialize handle
    memset(newHandle, 0, sizeof(*newHandle));
    newHandle->app_update_partition = esp_ota_get_next_update_partition(NULL);

    *handle = newHandle;

    return OTA_SERVICE_OK;
}

ota_service_err_t ota_service_app_update_begin(ota_state_handle_t handle)
{
    return OTA_SERVICE_OK;
}

ota_service_err_t ota_service_app_update_write(ota_state_handle_t handle, const char* data, size_t length)
{
    esp_err_t err;
    
    // Check if we need to write the header
    if(handle->nr_of_bytes_received < sizeof(handle->app_header))
    {
        size_t nrOfHeaderBytesRemaining = sizeof(handle->app_header) - handle->nr_of_bytes_received;
        size_t nrOfHeaderBytesToWrite = min(length, nrOfHeaderBytesRemaining);    

        memcpy(handle->app_header + handle->nr_of_bytes_received, data, nrOfHeaderBytesToWrite);

        if(handle->nr_of_bytes_received + length >= sizeof(handle->app_header))
        {
            // All header bytes received, read header and begin update
            LOG_I(TAG, "Read header");
            esp_app_desc_t new_app_info;
            memcpy(&new_app_info, &handle->app_header[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)], sizeof(esp_app_desc_t));
            LOG_I(TAG, "New firmware version: %s", new_app_info.version);
            LOG_I(TAG, "Length: %s", new_app_info.version);

            err = esp_ota_begin(handle->app_update_partition, OTA_SIZE_UNKNOWN, &handle->app_update_handle);
            if (err != ESP_OK) 
            {
                LOG_E(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
                return OTA_SERVICE_FAIL;
            }
            LOG_I(TAG, "esp_ota_begin succeeded");

            // Write the first piece of the ota
            ESP_ERROR_CHECK(esp_ota_write(handle->app_update_handle, (const void *)handle->app_header, sizeof(handle->app_header)));
            ESP_ERROR_CHECK(esp_ota_write(handle->app_update_handle, (const void *)data + nrOfHeaderBytesToWrite, length - nrOfHeaderBytesToWrite));
        }
    }
    else
    {
        ESP_ERROR_CHECK(esp_ota_write(handle->app_update_handle, (const void *)data, length));
    }
    
    // Update the amount of bytes we have received
    handle->nr_of_bytes_received += length;
    LOG_I(TAG, "Read %i", handle->nr_of_bytes_received);

    return OTA_SERVICE_OK;
}

ota_service_err_t ota_service_app_update_end(ota_state_handle_t handle)
{
    if (esp_ota_end(handle->app_update_handle) != ESP_OK) 
    {
        LOG_E(TAG, "esp_ota_end failed!");
        return OTA_SERVICE_ERR_OTA_END_FAILED;
    }

    LOG_I(TAG, "App Done!");
    return OTA_SERVICE_OK;
}

ota_service_err_t ota_service_spiffs_update_begin(ota_state_handle_t handle)
{
    return OTA_SERVICE_OK;
}

ota_service_err_t ota_service_spiffs_update_write(ota_state_handle_t handle, const char* data, size_t length)
{
    return OTA_SERVICE_OK;
}

ota_service_err_t ota_service_spiffs_update_end(ota_state_handle_t handle)
{
    return OTA_SERVICE_OK;
}

ota_service_err_t ota_service_finalize(ota_state_handle_t handle)
{
    esp_err_t err = esp_ota_set_boot_partition(handle->app_update_partition);

    free(handle);

    if (err != ESP_OK) 
    {
        LOG_E(TAG, "esp_ota_set_boot_partition failed!");
        return OTA_SERVICE_ERR_SET_BOOT_PARTITON_FAILED;
    }

    LOG_I(TAG, "OTA Done!");
    return OTA_SERVICE_OK;
}