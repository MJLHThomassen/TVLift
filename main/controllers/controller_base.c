#include "controller_base.h"

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

    default:
        // Unsupported
        nc->flags |= MG_F_CLOSE_IMMEDIATELY;
        break;
    }
}

void register_uri_handler(struct mg_connection* nc, uri_handler_info_t* uriHandlerInfo)
{
    mg_register_http_endpoint(nc, uriHandlerInfo->uri, http_request_handler, uriHandlerInfo);
}