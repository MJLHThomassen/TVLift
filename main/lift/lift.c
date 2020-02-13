#include "lift.h"

#include <esp_err.h>
#include <driver/ledc.h>

#include <services/logger_service.h>

#define ENDSTOP_ACTIVE 0
#define ENDSTOP_INACTIVE  1

#define DIR_DOWN 0
#define DIR_UP 1

static const char TAG[] = "lift";
static const ledc_timer_bit_t DUTY_RESOLUTION = LEDC_TIMER_8_BIT;

static lift_err_t lift_start_pul(const lift_device_t* const handle, uint32_t speed)
{
    esp_err_t err;

    LOG_I(TAG, "Start lift pulse at %i Hz", speed);

    // Configure PWM timer
    ledc_timer_config_t timer_conf = {
        .speed_mode         = LEDC_HIGH_SPEED_MODE,
        .duty_resolution    = DUTY_RESOLUTION,
        .timer_num          = LEDC_TIMER_0,
        .clk_cfg            = LEDC_AUTO_CLK,
        .freq_hz            = speed
    };

    err = ledc_timer_config(&timer_conf);
    if(err != ESP_OK)
    {
        return LIFT_FAIL;
    }

    // Configure PWM channel
    ledc_channel_config_t channel_config = {
        .gpio_num   = handle->gpioPul,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel    = LEDC_CHANNEL_0,
        .intr_type  = LEDC_INTR_DISABLE,
        .timer_sel  = LEDC_TIMER_0,
        .duty       = 1 << (DUTY_RESOLUTION - 1),
        .hpoint     = 0
    };

    // After ledc_channel_config has succesfully returned, the PWM signal is generated on the selected GPIO
    err = ledc_channel_config(&channel_config);
    if(err != ESP_OK)
    {
        return LIFT_FAIL;
    }

    return LIFT_OK;
}

static lift_err_t lift_stop_pul(const lift_device_t* const handle)
{
    esp_err_t err = ledc_stop(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0);

    return err == ESP_OK ? LIFT_OK : LIFT_FAIL;
}

static void IRAM_ATTR endstop_down_isr_handler(void* arg)
{
    lift_device_t* handle = (lift_device_t*) arg;

    // If we are moving down, stop
    if(gpio_get_level(handle->gpioDir) == DIR_DOWN)
    {
        lift_stop_pul(handle);
    }
}

static void IRAM_ATTR endstop_up_isr_handler(void* arg)
{
    lift_device_t* handle = (lift_device_t*) arg;

    // If we are moving up, stop
    if(gpio_get_level(handle->gpioDir) == DIR_UP)
    {
        lift_stop_pul(handle);
    }
}

lift_err_t lift_add_device(
    gpio_num_t gpioEna,
    gpio_num_t gpioDir,
    gpio_num_t gpioPul,
    gpio_num_t gpioEndstopDown,
    gpio_num_t gpioEndstopUp,
    lift_device_t* const handle)
{
    esp_err_t err;

    LOG_I(TAG, "Adding lift device %x", (unsigned int)handle);

    // Configure output pins with pullup
    // Set mode = GPIO_MODE_INPUT_OUTPUT so we can read back active value
    gpio_config_t pullupOutputIoConf = {
        .pin_bit_mask = BIT(gpioEna) | BIT(gpioDir),
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_PIN_INTR_DISABLE
    };
    
    err = gpio_config(&pullupOutputIoConf);
    if(err != ESP_OK)
    {
        return LIFT_FAIL;
    }

    // Configure output pins without pullup
    // Set mode = GPIO_MODE_INPUT_OUTPUT so we can read back active value
    gpio_config_t noPullupOutputIoConf = {
        .pin_bit_mask = BIT(gpioPul),
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_PIN_INTR_DISABLE
    };
    
    err = gpio_config(&noPullupOutputIoConf);
    if(err != ESP_OK)
    {
        return LIFT_FAIL;
    }

    // Configure input pins with interrupt on negative edge
    // gpio_config_t interruptInputIoConf = {
    //     .pin_bit_mask = BIT(gpioEndstopDown) | BIT(gpioEndstopUp),
    //     .mode= GPIO_MODE_INPUT,
    //     .pull_up_en = GPIO_PULLUP_DISABLE,
    //     .pull_down_en = GPIO_PULLDOWN_DISABLE,
    //     .intr_type = GPIO_PIN_INTR_NEGEDGE
    // };
    
    // err = gpio_config(&interruptInputIoConf);
    // if(err != ESP_OK)
    // {
    //     return LIFT_FAIL;
    // }

    // Initialize all values in handle
    handle->gpioEna = gpioEna;
    handle->gpioDir = gpioDir;
    handle->gpioPul = gpioPul;
    handle->gpioEndstopDown = gpioEndstopDown;
    handle->gpioEndstopUp = gpioEndstopUp;
    handle->speed = 320;

    return LIFT_OK;

    // Install gpio isr service
    err = gpio_install_isr_service(0);
    if(err != ESP_OK)
    {
        return LIFT_FAIL;
    }

    // Hook isr handlers
    err = gpio_isr_handler_add(gpioEndstopDown, endstop_down_isr_handler, (void*) handle);
    if(err != ESP_OK)
    {
        return LIFT_FAIL;
    }

    err = gpio_isr_handler_add(gpioEndstopUp, endstop_up_isr_handler, (void*) handle);
    if(err != ESP_OK)
    {
        return LIFT_FAIL;
    }

    return LIFT_OK;
}

lift_err_t lift_remove_device(const lift_device_t* const handle)
{
    LOG_I(TAG, "Removing lift device %x", (unsigned int)handle);

    return LIFT_OK;
}

lift_err_t lift_up(const lift_device_t* const handle)
{
    LOG_I(TAG, "Lift going up.");

    // Check if the up endstop is not active
    if(gpio_get_level(handle->gpioEndstopUp) == ENDSTOP_ACTIVE)
    {
        //return;
    }

    // Enable the Lift
    gpio_set_level(handle->gpioEna, 0);

    // Set up direction
    gpio_set_level(handle->gpioDir, DIR_UP);

    // Start moving
    return lift_start_pul(handle, handle->speed);
}

lift_err_t lift_down(const lift_device_t* const handle)
{
    LOG_I(TAG, "Lift going down.");

    // Check if the up endstop is not active
    if(gpio_get_level(handle->gpioEndstopDown) == ENDSTOP_ACTIVE)
    {
        //return;
    }

    // Enable the Lift
    gpio_set_level(handle->gpioEna, 0);

    // Set up direction
    gpio_set_level(handle->gpioDir, DIR_DOWN);

    // Start moving
    return lift_start_pul(handle, handle->speed);
}

lift_err_t lift_stop(const lift_device_t* const handle)
{
    LOG_I(TAG, "Lift stopping.");

    // Stop moving
    return lift_stop_pul(handle);
}

lift_err_t lift_disable(const lift_device_t* const handle)
{
    LOG_I(TAG, "Lift being disabled.");

    // Disable the Lift motors
    gpio_set_level(handle->gpioEna, 1);

    return LIFT_OK;
}

lift_err_t lift_set_speed(lift_device_t* const handle, uint32_t speed)
{
    handle->speed = speed;

    return LIFT_OK;
}