#include "logger_service.h"

#include <stdio.h>

#include <esp_log.h>

#include <utilities/slist.h>

typedef struct sink_s
{
    logger_sink_t fn;
    void* user_data;
} sink_t;

static slist sinks;
static vprintf_like_t originalLogger;

static int logger_service_log(const char * format, va_list arg)
{
    // Print to original logger
    //return originalLogger(format, arg);

    char str[512];
    int len = vsnprintf(str, 512, format, arg);

    // Print to other sinks 
    sink_t* sink;
    slist_iter sinksIter;
    slist_iter_new(sinks, &sinksIter);
    while(slist_iter_next(sinksIter, (void**)(&sink)) != UTIL_ITER_END)
    {
        // Call the sink
        if(sink != NULL)
        {
            sink->fn(str, len, sink->user_data);
        }
    }
    
    slist_iter_delete(sinksIter);

    return len;
}

sink_handle_t logger_service_register_sink(logger_sink_t callback, void* user_data)
{
    sink_t* newSink = (sink_t*)malloc(sizeof(*newSink));

    newSink->fn = callback;
    newSink->user_data = user_data;

    slist_add(sinks, newSink);

    ESP_LOGE("Logger", "Test Error");
    ESP_LOGW("Logger", "Test Warning");
    ESP_LOGI("Logger", "Test Info");
    ESP_LOGD("Logger", "Test Debug");
    ESP_LOGV("Logger", "Test Verbose");

    return newSink;
}

void logger_service_unregister_sink(sink_handle_t handle)
{
    slist_remove(sinks, handle);
}

void logger_service_init(void)
{
    originalLogger = esp_log_set_vprintf(logger_service_log);
    slist_new(&sinks);
}