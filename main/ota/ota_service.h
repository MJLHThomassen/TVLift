#ifndef OTA_SERVICE_H
#define OTA_SERVICE_H

#include <stdint.h>
#include <stdbool.h>
#include <esp_ota_ops.h>

typedef enum 
{
    OTA_SERVICE_OK = 0,
    OTA_SERVICE_FAIL = -1,

    OTA_SERVICE_ERR_OTA_END_FAILED = 1,
    OTA_SERVICE_ERR_SET_BOOT_PARTITON_FAILED

} ota_service_err_t;

typedef struct ota_state_t
{
    size_t nr_of_bytes_received;

    // App
    uint8_t app_header[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t)];  
    esp_ota_handle_t app_update_handle;
    const esp_partition_t* app_update_partition;

    // SPIFFS

} ota_state_t;

ota_service_err_t ota_service_initialize(ota_state_t* otaState);
ota_service_err_t ota_service_app_update_begin(ota_state_t* otaState);
ota_service_err_t ota_service_app_update_write(ota_state_t* otaState, const char* data, size_t length);
ota_service_err_t ota_service_app_update_end(ota_state_t* otaState);
ota_service_err_t ota_service_spiffs_update_begin(ota_state_t* otaState);
ota_service_err_t ota_service_spiffs_update_write(ota_state_t* otaState, const char* data, size_t length);
ota_service_err_t ota_service_spiffs_update_end(ota_state_t* otaState);
ota_service_err_t ota_service_finalize(ota_state_t* otaState);

#endif // OTA_SERVICE_H