#ifndef CONTROLLER_BASE_H
#define CONTROLLER_BASE_H

#include <mongoose.h>

#define HTTP_GET    "GET"
#define HTTP_POST   "POST"
#define HTTP_PUT    "PUT"
#define HTTP_DELETE "DELETE"

typedef void (*uri_handler_t)(struct mg_connection* nc, struct http_message* message);

typedef struct uri_handler_info_t
{
    const char* uri;
    uri_handler_t handler;
    const char* method;

    void* user_data;
} uri_handler_info_t;

void register_uri_handler(struct mg_connection* nc, uri_handler_info_t* uriHandlerInfo);

#endif // CONTROLLER_BASE_H