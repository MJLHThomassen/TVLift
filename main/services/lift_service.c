#include "lift_service.h"

#include <pins.h>
#include <services/settings_service.h>

static lift_device_handle_t _liftHandle = NULL;

lift_service_err_t lift_service_init(void)
{
    const settings_t* settings;
    
    settings_service_load(&settings);

    if(lift_add_device(
        PIN_NUM_ENA,
        PIN_NUM_DIR,
        PIN_NUM_PUL,
        PIN_NUM_END_DOWN,
        PIN_NUM_END_UP,
        settings->lift_default_speed,
        settings->lift_min_speed,
        settings->lift_max_speed,
        &_liftHandle) != LIFT_OK)
    {
        lift_remove_device(_liftHandle);
        _liftHandle = NULL;
        return LIFT_SERVICE_FAIL;
    }

    return LIFT_SERVICE_OK;
}

void lift_service_free(void)
{
    lift_remove_device(_liftHandle);
}

lift_device_handle_t lift_service_get_lift_device_handle()
{
    return _liftHandle;
}