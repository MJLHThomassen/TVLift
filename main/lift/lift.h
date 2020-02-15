#ifndef LIFT_H
#define LIFT_H

#include <driver/gpio.h>

typedef enum 
{
    LIFT_OK = 0,
    LIFT_FAIL = -1
} lift_err_t;

typedef struct lift_device_s* lift_device_handle_t;

lift_err_t lift_add_device(
    int gpioEna,
    int gpioDir,
    int gpioPul,
    int gpioEndstopDown,
    int gpioEndstopUp,
    lift_device_handle_t* handle);
lift_err_t lift_remove_device(lift_device_handle_t handle);

lift_err_t lift_up(const lift_device_handle_t handle);
lift_err_t lift_down(const lift_device_handle_t handle);
lift_err_t lift_stop(const lift_device_handle_t handle);
lift_err_t lift_disable(const lift_device_handle_t handle);

uint32_t lift_get_speed(lift_device_handle_t handle);
lift_err_t lift_set_speed(lift_device_handle_t handle, uint32_t speed);

#endif // LIFT_H