cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sensors_test_unit_tests)

# Include test source files
target_sources(app PRIVATE
    unit/test_suite.c
    unit/test_bme280.c
    unit/test_adxl345.c
)
