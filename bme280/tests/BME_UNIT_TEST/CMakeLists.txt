cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(bme280_test)

# Define the root directory
get_filename_component(LAB_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)

target_sources(app PRIVATE src/test_bme280.c)
target_include_directories(app PRIVATE ${ZEPHYR_BASE}/drivers/sensor/bme280)

