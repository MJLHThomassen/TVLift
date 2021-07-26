#include "settings_controller.h"
#include "controller_base.h"

#include <stdio.h>

#include <frozen.h>

#include <logger.h>
#include <services/settings_service.h>

#define controllerUri "/settings"

static char TAG[] = __FILE__;

static settings_t getSettings(struct http_message* message)
{
    settings_t settings;

    json_scanf(
        message->body.p,
        message->body.len,
        "{"
            "liftMinSpeed: %u",
            "liftMaxSpeed: %u",
            "liftDefaultSpeed: %u"
        "}",
        &settings.lift_min_speed,
        &settings.lift_max_speed,
        &settings.lift_default_speed);
    
    return settings;
}

static void settings_get_handler(struct mg_connection* nc, struct http_message* message, void* userData)
{
    settings_err_t err;
    settings_t settings;

    err = settings_service_load(&settings);

    if(err != SETTINGS_SERVICE_OK)
    {
        mg_http_send_error(nc, 500, "Can not read settings.");
        return;
    }

    char* str = json_asprintf(
        "{"
            "version": %u,
            "liftMinSpeed: %u",
            "liftMaxSpeed: %u",
            "liftDefaultSpeed: %u"
        "}",
        settings.version,
        settings.lift_min_speed,
        settings.lift_max_speed,
        settings.lift_default_speed
    );

    mg_send_head(nc, 200, strlen(str), NULL);
    mg_printf(nc, "%s", str);
    free(str);
}

static void settings_post_handler(struct mg_connection* nc, struct http_message* message, void* userData)
{
    settings_err_t err;

    settings_t settings = getSettings(message);
    err = settings_service_save(&settings);

    // TODO: Propagate settings

    if(err != SETTINGS_SERVICE_OK)
    {
        mg_http_send_error(nc, 500, "Can not modify settings.");
        return;
    }

    mg_send_head(nc, 200, 0, NULL);
}

static uri_handler_info_t settings_handler_info = {
    .uri = controllerUri,
    .methodHandlers = {
        {
            .method = HTTP_REQUEST_METHOD_GET,
            .handler = settings_get_handler,
            .user_data = NULL
        },
        {
            .method = HTTP_REQUEST_METHOD_POST,
            .handler = settings_post_handler,
            .user_data = NULL
        }
    }
    };

void settings_controller_register_uri_handlers(struct mg_connection* nc, const char* rootUri)
{   
    // Register uri's
    register_uri_handler(nc, rootUri, &settings_handler_info);
}
