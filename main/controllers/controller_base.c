#include "controller_base.h"

#include <esp_log.h>

static void http_request_handler(struct mg_connection* nc, int ev, void* ev_data, void* user_data)
{
    uri_handler_info_t* handlerInfo = (uri_handler_info_t*) user_data;
    struct http_message* message = (struct http_message*) ev_data;

    switch (ev) 
    {
    case MG_EV_HTTP_REQUEST:

        if(mg_vcmp(&message->method, handlerInfo->method) == 0)
        {
            // Call the handler
            handlerInfo->handler(nc, message);
        }
        else
        {
            // Method has no endpoint
            mg_http_send_error(nc, 404, NULL);
        }
        break;
      break;

    default:
        // Unsupported
        nc->flags |= MG_F_CLOSE_IMMEDIATELY;
        break;
    }
}

static void http_multipart_request_handler(struct mg_connection* nc, int ev, void* ev_data, void* user_data)
{
    ESP_LOGI(__FILE__, "Received multipart request: %i", ev);
    multipart_request_uri_handler_info_t* handlerInfo = (multipart_request_uri_handler_info_t*) user_data;
    ESP_LOGI(__FILE__,"1");

    switch (ev) 
    {
    case MG_EV_HTTP_MULTIPART_REQUEST:
    {
        ESP_LOGI(__FILE__,"2");
        struct http_message* message = (struct http_message*) ev_data;
        if(mg_vcmp(&message->method, handlerInfo->method) == 0)
        {
            // OK
            // Need to set user_data to handlerInfo so it is availabe to subsequent calls
            nc->user_data = handlerInfo;
        }
        else
        {
            // Method has no endpoint
            mg_http_send_error(nc, 404, NULL);
        }
        break;
    }

    case MG_EV_HTTP_PART_BEGIN:
    case MG_EV_HTTP_PART_DATA:
    case MG_EV_HTTP_PART_END:
    {
        ESP_LOGI(__FILE__,"3");
        struct mg_http_multipart_part* part = (struct mg_http_multipart_part*) ev_data;
          
        // Call the handler
        ESP_LOGI(__FILE__,"4, %s %i", handlerInfo->uri, (int)handlerInfo->handler);
        handlerInfo->handler(nc, part);

        ESP_LOGI(__FILE__,"5");
        break;
    }

    case MG_EV_HTTP_MULTIPART_REQUEST_END:
        ESP_LOGI(__FILE__,"6");
        mg_printf(nc,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "Connection: close\r\n\r\n"
                "Ok.\r\n");
        nc->flags |= MG_F_SEND_AND_CLOSE;
        break;
    }

    ESP_LOGI(__FILE__,"7");
}

void register_uri_handler(struct mg_connection* nc, uri_handler_info_t* uriHandlerInfo)
{
    mg_register_http_endpoint(nc, uriHandlerInfo->uri, http_request_handler, uriHandlerInfo);
}

void register_multipart_request_uri_handler(struct mg_connection* nc, multipart_request_uri_handler_info_t* uriHandlerInfo)
{
    mg_register_http_endpoint(nc, uriHandlerInfo->uri, http_multipart_request_handler, uriHandlerInfo);
}