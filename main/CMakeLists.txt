idf_component_register(
    SRCS    
    "controllers/controller_base.c"
    "controllers/lift_controller.c"
    "controllers/upload_controller.c"

    "lift/lift.c"

    "tasks/blink/blink_task.c"
    "tasks/webserver/webserver_task.c"

    "main.c"

    INCLUDE_DIRS ".")

spiffs_create_partition_image("spiffs" "data" FLASH_IN_PROJECT)