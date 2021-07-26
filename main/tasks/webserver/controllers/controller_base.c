#include "controller_base.h"

#include <string.h>

#include <logger.h>

static char TAG[] = "Controller Base";

static const char* http_request_method_to_str(const http_request_method_t method)
{
    switch(method)
    {
        case HTTP_REQUEST_METHOD_GET:
            return "GET";
        case HTTP_REQUEST_METHOD_HEAD:
            return "HEAD";
        case HTTP_REQUEST_METHOD_POST:
            return "POST";
        case HTTP_REQUEST_METHOD_PUT:
            return "PUT";
        case HTTP_REQUEST_METHOD_DELETE:
            return "DELETE";
        case HTTP_REQUEST_METHOD_CONNECT:
            return "CONNECT";
        case HTTP_REQUEST_METHOD_OPTIONS:
            return "OPTIONS";
        case HTTP_REQUEST_METHOD_TRACE:
            return "TRACE";
        case HTTP_REQUEST_METHOD_PATCH:
            return "PATCH";
        case HTTP_REQUEST_METHOD_UNKNOWN:
        default:
            return "Unknown";
    }
}

static http_request_method_t method_str_to_http_request_method(const struct mg_str* method)
{
    if(mg_vcmp(method, "GET") == 0)
    {
        return HTTP_REQUEST_METHOD_GET;
    }
    else if(mg_vcmp(method, "HEAD") == 0)
    {
        return HTTP_REQUEST_METHOD_HEAD;
    }
    else if(mg_vcmp(method, "POST") == 0)
    {
        return HTTP_REQUEST_METHOD_POST;
    }
    else if(mg_vcmp(method, "PUT") == 0)
    {
        return HTTP_REQUEST_METHOD_PUT;
    }
    else if(mg_vcmp(method, "DELETE") == 0)
    {
        return HTTP_REQUEST_METHOD_DELETE;
    }
    else if(mg_vcmp(method, "CONNECT") == 0)
    {
        return HTTP_REQUEST_METHOD_CONNECT;
    }
    else if(mg_vcmp(method, "OPTIONS") == 0)
    {
        return HTTP_REQUEST_METHOD_OPTIONS;
    }
    else if(mg_vcmp(method, "TRACE") == 0)
    {
        return HTTP_REQUEST_METHOD_TRACE;
    }
    else if(mg_vcmp(method, "PATCH") == 0)
    {
        return HTTP_REQUEST_METHOD_PATCH;
    }
    else
    {
        return HTTP_REQUEST_METHOD_UNKNOWN;
    }
}

static void http_request_handler(struct mg_connection* nc, int ev, void* ev_data, void* user_data)
{
    uri_handler_info_t* uriHandlerInfo = (uri_handler_info_t*) user_data;
    struct http_message* message = (struct http_message*) ev_data;

    switch (ev) 
    {
    case MG_EV_HTTP_REQUEST:

        LOG_D(
            TAG,
            "HTTP Request recieved: %.*s: %.*s",
            message->method.len, message->method.p,
            message->uri.len, message->uri.p);
        
        http_request_method_t method = method_str_to_http_request_method(&message->method);

        for(int i = 0; i < HTTP_REQUEST_METHOD_MAX; ++i)
        {
            const method_handler_info_t* methodHandlerInfo = &(uriHandlerInfo->methodHandlers[i]);
            if(methodHandlerInfo->method == method && methodHandlerInfo->handler != NULL)
            {
                // Call the handler
                LOG_D(TAG, "HTTP Request handler found, calling handler");
                methodHandlerInfo->handler(nc, message, methodHandlerInfo->user_data);
                return;
            }
        }

        // Method has no endpoint
        LOG_D(TAG, "HTTP Request handler not found");
        mg_http_send_error(nc, 404, NULL);
        return;

    default:
        // Unsupported
        nc->flags |= MG_F_CLOSE_IMMEDIATELY;
        return;
    }
}

static void http_multipart_request_handler(struct mg_connection* nc, int ev, void* ev_data, void* user_data)
{
    multipart_request_uri_handler_info_t* handlerInfo = (multipart_request_uri_handler_info_t*) user_data;
    struct http_message* message = (struct http_message*) ev_data;

    switch (ev) 
    {
    case MG_EV_HTTP_MULTIPART_REQUEST:
    {
        LOG_D(
            TAG,
            "HTTP Multipart Request recieved: %.*s: %.*s",
            message->method.len, message->method.p,
            message->uri.len, message->uri.p);

        if(mg_vcmp(&message->method, handlerInfo->method) == 0)
        {
            LOG_D(TAG, "HTTP Request handler found, calling handler");

            // Need to set user_data to handlerInfo so it is availabe to subsequent calls
            nc->user_data = handlerInfo;

            // Call the handler
            handlerInfo->handler(nc, message, NULL, MULTIPART_REQUEST_MESSAGE_TYPE_BEGIN, handlerInfo->user_data);
        }
        else
        {
            // Method has no endpoint
            LOG_D(TAG, "HTTP Request handler not found");
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
            LOG_D(TAG, "HTTP Multipart part '%s' begin recieved: %s: %s", part->file_name, message->method.p, message->uri.p);
            type = MULTIPART_REQUEST_MESSAGE_TYPE_PART_BEGIN;
            break;
        case MG_EV_HTTP_PART_DATA:
            LOG_D(TAG, "HTTP Multipart part '%s' data recieved: %s: %s", part->file_name, message->method.p, message->uri.p);
            type = MULTIPART_REQUEST_MESSAGE_TYPE_PART_DATA;
            break;
        case MG_EV_HTTP_PART_END:
            LOG_D(TAG, "HTTP Multipart part '%s' end recieved: %s: %s", part->file_name, message->method.p, message->uri.p);
            type = MULTIPART_REQUEST_MESSAGE_TYPE_PART_END;
            break;
        case MG_EV_HTTP_MULTIPART_REQUEST_END:
        default:
            LOG_D(TAG, "HTTP Multipart end recieved: %s: %s", message->method.p, message->uri.p);
            type = MULTIPART_REQUEST_MESSAGE_TYPE_END;
            break;
        }
          
        // Call the handler
        handlerInfo->handler(nc, NULL, part, type, handlerInfo->user_data);
        break;
    }
    }
}

void register_uri_handler(struct mg_connection* nc, const char* rootUri, uri_handler_info_t* uriHandlerInfo)
{
    char* uri = (char*)calloc(strlen(rootUri) + strlen(uriHandlerInfo->uri) + 1, sizeof(char));
    strcat(strcat(uri, rootUri), uriHandlerInfo->uri);
    
    // Check which methods are being registered and if this is allowed
    bool registeredMethods[HTTP_REQUEST_METHOD_MAX] = {false};
    for(int i = 0; i < HTTP_REQUEST_METHOD_MAX; ++i)
    {
        if(uriHandlerInfo->methodHandlers[i].handler != NULL)
        {
            if(registeredMethods[uriHandlerInfo->methodHandlers[i].method])
            {
                LOG_E(
                    TAG,
                    "Registering more then 1 handler for the same method:uri pair is not allowed. Uri: %s Method: %s",
                    uri,
                    http_request_method_to_str(uriHandlerInfo->methodHandlers[i].method));

                free(uri);
                return;
            }
            else
            {             
                LOG_D(TAG, "Registering handler for: %s:%s", http_request_method_to_str(uriHandlerInfo->methodHandlers[i].method), uri);
                registeredMethods[uriHandlerInfo->methodHandlers[i].method] = true;
            }       
        }
    }

    mg_register_http_endpoint(nc, uri, http_request_handler, uriHandlerInfo);
    free(uri);
}

void register_multipart_request_uri_handler(struct mg_connection* nc, const char* rootUri, multipart_request_uri_handler_info_t* uriHandlerInfo)
{
    char* uri = (char*)calloc(strlen(rootUri) + strlen(uriHandlerInfo->uri) + 1, sizeof(char));
    strcat(strcat(uri, rootUri), uriHandlerInfo->uri);

    LOG_D(TAG, "Registered multipart handler for: %s:%s", uriHandlerInfo->method, uri);

    mg_register_http_endpoint(nc, uri, http_multipart_request_handler, uriHandlerInfo);
    free(uri);
}