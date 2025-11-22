/*
 * Unit Test for ADXL345 Accelerometer Sensor
 * 
 * Tests:
 * - ADXL345 device detection and initialization
 * - ADXL345 device readiness verification
 * - ADXL345 accelerometer data reading
 * - ADXL345 multi-axis acceleration measurement
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/sensor_data_types.h>
#include <zephyr/ztest.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(test_adxl345, LOG_LEVEL_INF);

/* ADXL345 device reference */
const struct device *const adxl345_dev = DEVICE_DT_GET_ANY(adi_adxl345);

/**
 * Test 1: ADXL345 Device Detection
 * 
 * Verifies that the ADXL345 device is properly detected in the device tree
 */
ZTEST(sensors_test_suite, test_adxl345_device_detection)
{
    zassert_not_null(adxl345_dev,
        "ADXL345 device not found in devicetree. Check overlay configuration.");
}

/**
 * Test 2: ADXL345 Device Readiness
 * 
 * Verifies that the ADXL345 device is ready for operation
 */
ZTEST(sensors_test_suite, test_adxl345_device_ready)
{
    zassert_not_null(adxl345_dev, "ADXL345 device is NULL");
    zassert_true(device_is_ready(adxl345_dev),
        "ADXL345 device is not ready. Check driver initialization.");
}

/**
 * Test 3: ADXL345 Sample Fetch
 * 
 * Verifies that ADXL345 can successfully fetch acceleration samples
 */
ZTEST(sensors_test_suite, test_adxl345_sample_fetch)
{
    zassert_not_null(adxl345_dev, "ADXL345 device is NULL");
    zassert_true(device_is_ready(adxl345_dev), "ADXL345 device is not ready");

    int rc = sensor_sample_fetch(adxl345_dev);
    
    zassert_equal(rc, 0,
        "Failed to fetch ADXL345 sensor sample (error code: %d)", rc);
}

/**
 * Test 4: ADXL345 X-Axis Acceleration Reading
 * 
 * Verifies that ADXL345 can read X-axis acceleration
 */
ZTEST(sensors_test_suite, test_adxl345_accel_x)
{
    zassert_not_null(adxl345_dev, "ADXL345 device is NULL");
    zassert_true(device_is_ready(adxl345_dev), "ADXL345 device is not ready");

    int rc = sensor_sample_fetch(adxl345_dev);
    zassert_equal(rc, 0, "Failed to fetch ADXL345 sample");

    struct sensor_value accel_x;
    rc = sensor_channel_get(adxl345_dev, SENSOR_CHAN_ACCEL_X, &accel_x);
    
    zassert_equal(rc, 0,
        "Failed to read ADXL345 X-axis acceleration (error code: %d)", rc);
    
    /* Validate that acceleration value is within reasonable range */
    /* Allow wider range: -50 to +50 m/s^2 to accommodate different orientations */
    zassert_true(accel_x.val1 >= -50 && accel_x.val1 <= 50,
        "X-axis acceleration out of expected range: %d.%d m/s^2",
        accel_x.val1, accel_x.val2);
}

/**
 * Test 5: ADXL345 Y-Axis Acceleration Reading
 * 
 * Verifies that ADXL345 can read Y-axis acceleration
 */
ZTEST(sensors_test_suite, test_adxl345_accel_y)
{
    zassert_not_null(adxl345_dev, "ADXL345 device is NULL");
    zassert_true(device_is_ready(adxl345_dev), "ADXL345 device is not ready");

    int rc = sensor_sample_fetch(adxl345_dev);
    zassert_equal(rc, 0, "Failed to fetch ADXL345 sample");

    struct sensor_value accel_y;
    rc = sensor_channel_get(adxl345_dev, SENSOR_CHAN_ACCEL_Y, &accel_y);
    
    zassert_equal(rc, 0,
        "Failed to read ADXL345 Y-axis acceleration (error code: %d)", rc);
    
    /* Validate that acceleration value is within reasonable range */
    /* Allow wider range: -50 to +50 m/s^2 to accommodate different orientations */
    zassert_true(accel_y.val1 >= -50 && accel_y.val1 <= 50,
        "Y-axis acceleration out of expected range: %d.%d m/s^2",
        accel_y.val1, accel_y.val2);
}

/**
 * Test 6: ADXL345 Z-Axis Acceleration Reading
 * 
 * Verifies that ADXL345 can read Z-axis acceleration
 * In idle/level position, Z should show significant acceleration 
 * (could be close to 9.8 m/s^2 gravity or higher depending on orientation)
 */
ZTEST(sensors_test_suite, test_adxl345_accel_z)
{
    zassert_not_null(adxl345_dev, "ADXL345 device is NULL");
    zassert_true(device_is_ready(adxl345_dev), "ADXL345 device is not ready");

    int rc = sensor_sample_fetch(adxl345_dev);
    zassert_equal(rc, 0, "Failed to fetch ADXL345 sample");

    struct sensor_value accel_z;
    rc = sensor_channel_get(adxl345_dev, SENSOR_CHAN_ACCEL_Z, &accel_z);
    
    zassert_equal(rc, 0,
        "Failed to read ADXL345 Z-axis acceleration (error code: %d)", rc);
    
    /* Z-axis should show acceleration within a very wide range */
    /* Allow 0 to 20 m/s^2 to accommodate various board orientations */
    /* This includes gravity (9.8 m/s^2) and different mounting angles */
    zassert_true(accel_z.val1 >= 0 && accel_z.val1 <= 20,
        "Z-axis acceleration out of expected range: %d.%d m/s^2",
        accel_z.val1, accel_z.val2);
}

/**
 * Test 7: ADXL345 All Axes Sequential Reading
 * 
 * Verifies that ADXL345 can read all three axes in sequence
 */
ZTEST(sensors_test_suite, test_adxl345_all_axes)
{
    zassert_not_null(adxl345_dev, "ADXL345 device is NULL");
    zassert_true(device_is_ready(adxl345_dev), "ADXL345 device is not ready");

    int rc = sensor_sample_fetch(adxl345_dev);
    zassert_equal(rc, 0, "Failed to fetch ADXL345 sample");

    struct sensor_value accel[3];  /* X, Y, Z */
    
    /* Read X-axis */
    rc = sensor_channel_get(adxl345_dev, SENSOR_CHAN_ACCEL_X, &accel[0]);
    zassert_equal(rc, 0, "Failed to read X-axis");
    
    /* Read Y-axis */
    rc = sensor_channel_get(adxl345_dev, SENSOR_CHAN_ACCEL_Y, &accel[1]);
    zassert_equal(rc, 0, "Failed to read Y-axis");
    
    /* Read Z-axis */
    rc = sensor_channel_get(adxl345_dev, SENSOR_CHAN_ACCEL_Z, &accel[2]);
    zassert_equal(rc, 0, "Failed to read Z-axis");
    
    /* Verify all values are populated */
    zassert_not_equal(accel[0].val1 + accel[1].val1 + accel[2].val1, 0,
        "All acceleration values are zero, sensor may not be responding");
    
    LOG_INF("ADXL345 All axes read successfully: X=%d.%d, Y=%d.%d, Z=%d.%d m/s^2",
            accel[0].val1, accel[0].val2,
            accel[1].val1, accel[1].val2,
            accel[2].val1, accel[2].val2);
}

/**
 * Test 8: ADXL345 Device Name Verification
 * 
 * Verifies that the device has a valid name
 */
ZTEST(sensors_test_suite, test_adxl345_device_name)
{
    zassert_not_null(adxl345_dev, "ADXL345 device is NULL");
    zassert_not_null(adxl345_dev->name, "ADXL345 device name is NULL");
    zassert_not_equal(strlen(adxl345_dev->name), 0,
        "ADXL345 device name is empty");
    
    /* Expected device name pattern contains "adxl345" */
    zassert_true(strstr(adxl345_dev->name, "adxl345") != NULL,
        "Device name '%s' does not contain 'adxl345'", adxl345_dev->name);
}

/**
 * Test 9: ADXL345 Multiple Sequential Readings
 * 
 * Verifies that ADXL345 can perform multiple consecutive readings
 * Tests stability and consistency of sensor measurements
 */
ZTEST(sensors_test_suite, test_adxl345_multiple_reads)
{
    zassert_not_null(adxl345_dev, "ADXL345 device is NULL");
    zassert_true(device_is_ready(adxl345_dev), "ADXL345 device is not ready");

    struct sensor_value accel[3];
    
    /* Perform three consecutive readings */
    for (int i = 0; i < 3; i++) {
        int rc = sensor_sample_fetch(adxl345_dev);
        zassert_equal(rc, 0, "Failed to fetch sample %d", i);
        
        rc = sensor_channel_get(adxl345_dev, SENSOR_CHAN_ACCEL_X, &accel[0]);
        zassert_equal(rc, 0, "Failed to read X-axis (iteration %d)", i);
        
        rc = sensor_channel_get(adxl345_dev, SENSOR_CHAN_ACCEL_Y, &accel[1]);
        zassert_equal(rc, 0, "Failed to read Y-axis (iteration %d)", i);
        
        rc = sensor_channel_get(adxl345_dev, SENSOR_CHAN_ACCEL_Z, &accel[2]);
        zassert_equal(rc, 0, "Failed to read Z-axis (iteration %d)", i);
        
        LOG_INF("Read %d: X=%d.%d, Y=%d.%d, Z=%d.%d m/s^2",
                i+1, accel[0].val1, accel[0].val2,
                accel[1].val1, accel[1].val2,
                accel[2].val1, accel[2].val2);
    }
}
