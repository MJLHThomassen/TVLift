#pragma once

#include <driver/gpio.h>

typedef struct lift_device_t
{
    gpio_num_t gpioEna;
    gpio_num_t gpioDir;
    gpio_num_t gpioPul;
    gpio_num_t gpioEndstopDown;
    gpio_num_t gpioEndstopUp;
    uint32_t speed;
} lift_device_t;

void lift_add_device(
    gpio_num_t gpioEna,
    gpio_num_t gpioDir,
    gpio_num_t gpioPul,
    gpio_num_t gpioEndstopDown,
    gpio_num_t gpioEndstopUp,
    lift_device_t* const handle);
void lift_remove_device(const lift_device_t* const handle);

void lift_up(const lift_device_t* const handle);
void lift_down(const lift_device_t* const handle);
void lift_stop(const lift_device_t* const handle);
void lift_disable(const lift_device_t* const handle);
void lift_set_speed(lift_device_t* const handle, uint32_t speed);