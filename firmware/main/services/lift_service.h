#ifndef LIFT_SRVICE_H
#define LIFT_SERVICE_H

#include <lift/lift.h>

typedef enum 
{
    LIFT_SERVICE_OK = 0,
    LIFT_SERVICE_FAIL = -1,

} lift_service_err_t;

lift_service_err_t lift_service_init(void);
void lift_service_free(void);

lift_device_handle_t lift_service_get_lift_device_handle();

#endif // LIFT_SERVICE_H