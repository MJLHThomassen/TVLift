#include "webserver_task.h"

#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_event.h>

// #include <esp_http_server.h>
#include <mongoose.h>

#include "controllers/lift_controller.h"
#include "controllers/upload_controller.h"

#define WEBROOT "/spiffs/www/"

static struct mg_mgr manager;
static struct mg_connection* webserverConnection;
static struct mg_serve_http_opts http_server_opts;

static const char* TAG = WEBSERVER_TASK_TAG;

static void webserver_ev_handler(struct mg_connection* c, int ev, void* ev_data, void* user_data)
{
    struct http_message* message = (struct http_message*) ev_data;

    switch (ev)
    {
    case MG_EV_HTTP_REQUEST:
        if (mg_vcmp(&message->uri, "/") == 0) 
        {
            mg_http_serve_file(c, message, WEBROOT "index.html", mg_mk_str("text/html"), mg_mk_str(""));
        }
        else
        {
            mg_serve_http(c, message, http_server_opts);
        }
        break;
    }
}

static void wifi_event_sta_disconnected(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    // // Stop the web server
    // if (webserver)
    // {
    //     stop_webserver(webserver);
    //     webserver = NULL;
    // }
}

static void ip_event_sta_got_ip(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    // // Start the web server
    // if (webserver == NULL)
    // {
    //     webserver = start_webserver();
    // }
}

static void webserver_init()
{
    http_server_opts.document_root = WEBROOT;
    http_server_opts.enable_directory_listing = "no";
    http_server_opts.index_files =  WEBROOT "index.html";

    esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, wifi_event_sta_disconnected, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, ip_event_sta_got_ip, NULL);

    mg_mgr_init(&manager, NULL);

    // Set up webserver connection
    ESP_LOGI(TAG, "Starting webserver on port: '%d'", 80);
    webserverConnection = mg_bind(&manager, "80", webserver_ev_handler, NULL);
    mg_set_protocol_http_websocket(webserverConnection);
    
    // Set URI handlers
    ESP_LOGI(TAG, "Registering URI handlers");
    lift_controller_register_uri_handlers(webserverConnection);
    upload_controller_register_uri_handlers(webserverConnection);
}

void webserver_task_main(void* pvParameters)
{
    ESP_LOGD(TAG, "Starting task");
    ESP_LOGV(TAG, "Free stack space: %i", uxTaskGetStackHighWaterMark(NULL));
    webserver_init();

    ESP_LOGD(TAG, "Starting task loop");
    ESP_LOGV(TAG, "Free stack space: %i", uxTaskGetStackHighWaterMark(NULL));
    for (;;)
    {
        mg_mgr_poll(&manager, 100);
    }

    mg_mgr_free(&manager);
}
