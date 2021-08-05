#ifndef STATUS_SERVICE_H
#define STATUS_SERVICE_H

#include <stdint.h>

#include <map.h>

typedef uint32_t service_handle_t;
typedef uint32_t status_service_registration_handle_t;

typedef enum status_service_state_e
{
    STATUS_SERVICE_STATE_INACTIVE,
    STATUS_SERVICE_STATE_ACTIVE,
    STATUS_SERVICE_STATE_ERROR
} status_service_state_t;

typedef struct service_info_s
{
    service_handle_t       handle;
    const char*            name;
    status_service_state_t state;
} service_info_t;

typedef void (*on_service_status_changed_t)(const service_info_t* info);

service_handle_t status_service_add_service(const char* name);
void             status_service_remove_service(service_handle_t handle);

const service_info_t* status_service_get_service_info(service_handle_t handle);
map                   status_services_get_services();
void                  status_service_set_service_state(service_handle_t handle, status_service_state_t state);

status_service_registration_handle_t status_service_register(on_service_status_changed_t callback);
void                                 status_service_unregister(status_service_registration_handle_t handle);

#endif // STATUS_SERVICE_H