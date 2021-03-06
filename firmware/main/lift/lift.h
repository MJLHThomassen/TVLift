#ifndef LIFT_H
#define LIFT_H

#include <driver/gpio.h>

typedef enum 
{
    LIFT_OK = 0,
    LIFT_FAIL = -1,

    LIFT_AT_ENDSTOP = 1,
    LIFT_SPEED_TOO_HIGH,
    LIFT_SPEED_TOO_LOW,
    LIFT_LIMITS_INVALID
} lift_err_t;

typedef struct lift_device_s* lift_device_handle_t;

lift_err_t lift_add_device(
    gpio_num_t gpioEna,
    gpio_num_t gpioDir,
    gpio_num_t gpioPul,
    gpio_num_t gpioEndstopDown,
    gpio_num_t gpioEndstopUp,
    uint32_t speed,
    uint32_t min_speed,
    uint32_t max_speed,
    lift_device_handle_t* handle);
void lift_remove_device(lift_device_handle_t handle);

lift_err_t lift_up(const lift_device_handle_t handle);
lift_err_t lift_down(const lift_device_handle_t handle);
lift_err_t lift_stop(const lift_device_handle_t handle);

uint32_t lift_get_speed(const lift_device_handle_t handle);
lift_err_t lift_set_speed(lift_device_handle_t handle, uint32_t speed);
lift_err_t lift_set_speed_limits(lift_device_handle_t handle, uint32_t minSpeed, uint32_t maxSpeed);

#endif // LIFT_H