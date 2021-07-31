#ifndef SETTINGS_SERVICE_H
#define SETTINGS_SERVICE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum 
{
    SETTINGS_SERVICE_OK = 0,
    SETTINGS_SERVICE_FAIL = -1,

} settings_service_err_t;

typedef struct settings_s
{
    uint32_t    version;

    uint32_t    lift_min_speed;
    uint32_t    lift_max_speed;
    uint32_t    lift_default_speed;
} settings_t;

settings_service_err_t settings_service_load(const settings_t ** settings);
settings_service_err_t settings_service_save(const settings_t * settings);

#endif // SETTINGS_SERVICE_H