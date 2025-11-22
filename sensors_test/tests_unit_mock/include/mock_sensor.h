/*
 * Mock Sensor Interface
 * 
 * Provides mock implementations of sensor device and I2C interfaces
 * for unit testing without hardware
 */

#ifndef MOCK_SENSOR_H
#define MOCK_SENSOR_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>

/* Sensor channel types */
typedef enum {
    SENSOR_CHAN_AMBIENT_TEMP = 0,
    SENSOR_CHAN_PRESS = 1,
    SENSOR_CHAN_HUMIDITY = 2,
    SENSOR_CHAN_ACCEL_X = 3,
    SENSOR_CHAN_ACCEL_Y = 4,
    SENSOR_CHAN_ACCEL_Z = 5,
} sensor_channel_t;

/* Sensor value structure - Q31 or regular */
struct sensor_value {
    int32_t val1;  /* Integer part */
    int32_t val2;  /* Fractional part (1/1000000) */
};

/* Mock device structure */
struct device {
    const char *name;
    void *data;
};

/* Mock decoder structure */
struct sensor_decoder_api {
    int (*decode)(const uint8_t *buffer,
                  struct sensor_value *channels,
                  uint32_t *fit,
                  uint8_t num_channels,
                  void *out);
};

/* Mock I2C spec */
struct i2c_dt_spec {
    struct device *bus;
    uint16_t addr;
};

/* ==================== Mock Device Functions ==================== */

/**
 * Get mock BME280 device
 */
struct device *mock_bme280_get_device(void);

/**
 * Get mock ADXL345 device
 */
struct device *mock_adxl345_get_device(void);

/**
 * Check if device is ready (always true for mocks)
 */
bool mock_device_is_ready(const struct device *dev);

/* ==================== Mock Sensor API ==================== */

/**
 * Mock sensor_sample_fetch - always returns 0
 */
int mock_sensor_sample_fetch(const struct device *dev);

/**
 * Mock sensor_channel_get - returns preset test values
 */
int mock_sensor_channel_get(const struct device *dev,
                            sensor_channel_t chan,
                            struct sensor_value *val);

/**
 * Mock sensor_read - reads from mock buffer
 */
int mock_sensor_read(const struct device *dev,
                     uint8_t *buffer,
                     size_t buf_size);

/**
 * Mock sensor_get_decoder - returns mock decoder
 */
int mock_sensor_get_decoder(const struct device *dev,
                            const struct sensor_decoder_api **decoder);

/* ==================== Mock Configuration ==================== */

/**
 * Set test values for ADXL345 readings
 * Used to simulate different sensor orientations and movements
 */
void mock_adxl345_set_values(int32_t x, int32_t y, int32_t z);

/**
 * Set test values for BME280 readings
 * Used to simulate different environmental conditions
 */
void mock_bme280_set_values(int32_t temp, int32_t pressure, int32_t humidity);

/**
 * Configure mock to simulate device not found
 */
void mock_device_set_not_found(bool not_found);

/**
 * Configure mock to simulate read failure
 */
void mock_sensor_set_read_failure(bool failure);

/**
 * Reset all mocks to default state
 */
void mock_sensor_reset_all(void);

#endif /* MOCK_SENSOR_H */
