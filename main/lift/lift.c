#include "lift.h"

#include <esp_log.h>
#include <esp_err.h>
#include <driver/ledc.h>

static const char TAG[] = "lift";

static void lift_start_pul(const lift_device_t* const handle, uint32_t speed)
{
    // Configure PWM timer
    ledc_timer_config_t timer_conf = {
        .speed_mode         = LEDC_HIGH_SPEED_MODE,
        .duty_resolution    = LEDC_TIMER_1_BIT,
        .timer_num          = LEDC_TIMER_0,
        .freq_hz            = speed
    };

    ESP_ERROR_CHECK(ledc_timer_config(&timer_conf));

    // Configure PWM channel
    ledc_channel_config_t channel_config = {
        .gpio_num   = handle->gpioPul,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel    = LEDC_CHANNEL_0,
        .intr_type  = LEDC_INTR_DISABLE,
        .timer_sel  = LEDC_TIMER_0,
        .duty       = 1,
        .hpoint     = 0
    };

    ESP_ERROR_CHECK(ledc_channel_config(&channel_config));
}

static void lift_stop_pul(const lift_device_t* const handle)
{
    ESP_ERROR_CHECK(ledc_stop(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0));
}

void lift_add_device(gpio_num_t gpioEna, gpio_num_t gpioDir, gpio_num_t gpioPul, lift_device_t* const handle)
{
    ESP_LOGI(TAG, "Adding lift device %x", (unsigned int)handle);

    gpio_config_t enaDirIo_conf = {
        .pin_bit_mask = BIT(gpioEna) | BIT(gpioDir),
        .mode= GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_PIN_INTR_DISABLE
    };
    
    ESP_ERROR_CHECK(gpio_config(&enaDirIo_conf));

    gpio_config_t pulIo_conf = {
        .pin_bit_mask = BIT(gpioPul),
        .mode= GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_PIN_INTR_DISABLE
    };
    
    ESP_ERROR_CHECK(gpio_config(&pulIo_conf));

    handle->gpioEna = gpioEna;
    handle->gpioDir = gpioDir;
    handle->gpioPul = gpioPul;
    handle->speed = 6400;
}

void lift_remove_device(const lift_device_t* const handle)
{
    ESP_LOGI(TAG, "Removing lift device %x", (unsigned int)handle);
}

void lift_up(const lift_device_t* const handle)
{
    ESP_LOGI(TAG, "Lift going up.");

    // Enable the Lift
    gpio_set_level(handle->gpioEna, 0);

    // Set up direction
    gpio_set_level(handle->gpioDir, 1);

    // Start moving
    lift_start_pul(handle, handle->speed);
}

void lift_down(const lift_device_t* const handle)
{
    ESP_LOGI(TAG, "Lift going down.");

    // Enable the Lift
    gpio_set_level(handle->gpioEna, 0);

    // Set up direction
    gpio_set_level(handle->gpioDir, 0);

    // Start moving
    lift_start_pul(handle, handle->speed);
}

void lift_stop(const lift_device_t* const handle)
{
    ESP_LOGI(TAG, "Lift stopping.");

    // Stop moving
    lift_stop_pul(handle);
}

void lift_disable(const lift_device_t* const handle)
{
    ESP_LOGI(TAG, "Lift being disabled.");

    // Disable the Lift motors
    gpio_set_level(handle->gpioEna, 1);
}

void lift_set_speed(lift_device_t* const handle, uint32_t speed)
{
    handle->speed = speed;
}