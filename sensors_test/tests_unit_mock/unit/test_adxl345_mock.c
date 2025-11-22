/*
 * ADXL345 Mock Unit Tests
 * 
 * Tests ADXL345 accelerometer functionality using mocked sensor interfaces
 * Can run on any platform without hardware or Zephyr
 */

#include "../include/mock_sensor.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* ==================== Test Utilities ==================== */

#define TEST_PASS(name) printf("✓ PASS: %s\n", name)
#define TEST_FAIL(name, msg) printf("✗ FAIL: %s - %s\n", name, msg)
#define ASSERT_TRUE(cond, msg) \
    if (!(cond)) { fprintf(stderr, "Assertion failed: %s\n", msg); return 1; }
#define ASSERT_EQUAL(a, b, msg) \
    if ((a) != (b)) { fprintf(stderr, "Assertion failed: %s (%d != %d)\n", msg, a, b); return 1; }
#define ASSERT_NOT_NULL(ptr, msg) \
    if ((ptr) == NULL) { fprintf(stderr, "Assertion failed: %s (NULL pointer)\n", msg); return 1; }
#define ASSERT_RANGE(val, min, max, msg) \
    if ((val) < (min) || (val) > (max)) { fprintf(stderr, "Assertion failed: %s (%d not in [%d,%d])\n", msg, val, min, max); return 1; }

/* ==================== Test Functions ==================== */

/**
 * Test 1: ADXL345 Device Detection
 */
int test_adxl345_device_detection(void)
{
    printf("\n[TEST 1] ADXL345 Device Detection\n");
    
    struct device *dev = mock_adxl345_get_device();
    ASSERT_NOT_NULL(dev, "ADXL345 device should be detected");
    
    TEST_PASS("test_adxl345_device_detection");
    return 0;
}

/**
 * Test 2: ADXL345 Device Not Found (Negative Case)
 */
int test_adxl345_device_not_found(void)
{
    printf("\n[TEST 2] ADXL345 Device Not Found\n");
    
    /* Simulate device not found */
    mock_device_set_not_found(true);
    
    struct device *dev = mock_adxl345_get_device();
    ASSERT_TRUE(dev == NULL, "Device should be NULL when not found");
    
    /* Reset */
    mock_device_set_not_found(false);
    
    TEST_PASS("test_adxl345_device_not_found");
    return 0;
}

/**
 * Test 3: ADXL345 Device Readiness
 */
int test_adxl345_device_ready(void)
{
    printf("\n[TEST 3] ADXL345 Device Readiness\n");
    
    struct device *dev = mock_adxl345_get_device();
    ASSERT_NOT_NULL(dev, "Device should exist");
    
    bool ready = mock_device_is_ready(dev);
    ASSERT_TRUE(ready, "Device should be ready");
    
    TEST_PASS("test_adxl345_device_ready");
    return 0;
}

/**
 * Test 4: ADXL345 Device Name Verification
 */
int test_adxl345_device_name(void)
{
    printf("\n[TEST 4] ADXL345 Device Name\n");
    
    struct device *dev = mock_adxl345_get_device();
    ASSERT_NOT_NULL(dev, "Device should exist");
    ASSERT_NOT_NULL(dev->name, "Device name should not be NULL");
    
    bool has_name = strstr(dev->name, "adxl345") != NULL;
    ASSERT_TRUE(has_name, "Device name should contain 'adxl345'");
    
    printf("   Device name: %s\n", dev->name);
    TEST_PASS("test_adxl345_device_name");
    return 0;
}

/**
 * Test 5: ADXL345 Sample Fetch
 */
int test_adxl345_sample_fetch(void)
{
    printf("\n[TEST 5] ADXL345 Sample Fetch\n");
    
    struct device *dev = mock_adxl345_get_device();
    ASSERT_NOT_NULL(dev, "Device should exist");
    
    int rc = mock_sensor_sample_fetch(dev);
    ASSERT_EQUAL(rc, 0, "Sample fetch should succeed");
    
    TEST_PASS("test_adxl345_sample_fetch");
    return 0;
}

/**
 * Test 6: ADXL345 X-Axis Read
 */
int test_adxl345_x_axis(void)
{
    printf("\n[TEST 6] ADXL345 X-Axis Read\n");
    
    struct device *dev = mock_adxl345_get_device();
    ASSERT_NOT_NULL(dev, "Device should exist");
    
    struct sensor_value val;
    int rc = mock_sensor_channel_get(dev, SENSOR_CHAN_ACCEL_X, &val);
    ASSERT_EQUAL(rc, 0, "Channel read should succeed");
    
    printf("   X-Axis: %d m/s²\n", val.val1);
    ASSERT_RANGE(val.val1, -50, 50, "X-axis should be in valid range");
    
    TEST_PASS("test_adxl345_x_axis");
    return 0;
}

/**
 * Test 7: ADXL345 Y-Axis Read
 */
int test_adxl345_y_axis(void)
{
    printf("\n[TEST 7] ADXL345 Y-Axis Read\n");
    
    struct device *dev = mock_adxl345_get_device();
    ASSERT_NOT_NULL(dev, "Device should exist");
    
    struct sensor_value val;
    int rc = mock_sensor_channel_get(dev, SENSOR_CHAN_ACCEL_Y, &val);
    ASSERT_EQUAL(rc, 0, "Channel read should succeed");
    
    printf("   Y-Axis: %d m/s²\n", val.val1);
    ASSERT_RANGE(val.val1, -50, 50, "Y-axis should be in valid range");
    
    TEST_PASS("test_adxl345_y_axis");
    return 0;
}

/**
 * Test 8: ADXL345 Z-Axis Read
 */
int test_adxl345_z_axis(void)
{
    printf("\n[TEST 8] ADXL345 Z-Axis Read\n");
    
    struct device *dev = mock_adxl345_get_device();
    ASSERT_NOT_NULL(dev, "Device should exist");
    
    struct sensor_value val;
    int rc = mock_sensor_channel_get(dev, SENSOR_CHAN_ACCEL_Z, &val);
    ASSERT_EQUAL(rc, 0, "Channel read should succeed");
    
    printf("   Z-Axis: %d m/s²\n", val.val1);
    ASSERT_RANGE(val.val1, 0, 20, "Z-axis should be in valid range");
    
    TEST_PASS("test_adxl345_z_axis");
    return 0;
}

/**
 * Test 9: ADXL345 All Axes Sequential Read
 */
int test_adxl345_all_axes_sequential(void)
{
    printf("\n[TEST 9] ADXL345 All Axes Sequential Read\n");
    
    struct device *dev = mock_adxl345_get_device();
    ASSERT_NOT_NULL(dev, "Device should exist");
    
    struct sensor_value x_val, y_val, z_val;
    
    int rc = mock_sensor_channel_get(dev, SENSOR_CHAN_ACCEL_X, &x_val);
    ASSERT_EQUAL(rc, 0, "X-axis read should succeed");
    
    rc = mock_sensor_channel_get(dev, SENSOR_CHAN_ACCEL_Y, &y_val);
    ASSERT_EQUAL(rc, 0, "Y-axis read should succeed");
    
    rc = mock_sensor_channel_get(dev, SENSOR_CHAN_ACCEL_Z, &z_val);
    ASSERT_EQUAL(rc, 0, "Z-axis read should succeed");
    
    printf("   X: %d, Y: %d, Z: %d m/s²\n", x_val.val1, y_val.val1, z_val.val1);
    
    TEST_PASS("test_adxl345_all_axes_sequential");
    return 0;
}

/**
 * Test 10: ADXL345 Custom Values
 */
int test_adxl345_custom_values(void)
{
    printf("\n[TEST 10] ADXL345 Custom Values\n");
    
    struct device *dev = mock_adxl345_get_device();
    ASSERT_NOT_NULL(dev, "Device should exist");
    
    /* Set custom values */
    mock_adxl345_set_values(10, -15, 18);
    
    struct sensor_value x_val, y_val, z_val;
    mock_sensor_channel_get(dev, SENSOR_CHAN_ACCEL_X, &x_val);
    mock_sensor_channel_get(dev, SENSOR_CHAN_ACCEL_Y, &y_val);
    mock_sensor_channel_get(dev, SENSOR_CHAN_ACCEL_Z, &z_val);
    
    ASSERT_EQUAL(x_val.val1, 10, "X-axis should match set value");
    ASSERT_EQUAL(y_val.val1, -15, "Y-axis should match set value");
    ASSERT_EQUAL(z_val.val1, 18, "Z-axis should match set value");
    
    printf("   Custom values verified: X=%d, Y=%d, Z=%d\n", x_val.val1, y_val.val1, z_val.val1);
    
    /* Reset to defaults */
    mock_sensor_reset_all();
    
    TEST_PASS("test_adxl345_custom_values");
    return 0;
}

/**
 * Test 11: ADXL345 Read Failure Scenario
 */
int test_adxl345_read_failure(void)
{
    printf("\n[TEST 11] ADXL345 Read Failure\n");
    
    struct device *dev = mock_adxl345_get_device();
    ASSERT_NOT_NULL(dev, "Device should exist");
    
    /* Simulate read failure */
    mock_sensor_set_read_failure(true);
    
    int rc = mock_sensor_sample_fetch(dev);
    ASSERT_EQUAL(rc, -1, "Sample fetch should fail");
    
    /* Reset */
    mock_sensor_set_read_failure(false);
    
    TEST_PASS("test_adxl345_read_failure");
    return 0;
}

/**
 * Test 12: ADXL345 Multiple Reads
 */
int test_adxl345_multiple_reads(void)
{
    printf("\n[TEST 12] ADXL345 Multiple Consecutive Reads\n");
    
    struct device *dev = mock_adxl345_get_device();
    ASSERT_NOT_NULL(dev, "Device should exist");
    
    struct sensor_value val;
    for (int i = 0; i < 5; i++) {
        int rc = mock_sensor_channel_get(dev, SENSOR_CHAN_ACCEL_X, &val);
        ASSERT_EQUAL(rc, 0, "All reads should succeed");
    }
    
    printf("   Successfully performed 5 consecutive reads\n");
    TEST_PASS("test_adxl345_multiple_reads");
    return 0;
}

/* ==================== Test Runner ==================== */

int main(void)
{
    printf("╔════════════════════════════════════════╗\n");
    printf("║  ADXL345 Mock Unit Tests               ║\n");
    printf("║  No Hardware Required                  ║\n");
    printf("╚════════════════════════════════════════╝\n");
    
    int failed = 0;
    
    /* Reset mock state before all tests */
    mock_sensor_reset_all();
    
    /* Run all tests */
    failed += test_adxl345_device_detection();
    failed += test_adxl345_device_not_found();
    failed += test_adxl345_device_ready();
    failed += test_adxl345_device_name();
    failed += test_adxl345_sample_fetch();
    failed += test_adxl345_x_axis();
    failed += test_adxl345_y_axis();
    failed += test_adxl345_z_axis();
    failed += test_adxl345_all_axes_sequential();
    failed += test_adxl345_custom_values();
    failed += test_adxl345_read_failure();
    failed += test_adxl345_multiple_reads();
    
    /* Summary */
    printf("\n╔════════════════════════════════════════╗\n");
    if (failed == 0) {
        printf("║  ✓ ALL TESTS PASSED                   ║\n");
    } else {
        printf("║  ✗ %d TEST(S) FAILED                 ║\n", failed);
    }
    printf("╚════════════════════════════════════════╝\n");
    
    return failed;
}
