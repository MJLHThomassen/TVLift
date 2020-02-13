#include "lift_controller.h"
#include "controller_base.h"

#include <stdio.h>

#include <frozen.h>

#include <services/logger_service.h>
#include <lift/lift.h>

#define controllerUri "/lift/"

static char TAG[] = __FILE__;

static lift_device_t liftHandle;

static uint32_t getSpeed(struct http_message* message)
{
    uint32_t speed = 3200;
    char param[32];

    if(mg_get_http_var(&message->query_string, "speed", param, sizeof(param)) > 0)
    {
        speed = (uint32_t)strtoul(param, NULL, 10);
    }

    return speed;
}

static void up_post_handler(struct mg_connection* nc, struct http_message* message)
{
    lift_err_t liftErr;

    uint32_t speed = getSpeed(message);
    liftErr = lift_set_speed(&liftHandle, speed);
    if(liftErr != LIFT_OK)
    {
        mg_http_send_error(nc, 500, "Can not set requested speed.");
        return;
    }

    liftErr = lift_up(&liftHandle);
    if(liftErr != LIFT_OK)
    {
        mg_http_send_error(nc, 500, "Can not move lift.");
        return;
    }

    mg_send_head(nc, 200, 0, NULL);
}

static void down_post_handler(struct mg_connection* nc, struct http_message* message)
{
    lift_err_t liftErr;

    uint32_t speed = getSpeed(message);
    liftErr = lift_set_speed(&liftHandle, speed);
    if(liftErr != LIFT_OK)
    {
        mg_http_send_error(nc, 500, "Can not set requested speed.");
        return;
    }

    liftErr = lift_down(&liftHandle);
    if(liftErr != LIFT_OK)
    {
        mg_http_send_error(nc, 500, "Can not move lift.");
        return;
    }

    mg_send_head(nc, 200, 0, NULL);
}

static void stop_post_handler(struct mg_connection* nc, struct http_message* message)
{
    lift_err_t liftErr = lift_stop(&liftHandle);
    if(liftErr != LIFT_OK)
    {
        mg_http_send_error(nc, 500, "Can not stop lift.");
        return;
    }

    mg_send_head(nc, 200, 0, NULL);
}

static void speed_get_handler(struct mg_connection* nc, struct http_message* message)
{
    char* str = json_asprintf("{speed:%u}", liftHandle.speed);
    mg_send_head(nc, 200, strlen(str), NULL);
    mg_printf(nc, "%s", str);
    free(str);
}

static uri_handler_info_t up_post_handler_info = {
    .uri = controllerUri "up",
    .method = HTTP_POST,
    .handler = up_post_handler,
    .user_data = NULL
    };

static uri_handler_info_t down_post_handler_info = {
    .uri = controllerUri "down",
    .method = HTTP_POST,
    .handler = down_post_handler,
    .user_data = NULL
    };

static uri_handler_info_t stop_post_handler_info = {
    .uri = controllerUri "stop",
    .method = HTTP_POST,
    .handler = stop_post_handler,
    .user_data = NULL
    };

static uri_handler_info_t speed_get_handler_info = {
    .uri = controllerUri "speed",
    .method = HTTP_GET,
    .handler = speed_get_handler,
    .user_data = NULL
    };

void lift_controller_register_uri_handlers(struct mg_connection* nc, const char* rootUri)
{
    register_uri_handler(nc, rootUri, &up_post_handler_info);
    register_uri_handler(nc, rootUri, &down_post_handler_info);
    register_uri_handler(nc, rootUri, &stop_post_handler_info);
    register_uri_handler(nc, rootUri, &speed_get_handler_info);

    // Initialize lift
    lift_add_device(26, 21, 22, 16, 17, &liftHandle);
}
