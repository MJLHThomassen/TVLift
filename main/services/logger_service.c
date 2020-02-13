#include "logger_service.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <esp_log.h>
#include <esp_timer.h>

#include <utilities/slist.h>

struct sink_s
{
    logger_sink_t callback;
    void* user_data;
};

static slist sinks;

static void logger_service_log_to_sinks(const char* str, size_t len)
{
    // Print to sinks 
    sink_handle_t sink;
    slist_iter sinksIter;
    if(slist_iter_new(sinks, &sinksIter) != UTIL_ERR_ALLOC)
    {
        while(slist_iter_next(sinksIter, (void**)(&sink)) != UTIL_ITER_END)
        {
            // Call the sink
            if(sink != NULL)
            {
                sink->callback(str, len, sink->user_data);
            }
        }
        
        slist_iter_delete(sinksIter);
    }
    else
    {
        ESP_LOGW("Logger", "Could not allocate memory for itterating logger service sinks");
    }
}

static int newlog(const char * format, va_list list)
{
    char* str = (char*) calloc(512, sizeof(char));

    if(str == NULL)
    {
        return 0 ;
    }

    // Format string
    int len = vsnprintf(str, 512, format, list);

    logger_service_log_to_sinks(str, len);

    free(str);

    return len;
}

void logger_service_init(void)
{
    ESP_LOGI("logger", "Initializing Logger");
    slist_new(&sinks);

    esp_log_set_vprintf(newlog);
}

void logger_service_log(logger_service_loglevel_t level, const char* tag, const char* format, ...)
{
    char* str = (char*) calloc(512, sizeof(char));

    if(str == NULL)
    {
        ESP_LOGW("Logger", "Could not allocate memory for writing to logger service sinks");
        return;
    }

    // Format string
    va_list list;
    va_start(list, format);
    int len = vsnprintf(str, 512, format, list);
    va_end(list);

    logger_service_log_to_sinks(str, len);
    
    free(str);
}

int64_t logger_service_log_timestamp()
{
    return esp_timer_get_time();
}

sink_handle_t logger_service_register_sink(logger_sink_t callback, void* user_data)
{
    sink_handle_t newSink = (sink_handle_t)malloc(sizeof(*newSink));

    if(newSink == NULL)
    {
        ESP_LOGW("Logger", "Could not allocate memory for adding sink");
        return NULL;
    }

    newSink->callback = callback;
    newSink->user_data = user_data;

    slist_add(sinks, (void*)newSink);

    return newSink;
}

void logger_service_unregister_sink(sink_handle_t handle)
{
    slist_remove(sinks, (void*)handle);
}
