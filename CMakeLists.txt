# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

message("ESP_PLATFORM: ${ESP_PLATFORM}")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(tv-lift)

# set(CMAKE_VERBOSE_MAKEFILE ON)
# set(CMAKE_C_STANDARD_REQUIRED ON)
# set(CMAKE_C_STANDARD 11)

# Create update file after bin file has been created
idf_build_get_property(build_dir BUILD_DIR)
add_custom_target(create-ota-file ALL
    COMMAND python concat.py ${build_dir}/spiffs_0.bin ${build_dir}/tv-lift.bin ${build_dir}/ota.bin
    DEPENDS "${build_dir}/.bin_timestamp"
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    VERBATIM)