#include "upload_controller.h"
#include "controller_base.h"

#include <stdio.h>
#include <esp_log.h>
#include <frozen.h>

#define rootUri "/upload/"

static char TAG[] = __FILE__;

static void firmware_post_handler(struct mg_connection* nc, struct mg_http_multipart_part* part)
{
    ESP_LOGI(TAG, "Received multipart request: %s, %s", part->var_name, part->file_name);
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
