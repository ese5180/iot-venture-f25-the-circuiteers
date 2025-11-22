cmake_minimum_required(VERSION 3.20)
project(sensors_mock_unit_tests C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -O2 -coverage")

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/unit
)

# Mock sensor library (shared code)
set(MOCK_SOURCES
    unit/mock_sensor.c
)

add_library(mock_sensor STATIC ${MOCK_SOURCES})

# BME280 Mock Test
add_executable(test_bme280_mock 
    unit/test_bme280_mock.c
)
target_link_libraries(test_bme280_mock mock_sensor)

# ADXL345 Mock Test
add_executable(test_adxl345_mock
    unit/test_adxl345_mock.c
)
target_link_libraries(test_adxl345_mock mock_sensor)

# Enable testing
enable_testing()
add_test(NAME test_bme280_mock COMMAND test_bme280_mock)
add_test(NAME test_adxl345_mock COMMAND test_adxl345_mock)

# Print info
message(STATUS "Mock Unit Tests configured - can run without hardware!")
message(STATUS "Build: cmake --build . && ctest")
