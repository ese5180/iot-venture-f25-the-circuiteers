/*
 * Mock Sensor Implementation
 * 
 * Provides mock implementations of sensor and I2C interfaces
 * for testing without hardware
 */

#include "../include/mock_sensor.h"
#include <string.h>
#include <stdio.h>

/* ==================== Global Mock State ==================== */

static struct {
    bool device_not_found;
    bool read_failure;
    
    /* ADXL345 mock values */
    struct {
        int32_t x;
        int32_t y;
        int32_t z;
    } adxl345;
    
    /* BME280 mock values */
    struct {
        int32_t temp;
        int32_t pressure;
        int32_t humidity;
    } bme280;
} mock_state = {
    .device_not_found = false,
    .read_failure = false,
    .adxl345 = {.x = -5, .y = -1, .z = 16},      /* Typical values */
    .bme280 = {.temp = 25, .pressure = 101325, .humidity = 52},
};

/* ==================== Mock Device Instances ==================== */

static struct device mock_bme280_device = {
    .name = "bme280@77",
    .data = NULL,
};

static struct device mock_adxl345_device = {
    .name = "adxl345@53",
    .data = NULL,
};

/* ==================== Public API - Device Functions ==================== */

struct device *mock_bme280_get_device(void)
{
    if (mock_state.device_not_found) {
        return NULL;
    }
    return &mock_bme280_device;
}

struct device *mock_adxl345_get_device(void)
{
    if (mock_state.device_not_found) {
        return NULL;
    }
    return &mock_adxl345_device;
}

bool mock_device_is_ready(const struct device *dev)
{
    if (dev == NULL) {
        return false;
    }
    return !mock_state.device_not_found;
}

/* ==================== Public API - Sensor Functions ==================== */

int mock_sensor_sample_fetch(const struct device *dev)
{
    if (mock_state.read_failure) {
        return -1;  /* EIO equivalent */
    }
    if (dev == NULL) {
        return -1;
    }
    return 0;  /* Success */
}

int mock_sensor_channel_get(const struct device *dev,
                            sensor_channel_t chan,
                            struct sensor_value *val)
{
    if (mock_state.read_failure || dev == NULL || val == NULL) {
        return -1;
    }
    
    /* Return mock values based on device type */
    if (dev == &mock_adxl345_device) {
        switch (chan) {
            case SENSOR_CHAN_ACCEL_X:
                val->val1 = mock_state.adxl345.x;
                val->val2 = 0;
                break;
            case SENSOR_CHAN_ACCEL_Y:
                val->val1 = mock_state.adxl345.y;
                val->val2 = 0;
                break;
            case SENSOR_CHAN_ACCEL_Z:
                val->val1 = mock_state.adxl345.z;
                val->val2 = 0;
                break;
            default:
                return -1;
        }
    } else if (dev == &mock_bme280_device) {
        /* BME280 would need decoder for proper implementation */
        return 0;
    }
    
    return 0;
}

int mock_sensor_read(const struct device *dev,
                     uint8_t *buffer,
                     size_t buf_size)
{
    if (mock_state.read_failure || dev == NULL || buffer == NULL) {
        return -1;
    }
    if (buf_size < 4) {
        return -1;
    }
    
    /* Fill mock buffer with dummy data */
    memset(buffer, 0xAA, buf_size);
    
    return 0;
}

int mock_sensor_get_decoder(const struct device *dev,
                            const struct sensor_decoder_api **decoder)
{
    if (mock_state.read_failure || dev == NULL || decoder == NULL) {
        return -1;
    }
    
    /* Return dummy decoder pointer (won't be called in these tests) */
    static struct sensor_decoder_api dummy_decoder = {
        .decode = NULL,
    };
    
    *decoder = &dummy_decoder;
    return 0;
}

/* ==================== Public API - Configuration Functions ==================== */

void mock_adxl345_set_values(int32_t x, int32_t y, int32_t z)
{
    mock_state.adxl345.x = x;
    mock_state.adxl345.y = y;
    mock_state.adxl345.z = z;
}

void mock_bme280_set_values(int32_t temp, int32_t pressure, int32_t humidity)
{
    mock_state.bme280.temp = temp;
    mock_state.bme280.pressure = pressure;
    mock_state.bme280.humidity = humidity;
}

void mock_device_set_not_found(bool not_found)
{
    mock_state.device_not_found = not_found;
}

void mock_sensor_set_read_failure(bool failure)
{
    mock_state.read_failure = failure;
}

void mock_sensor_reset_all(void)
{
    mock_state.device_not_found = false;
    mock_state.read_failure = false;
    mock_state.adxl345.x = -5;
    mock_state.adxl345.y = -1;
    mock_state.adxl345.z = 16;
    mock_state.bme280.temp = 25;
    mock_state.bme280.pressure = 101325;
    mock_state.bme280.humidity = 52;
}
