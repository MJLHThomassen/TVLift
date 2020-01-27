#ifndef LOGGER_SERVICE_H
#define LOGGER_SERVICE_H

#include <stddef.h>
#include <stdint.h>

typedef void* sink_handle_t;

typedef void (*logger_sink_t)(const char* message, const size_t len, void* user_data);

sink_handle_t logger_service_register_sink(logger_sink_t callback, void* user_data);
void          logger_service_unregister_sink(sink_handle_t handle);

void logger_service_init(void);

#endif // LOGGER_SERVICE_H