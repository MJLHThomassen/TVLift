#include "blink_task.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

#include <driver/gpio.h>

static const char* TAG = BLINK_TASK_TAG;

static void blink_init()
{
    // Configure the IOMUX register for pad BLINK_GPIO
    gpio_pad_select_gpio(BLINK_GPIO);

    // Set the GPIO as a push/pull output
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}

void blink_task_main(void* pvParameters)
{
    ESP_LOGD(TAG, "Starting task");
    ESP_LOGV(TAG, "Free stack space: %i", uxTaskGetStackHighWaterMark(NULL));
    blink_init();
    
    ESP_LOGD(TAG, "Starting task loop");
    ESP_LOGV(TAG, "Free stack space: %i", uxTaskGetStackHighWaterMark(NULL));
    while (1)
    {
        // Blink off (output low)
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        // Blink on (output high)
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
