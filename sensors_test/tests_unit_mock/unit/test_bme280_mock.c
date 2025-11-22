/*
 * BME280 Mock Unit Tests
 * 
 * Tests BME280 functionality using mocked sensor interfaces
 * Can run on any platform without hardware or Zephyr
 */

#include "../include/mock_sensor.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ==================== Test Utilities ==================== */

#define TEST_PASS(name) printf("✓ PASS: %s\n", name)
#define TEST_FAIL(name, msg) printf("✗ FAIL: %s - %s\n", name, msg)
#define ASSERT_TRUE(cond, msg) \
    if (!(cond)) { fprintf(stderr, "Assertion failed: %s\n", msg); return 1; }
#define ASSERT_EQUAL(a, b, msg) \
    if ((a) != (b)) { fprintf(stderr, "Assertion failed: %s (%d != %d)\n", msg, a, b); return 1; }
#define ASSERT_NOT_NULL(ptr, msg) \
    if ((ptr) == NULL) { fprintf(stderr, "Assertion failed: %s (NULL pointer)\n", msg); return 1; }

/* ==================== Test Functions ==================== */

/**
 * Test 1: BME280 Device Detection
 */
int test_bme280_device_detection(void)
{
    printf("\n[TEST 1] BME280 Device Detection\n");
    
    struct device *dev = mock_bme280_get_device();
    ASSERT_NOT_NULL(dev, "BME280 device should be detected");
    
    TEST_PASS("test_bme280_device_detection");
    return 0;
}

/**
 * Test 2: BME280 Device Not Found (Negative Case)
 */
int test_bme280_device_not_found(void)
{
    printf("\n[TEST 2] BME280 Device Not Found\n");
    
    /* Simulate device not found */
    mock_device_set_not_found(true);
    
    struct device *dev = mock_bme280_get_device();
    ASSERT_TRUE(dev == NULL, "Device should be NULL when not found");
    
    /* Reset */
    mock_device_set_not_found(false);
    
    TEST_PASS("test_bme280_device_not_found");
    return 0;
}

/**
 * Test 3: BME280 Device Readiness
 */
int test_bme280_device_ready(void)
{
    printf("\n[TEST 3] BME280 Device Readiness\n");
    
    struct device *dev = mock_bme280_get_device();
    ASSERT_NOT_NULL(dev, "Device should exist");
    
    bool ready = mock_device_is_ready(dev);
    ASSERT_TRUE(ready, "Device should be ready");
    
    TEST_PASS("test_bme280_device_ready");
    return 0;
}

/**
 * Test 4: BME280 Device Name Verification
 */
int test_bme280_device_name(void)
{
    printf("\n[TEST 4] BME280 Device Name\n");
    
    struct device *dev = mock_bme280_get_device();
    ASSERT_NOT_NULL(dev, "Device should exist");
    ASSERT_NOT_NULL(dev->name, "Device name should not be NULL");
    
    bool has_name = strstr(dev->name, "bme280") != NULL;
    ASSERT_TRUE(has_name, "Device name should contain 'bme280'");
    
    printf("   Device name: %s\n", dev->name);
    TEST_PASS("test_bme280_device_name");
    return 0;
}

/**
 * Test 5: BME280 Sensor Read
 */
int test_bme280_sensor_read(void)
{
    printf("\n[TEST 5] BME280 Sensor Read\n");
    
    struct device *dev = mock_bme280_get_device();
    ASSERT_NOT_NULL(dev, "Device should exist");
    
    uint8_t buffer[128];
    int rc = mock_sensor_read(dev, buffer, sizeof(buffer));
    ASSERT_EQUAL(rc, 0, "Read should succeed");
    
    TEST_PASS("test_bme280_sensor_read");
    return 0;
}

/**
 * Test 6: BME280 Read Failure Scenario
 */
int test_bme280_read_failure(void)
{
    printf("\n[TEST 6] BME280 Read Failure\n");
    
    struct device *dev = mock_bme280_get_device();
    ASSERT_NOT_NULL(dev, "Device should exist");
    
    /* Simulate read failure */
    mock_sensor_set_read_failure(true);
    
    uint8_t buffer[128];
    int rc = mock_sensor_read(dev, buffer, sizeof(buffer));
    ASSERT_EQUAL(rc, -1, "Read should fail");
    
    /* Reset */
    mock_sensor_set_read_failure(false);
    
    TEST_PASS("test_bme280_read_failure");
    return 0;
}

/**
 * Test 7: BME280 Decoder API
 */
int test_bme280_decoder(void)
{
    printf("\n[TEST 7] BME280 Decoder API\n");
    
    struct device *dev = mock_bme280_get_device();
    ASSERT_NOT_NULL(dev, "Device should exist");
    
    const struct sensor_decoder_api *decoder = NULL;
    int rc = mock_sensor_get_decoder(dev, &decoder);
    
    ASSERT_EQUAL(rc, 0, "Decoder retrieval should succeed");
    ASSERT_NOT_NULL(decoder, "Decoder should not be NULL");
    
    TEST_PASS("test_bme280_decoder");
    return 0;
}

/**
 * Test 8: BME280 Multiple Reads
 */
int test_bme280_multiple_reads(void)
{
    printf("\n[TEST 8] BME280 Multiple Consecutive Reads\n");
    
    struct device *dev = mock_bme280_get_device();
    ASSERT_NOT_NULL(dev, "Device should exist");
    
    uint8_t buffer[128];
    for (int i = 0; i < 5; i++) {
        int rc = mock_sensor_read(dev, buffer, sizeof(buffer));
        ASSERT_EQUAL(rc, 0, "All reads should succeed");
    }
    
    printf("   Successfully performed 5 consecutive reads\n");
    TEST_PASS("test_bme280_multiple_reads");
    return 0;
}

/* ==================== Test Runner ==================== */

int main(void)
{
    printf("╔════════════════════════════════════════╗\n");
    printf("║  BME280 Mock Unit Tests                ║\n");
    printf("║  No Hardware Required                  ║\n");
    printf("╚════════════════════════════════════════╝\n");
    
    int failed = 0;
    
    /* Reset mock state before all tests */
    mock_sensor_reset_all();
    
    /* Run all tests */
    failed += test_bme280_device_detection();
    failed += test_bme280_device_not_found();
    failed += test_bme280_device_ready();
    failed += test_bme280_device_name();
    failed += test_bme280_sensor_read();
    failed += test_bme280_read_failure();
    failed += test_bme280_decoder();
    failed += test_bme280_multiple_reads();
    
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
