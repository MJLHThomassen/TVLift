#ifndef CONTROLLER_BASE_H
#define CONTROLLER_BASE_H

#include <mongoose.h>

typedef enum http_request_method_e
{
    HTTP_REQUEST_METHOD_UNKNOWN = -1,
    
    HTTP_REQUEST_METHOD_GET = 0,
    HTTP_REQUEST_METHOD_HEAD,
    HTTP_REQUEST_METHOD_POST,
    HTTP_REQUEST_METHOD_PUT,
    HTTP_REQUEST_METHOD_DELETE,
    HTTP_REQUEST_METHOD_CONNECT,
    HTTP_REQUEST_METHOD_OPTIONS,
    HTTP_REQUEST_METHOD_TRACE,
    HTTP_REQUEST_METHOD_PATCH,

    HTTP_REQUEST_METHOD_MAX
} http_request_method_t;

typedef enum multipart_request_message_type_e
{
    MULTIPART_REQUEST_MESSAGE_TYPE_BEGIN,
    MULTIPART_REQUEST_MESSAGE_TYPE_PART_BEGIN,
    MULTIPART_REQUEST_MESSAGE_TYPE_PART_DATA,
    MULTIPART_REQUEST_MESSAGE_TYPE_PART_END,
    MULTIPART_REQUEST_MESSAGE_TYPE_END
} multipart_request_message_type_t;

typedef void (*request_handler_t)(
    struct mg_connection* nc,
    struct http_message* const message,
    void* userData);

typedef void (*multipart_request_handler_t)(
    struct mg_connection* const nc,
    struct http_message* const message,
    struct mg_http_multipart_part* const part,
    const multipart_request_message_type_t type,
    void* userData);

typedef struct method_handler_info_s
{
    http_request_method_t method;
    request_handler_t handler;

    void* user_data;
} method_handler_info_t;

typedef struct uri_handler_info_s
{
    const char* uri;
    method_handler_info_t methodHandlers[HTTP_REQUEST_METHOD_MAX];
} uri_handler_info_t;

typedef struct multipart_request_uri_handler_info_t
{
    const char* uri;
    multipart_request_handler_t handler;
    const char* method;

    void* user_data;
} multipart_request_uri_handler_info_t;

void register_uri_handler(struct mg_connection* nc, const char* rootUri, uri_handler_info_t* uriHandlerInfo);
void register_multipart_request_uri_handler(struct mg_connection* nc, const char* rootUri, multipart_request_uri_handler_info_t* uriHandlerInfo);

#endif // CONTROLLER_BASE_H