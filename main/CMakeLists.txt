idf_component_register(
    SRCS    
    "lift/lift.c"
    
    "ota/ota_service.c"

    "services/logger_service.c"
    "services/status_service.c"

    "tasks/blink/blink_task.c"
    
    "tasks/webserver/webserver_task.c"
    "tasks/webserver/controllers/controller_base.c"
    "tasks/webserver/controllers/lift_controller.c"
    "tasks/webserver/controllers/upload_controller.c"

    "utilities/map.c"
    "utilities/slist.c"

    "main.c"

    INCLUDE_DIRS ".")

spiffs_create_partition_image("spiffs_0" "data" FLASH_IN_PROJECT)