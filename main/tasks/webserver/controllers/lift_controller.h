#ifndef LIFT_CONTROLLER_H
#define LIFT_CONTROLLER_H

#include <mongoose.h>

void lift_controller_register_uri_handlers(struct mg_connection* nc, const char* rootUri);

#endif // LIFT_CONTROLLER_H