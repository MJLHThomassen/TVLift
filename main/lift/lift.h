#pragma once

#include <driver/gpio.h>

typedef enum 
{
    LIFT_OK = 0,
    LIFT_FAIL = -1
} lift_err_t;

typedef struct lift_device_t
{
    gpio_num_t gpioEna;
    gpio_num_t gpioDir;
    gpio_num_t gpioPul;
    gpio_num_t gpioEndstopDown;
    gpio_num_t gpioEndstopUp;
    uint32_t speed;
} lift_device_t;

lift_err_t lift_add_device(
    gpio_num_t gpioEna,
    gpio_num_t gpioDir,
    gpio_num_t gpioPul,
    gpio_num_t gpioEndstopDown,
    gpio_num_t gpioEndstopUp,
    lift_device_t* const handle);
lift_err_t lift_remove_device(const lift_device_t* const handle);

lift_err_t lift_up(const lift_device_t* const handle);
lift_err_t lift_down(const lift_device_t* const handle);
lift_err_t lift_stop(const lift_device_t* const handle);
lift_err_t lift_disable(const lift_device_t* const handle);
lift_err_t lift_set_speed(lift_device_t* const handle, uint32_t speed);