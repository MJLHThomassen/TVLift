#ifndef CONTROLLER_BASE_H
#define CONTROLLER_BASE_H

#include <mongoose.h>

#define HTTP_GET    "GET"
#define HTTP_POST   "POST"
#define HTTP_PUT    "PUT"
#define HTTP_DELETE "DELETE"

typedef enum
{
    MULTIPART_REQUEST_MESSAGE_TYPE_BEGIN,
    MULTIPART_REQUEST_MESSAGE_TYPE_PART_BEGIN,
    MULTIPART_REQUEST_MESSAGE_TYPE_PART_DATA,
    MULTIPART_REQUEST_MESSAGE_TYPE_PART_END,
    MULTIPART_REQUEST_MESSAGE_TYPE_END
} multipart_request_message_type_t;

typedef void (*uri_handler_t)(
    struct mg_connection* nc,
    struct http_message* const message);
typedef void (*multipart_request_uri_handler_t)(
    struct mg_connection* const nc,
    struct mg_http_multipart_part* const part,
    const multipart_request_message_type_t type,
    void* userData);

typedef struct uri_handler_info_t
{
    const char* uri;
    uri_handler_t handler;
    const char* method;

    void* user_data;
} uri_handler_info_t;

typedef struct multipart_request_uri_handler_info_t
{
    const char* uri;
    multipart_request_uri_handler_t handler;
    const char* method;

    void* user_data;
} multipart_request_uri_handler_info_t;

void register_uri_handler(struct mg_connection* nc, uri_handler_info_t* uriHandlerInfo);
void register_multipart_request_uri_handler(struct mg_connection* nc, multipart_request_uri_handler_info_t* uriHandlerInfo);

#endif // CONTROLLER_BASE_H