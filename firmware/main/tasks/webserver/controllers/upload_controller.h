#ifndef UPLOAD_CONTROLLER_H
#define UPLOAD_CONTROLLER_H

#include <mongoose.h>

void upload_controller_register_uri_handlers(struct mg_connection* nc, const char* rootUri);

#endif // UPLOAD_CONTROLLER_H