#include "lift_controller.h"
#include "controller_base.h"

#include <stdio.h>

#include <frozen.h>

#include <pins.h>
#include <logger.h>
#include <lift/lift.h>
#include <services/lift_service.h>
#include <services/settings_service.h>

#define controllerUri "/lift"

static char TAG[] = __FILE__;

static uint32_t getSpeed(struct http_message* message)
{
    const settings_t* settings;
    settings_service_load(&settings);

    uint32_t speed = settings->lift_default_speed;   

    json_scanf(
        message->body.p,
        message->body.len,
        "{"
            "speed: %u" 
        "}",
        &speed);
    
    return speed;
}

static void status_get_handler(struct mg_connection* nc, struct http_message* message, void* userData)
{
    lift_device_handle_t liftHandle = lift_service_get_lift_device_handle();

    char* str = json_asprintf(
        "{"
            "status: %Q"
        "}",
        liftHandle == NULL ? "offline" : "online"
    );
    mg_send_head(nc, 200, strlen(str), NULL);
    mg_printf(nc, "%s", str);
    free(str);
}

static void up_post_handler(struct mg_connection* nc, struct http_message* message, void* userData)
{
    lift_err_t liftErr;
    lift_device_handle_t liftHandle = lift_service_get_lift_device_handle();

    uint32_t speed = getSpeed(message);
    liftErr = lift_set_speed(liftHandle, speed);
    if(liftErr != LIFT_OK)
    {
        mg_http_send_error(nc, 500, "Can not set requested speed.");
        return;
    }

    liftErr = lift_up(liftHandle);
    if(liftErr != LIFT_OK)
    {
        mg_http_send_error(nc, 500, "Can not move lift.");
        return;
    }

    mg_send_head(nc, 200, 0, NULL);
}

static void down_post_handler(struct mg_connection* nc, struct http_message* message, void* userData)
{
    lift_err_t liftErr;
    lift_device_handle_t liftHandle = lift_service_get_lift_device_handle();

    uint32_t speed = getSpeed(message);
    liftErr = lift_set_speed(liftHandle, speed);
    if(liftErr != LIFT_OK)
    {
        mg_http_send_error(nc, 500, "Can not set requested speed.");
        return;
    }

    liftErr = lift_down(liftHandle);
    if(liftErr != LIFT_OK)
    {
        mg_http_send_error(nc, 500, "Can not move lift.");
        return;
    }

    mg_send_head(nc, 200, 0, NULL);
}

static void stop_post_handler(struct mg_connection* nc, struct http_message* message, void* userData)
{
    lift_device_handle_t liftHandle = lift_service_get_lift_device_handle();
    lift_err_t liftErr = lift_stop(liftHandle);
    if(liftErr != LIFT_OK)
    {
        mg_http_send_error(nc, 500, "Can not stop lift.");
        return;
    }

    mg_send_head(nc, 200, 0, NULL);
}

static void speed_get_handler(struct mg_connection* nc, struct http_message* message, void* userData)
{
    lift_device_handle_t liftHandle = lift_service_get_lift_device_handle();

    char* str = json_asprintf(
        "{"
            "speed: %u"
        "}",
        lift_get_speed(liftHandle)
    );
    mg_send_head(nc, 200, strlen(str), NULL);
    mg_printf(nc, "%s", str);
    free(str);
}

static void speed_post_handler(struct mg_connection* nc, struct http_message* message, void* userData)
{
    lift_err_t liftErr;
    lift_device_handle_t liftHandle = lift_service_get_lift_device_handle();

    uint32_t speed = getSpeed(message);
    liftErr = lift_set_speed(liftHandle, speed);
    if(liftErr != LIFT_OK)
    {
        mg_http_send_error(nc, 500, "Can not set requested speed.");
        return;
    }

    mg_send_head(nc, 200, 0, NULL);
}

static uri_handler_info_t status_handler_info = {
    .uri = controllerUri "/status",
    .methodHandlers = {
        {
            .method = HTTP_REQUEST_METHOD_GET,
            .handler = status_get_handler,
            .user_data = NULL
        }
    }
    };

static uri_handler_info_t up_handler_info = {
    .uri = controllerUri "/up",
    .methodHandlers = {
        {
            .method = HTTP_REQUEST_METHOD_POST,
            .handler = up_post_handler,
            .user_data = NULL
        }
    }
    };

static uri_handler_info_t down_handler_info = {
    .uri = controllerUri "/down",
    .methodHandlers = {
        {
            .method = HTTP_REQUEST_METHOD_POST,
            .handler = down_post_handler,
            .user_data = NULL
        }
    }
    };

static uri_handler_info_t stop_handler_info = {
    .uri = controllerUri "/stop",
    .methodHandlers = {
        {
            .method = HTTP_REQUEST_METHOD_POST,
            .handler = stop_post_handler,
            .user_data = NULL
        }
    }
    };

static uri_handler_info_t speed_handler_info = {
    .uri = controllerUri "/speed",
    .methodHandlers = {
        {
            .method = HTTP_REQUEST_METHOD_GET,
            .handler = speed_get_handler,
            .user_data = NULL
        },
        {
            .method = HTTP_REQUEST_METHOD_POST,
            .handler = speed_post_handler,
            .user_data = NULL
        }
    }
    };

void lift_controller_register_uri_handlers(struct mg_connection* nc, const char* rootUri)
{   
    // Register status uri
    register_uri_handler(nc, rootUri, &status_handler_info);

    if(lift_service_get_lift_device_handle() != NULL)
    {
        // Register other uri's
        register_uri_handler(nc, rootUri, &up_handler_info);
        register_uri_handler(nc, rootUri, &down_handler_info);
        register_uri_handler(nc, rootUri, &stop_handler_info);
        register_uri_handler(nc, rootUri, &speed_handler_info);
    }
}
