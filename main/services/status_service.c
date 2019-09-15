#include "status_service.h"

#include <stddef.h>
#include <stdlib.h>

static service_handle_t handleCounter = 0;
static slist_service_info_t _services = NULL;

typedef struct service_changed_registration_list_item_t
{
    status_service_registration_handle_t handle;
    on_service_status_changed_t item;
    struct service_changed_registration_list_item_t* next;
} service_changed_registration_list_item_t;

static status_service_registration_handle_t registrationCounter = 0;
static service_changed_registration_list_item_t* registrations = NULL;
static service_changed_registration_list_item_t* lastRegistration = NULL;

service_handle_t status_service_add_service(const char* name)
{
    if(_services == NULL)
    {
        slist_service_info_t_new(&_services);
    }

    service_handle_t handle = handleCounter++;

    service_info_t* item = (service_info_t*) malloc(sizeof(service_info_t));
    item->name = name;
    item->handle = handle;
    item->state = STATUS_SERVICE_STATE_INACTIVE;

    slist_service_info_t_add(_services, item);

    return handle;
}

void status_service_remove_service(service_handle_t handle)
{

}

const service_info_t* status_service_get_service_info(service_handle_t handle)
{
    service_info_t* info;
    return slist_service_info_t_find(_services, , &info);

    return info;
}

const slist_service_info_t status_services_get_services(slist_service_info_t* services)
{
    slist_service_info_t_new(services);

    slist_service_info_t_add(services, registeredServices->item);

    return services;
}

void status_service_set_service_state(service_handle_t handle, status_service_state_t state)
{
    service_info_t* serviceInfo = find_service(handle);
    serviceInfo->state = state;

    service_changed_registration_list_item_t* registration = registrations;
    while(registration != NULL && registration->next != NULL)
    {
        registration->item(serviceInfo);
        registration = registration->next;
    }
}

status_service_registration_handle_t status_service_register(on_service_status_changed_t callback)
{
    if(lastRegistration == NULL)
    {
        registrations = (service_changed_registration_list_item_t*) malloc(sizeof(service_changed_registration_list_item_t));
        lastRegistration = registrations;
    }

    status_service_registration_handle_t handle = registrationCounter++;
    lastRegistration->handle = handle;
    lastRegistration->item = callback;

    lastRegistration->next = (service_changed_registration_list_item_t*) malloc(sizeof(service_changed_registration_list_item_t));
    lastRegistration->next->item = NULL;
    lastRegistration->next->next = NULL;
    lastRegistration = lastRegistration->next;

    return handle;
}

void status_service_unregister(status_service_registration_handle_t handle)
{
    service_changed_registration_list_item_t* registration = registrations;
    service_changed_registration_list_item_t* previous = NULL;
    while(registration != NULL && registration->next != NULL)
    {
        if(registration->handle == handle)
        {
            if(previous != NULL)
            {
                previous->next = registration->next;
            }

            free(registration);
            
            return;
        }
    }
}