#include "ota_service.h"

#include <string.h>
#include <assert.h>

#include <esp_err.h>
#include <esp_ota_ops.h>
#include <esp_flash_partitions.h>
#include <esp_partition.h>

#include <services/spiffs_service.h>
#include <logger.h>

static const char TAG[] = "Ota Service";

static inline int max ( int a, int b ) { return a > b ? a : b; }
static inline int min ( int a, int b ) { return a < b ? a : b; }

typedef enum ota_progress_state_e
{
    OTA_PROGRESS_SPIFFS,
    OTA_PROGRESS_APP_HEADER,
    OTA_PROGRESS_APP_DATA,

    OTA_PROGRESS_OTA_FAILED
} ota_progress_state_t;


typedef struct app_header_s
{
    esp_image_header_t image_header;
    esp_image_segment_header_t image_segment_header;
    esp_app_desc_t app_desc;  
} app_header_t;

static_assert(sizeof(app_header_t) == (sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t)), "app_header_s is not packed");

struct ota_state_s
{
    // Progress
    size_t nr_of_bytes_processed;
    ota_progress_state_t progress;

    // Data
    const char* data;
    size_t length;

    // SPIFFS
    const esp_partition_t* spiffs_update_partition;

    // App
    app_header_t app_header;
    esp_ota_handle_t app_update_handle;
    const esp_partition_t* app_update_partition;
};

static const char * read_handle_data(ota_state_handle_t handle, size_t length)
{
    const char* data = handle->data;

    handle->data += length;
    handle->length -= length;
    handle->nr_of_bytes_processed += length;

    LOG_V(TAG, "Firmware update processed %i bytes", handle->nr_of_bytes_processed);

    return data;
}

static ota_service_err_t ota_service_app_update_begin(ota_state_handle_t handle)
{
    const esp_partition_t* configured = esp_ota_get_boot_partition();
    const esp_partition_t* running = esp_ota_get_running_partition();

    if (configured != running) 
    {
        LOG_W(TAG, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x", configured->address, running->address);
        LOG_W(TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow)");
    }

    handle->app_update_partition = esp_ota_get_next_update_partition(NULL);
    if(handle->app_update_partition == NULL)
    {
        LOG_E(TAG, "No valid ota partition found");
        handle->progress = OTA_PROGRESS_OTA_FAILED;
        return OTA_SERVICE_FAIL;
    }
    
    LOG_I(TAG, "Flashing app partition %s", handle->app_update_partition->label);

    return OTA_SERVICE_OK;
}

static ota_service_err_t ota_service_spiffs_update_begin(ota_state_handle_t handle)
{
    esp_err_t err;

    const char* spiffsLabel = spiffs_service_get_spiffs_partition_label_for_app_partition(handle->app_update_partition->label);

    handle->spiffs_update_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, spiffsLabel);
    if(handle->spiffs_update_partition == NULL)
    {
        LOG_E(TAG, "No valid spiffs partition found");
        handle->progress = OTA_PROGRESS_OTA_FAILED;
        return OTA_SERVICE_FAIL;
    }

    LOG_I(TAG, "Flashing spiffs partition %s", handle->spiffs_update_partition->label);

    err = esp_partition_erase_range(handle->spiffs_update_partition, 0, handle->spiffs_update_partition->size);
    if (err != ESP_OK) 
    {
        LOG_E(TAG, "esp_partition_erase_range failed (%s)", esp_err_to_name(err));
        handle->progress = OTA_PROGRESS_OTA_FAILED;
        return OTA_SERVICE_FAIL;
    }

    return OTA_SERVICE_OK;
}

static ota_service_err_t ota_service_spiffs_update_write(ota_state_handle_t handle)
{
    esp_err_t err;

    const size_t nrOfSpiffsBytesProcessed = handle->nr_of_bytes_processed;
    const size_t nrOfSpiffsBytesRemaining = handle->spiffs_update_partition->size - nrOfSpiffsBytesProcessed;
    const size_t nrOfSpiffsBytesToWrite = min(handle->length, nrOfSpiffsBytesRemaining);

    err = esp_partition_write(handle->spiffs_update_partition, nrOfSpiffsBytesProcessed, (const void *)read_handle_data(handle, nrOfSpiffsBytesToWrite), nrOfSpiffsBytesToWrite);
    if (err != ESP_OK) 
    {
        LOG_E(TAG, "esp_partition_write failed (%s)", esp_err_to_name(err));
        handle->progress = OTA_PROGRESS_OTA_FAILED;
        return OTA_SERVICE_FAIL;
    }

    if(handle->nr_of_bytes_processed == handle->spiffs_update_partition->size)
    {
        // All spiffs bytes received
        LOG_I(TAG, "OTA_PROGRESS_APP_HEADER");
        handle->progress = OTA_PROGRESS_APP_HEADER;
    }

    return OTA_SERVICE_OK; 
}

static ota_service_err_t ota_service_app_update_write_header(ota_state_handle_t handle)
{
    esp_err_t err;
    
    const size_t nrOfAppBytesProcessed = handle->nr_of_bytes_processed - handle->spiffs_update_partition->size;
    const size_t nrOfHeaderBytesRemaining = sizeof(handle->app_header) - nrOfAppBytesProcessed;
    const size_t nrOfHeaderBytesToWrite = min(handle->length, nrOfHeaderBytesRemaining);

    memcpy(&handle->app_header + nrOfAppBytesProcessed, read_handle_data(handle, nrOfHeaderBytesToWrite), nrOfHeaderBytesToWrite);

    if(handle->nr_of_bytes_processed - handle->spiffs_update_partition->size == sizeof(handle->app_header))
    {
        // All header bytes received, read header and begin update
        LOG_I(TAG, "Read header");
        LOG_I(TAG, "New firmware version: %s", handle->app_header.app_desc.version);

        // Begin OTA
        err = esp_ota_begin(handle->app_update_partition, OTA_SIZE_UNKNOWN, &handle->app_update_handle);
        if (err != ESP_OK) 
        {
            LOG_E(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
            handle->progress = OTA_PROGRESS_OTA_FAILED;
            return OTA_SERVICE_FAIL;
        }

        // Write the header OTA
        err = esp_ota_write(handle->app_update_handle, (const void *)&handle->app_header, sizeof(handle->app_header));
        if (err != ESP_OK) 
        {
            LOG_E(TAG, "esp_ota_write failed (%s)", esp_err_to_name(err));
            handle->progress = OTA_PROGRESS_OTA_FAILED;
            return OTA_SERVICE_FAIL;
        }

        LOG_I(TAG, "OTA_PROGRESS_APP_DATA");
        handle->progress = OTA_PROGRESS_APP_DATA;
    }

    return OTA_SERVICE_OK;
}

static ota_service_err_t ota_service_app_update_write_data(ota_state_handle_t handle)
{
    esp_err_t err;

    const size_t nrOfAppBytesToWrite = handle->length;

    err = esp_ota_write(handle->app_update_handle, (const void *)read_handle_data(handle, nrOfAppBytesToWrite), nrOfAppBytesToWrite);
    if (err != ESP_OK) 
    {
        LOG_E(TAG, "esp_ota_write failed (%s)", esp_err_to_name(err));
        handle->progress = OTA_PROGRESS_OTA_FAILED;
        return OTA_SERVICE_FAIL;
    }

    return OTA_SERVICE_OK; 
}

ota_service_err_t ota_service_firmware_update_begin(ota_state_handle_t* handle)
{
    ota_state_handle_t newHandle = (ota_state_handle_t)malloc(sizeof(*newHandle));
    if(newHandle == NULL)
    {
        LOG_E(TAG, "Can not allocate memory for firmware update ota state");
        return OTA_SERVICE_FAIL;
    }

    // Initialize handle
    newHandle->nr_of_bytes_processed = 0;
    newHandle->progress = OTA_PROGRESS_SPIFFS;

    newHandle->data = NULL;
    newHandle->length = 0;

    ota_service_app_update_begin(newHandle);
    ota_service_spiffs_update_begin(newHandle);

    *handle = newHandle;

    LOG_I(TAG, "OTA_PROGRESS_SPIFFS");
    return OTA_SERVICE_OK;
}

ota_service_err_t ota_service_firmware_update_write(ota_state_handle_t handle, const char* data, size_t length)
{
    ota_service_err_t err = OTA_SERVICE_OK;

    handle->data = data;
    handle->length = length;

    while(handle->length > 0)
    {
        switch(handle->progress)
        {
            case OTA_PROGRESS_SPIFFS:
                err = ota_service_spiffs_update_write(handle);
                break;

            case OTA_PROGRESS_APP_HEADER:
                err = ota_service_app_update_write_header(handle);
                break;

            case OTA_PROGRESS_APP_DATA:
                err =  ota_service_app_update_write_data(handle);
                break;
            
            case OTA_PROGRESS_OTA_FAILED:
            default:
                // Update failed or unknown state
                return OTA_SERVICE_FAIL;
        }

        if(err != OTA_SERVICE_OK)
        {
            return err;
        }
    }

    return OTA_SERVICE_OK;
}

ota_service_err_t ota_service_firmware_update_end(ota_state_handle_t handle)
{
    esp_err_t err;

    // First end app update
    err = esp_ota_end(handle->app_update_handle);
    if (err != ESP_OK) 
    {
        LOG_E(TAG, "esp_ota_end failed (%s)", esp_err_to_name(err));
        handle->progress = OTA_PROGRESS_OTA_FAILED;
        return OTA_SERVICE_ERR_OTA_END_FAILED;
    }

    // Set new boot partition
    err = esp_ota_set_boot_partition(handle->app_update_partition);

    // Free resources
    free(handle);

    // Error checking
    if (err != ESP_OK) 
    {
        LOG_E(TAG, "esp_ota_set_boot_partition failed");
        return OTA_SERVICE_ERR_SET_BOOT_PARTITON_FAILED;
    }

    LOG_I(TAG, "Firmware update done");
    return OTA_SERVICE_OK;
}