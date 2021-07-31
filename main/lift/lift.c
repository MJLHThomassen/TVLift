#include "lift.h"

#include <driver/ledc.h>
#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include <logger.h>

#define MOTORS_ENABLED 0
#define MOTORS_DISABLED 1

#define ENDSTOP_ACTIVE 0
#define ENDSTOP_INACTIVE 1

#define DIR_DOWN 0
#define DIR_UP 1

static const char             TAG[] = "lift";
static const ledc_timer_bit_t DUTY_RESOLUTION = LEDC_TIMER_13_BIT;

typedef enum lift_command_e
{
    LIFT_COMMAND_STOP,
    LIFT_COMMAND_UP,
    LIFT_COMMAND_DOWN
} lift_command_t;

typedef enum lift_state_e
{
    LIFT_STATE_STOPPED_DOWN = 0,
    LIFT_STATE_MOVING_UP,
    LIFT_STATE_STOPPED_MID,
    LIFT_STATE_REACHED_UP,
    LIFT_STATE_SETTLING_UP,
    LIFT_STATE_STOPPED_UP,
    LIFT_STATE_MOVING_DOWN,
    LIFT_STATE_REACHED_DOWN,
    LIFT_STATE_SETTLING_DOWN,

} lift_state_t;

typedef struct lift_endstop_config_s
{
    gpio_num_t    gpio;
    QueueHandle_t endstopEvtQueue;
} lift_endstop_config_t;

struct lift_device_s
{
    gpio_num_t            gpioEna;
    gpio_num_t            gpioDir;
    gpio_num_t            gpioPul;
    lift_endstop_config_t endstopDownConfig;
    lift_endstop_config_t endstopUpConfig;
    uint32_t              speed;
    uint32_t              min_speed;
    uint32_t              max_speed;
    uint32_t              settle_speed;

    QueueHandle_t commandEvtQueue;
    lift_state_t  state;
    TaskHandle_t  monitorTaskHandle;
};

static lift_err_t lift_start_pul(const lift_device_handle_t handle)
{
    LOG_D(TAG, "Start lift pulse");

    // Configure PWM channel
    ledc_channel_config_t channel_config = {
        .gpio_num = handle->gpioPul,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 1 << (DUTY_RESOLUTION - 1),
        .hpoint = 0};

    // After ledc_channel_config has succesfully returned, the PWM signal is generated on the selected GPIO
    esp_err_t err = ledc_channel_config(&channel_config);
    if (err != ESP_OK)
    {
        return LIFT_FAIL;
    }

    return LIFT_OK;
}

static lift_err_t lift_stop_pul(const lift_device_handle_t handle)
{
    esp_err_t err = ledc_stop(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0);

    return err == ESP_OK ? LIFT_OK : LIFT_FAIL;
}

static lift_err_t lift_move_up(const lift_device_handle_t handle)
{
    LOG_I(TAG, "Lift going up.");

    // Check if the up endstop is not active
    if (gpio_get_level(handle->endstopUpConfig.gpio) == ENDSTOP_ACTIVE)
    {
        LOG_I(TAG, "Lift already fully up, not moving.");
        return LIFT_AT_ENDSTOP;
    }

    // Set up direction
    gpio_set_level(handle->gpioDir, DIR_UP);

    // Wait atleast 20 ns
    // 1 tick delay is the least delay possible, which is more then 20 ns
    vTaskDelay(1);

    // Enable the Lift motors
    gpio_set_level(handle->gpioEna, MOTORS_ENABLED);

    // Start moving
    return lift_start_pul(handle);
}

static lift_err_t lift_move_down(const lift_device_handle_t handle)
{
    LOG_I(TAG, "Lift going down.");

    // Check if the up endstop is not active
    if (gpio_get_level(handle->endstopDownConfig.gpio) == ENDSTOP_ACTIVE)
    {
        LOG_I(TAG, "Lift already fully down, not moving.");
        return LIFT_AT_ENDSTOP;
    }

    // Set down direction
    gpio_set_level(handle->gpioDir, DIR_DOWN);

    // Wait atleast 20 ns
    // 1 tick delay is the least delay possible, which is more then 20 ns
    vTaskDelay(1);

    // Enable the Lift motors
    gpio_set_level(handle->gpioEna, MOTORS_ENABLED);

    // Start moving
    return lift_start_pul(handle);
}

static void IRAM_ATTR endstop_isr_handler(void* arg)
{
    lift_endstop_config_t* endstopConfig = (lift_endstop_config_t*)arg;
    xQueueSendFromISR(endstopConfig->endstopEvtQueue, &endstopConfig->gpio, NULL);
}

static void lift_monitor_task(void* arg)
{
    lift_device_handle_t handle = (lift_device_handle_t)arg;

    gpio_num_t     endstopGpio;
    lift_command_t command;
    for (;;)
    {
        if (xQueueReceive(handle->endstopDownConfig.endstopEvtQueue, &endstopGpio, 1))
        {
            bool isEndstopActive = gpio_get_level(endstopGpio) == ENDSTOP_ACTIVE;

            // If any endstop is depressed, just stop the lift immediately
            if (isEndstopActive)
            {
                lift_stop_pul(handle);
            }

            if (endstopGpio == handle->endstopDownConfig.gpio)
            {
                LOG_I(TAG, "Endstop down triggered");

                handle->state = LIFT_STATE_REACHED_DOWN;
            }
            else if (endstopGpio == handle->endstopUpConfig.gpio && isEndstopActive)
            {
                LOG_I(TAG, "Endstop up triggered");

                handle->state = LIFT_STATE_REACHED_UP;
            }
        }

        if (xQueueReceive(handle->commandEvtQueue, &command, 1))
        {
            // Stop immediately and figure out state later if we need to stop
            if (command == LIFT_COMMAND_STOP)
            {
                lift_stop_pul(handle);
            }

            lift_state_t currentState = handle->state;
            switch (currentState)
            {

            case LIFT_STATE_MOVING_UP:
                switch (command)
                {
                case LIFT_COMMAND_STOP:
                    handle->state = LIFT_STATE_STOPPED_MID;
                    break;

                case LIFT_COMMAND_UP:
                    handle->state = LIFT_STATE_MOVING_UP;
                    break;

                case LIFT_COMMAND_DOWN:
                    lift_move_down(handle);
                    handle->state = LIFT_STATE_MOVING_DOWN;
                    break;

                default:
                    break;
                }
                break;

            case LIFT_STATE_STOPPED_MID:
                switch (command)
                {
                case LIFT_COMMAND_STOP:
                    handle->state = LIFT_STATE_STOPPED_MID;
                    break;

                case LIFT_COMMAND_UP:
                    lift_move_up(handle);
                    handle->state = LIFT_STATE_MOVING_UP;
                    break;

                case LIFT_COMMAND_DOWN:
                    lift_move_down(handle);
                    handle->state = LIFT_STATE_MOVING_DOWN;
                    break;

                default:
                    break;
                }
                break;

            case LIFT_STATE_REACHED_UP:
                switch (command)
                {
                case LIFT_COMMAND_STOP:
                    handle->state = LIFT_STATE_REACHED_UP;
                    break;

                case LIFT_COMMAND_UP:
                    handle->state = LIFT_STATE_REACHED_UP;
                    break;

                case LIFT_COMMAND_DOWN:
                    lift_move_down(handle);
                    handle->state = LIFT_STATE_MOVING_DOWN;
                    break;

                default:
                    break;
                }
                break;

            case LIFT_STATE_MOVING_DOWN:
                switch (command)
                {
                case LIFT_COMMAND_STOP:
                    handle->state = LIFT_STATE_STOPPED_MID;
                    break;

                case LIFT_COMMAND_UP:
                    lift_move_up(handle);
                    handle->state = LIFT_STATE_MOVING_UP;
                    break;

                case LIFT_COMMAND_DOWN:
                    handle->state = LIFT_STATE_MOVING_DOWN;
                    break;

                default:
                    break;
                }
                break;

            case LIFT_STATE_REACHED_DOWN:
                switch (command)
                {
                case LIFT_COMMAND_STOP:
                    handle->state = LIFT_STATE_REACHED_DOWN;
                    break;

                case LIFT_COMMAND_UP:
                    lift_move_up(handle);
                    handle->state = LIFT_STATE_MOVING_UP;
                    break;

                case LIFT_COMMAND_DOWN:
                    handle->state = LIFT_STATE_REACHED_DOWN;
                    break;

                default:
                    break;
                }
                break;

            case LIFT_STATE_STOPPED_DOWN:
            case LIFT_STATE_SETTLING_DOWN:
            case LIFT_STATE_STOPPED_UP:
            case LIFT_STATE_SETTLING_UP:
            default:
                // IMPOSSIBURU!
                continue;
            }

            LOG_I(TAG, "State: %i, Command: %i, New State: %i", currentState, command, handle->state);
        }
    }
}

lift_err_t lift_add_device(
    gpio_num_t            gpioEna,
    gpio_num_t            gpioDir,
    gpio_num_t            gpioPul,
    gpio_num_t            gpioEndstopDown,
    gpio_num_t            gpioEndstopUp,
    uint32_t              speed,
    uint32_t              min_speed,
    uint32_t              max_speed,
    lift_device_handle_t* handle)
{
    esp_err_t err;

    LOG_I(TAG, "Adding lift device %x", (unsigned int)handle);

    lift_device_handle_t newHandle = (lift_device_handle_t)calloc(1, sizeof(*newHandle));

    if (newHandle == NULL)
    {
        LOG_E(TAG, "Can not allocate memory for lift device");
        return LIFT_FAIL;
    }

    // Assign the new handle
    *handle = newHandle;

    // Configure output pins with pullup
    gpio_config_t pullupOutputIoConf = {
        .pin_bit_mask = BIT(gpioEna),
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_PIN_INTR_DISABLE};

    err = gpio_config(&pullupOutputIoConf);
    if (err != ESP_OK)
    {
        return LIFT_FAIL;
    }

    // Configure output pins without pullup or pulldown
    gpio_config_t noPullupOutputIoConf = {
        .pin_bit_mask = BIT(gpioPul) | BIT(gpioDir),
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_PIN_INTR_DISABLE};

    err = gpio_config(&noPullupOutputIoConf);
    if (err != ESP_OK)
    {
        return LIFT_FAIL;
    }

    // Configure input pins with interrupt on negative edge
    gpio_config_t interruptInputIoConf = {
        .pin_bit_mask = BIT(gpioEndstopDown) | BIT(gpioEndstopUp),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_PIN_INTR_ANYEDGE};

    err = gpio_config(&interruptInputIoConf);
    if (err != ESP_OK)
    {
        return LIFT_FAIL;
    }

    // Configure queue to handle gpio event from isr
    QueueHandle_t endstopEvtQueue = xQueueCreate(10, sizeof(gpio_num_t));
    if (endstopEvtQueue == NULL)
    {
        LOG_E(TAG, "Can not allocate memory for endstop event queue");
        return LIFT_FAIL;
    }

    QueueHandle_t commandEvtQueue = xQueueCreate(10, sizeof(lift_command_t));
    if (commandEvtQueue == NULL)
    {
        LOG_E(TAG, "Can not allocate memory for command event queue");
        return LIFT_FAIL;
    }

    // Initialize all values in handle as far as possible
    newHandle->gpioEna = gpioEna;
    newHandle->gpioDir = gpioDir;
    newHandle->gpioPul = gpioPul;
    newHandle->endstopDownConfig.endstopEvtQueue = endstopEvtQueue;
    newHandle->endstopUpConfig.endstopEvtQueue = endstopEvtQueue;
    newHandle->speed = speed > max_speed ? max_speed : (speed < min_speed ? min_speed : speed);
    newHandle->min_speed = min_speed;
    newHandle->max_speed = max_speed;
    newHandle->settle_speed = newHandle->speed / 2;
    newHandle->commandEvtQueue = commandEvtQueue;

    // Check endstops before initializing state
    bool isDown = gpio_get_level(gpioEndstopDown) == ENDSTOP_ACTIVE;
    bool isUp = gpio_get_level(gpioEndstopUp) == ENDSTOP_ACTIVE;
    if (isDown && isUp)
    {
        // Strange, something is wrong
        LOG_W(TAG, "Both endstops depressed, something is wrong! Cancelling initialisation.");
        return LIFT_FAIL;
    }
    else if (isDown)
    {
        LOG_I(TAG, "Lift is down");
        newHandle->state = LIFT_STATE_REACHED_DOWN;
    }
    else if (isUp)
    {
        LOG_I(TAG, "Lift is up");
        newHandle->state = LIFT_STATE_REACHED_UP;
    }
    else
    {
        // Most likely if powered up after lift is in settled position
        LOG_I(TAG, "Lift is somewhere in the middle");
        newHandle->state = LIFT_STATE_STOPPED_MID;
    }

    // Start lift monitoring task
    BaseType_t xReturned = xTaskCreate(lift_monitor_task, "lift_monitor_task", 2048, (void*)newHandle, 10, &newHandle->monitorTaskHandle);
    if (xReturned != pdPASS)
    {
        LOG_E(TAG, "Can not allocate memory for lift monitor task");
        return LIFT_FAIL;
    }

    // Install gpio isr service
    err = gpio_install_isr_service(0);
    if (err != ESP_OK)
    {
        return LIFT_FAIL;
    }

    // Hook isr handlers
    err = gpio_isr_handler_add(gpioEndstopDown, endstop_isr_handler, (void*)&(newHandle->endstopDownConfig));
    if (err != ESP_OK)
    {
        return LIFT_FAIL;
    }
    else
    {
        newHandle->endstopDownConfig.gpio = gpioEndstopDown;
    }

    err = gpio_isr_handler_add(gpioEndstopUp, endstop_isr_handler, (void*)&(newHandle->endstopUpConfig));
    if (err != ESP_OK)
    {
        return LIFT_FAIL;
    }
    else
    {
        newHandle->endstopUpConfig.gpio = gpioEndstopUp;
    }

    // Configure PWM timer
    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = DUTY_RESOLUTION,
        .timer_num = LEDC_TIMER_0,
        .clk_cfg = LEDC_AUTO_CLK,
        .freq_hz = newHandle->speed};

    err = ledc_timer_config(&timer_conf);
    if (err != ESP_OK)
    {
        return LIFT_FAIL;
    }

    return LIFT_OK;
}

void lift_remove_device(lift_device_handle_t handle)
{
    LOG_I(TAG, "Removing lift device %x", (unsigned int)handle);

    if (handle != NULL)
    {
        if (handle->endstopUpConfig.gpio != 0)
        {
            gpio_isr_handler_remove(handle->endstopUpConfig.gpio);
        }

        if (handle->endstopDownConfig.gpio != 0)
        {
            gpio_isr_handler_remove(handle->endstopDownConfig.gpio);
        }

        if (handle->endstopDownConfig.endstopEvtQueue != NULL)
        {
            vQueueDelete(handle->endstopDownConfig.endstopEvtQueue);
        }

        if (handle->monitorTaskHandle != NULL)
        {
            vTaskDelete(handle->monitorTaskHandle);
        }

        free(handle);
    }
}

static lift_err_t lift_send_command(const lift_device_handle_t handle, const lift_command_t command)
{
    // Queue the command
    xQueueSendToBack(handle->commandEvtQueue, &command, portMAX_DELAY);

    // TODO: Propper return value: wait for state machine
    return LIFT_OK;
}

lift_err_t lift_up(const lift_device_handle_t handle)
{
    return lift_send_command(handle, LIFT_COMMAND_UP);
}

lift_err_t lift_down(const lift_device_handle_t handle)
{
    return lift_send_command(handle, LIFT_COMMAND_DOWN);
}

lift_err_t lift_stop(const lift_device_handle_t handle)
{
    // Remove all pending commands from the queue
    xQueueReset(handle->commandEvtQueue);

    return lift_send_command(handle, LIFT_COMMAND_STOP);
}

// lift_err_t lift_disable(const lift_device_handle_t handle)
// {
//     LOG_I(TAG, "Lift being disabled.");

//     // Disable the Lift motors
//     gpio_set_level(handle->gpioEna, MOTORS_DISABLED);

//     return LIFT_OK;
// }

uint32_t lift_get_speed(const lift_device_handle_t handle)
{
    return handle->speed;
}

lift_err_t lift_set_speed(lift_device_handle_t handle, uint32_t speed)
{
    if (speed > handle->max_speed)
    {
        return LIFT_SPEED_TOO_HIGH;
    }
    else if (speed < handle->min_speed)
    {
        return LIFT_SPEED_TOO_LOW;
    }

    esp_err_t err = ledc_set_freq(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0, speed);
    if (err != ESP_OK)
    {
        return LIFT_FAIL;
    }

    handle->speed = speed;
    return LIFT_OK;
}