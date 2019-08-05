idf_component_register(
    SRCS    
    "controllers/lift_controller.c"

    "lift/lift.c"

    "tasks/blink/blink_task.c"
    "tasks/webserver/webserver_task.c"

    "main.c"

    INCLUDE_DIRS ".")