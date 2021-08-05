#ifndef SETTINGS_CONTROLLER_H
#define SETTINGS_CONTROLLER_H

#include <mongoose.h>

void settings_controller_register_uri_handlers(struct mg_connection* nc, const char* rootUri);

#endif // SETTINGS_CONTROLLER_H