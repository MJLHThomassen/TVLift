#ifndef WEBSERVER_TASK_H
#define WEBSERVER_TASK_H

#include <stdint.h>

#define WEBSERVER_TASK_TAG "Webserver"
#define WEBSERVER_TASK_STACK_SIZE_KB 8

void webserver_task_main(void* pvParameters);

#endif // WEBSERVER_TASK_H