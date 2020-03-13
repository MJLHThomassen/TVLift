#ifndef SPIFFS_SERVICE_H
#define SPIFFS_SERVICE_H

const char* spiffs_service_get_spiffs_partition_label_for_app_partition(const char* appPartitionLabel);
void        spiffs_service_mount(const char* partitionLabel, const char* basePath);

#endif // SPIFFS_SERVICE_H