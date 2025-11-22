/*
 * Test Suite Setup
 * 
 * Defines the common test suite for all sensor unit tests
 */

#include <zephyr/ztest.h>

/**
 * Test suite for sensors_test
 * Contains all unit tests for BME280 and ADXL345 sensors
 */
ZTEST_SUITE(sensors_test_suite, NULL, NULL, NULL, NULL, NULL);
