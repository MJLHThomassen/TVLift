#include "upload_controller.h"
#include "controller_base.h"

#include <esp_log.h>
#include <esp_ota_ops.h>
#include <esp_flash_partitions.h>
#include <esp_partition.h>

#define rootUri "/upload/"

inline int max ( int a, int b ) { return a > b ? a : b; }
inline int min ( int a, int b ) { return a < b ? a : b; }

static char TAG[] = __FILE__;

typedef struct ota_data_t
{
    size_t nr_of_bytes_received;
    uint8_t header[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t)];  

    esp_ota_handle_t update_handle;
    const esp_partition_t* update_partition;
} ota_data_t;


static void firmware_post_handler(struct mg_connection* nc, struct mg_http_multipart_part* part, multipart_request_part_type_t type)
{
    esp_err_t err;
    ota_data_t* ota_data = NULL;

    switch (type)
    {
    case MULTIPART_REQUEST_PART_TYPE_BEGIN:
    {
        const esp_partition_t* configured = esp_ota_get_boot_partition();
        const esp_partition_t* running = esp_ota_get_running_partition();

        if (configured != running) 
        {
            ESP_LOGW(TAG, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x", configured->address, running->address);
            ESP_LOGW(TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
        }

        ESP_LOGI(TAG, "Running partition %s", running->label);

        // Initialize ota_data struct
        ota_data = (ota_data_t*)calloc(1, sizeof(ota_data_t));
        ota_data->update_partition = esp_ota_get_next_update_partition(NULL);

        // Set the ota_data struct as user_data for use in subsequent calls
        part->user_data = ota_data;
        break;
    }
    
    case MULTIPART_REQUEST_PART_TYPE_DATA:
    {
        ota_data = (ota_data_t*)part->user_data;
        break;
    }

    case MULTIPART_REQUEST_PART_TYPE_END:
    {
        ota_data = (ota_data_t*)part->user_data;

        // Check if the upload was completed succesfully
        if(part->status < 0)
        {
            // Multipart message was not completed
            mg_http_send_error(nc, 400, NULL);
            return;
        }
        else
        {
            if (esp_ota_end(ota_data->update_handle) != ESP_OK) 
            {
                ESP_LOGE(TAG, "esp_ota_end failed!");
                mg_http_send_error(nc, 400, "Could not finalize OTA update: Failed to end OTA.");
                nc->flags |= MG_F_SEND_AND_CLOSE;
                return;
            }

            if (esp_ota_set_boot_partition(ota_data->update_partition)!= ESP_OK) 
            {
                ESP_LOGE(TAG, "esp_ota_set_boot_partition failed!");
                mg_http_send_error(nc, 400, "Could not finalize OTA update: Failed to set new boot partition.");
                nc->flags |= MG_F_SEND_AND_CLOSE;
                return;
            }

            ESP_LOGI(TAG, "OTA Done!");

            // Redirect client to reset page
            mg_http_send_redirect(nc, 301, mg_mk_str("/"),mg_mk_str(NULL));
            nc->flags |= MG_F_SEND_AND_CLOSE;

            // Restart the system
            ESP_LOGI(TAG, "Prepare to restart system!");
            esp_restart();

            return;
        }
    }
    }

    // Check if we need to write the header
    if(ota_data->nr_of_bytes_received < sizeof(ota_data->header))
    {
        size_t nrOfHeaderBytesRemaining = sizeof(ota_data->header) - ota_data->nr_of_bytes_received;
        size_t nrOfHeaderBytesToWrite = min(part->data.len, nrOfHeaderBytesRemaining);    

        memcpy(ota_data->header + ota_data->nr_of_bytes_received, part->data.p, nrOfHeaderBytesToWrite);

        if(ota_data->nr_of_bytes_received + part->data.len >= sizeof(ota_data->header))
        {
            // All header bytes received, read header and begin update
            ESP_LOGI(TAG, "Read header");
            esp_app_desc_t new_app_info;
            memcpy(&new_app_info, &ota_data->header[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)], sizeof(esp_app_desc_t));
            ESP_LOGI(TAG, "New firmware version: %s", new_app_info.version);

            err = esp_ota_begin(ota_data->update_partition, OTA_SIZE_UNKNOWN, &ota_data->update_handle);
            if (err != ESP_OK) 
            {
                ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
            }
            ESP_LOGI(TAG, "esp_ota_begin succeeded");

            // Write the first bit
            ESP_ERROR_CHECK(esp_ota_write(ota_data->update_handle, (const void *)ota_data->header, sizeof(ota_data->header)));
            ESP_ERROR_CHECK(esp_ota_write(ota_data->update_handle, (const void *)part->data.p + nrOfHeaderBytesToWrite, part->data.len - nrOfHeaderBytesToWrite));
        }
    }
    else
    {
        ESP_ERROR_CHECK(esp_ota_write(ota_data->update_handle, (const void *)part->data.p, part->data.len));
    }
    
    // Update the amount of bytes we have received
    ota_data->nr_of_bytes_received += part->data.len;
    ESP_LOGI(TAG, "Read %i", ota_data->nr_of_bytes_received);
}

static multipart_request_uri_handler_info_t firmware_post_handler_info = {
    .uri = rootUri "firmware",
    .method = HTTP_POST,
    .handler = firmware_post_handler,
    .user_data = NULL
    };

void upload_controller_register_uri_handlers(struct mg_connection* nc)
{
    register_multipart_request_uri_handler(nc, &firmware_post_handler_info);
}
