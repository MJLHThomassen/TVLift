#include "settings_service.h"

#include <stddef.h>

#include <nvs.h>

#define SETTINGS_NAMESPACE  "settings"

#define VERSION_KEY         "version"
#define SETTINGS_KEY        "settings"

#define CURRENT_VERSION     1

settings_t _cachedSettings;
bool _cachedSettingsValid = false;

static void initialize_default_settings(settings_t * settings)
{
    settings->version = CURRENT_VERSION;
    settings->lift_min_speed = 5;
    settings->lift_max_speed = 8000;
    settings->lift_default_speed = 40;
}

static settings_service_err_t initialize_settings(settings_t * settings)
{
    initialize_default_settings(settings);
    return settings_service_save(settings);
}

settings_service_err_t settings_service_load(const settings_t ** settings)
{
    if(_cachedSettingsValid)
    {
        *settings = &_cachedSettings;
        return SETTINGS_SERVICE_OK;
    }

    esp_err_t err;
    nvs_handle_t handle;
    
    err = nvs_open(SETTINGS_NAMESPACE, NVS_READONLY, &handle);
    if(err != ESP_OK)
    {
        initialize_default_settings(&_cachedSettings);
        *settings = &_cachedSettings;
        return SETTINGS_SERVICE_FAIL;
    }

    uint32_t version;
    nvs_get_u32(handle, VERSION_KEY, &version);
    if(err == ESP_ERR_NVS_NOT_FOUND)
    {
        nvs_close(handle);
        settings_service_err_t settingsErr = initialize_settings(&_cachedSettingsValid);
        *settings = &_cachedSettings;
        return settingsErr;
    }
    else if(err != ESP_OK)
    {
        initialize_default_settings(&_cachedSettings);
        *settings = &_cachedSettings;
        return SETTINGS_SERVICE_FAIL;
    }

    size_t length;
    err = nvs_get_blob(handle, SETTINGS_KEY, &_cachedSettings, &length);
    if(err != ESP_OK)
    {
        initialize_default_settings(&_cachedSettings);
        *settings = &_cachedSettings;
        return SETTINGS_SERVICE_FAIL;
    }

    nvs_close(handle);

    *settings = &_cachedSettings;
    return SETTINGS_SERVICE_OK;
}

settings_service_err_t settings_service_save(const settings_t * settings)
{
    esp_err_t err;
    nvs_handle_t handle;
    
    err = nvs_open(SETTINGS_NAMESPACE, NVS_READWRITE, &handle);
    if(err != ESP_OK)
    {
        return SETTINGS_SERVICE_FAIL;
    }

    nvs_set_u32(handle, VERSION_KEY, settings->version);
    if(err != ESP_OK)
    {
        return SETTINGS_SERVICE_FAIL;
    }

    err = nvs_set_blob(handle, SETTINGS_KEY, settings, sizeof(settings_t));
    if(err != ESP_OK)
    {
        return SETTINGS_SERVICE_FAIL;
    }

    err = nvs_commit(handle);
    if(err != ESP_OK)
    {
        return SETTINGS_SERVICE_FAIL;
    }

    nvs_close(handle);

    _cachedSettings = *settings;
    _cachedSettingsValid = true;

    return SETTINGS_SERVICE_OK;
}