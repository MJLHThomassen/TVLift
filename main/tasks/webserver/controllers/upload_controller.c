#include "upload_controller.h"
#include "controller_base.h"

#include <services/ota_service.h>
#include <services/logger_service.h>

#define controllerUri "/upload/"

static char TAG[] = "Upload Controller";

static void firmware_post_handler(
    struct mg_connection* nc,
    struct http_message* const message,
    struct mg_http_multipart_part* part,
    multipart_request_message_type_t type,
    void* userData)
{
    ota_service_err_t otaErr = OTA_SERVICE_OK;
    ota_state_handle_t* otaState = NULL;

    switch (type)
    {
    case MULTIPART_REQUEST_MESSAGE_TYPE_BEGIN:
    {
        // Nothing to do
        break;
    }

    case MULTIPART_REQUEST_MESSAGE_TYPE_PART_BEGIN:
    {
        otaState = (ota_state_handle_t*)userData;
        if(strcmp(part->var_name, "firmware") == 0)
        {
            otaErr = ota_service_firmware_update_begin(otaState);
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
        otaState = (ota_state_handle_t*)userData;

        if(strcmp(part->var_name, "firmware") == 0)
        {
            otaErr = ota_service_firmware_update_write(*otaState, part->data.p, part->data.len);
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
        otaState = (ota_state_handle_t*)userData;

        // Check if the upload was completed succesfully
        if(part->status < 0)
        {
            // Multipart message was not completed
            mg_http_send_error(nc, 400, "Multipart message part not properly terminated.");
            return;
        }
        else
        {
            if(strcmp(part->var_name, "firmware") == 0)
            {
                otaErr = ota_service_firmware_update_end(*otaState);
            }
            else
            {
                mg_http_send_error(nc, 400, "Multipart variable name not recognized.");
                return;
            }
        }

        break;
    }

    case MULTIPART_REQUEST_MESSAGE_TYPE_END:
    {
        // Redirect client to reset page
        mg_http_send_redirect(nc, 301, mg_mk_str("/"), mg_mk_str(NULL));
        nc->flags |= MG_F_SEND_AND_CLOSE;

        // Restart the system
        //LOG_I(TAG, "Prepare to restart system!");
        //esp_restart();
        return;
    }

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

static ota_state_handle_t ota_state;
static multipart_request_uri_handler_info_t firmware_post_handler_info = {
    .uri = controllerUri "firmware",
    .method = HTTP_POST,
    .handler = firmware_post_handler,
    .user_data = &ota_state
    };

void upload_controller_register_uri_handlers(struct mg_connection* nc, const char* rootUri)
{
    register_multipart_request_uri_handler(nc, rootUri, &firmware_post_handler_info);
}
