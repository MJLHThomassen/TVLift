#ifndef OTA_SERVICE_H
#define OTA_SERVICE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum 
{
    OTA_SERVICE_OK = 0,
    OTA_SERVICE_FAIL = -1,

    OTA_SERVICE_ERR_OTA_END_FAILED = 1,
    OTA_SERVICE_ERR_SET_BOOT_PARTITON_FAILED

} ota_service_err_t;

typedef struct ota_state_s* ota_state_handle_t;

/**
 * @brief Initializes an OTA update and allocates memory for the OTA state pointed to by handle.
 * 
 * @param[out] handle Pointer to handle that will hold resources for the OTA update.
 * 
 * @return ota_service_err_t OTA_SERVICE_OK if initialization succeeded, or
 * OTA_SERVICE_FAIL if it did not.
 */
ota_service_err_t ota_service_initialize(ota_state_handle_t* handle);
ota_service_err_t ota_service_app_update_begin(ota_state_handle_t handle);
ota_service_err_t ota_service_app_update_write(ota_state_handle_t handle, const char* data, size_t length);
ota_service_err_t ota_service_app_update_end(ota_state_handle_t handle);
ota_service_err_t ota_service_spiffs_update_begin(ota_state_handle_t handle);
ota_service_err_t ota_service_spiffs_update_write(ota_state_handle_t handle, const char* data, size_t length);
ota_service_err_t ota_service_spiffs_update_end(ota_state_handle_t handle);

/**
 * @brief Finalizes the OTA update and frees resources of the OTA state pointed to by handle.
 * 
 * @param[in] handle Handle that holds resources for the OTA update.
 * Will be invalid after this call wether the update succeeded or not.
 * 
 * @return ota_service_err_t OTA_SERVICE_OK if finalization succeeded, or
 * OTA_SERVICE_ERR_SET_BOOT_PARTITON_FAILED if the new boot partition could not be set.
 */
ota_service_err_t ota_service_finalize(ota_state_handle_t handle);

#endif // OTA_SERVICE_H