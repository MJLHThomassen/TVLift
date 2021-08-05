
#include "spiffs_service.h"

#include <esp_log.h>
#include <esp_err.h>
#include <esp_ota_ops.h>
#include <esp_vfs_dev.h>
#include <esp_spiffs.h>

#include <logger.h>

static const char TAG[] = "Spiffs Service";

const char* spiffs_service_get_spiffs_partition_label_for_app_partition(const char* appPartitionLabel)
{
    const esp_partition_t* appPartition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, appPartitionLabel);

    // TODO: Do this a bit better
    assert(appPartition != NULL);

    // Find spiffs partition belonging to current app partition
    const esp_partition_t* spiffsPartition = NULL;
    esp_partition_iterator_t iter = esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, NULL);

    do
    {
        // TODO: Do this a bit better
        assert(iter != NULL);

        spiffsPartition = esp_partition_get(iter);
        iter = esp_partition_next(iter);
    } while (spiffsPartition->address != appPartition->address + appPartition->size);

    // If we got here, the spiffs partition is the partition right after the app partition
    esp_partition_iterator_release(iter);

    return spiffsPartition->label;
}

void spiffs_service_mount(const char* partitionLabel, const char* basePath)
{    
    esp_vfs_spiffs_conf_t conf = {
        .base_path = basePath,
        .partition_label = partitionLabel,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    LOG_I(TAG, "Mounting SPIFFS partition %s at %s", partitionLabel, basePath);
    
    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t err = esp_vfs_spiffs_register(&conf);

    if (err != ESP_OK) 
    {
        if (err == ESP_FAIL) 
        {
            LOG_E(TAG, "Failed to mount or format filesystem");
        }
        else if (err == ESP_ERR_NOT_FOUND) 
        {
            LOG_E(TAG, "Failed to find SPIFFS partition %s", partitionLabel);
        }
        else 
        {
            LOG_E(TAG, "Failed to initialize SPIFFS partition %s (%s)", partitionLabel, esp_err_to_name(err));
        }
        return;
    }

    size_t total = 0, used = 0;
    err = esp_spiffs_info(partitionLabel, &total, &used);
    if (err != ESP_OK) 
    {
        LOG_E(TAG, "Failed to get SPIFFS partition %s information (%s)", partitionLabel, esp_err_to_name(err));
    }
    else 
    {
        LOG_I(TAG, "Partition size: total: %d, used: %d", total, used);
    }
}