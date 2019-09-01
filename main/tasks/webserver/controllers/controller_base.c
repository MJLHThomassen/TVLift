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
    multipart_request_uri_handler_info_t* handlerInfo = (multipart_request_uri_handler_info_t*) user_data;

    switch (ev) 
    {
    case MG_EV_HTTP_MULTIPART_REQUEST:
    {
        struct http_message* message = (struct http_message*) ev_data;
        if(mg_vcmp(&message->method, handlerInfo->method) == 0)
        {
            // Need to set user_data to handlerInfo so it is availabe to subsequent calls
            nc->user_data = handlerInfo;

            // Call the handler
            handlerInfo->handler(nc, NULL, MULTIPART_REQUEST_MESSAGE_TYPE_BEGIN, handlerInfo->user_data);
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
    case MG_EV_HTTP_MULTIPART_REQUEST_END:
    {
        struct mg_http_multipart_part* part = (struct mg_http_multipart_part*) ev_data;

        multipart_request_message_type_t type;
        switch (ev)
        {
        case MG_EV_HTTP_PART_BEGIN:
            type = MULTIPART_REQUEST_MESSAGE_TYPE_PART_BEGIN;
            break;
        case MG_EV_HTTP_PART_DATA:
            type = MULTIPART_REQUEST_MESSAGE_TYPE_PART_DATA;
            break;
        case MG_EV_HTTP_PART_END:
            type = MULTIPART_REQUEST_MESSAGE_TYPE_PART_END;
            break;
        case MG_EV_HTTP_MULTIPART_REQUEST_END:
        default:
            type = MULTIPART_REQUEST_MESSAGE_TYPE_END;
            break;
        }
          
        // Call the handler
        handlerInfo->handler(nc, part, type, handlerInfo->user_data);
        break;
    }
    }
}

void register_uri_handler(struct mg_connection* nc, uri_handler_info_t* uriHandlerInfo)
{
    mg_register_http_endpoint(nc, uriHandlerInfo->uri, http_request_handler, uriHandlerInfo);
}

void register_multipart_request_uri_handler(struct mg_connection* nc, multipart_request_uri_handler_info_t* uriHandlerInfo)
{
    mg_register_http_endpoint(nc, uriHandlerInfo->uri, http_multipart_request_handler, uriHandlerInfo);
}