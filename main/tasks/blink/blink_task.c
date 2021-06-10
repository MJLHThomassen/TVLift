#include "blink_task.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <driver/gpio.h>

#include <services/logger_service.h>
#include <services/status_service.h>

static const char* TAG = BLINK_TASK_TAG;

static float blinkOnPercentage = 0;

static void on_service_status_changed(const service_info_t* info)
{
    LOG_I(TAG, "Service %s changed state to %i", info->name, info->state);
    if(info->state == STATUS_SERVICE_STATE_ACTIVE)
    {
        blinkOnPercentage = 1;
    }
}

static void blink_init()
{
    // Configure the IOMUX register for pad BLINK_GPIO
    gpio_pad_select_gpio(BLINK_GPIO);

    // Set the GPIO as a push/pull output
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    // Register to status service
    status_service_register(on_service_status_changed);
}

void blink_task_main(void* pvParameters)
{
    LOG_I(TAG, "Starting task");
    LOG_V(TAG, "Free stack space: %i", uxTaskGetStackHighWaterMark(NULL));
    blink_init();
    
    LOG_D(TAG, "Starting task loop");
    LOG_V(TAG, "Free stack space: %i", uxTaskGetStackHighWaterMark(NULL));
    while (1)
    {
        // Blink off (output low)
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay((1- blinkOnPercentage) * 1000 / portTICK_PERIOD_MS);

        // Blink on (output high)
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(blinkOnPercentage * 1000 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}
