#include "upload_controller.h"
#include "controller_base.h"

#include "ota/ota_service.h"

#include <esp_log.h>

#define rootUri "/upload/"

static char TAG[] = __FILE__;

static void firmware_post_handler(struct mg_connection* nc, struct mg_http_multipart_part* part, multipart_request_message_type_t type, void* userData)
{
    esp_err_t err;
    ota_service_err_t otaErr = OTA_SERVICE_OK;
    ota_state_t* otaState = NULL;

    switch (type)
    {
    case MULTIPART_REQUEST_MESSAGE_TYPE_BEGIN:
    {
        otaState = (ota_state_t*)userData;
        ota_service_initialize(otaState);
    }
    break;
    case MULTIPART_REQUEST_MESSAGE_TYPE_PART_BEGIN:
    {
        otaState = (ota_state_t*)userData;
        if(strcmp(part->var_name, "app") == 0)
        {
            otaErr = ota_service_app_update_begin(otaState);
        }
        else if(strcmp(part->var_name, "spiffs") == 0)
        {
            otaErr = ota_service_spiffs_update_begin(otaState);
        }
        else
        {
            mg_http_send_error(nc, 400, "Multipart variable name not recognized.");
            return;
        }
        break;
    }
    
    case MULTIPART_REQUEST_MESSAGE_TYPE_PART_DATA:
    {
        otaState = (ota_state_t*)userData;

        if(strcmp(part->var_name, "app") == 0)
        {
            otaErr = ota_service_app_update_write(otaState, part->data.p, part->data.len);
        }
        else if(strcmp(part->var_name, "spiffs") == 0)
        {
            otaErr = ota_service_spiffs_update_begin(otaState);
        }
        else
        {
            mg_http_send_error(nc, 400, "Multipart variable name not recognized.");
            return;
        }

        if(otaErr != OTA_SERVICE_OK)
        {
            mg_http_send_error(nc, 500, "Writing update file failed.");
            return;
        }

        break;
    }

    case MULTIPART_REQUEST_MESSAGE_TYPE_PART_END:
    {
        otaState = (ota_state_t*)userData;

        // Check if the upload was completed succesfully
        if(part->status < 0)
        {
            // Multipart message was not completed
            mg_http_send_error(nc, 400, "Multipart message part not properly terminated.");
            return;
        }
        else
        {
            if(strcmp(part->var_name, "app") == 0)
            {
                otaErr = ota_service_app_update_end(otaState);
            }
            else if(strcmp(part->var_name, "spiffs") == 0)
            {
                otaErr = ota_service_spiffs_update_end(otaState);
            }
            else
            {
                mg_http_send_error(nc, 400, "Multipart variable name not recognized.");
                return;
            }
        }
    }
    case MULTIPART_REQUEST_MESSAGE_TYPE_END:
    {
        // Redirect client to reset page
        mg_http_send_redirect(nc, 301, mg_mk_str("/"),mg_mk_str(NULL));
        nc->flags |= MG_F_SEND_AND_CLOSE;

        // Restart the system
        ESP_LOGI(TAG, "Prepare to restart system!");
        esp_restart();
        return;
    }
    break;
    }
      
    switch (otaErr)
    {
    case OTA_SERVICE_OK:
        // Everyting went well
        break;
    case OTA_SERVICE_ERR_OTA_END_FAILED:
        mg_http_send_error(nc, 400, "Could not finalize OTA update: Failed to end OTA.");
        break;
    case OTA_SERVICE_ERR_SET_BOOT_PARTITON_FAILED:
        mg_http_send_error(nc, 400, "Could not finalize OTA update: Failed to set new boot partition.");
        break;
    default:
        // Something unknown went wrong
        mg_http_send_error(nc, 500, "OTA update failed.");
        break;
    }
}

static ota_state_t ota_state;
static multipart_request_uri_handler_info_t firmware_post_handler_info = {
    .uri = rootUri "firmware",
    .method = HTTP_POST,
    .handler = firmware_post_handler,
    .user_data = &ota_state
    };

void upload_controller_register_uri_handlers(struct mg_connection* nc)
{
    register_multipart_request_uri_handler(nc, &firmware_post_handler_info);
}
