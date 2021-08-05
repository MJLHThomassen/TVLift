#ifndef SETTINGS_SERVICE_H
#define SETTINGS_SERVICE_H

#include <stdint.h>

typedef uint32_t settings_service_registration_handle_t;

typedef enum 
{
    SETTINGS_SERVICE_OK = 0,
    SETTINGS_SERVICE_FAIL = -1,

    SETTINGS_SERVICE_INVALID_SETTINGS = 1,

    SETTINGS_SERVICE_LOADED_DEFAULT

} settings_service_err_t;

typedef enum 
{
    SETTINGS_CHANGE_OK = 0,
    SETTINGS_CHANGE_FAIL = -1

} settings_change_err_t;

typedef struct settings_s
{
    uint32_t    version;

    uint32_t    lift_min_speed;
    uint32_t    lift_max_speed;
    uint32_t    lift_default_speed;
} settings_t;

typedef settings_change_err_t (*on_settings_changed_t)(const settings_t* new, const settings_t* old);

settings_service_err_t settings_service_load(const settings_t ** settings);
settings_service_err_t settings_service_save(const settings_t * settings);

settings_service_registration_handle_t  settings_service_register(on_settings_changed_t callback);
void                                    settings_service_unregister(settings_service_registration_handle_t handle);

#endif // SETTINGS_SERVICE_H