#include "status_service.h"

#include <stddef.h>
#include <stdlib.h>

static service_handle_t handleCounter = 0;
static map _services = NULL;

static status_service_registration_handle_t registrationCounter = 0;
static map _registrations = NULL;

service_handle_t status_service_add_service(const char* name)
{
    if(_services == NULL)
    {
        map_new(&_services);
    }

    service_handle_t handle = handleCounter++;

    service_info_t* item = (service_info_t*) malloc(sizeof(service_info_t));
    item->name = name;
    item->handle = handle;
    item->state = STATUS_SERVICE_STATE_INACTIVE;

    map_add(_services, (void*)handle, item);

    return handle;
}

void status_service_remove_service(service_handle_t handle)
{

}

map status_services_get_services()
{
    return _services;
}

void status_service_set_service_state(service_handle_t handle, status_service_state_t state)
{
    service_info_t* serviceInfo;
    map_get(_services, (void*)handle, &serviceInfo);
    serviceInfo->state = state;

    map_iter iter;
    map_iter_new(_registrations, &iter);

    status_service_registration_handle_t registrationHandle = NULL;
    on_service_status_changed_t callback = NULL;
    do
    {
        map_iter_next(iter, (void**)&registrationHandle, (void**)&callback);
        
        if(callback != NULL)
        {
            callback(serviceInfo);
        }
    } while(registrationHandle != NULL);
    
    map_iter_delete(iter);
}

status_service_registration_handle_t status_service_register(on_service_status_changed_t callback)
{
    if(_registrations == NULL)
    {
        map_new(&_registrations);
    }

    status_service_registration_handle_t handle = registrationCounter++;

    map_add(_registrations, (void*)handle, callback);

    return handle;
}

void status_service_unregister(status_service_registration_handle_t handle)
{
    map_remove(_registrations, (void*)handle);
}