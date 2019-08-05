#ifndef LIFT_CONTROLLER_H
#define LIFT_CONTROLLER_H

#include <esp_http_server.h>

void lift_controller_register_uri_handlers(httpd_handle_t server);

#endif // LIFT_CONTROLLER_H