#include "webserver_task.h"

#include <stdbool.h>
#include <string.h>

#include <soc/soc.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_event.h>

#include <mongoose.h>

#include <shared.h>
#include <logger.h>
#include <services/status_service.h>

#include "controllers/lift_controller.h"
#include "controllers/settings_controller.h"
#include "controllers/upload_controller.h"

#define WEBROOT "/data/www/"
#define WEBSERVER_THREAD_TAG "WebserverThread"
#define WEBSERVER_THREAD_STACK_SIZE_KB 8

static service_handle_t serviceHandle;
static TaskHandle_t webserverTaskHandle = NULL;
static TaskHandle_t webserverThreadHandle = NULL;
static SemaphoreHandle_t webserverControlSemaphore = NULL;

static struct mg_mgr manager;
static struct mg_connection* webserverConnection;
static struct mg_serve_http_opts http_server_opts = {
    .document_root = WEBROOT,
    .enable_directory_listing = "no",
    .index_files =  WEBROOT "index.html",
    .extra_headers = "Access-Control-Allow-Origin: *"
};

static const char* TAG = WEBSERVER_TASK_TAG;

static void websocket_sink(const char* message, const size_t len, void* user_data)
{
    struct mg_connection* websocketLoggerConnection = (struct mg_connection*) user_data;
    mg_send_websocket_frame(websocketLoggerConnection, WEBSOCKET_OP_TEXT, message, len);
}

static void webserver_ev_handler(struct mg_connection* c, int ev, void* ev_data, void* user_data)
{
    struct http_message* message = (struct http_message*) ev_data;
    // struct websocket_message* wm = (struct websocket_message *) ev_data;

    switch (ev)
    {
    case MG_EV_HTTP_REQUEST:
        if (mg_vcmp(&message->uri, "/") == 0) 
        {
            mg_http_serve_file(c, message, WEBROOT "index.html", mg_mk_str("text/html"), mg_mk_str(NULL));
        }
        else
        {
            mg_serve_http(c, message, http_server_opts);
        }
        break;

    case MG_EV_WEBSOCKET_HANDSHAKE_DONE:
        // New websocket connection
        LOG_I(TAG, "Websocket Connect");
        sink_handle_t handle = logger_service_register_sink(websocket_sink, (void*)c);

        // Save the handle
        c->user_data = handle;

        break;
        
    case MG_EV_CLOSE: 
        // Disconnect
        if (c->flags & MG_F_IS_WEBSOCKET)
        {
            logger_service_unregister_sink((sink_handle_t)c->user_data);
            LOG_I(TAG, "Websocket Disconnect");

            // Remove the handle
            free(c->user_data);
            c->user_data = NULL;
        }
        break;
    }
    
}

static void webserver_thread(void* pvParameters)
{    
    
    LOG_I(TAG, "Webserver polling loop started");

    for (;;)
    {
        // Webserver event loop
        mg_mgr_poll(&manager, 100);

        // Check to see if we need to stop
        uint32_t stop = ulTaskNotifyTake(pdTRUE, 0);
        if( stop == 1 )
        {
            break;
        }
    }
    LOG_I(TAG, "Webserver polling loop stopped");

    // Notify the webserver task we have stopped
    xTaskNotifyGive(webserverTaskHandle);

    // Delete the task before returning
    vTaskDelete(NULL);
}

static void start_webserver()
{
    LOG_I(TAG, "Starting webserver");

    // Initialise the webserver manager
    mg_mgr_init(&manager, NULL);

    // Set up webserver connection
    LOG_I(TAG, "Starting webserver on port: '%d'", 80);
    webserverConnection = mg_bind(&manager, "80", webserver_ev_handler, NULL);
    mg_set_protocol_http_websocket(webserverConnection);
    
    // Set URI handlers
    LOG_I(TAG, "Registering URI handlers");
    const char* rootUri = "/api";
    lift_controller_register_uri_handlers(webserverConnection, rootUri);
    settings_controller_register_uri_handlers(webserverConnection, rootUri);
    upload_controller_register_uri_handlers(webserverConnection, rootUri);

    // Start the webserver thread
    BaseType_t taskCreateResult = xTaskCreatePinnedToCore(
        webserver_thread,
        WEBSERVER_THREAD_TAG,
        WEBSERVER_THREAD_STACK_SIZE_KB * STACK_KB,
        NULL,
        tskIDLE_PRIORITY+12,
        &webserverThreadHandle,
        APP_CPU_NUM);

    if(taskCreateResult == pdPASS)
    {
        LOG_I(TAG, "Webserver started");
    }
    else
    {
        LOG_E(TAG, "Webserver could not be started");
        
        // Free webserver resources
        mg_mgr_free(&manager);
    }
}

static void stop_webserver()
{
    LOG_I(TAG, "Stopping webserver");

    // Notify webserver thread to stop
    xTaskNotifyGive(webserverThreadHandle);

    // Wait for websever thread to be stopped
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // Free webserver resources
    mg_mgr_free(&manager);

    LOG_I(TAG, "Webserver stopped");
}

static void wifi_event_sta_disconnected(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    xSemaphoreTake(webserverControlSemaphore, portMAX_DELAY);

    // Stop the web server
    if (webserverThreadHandle != NULL)
    {
        stop_webserver();
        webserverThreadHandle = NULL;
    }

    xSemaphoreGive(webserverControlSemaphore);
}

static void ip_event_sta_got_ip(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    xSemaphoreTake(webserverControlSemaphore, portMAX_DELAY);

    // Start the web server
    if (webserverThreadHandle == NULL)
    {
        start_webserver();
    }

    xSemaphoreGive(webserverControlSemaphore);
}

static void webserver_init()
{
    // Register to status service
    serviceHandle = status_service_add_service("webserver");

    // Get our own task handle
    webserverTaskHandle = xTaskGetCurrentTaskHandle();

    // Create the semaphore for webserver start/stop control
    webserverControlSemaphore = xSemaphoreCreateMutex();

    // Register events
    esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, wifi_event_sta_disconnected, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, ip_event_sta_got_ip, NULL);
}

void webserver_task_main(void* pvParameters)
{
    LOG_I(TAG, "Starting task");
    LOG_V(TAG, "Free stack space: %i", uxTaskGetStackHighWaterMark(NULL));
    webserver_init();

    // Signal status service we are ready
    status_service_set_service_state(serviceHandle, STATUS_SERVICE_STATE_ACTIVE);

    LOG_D(TAG, "Starting task loop");
    LOG_V(TAG, "Free stack space: %i", uxTaskGetStackHighWaterMark(NULL));
    for (;;)
    {
        vTaskSuspend(NULL);
    }

    vTaskDelete(NULL);
}
