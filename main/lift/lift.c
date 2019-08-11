#include "lift.h"

#include <esp_log.h>
#include <esp_err.h>
#include <driver/ledc.h>

#define ENDSTOP_ACTIVE 0
#define ENDSTOP_INACTIVE  1

#define DIR_DOWN 0
#define DIR_UP 1

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

void lift_add_device(
    gpio_num_t gpioEna,
    gpio_num_t gpioDir,
    gpio_num_t gpioPul,
    gpio_num_t gpioEndstopDown,
    gpio_num_t gpioEndstopUp,
    lift_device_t* const handle)
{
    ESP_LOGI(TAG, "Adding lift device %x", (unsigned int)handle);

    // Configure output pins with pullup
    // Set mode = GPIO_MODE_INPUT_OUTPUT so we can read back active value
    gpio_config_t pullupOutputIoConf = {
        .pin_bit_mask = BIT(gpioEna) | BIT(gpioDir),
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_PIN_INTR_DISABLE
    };
    
    ESP_ERROR_CHECK(gpio_config(&pullupOutputIoConf));

    // Configure output pins without pullup
    // Set mode = GPIO_MODE_INPUT_OUTPUT so we can read back active value
    gpio_config_t noPullOutputIoConf = {
        .pin_bit_mask = BIT(gpioPul),
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_PIN_INTR_DISABLE
    };
    
    ESP_ERROR_CHECK(gpio_config(&noPullOutputIoConf));

    // Configure input pins with interrupt on negative edge
    gpio_config_t interruptInputIoConf = {
        .pin_bit_mask = BIT(gpioEndstopDown) | BIT(gpioEndstopUp),
        .mode= GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_PIN_INTR_NEGEDGE
    };
    
    ESP_ERROR_CHECK(gpio_config(&interruptInputIoConf));

    // Initialize all values in handle
    handle->gpioEna = gpioEna;
    handle->gpioDir = gpioDir;
    handle->gpioPul = gpioPul;
    handle->gpioEndstopDown = gpioEndstopDown;
    handle->gpioEndstopUp = gpioEndstopUp;
    handle->speed = 6400;

    // Install gpio isr service
    ESP_ERROR_CHECK(gpio_install_isr_service(0));

    // Hook isr handlers
    ESP_ERROR_CHECK(gpio_isr_handler_add(gpioEndstopDown, endstop_down_isr_handler, (void*) handle));
    ESP_ERROR_CHECK(gpio_isr_handler_add(gpioEndstopUp, endstop_up_isr_handler, (void*) handle));
}

void lift_remove_device(const lift_device_t* const handle)
{
    ESP_LOGI(TAG, "Removing lift device %x", (unsigned int)handle);
}

void lift_up(const lift_device_t* const handle)
{
    ESP_LOGI(TAG, "Lift going up.");

    // Check if the up endstop is not active
    if(gpio_get_level(handle->gpioEndstopUp) == ENDSTOP_ACTIVE)
    {
        return;
    }

    // Enable the Lift
    gpio_set_level(handle->gpioEna, 0);

    // Set up direction
    gpio_set_level(handle->gpioDir, DIR_UP);

    // Start moving
    lift_start_pul(handle, handle->speed);
}

void lift_down(const lift_device_t* const handle)
{
    ESP_LOGI(TAG, "Lift going down.");

 // Check if the up endstop is not active
    if(gpio_get_level(handle->gpioEndstopDown) == ENDSTOP_ACTIVE)
    {
        return;
    }

    // Enable the Lift
    gpio_set_level(handle->gpioEna, 0);

    // Set up direction
    gpio_set_level(handle->gpioDir, DIR_DOWN);

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