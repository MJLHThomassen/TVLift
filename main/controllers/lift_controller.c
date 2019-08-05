#include "lift_controller.h"

#include <stdio.h>
#include <esp_log.h>
#include <frozen.h>

#include "lift/lift.h"

#define rootUri "/lift/"

static char TAG[] = __FILE__;

static lift_device_t liftHandle;

static uint32_t getSpeed(httpd_req_t *req)
{
    uint32_t speed = 6400;

    char* buf;
    size_t buf_len = httpd_req_get_url_query_len(req) + 1;

    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK)
        {
            char param[32];
            
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "speed", param, sizeof(param)) == ESP_OK)
            {
                speed = (uint32_t)strtoul(param, NULL, 10);
            }
        }
        free(buf);
    }

    return speed;
}

static esp_err_t up_post_handler(httpd_req_t *req)
{
    uint32_t speed = getSpeed(req);
    lift_set_speed(&liftHandle, speed);

    lift_up(&liftHandle);

    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t down_post_handler(httpd_req_t *req)
{
    uint32_t speed = getSpeed(req);
    lift_set_speed(&liftHandle, speed);

    lift_down(&liftHandle);

    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t stop_post_handler(httpd_req_t *req)
{
    lift_stop(&liftHandle);

    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t disable_post_handler(httpd_req_t *req)
{
    lift_disable(&liftHandle);

    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t speed_post_handler(httpd_req_t *req)
{
    char *str = json_asprintf("{speed:%u}", liftHandle.speed);
    httpd_resp_set_type(req, HTTPD_TYPE_JSON);
    httpd_resp_send(req, str, HTTPD_RESP_USE_STRLEN);
    free(str);

    return ESP_OK;
}

static httpd_uri_t lift_up_uri_handler = {
    .uri = rootUri "up",
    .method = HTTP_POST,
    .handler = up_post_handler,
    .user_ctx = NULL
    };

static httpd_uri_t lift_down_uri_handler = {
    .uri = rootUri "down",
    .method = HTTP_POST,
    .handler = down_post_handler,
    .user_ctx = NULL
    };

static httpd_uri_t lift_stop_uri_handler = {
    .uri = rootUri "stop",
    .method = HTTP_POST,
    .handler = stop_post_handler,
    .user_ctx = NULL
    };

static httpd_uri_t lift_disable_uri_handler = {
    .uri = rootUri "disable",
    .method = HTTP_POST,
    .handler = disable_post_handler,
    .user_ctx = NULL
    };

static httpd_uri_t lift_speed_uri_handler = {
    .uri = rootUri "speed",
    .method = HTTP_GET,
    .handler = speed_post_handler,
    .user_ctx = NULL
    };

void lift_controller_register_uri_handlers(httpd_handle_t server)
{
    httpd_register_uri_handler(server, &lift_up_uri_handler);
    httpd_register_uri_handler(server, &lift_down_uri_handler);
    httpd_register_uri_handler(server, &lift_stop_uri_handler);
    httpd_register_uri_handler(server, &lift_disable_uri_handler);
    httpd_register_uri_handler(server, &lift_speed_uri_handler);

    // Initialize lift
    lift_add_device(26, 18, 19, &liftHandle);
}
