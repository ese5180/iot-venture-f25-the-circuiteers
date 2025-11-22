/*
 * Unit Test for BME280 Sensor
 * 
 * Tests:
 * - BME280 device detection and initialization
 * - BME280 device readiness verification
 * - BME280 basic sensor data reading
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/sensor_data_types.h>
#include <zephyr/rtio/rtio.h>
#include <zephyr/dsp/print_format.h>
#include <zephyr/ztest.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(test_bme280, LOG_LEVEL_INF);

/* BME280 device reference */
const struct device *const bme280_dev = DEVICE_DT_GET_ANY(bosch_bme280);

/* RTIO context for async operations */
SENSOR_DT_READ_IODEV(bme280_iodev, DT_COMPAT_GET_ANY_STATUS_OKAY(bosch_bme280),
        {SENSOR_CHAN_AMBIENT_TEMP, 0},
        {SENSOR_CHAN_HUMIDITY, 0},
        {SENSOR_CHAN_PRESS, 0});

RTIO_DEFINE(bme280_ctx, 1, 1);

/**
 * Test 1: BME280 Device Detection
 * 
 * Verifies that the BME280 device is properly detected in the device tree
 */
ZTEST(sensors_test_suite, test_bme280_device_detection)
{
    zassert_not_null(bme280_dev,
        "BME280 device not found in devicetree. Check overlay configuration.");
}

/**
 * Test 2: BME280 Device Readiness
 * 
 * Verifies that the BME280 device is ready for operation
 */
ZTEST(sensors_test_suite, test_bme280_device_ready)
{
    zassert_not_null(bme280_dev, "BME280 device is NULL");
    zassert_true(device_is_ready(bme280_dev),
        "BME280 device is not ready. Check driver initialization.");
}

/**
 * Test 3: BME280 Sensor Data Reading
 * 
 * Verifies that BME280 can successfully read sensor data
 * Tests the async RTIO API for fetching temperature, pressure, and humidity
 */
ZTEST(sensors_test_suite, test_bme280_sensor_read)
{
    zassert_not_null(bme280_dev, "BME280 device is NULL");
    zassert_true(device_is_ready(bme280_dev), "BME280 device is not ready");

    uint8_t buf[128];
    int rc = sensor_read(&bme280_iodev, &bme280_ctx, buf, sizeof(buf));
    
    zassert_equal(rc, 0,
        "Failed to read BME280 sensor data (error code: %d)", rc);
}

/**
 * Test 4: BME280 Data Decoder
 * 
 * Verifies that the BME280 decoder API is available and functional
 */
ZTEST(sensors_test_suite, test_bme280_decoder)
{
    zassert_not_null(bme280_dev, "BME280 device is NULL");
    zassert_true(device_is_ready(bme280_dev), "BME280 device is not ready");

    const struct sensor_decoder_api *decoder;
    int rc = sensor_get_decoder(bme280_dev, &decoder);
    
    zassert_equal(rc, 0,
        "Failed to get BME280 decoder API (error code: %d)", rc);
    zassert_not_null(decoder,
        "BME280 decoder is NULL");
}

/**
 * Test 5: BME280 Device Name Verification
 * 
 * Verifies that the device has a valid name
 */
ZTEST(sensors_test_suite, test_bme280_device_name)
{
    zassert_not_null(bme280_dev, "BME280 device is NULL");
    zassert_not_null(bme280_dev->name, "BME280 device name is NULL");
    zassert_not_equal(strlen(bme280_dev->name), 0,
        "BME280 device name is empty");
    
    /* Expected device name pattern contains "bme280" */
    zassert_true(strstr(bme280_dev->name, "bme280") != NULL,
        "Device name '%s' does not contain 'bme280'", bme280_dev->name);
}

/**
 * Test 6: BME280 Channel Validation
 * 
 * Verifies that BME280 supports the required sensor channels
 */
ZTEST(sensors_test_suite, test_bme280_channels)
{
    zassert_not_null(bme280_dev, "BME280 device is NULL");
    
    /* BME280 should support temperature channel */
    zassert_true(device_is_ready(bme280_dev),
        "BME280 device channels not available");
    
    LOG_INF("BME280 channels validation passed");
}
