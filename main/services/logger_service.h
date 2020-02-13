#ifndef LOGGER_SERVICE_H
#define LOGGER_SERVICE_H

#include <stddef.h>
#include <stdint.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define LOGGER_SERVICE_COLOR_BLACK     "30"
#define LOGGER_SERVICE_COLOR_RED       "31"
#define LOGGER_SERVICE_COLOR_GREEN     "32"
#define LOGGER_SERVICE_COLOR_YELLOW    "33"
#define LOGGER_SERVICE_COLOR_BLUE      "34"
#define LOGGER_SERVICE_COLOR_PURPLE    "35"
#define LOGGER_SERVICE_COLOR_CYAN      "36"
#define LOGGER_SERVICE_COLOR_WHITE     "37"
#define LOGGER_SERVICE_COLOR(COLOR)    "\033[0;" COLOR "m"
#define LOGGER_SERVICE_BOLD(COLOR)     "\033[1;" COLOR "m"
#define LOGGER_SERVICE_FAINT(COLOR)    "\033[2;" COLOR "m"
#define LOGGER_SERVICE_RESET_COLOR     "\033[0m"
#define LOGGER_SERVICE_COLOR_E         LOGGER_SERVICE_COLOR(LOGGER_SERVICE_COLOR_RED)
#define LOGGER_SERVICE_COLOR_W         LOGGER_SERVICE_COLOR(LOGGER_SERVICE_COLOR_YELLOW)
#define LOGGER_SERVICE_COLOR_I         LOGGER_SERVICE_COLOR(LOGGER_SERVICE_COLOR_GREEN)
#define LOGGER_SERVICE_COLOR_D         LOGGER_SERVICE_COLOR(LOGGER_SERVICE_COLOR_WHITE)
#define LOGGER_SERVICE_COLOR_V         LOGGER_SERVICE_FAINT(LOGGER_SERVICE_COLOR_WHITE)

#define LOGGER_SERVICE_FORMAT(letter, format)  LOGGER_SERVICE_COLOR_ ## letter #letter " (%lld) (%s) %s: " format LOGGER_SERVICE_RESET_COLOR "\n"

#define LOG_E( tag, format, ... ) logger_service_log(LOGGER_SERVICE_LOGLEVEL_ERROR,   tag, LOGGER_SERVICE_FORMAT(E, format), logger_service_log_timestamp(), pcTaskGetTaskName(NULL), tag, ##__VA_ARGS__)
#define LOG_W( tag, format, ... ) logger_service_log(LOGGER_SERVICE_LOGLEVEL_WARN,    tag, LOGGER_SERVICE_FORMAT(W, format), logger_service_log_timestamp(), pcTaskGetTaskName(NULL), tag, ##__VA_ARGS__)
#define LOG_I( tag, format, ... ) logger_service_log(LOGGER_SERVICE_LOGLEVEL_INFO,    tag, LOGGER_SERVICE_FORMAT(I, format), logger_service_log_timestamp(), pcTaskGetTaskName(NULL), tag, ##__VA_ARGS__)
#define LOG_D( tag, format, ... ) logger_service_log(LOGGER_SERVICE_LOGLEVEL_DEBUG,   tag, LOGGER_SERVICE_FORMAT(D, format), logger_service_log_timestamp(), pcTaskGetTaskName(NULL), tag, ##__VA_ARGS__)
#define LOG_V( tag, format, ... ) logger_service_log(LOGGER_SERVICE_LOGLEVEL_VERBOSE, tag, LOGGER_SERVICE_FORMAT(V, format), logger_service_log_timestamp(), pcTaskGetTaskName(NULL), tag, ##__VA_ARGS__)

typedef enum logger_service_loglevel_e
{
    LOGGER_SERVICE_LOGLEVEL_NONE    = 0,
    LOGGER_SERVICE_LOGLEVEL_ERROR   = 1,
    LOGGER_SERVICE_LOGLEVEL_WARN    = 2,
    LOGGER_SERVICE_LOGLEVEL_INFO    = 3,
    LOGGER_SERVICE_LOGLEVEL_DEBUG   = 4,
    LOGGER_SERVICE_LOGLEVEL_VERBOSE = 5
} logger_service_loglevel_t; 

typedef struct sink_s* sink_handle_t;
typedef void (*logger_sink_t)(const char* message, const size_t len, void* user_data);

void        logger_service_init(void);
void        logger_service_log(logger_service_loglevel_t level, const char* tag, const char* format, ...);
int64_t     logger_service_log_timestamp();

sink_handle_t logger_service_register_sink(logger_sink_t callback, void* user_data);
void          logger_service_unregister_sink(sink_handle_t handle);

#endif // LOGGER_SERVICE_H