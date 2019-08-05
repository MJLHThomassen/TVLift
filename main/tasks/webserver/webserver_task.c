#include "webserver_task.h"

#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_event.h>

// #include <esp_http_server.h>
#include <mongoose.h>

#include "controllers/lift_controller.h"

#define WEBROOT "/spiffs/www"

static struct mg_mgr manager;
static struct mg_connection* webserverConnection;
static struct mg_serve_http_opts http_server_opts;

static const char* TAG = WEBSERVER_TASK_TAG;
//static httpd_handle_t webserver = NULL;

// static esp_err_t root_get_handler(httpd_req_t *req)
// {
//     ESP_LOGI(TAG, "Received root request: %s", req->uri);

//     // Get filename from uri
//     char filename[2048 + 12] = WEBROOT;
//     if(strcmp(req->uri, "/") == 0 || strcmp(req->uri, "/index.html") == 0)
//     {
//         // If root or index.html is requested, serve index.html
//         strcat(filename, "/index.html");
//     }
//     else
//     {
//         // If other file is requisted, serve that one
//         strcat(filename, req->uri);
//     }

//     // Open file for reading
//     ESP_LOGD(TAG, "Reading file: %s", filename);
//     FILE* fp = fopen(filename, "r");
//     if (fp == NULL) 
//     {
//         // If the file pointer is NULL, the file can not be read / does not exist so return a 404
//         ESP_LOGE(TAG, "Failed to open file for reading");
//         httpd_resp_send_404(req);
//         return ESP_OK;
//     }

//     // Get the length of the file
//     fseek(fp, 0L, SEEK_END);
//     int fsize = ftell(fp);
//     rewind(fp);

//     // Allocate enough memory to read the entire file
//     char* fcontent = (char*) malloc(sizeof(char) * fsize);

//     // Read the entire file
//     fread(fcontent, 1, fsize, fp);
//     fclose(fp);

//     // Send the file as response
//     httpd_resp_send(req, fcontent, fsize);

//     // Free file memory
//     free(fcontent);

//     return ESP_OK;
// }

// static httpd_uri_t root = {
//     .uri = "/?*",
//     .method = HTTP_GET,
//     .handler = root_get_handler,
//     .user_ctx = NULL};

// static httpd_handle_t start_webserver(void)
// {
//     httpd_handle_t server = NULL;
//     httpd_config_t config = HTTPD_DEFAULT_CONFIG();
//     config.uri_match_fn = httpd_uri_match_wildcard;

//     // Start the httpd server
//     ESP_LOGI(TAG, "Starting webserver on port: '%d'", config.server_port);
//     if (httpd_start(&server, &config) == ESP_OK)
//     {
//         // Set URI handlers
//         ESP_LOGI(TAG, "Registering URI handlers"); 
//         lift_controller_register_uri_handlers(server);
//         httpd_register_uri_handler(server, &root);
//         return server;
//     }

//     ESP_LOGI(TAG, "Error starting server!");
//     return NULL;
// }

// static void stop_webserver(httpd_handle_t server)
// {
//     // Stop the httpd server
//     ESP_LOGI(TAG, "Stopping webserver");
//     httpd_stop(server);
// }

static void webserver_ev_handler(struct mg_connection *c, int ev, void *p)
{
    switch (ev)
    {
    case MG_EV_HTTP_REQUEST:
        mg_serve_http(c, (struct http_message *) p, http_server_opts);
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
    http_server_opts.index_files = "index.html";

    esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, wifi_event_sta_disconnected, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, ip_event_sta_got_ip, NULL);

    mg_mgr_init(&manager, NULL);

    webserverConnection = mg_bind(&manager, "80", webserver_ev_handler);
    mg_set_protocol_http_websocket(webserverConnection);
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
        mg_mgr_poll(&manager, 1000);
    }

    mg_mgr_free(&manager);
}
