#include "settings_service.h"

#include <stdbool.h>
#include <stddef.h>

#include <nvs.h>

#include <logger.h>
#include <map.h>

#define SETTINGS_NAMESPACE  "settings"

#define VERSION_KEY         "version"
#define SETTINGS_KEY        "settings"

#define CURRENT_VERSION     1

static const char TAG[] = "Settings Service";

settings_t _cachedSettings;
bool _cachedSettingsValid = false;

static settings_service_registration_handle_t _registrationCounter = 0;
static map _registrations = NULL;

static void initialize_default_settings(settings_t * settings)
{
    settings->version = CURRENT_VERSION;
    settings->lift_min_speed = 5;
    settings->lift_max_speed = 8000;
    settings->lift_default_speed = 40;
}

static settings_service_err_t initialize_settings()
{
    LOG_I(TAG, "Initializing settings");
    
    initialize_default_settings(&_cachedSettings);
    _cachedSettingsValid = true;

    return settings_service_save(&_cachedSettings);
}

settings_service_err_t settings_service_load(const settings_t ** settings)
{
    LOG_D(TAG, "Loading settings");

    if(_cachedSettingsValid)
    {
        *settings = &_cachedSettings;

        LOG_D(TAG, "Loaded settings from cache");

        return SETTINGS_SERVICE_OK;
    }

    esp_err_t err;
    nvs_handle_t handle;
    
    err = nvs_open(SETTINGS_NAMESPACE, NVS_READONLY, &handle);
    if(err != ESP_OK)
    {
        nvs_close(handle);

        initialize_default_settings(&_cachedSettings);
        *settings = &_cachedSettings;

        LOG_W(TAG, "Can not open nvs. Loaded default settings.");

        return SETTINGS_SERVICE_LOADED_DEFAULT;
    }

    uint32_t version;
    nvs_get_u32(handle, VERSION_KEY, &version);
    if(err == ESP_ERR_NVS_NOT_FOUND)
    {
        nvs_close(handle);

        LOG_I(TAG, "Settings not found in NVS.");

        settings_service_err_t settingsErr = initialize_settings();
        *settings = &_cachedSettings;
        
        return settingsErr;
    }
    else if(err != ESP_OK)
    {
        nvs_close(handle);

        initialize_default_settings(&_cachedSettings);
        *settings = &_cachedSettings;

        LOG_W(TAG, "Can not read version from NVS. Loaded default settings.");

        return SETTINGS_SERVICE_LOADED_DEFAULT;
    }

    size_t length;
    err = nvs_get_blob(handle, SETTINGS_KEY, &_cachedSettings, &length);
    if(err != ESP_OK)
    {
        nvs_close(handle);

        initialize_default_settings(&_cachedSettings);
        *settings = &_cachedSettings;

        LOG_W(TAG, "Can not read settings blob from NVS. Loaded default settings.");

        return SETTINGS_SERVICE_LOADED_DEFAULT;
    }

    nvs_close(handle);

    *settings = &_cachedSettings;

    LOG_I(TAG, "Loaded settings from NVS");

    return SETTINGS_SERVICE_OK;
}

settings_service_err_t settings_service_save(const settings_t * settings)
{
    LOG_D(TAG, "Saving settings");

    // Inform all registrations about the changed settings
    settings_t* currentSettings;
    settings_service_err_t settingsErr = settings_service_load(&currentSettings);
    if(settingsErr != SETTINGS_SERVICE_OK && settingsErr != SETTINGS_SERVICE_LOADED_DEFAULT)
    {
        return settingsErr;
    }

    map_iter iter;
    map_iter_new(_registrations, &iter);

    settings_service_registration_handle_t registrationHandle = NULL;
    on_settings_changed_t callback = NULL;
    settings_change_err_t changeErr = SETTINGS_CHANGE_OK;
    do
    {
        map_iter_next(iter, (void**)&registrationHandle, (void**)&callback);
        
        if(callback != NULL)
        {
            LOG_V(TAG, "Checking new settings with callback handle %i", registrationHandle);

            changeErr = callback(settings, currentSettings);
            if(changeErr == SETTINGS_CHANGE_FAIL)
            {
                LOG_W(TAG, "New settings not accepted by callback handle %i", registrationHandle);
                break;
            }
        }
    } while(registrationHandle != NULL);
    
    map_iter_delete(iter);

    // If the new settings are not acceptable, return error
    if(changeErr != SETTINGS_CHANGE_OK)
    {
        LOG_W(TAG, "New settings invalid");

        return SETTINGS_SERVICE_INVALID_SETTINGS;
    }

    // If the new settings are accepted and applied, cache them, before trying to save them
    _cachedSettings = *settings;
    _cachedSettingsValid = true;

    // Save new settings to nvs
    esp_err_t err;
    nvs_handle_t handle;
    
    err = nvs_open(SETTINGS_NAMESPACE, NVS_READWRITE, &handle);
    if(err != ESP_OK)
    {
        LOG_W(TAG, "Can not open nvs. Settings not saved.");

        return SETTINGS_SERVICE_FAIL;
    }

    nvs_set_u32(handle, VERSION_KEY, settings->version);
    if(err != ESP_OK)
    {
        nvs_close(handle);

        LOG_W(TAG, "Can not save version to NVS");

        return SETTINGS_SERVICE_FAIL;
    }

    err = nvs_set_blob(handle, SETTINGS_KEY, settings, sizeof(settings_t));
    if(err != ESP_OK)
    {
        nvs_close(handle);

        LOG_W(TAG, "Can not save settings blob to NVS");

        return SETTINGS_SERVICE_FAIL;
    }

    err = nvs_commit(handle);
    if(err != ESP_OK)
    {
        nvs_close(handle);
        
        LOG_W(TAG, "Can not commit changes to NVS");

        return SETTINGS_SERVICE_FAIL;
    }

    nvs_close(handle);

    LOG_I(TAG, "Settings saved to NVS");

    return SETTINGS_SERVICE_OK;
}

settings_service_registration_handle_t settings_service_register(on_settings_changed_t callback)
{
    if(_registrations == NULL)
    {
        map_new(&_registrations);
    }

    settings_service_registration_handle_t handle = _registrationCounter++;

    map_add(_registrations, (void*)handle, callback);

    return handle;
}

void settings_service_unregister(settings_service_registration_handle_t handle)
{
    map_remove(_registrations, (void*)handle);
}