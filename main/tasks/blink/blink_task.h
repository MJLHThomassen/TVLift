#ifndef BLINK_TASK_H
#define BLINK_TASK_H

#include <stdint.h>

#define BLINK_GPIO 2

#define BLINK_TASK_TAG "Blink"
#define BLINK_TASK_STACK_SIZE_KB 8

void blink_task_main(void* pvParameters);

#endif // BINK_TASK_H