#include "status_service.h"

#include <stddef.h>
#include <stdlib.h>

typedef struct service_info_list_item_t
{
    service_info_t* item;
    struct service_info_list_item_t* next;
} service_info_list_item_t;

typedef struct service_changed_registration_list_item_t
{
    status_service_registration_handle_t handle;
    on_service_status_changed_t item;
    struct service_changed_registration_list_item_t* next;
} service_changed_registration_list_item_t;

static service_handle_t handleCounter = 0;
static service_info_list_item_t* registeredServices = NULL;
static service_info_list_item_t* lastService = NULL;

static status_service_registration_handle_t registrationCounter = 0;
static service_changed_registration_list_item_t* registrations = NULL;
static service_changed_registration_list_item_t* lastRegistration = NULL;

static service_info_t* find_service(service_handle_t handle)
{
    service_info_list_item_t* service = registeredServices;
    while(service != NULL && service->next != NULL)
    {
        if(service->item->handle == handle)
        {
            return service->item;
        }
        service = service->next;
    }

    return NULL;
}

service_handle_t status_service_add_service(const char* name)
{
    if(lastService == NULL)
    {
        registeredServices = (service_info_list_item_t*) malloc(sizeof(service_info_list_item_t));
        lastService = registeredServices;
    }

    service_handle_t handle = handleCounter++;
    lastService->item = (service_info_t*) malloc(sizeof(service_info_t));
    lastService->item->name = name;
    lastService->item->handle = handle;
    lastService->item->state = STATUS_SERVICE_STATE_INACTIVE;

    lastService->next = (service_info_list_item_t*) malloc(sizeof(service_info_list_item_t));
    lastService->next->item = NULL;
    lastService->next->next = NULL;
    lastService = lastService->next;

    return handle;
}

const service_info_t* status_service_get_service_into(service_handle_t handle)
{
    return find_service(handle);
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

void status_service_remove_service(service_handle_t handle)
{

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