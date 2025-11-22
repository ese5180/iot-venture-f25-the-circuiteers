/*
 * Dual Sensor Reader for Nucleo WL55JC1 using Zephyr sensor API
 * 
 * Simultaneously reads:
 * - BME280 (Temperature, Pressure, Humidity) via I2C2
 * - ADXL345 (3-axis Acceleration) via I2C3
 * 
 * Outputs sensor data to UART console every second
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/sensor_data_types.h>
#include <zephyr/rtio/rtio.h>
#include <zephyr/dsp/print_format.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sensors_test, LOG_LEVEL_INF);

/* ==================== BME280 Setup ==================== */
const struct device *const bme280_dev = DEVICE_DT_GET_ANY(bosch_bme280);

SENSOR_DT_READ_IODEV(bme280_iodev, DT_COMPAT_GET_ANY_STATUS_OKAY(bosch_bme280),
        {SENSOR_CHAN_AMBIENT_TEMP, 0},
        {SENSOR_CHAN_HUMIDITY, 0},
        {SENSOR_CHAN_PRESS, 0});

RTIO_DEFINE(bme280_ctx, 1, 1);

/* ==================== ADXL345 Setup ==================== */
const struct device *const adxl345_dev = DEVICE_DT_GET_ANY(adi_adxl345);

/* ==================== Device Initialization ==================== */

/**
 * Initialize and verify BME280 sensor
 * Returns the device pointer or NULL if initialization failed
 */
static const struct device *init_bme280(void)
{
    if (bme280_dev == NULL) {
        LOG_ERR("BME280: Device not found in devicetree. Check overlay configuration.");
        return NULL;
    }

    if (!device_is_ready(bme280_dev)) {
        LOG_ERR("BME280: Device '%s' is not ready.", bme280_dev->name);
        return NULL;
    }

    LOG_INF("BME280: Initialized on I2C2 (PA11/PA12)");
    return bme280_dev;
}

/**
 * Initialize and verify ADXL345 sensor
 * Returns the device pointer or NULL if initialization failed
 */
static const struct device *init_adxl345(void)
{
    if (adxl345_dev == NULL) {
        LOG_ERR("ADXL345: Device not found in devicetree. Check overlay configuration.");
        return NULL;
    }

    if (!device_is_ready(adxl345_dev)) {
        LOG_ERR("ADXL345: Device '%s' is not ready.", adxl345_dev->name);
        return NULL;
    }

    LOG_INF("ADXL345: Initialized on I2C3 (PB13/PB14)");
    return adxl345_dev;
}

/* ==================== Sensor Reading Functions ==================== */

/**
 * Read BME280 sensor data (Temperature, Pressure, Humidity)
 * Returns 0 on success, error code on failure
 */
static int read_bme280_data(struct sensor_q31_data *temp_data,
                            struct sensor_q31_data *press_data,
                            struct sensor_q31_data *hum_data)
{
    uint8_t buf[128];
    
    /* Read sensor data asynchronously using RTIO */
    int rc = sensor_read(&bme280_iodev, &bme280_ctx, buf, sizeof(buf));
    if (rc != 0) {
        LOG_ERR("BME280: sensor_read() failed: %d", rc);
        return rc;
    }

    /* Get the decoder API */
    const struct sensor_decoder_api *decoder;
    rc = sensor_get_decoder(bme280_dev, &decoder);
    if (rc != 0) {
        LOG_ERR("BME280: sensor_get_decoder() failed: %d", rc);
        return rc;
    }

    /* Decode temperature data */
    uint32_t temp_fit = 0;
    decoder->decode(buf,
                    (struct sensor_chan_spec){SENSOR_CHAN_AMBIENT_TEMP, 0},
                    &temp_fit, 1, temp_data);

    /* Decode pressure data */
    uint32_t press_fit = 0;
    decoder->decode(buf,
                    (struct sensor_chan_spec){SENSOR_CHAN_PRESS, 0},
                    &press_fit, 1, press_data);

    /* Decode humidity data */
    uint32_t hum_fit = 0;
    decoder->decode(buf,
                    (struct sensor_chan_spec){SENSOR_CHAN_HUMIDITY, 0},
                    &hum_fit, 1, hum_data);

    return 0;
}

/**
 * Read ADXL345 sensor data (X, Y, Z acceleration)
 * Returns 0 on success, error code on failure
 */
static int read_adxl345_data(struct sensor_value accel[3])
{
    /* Fetch sample from the sensor */
    int rc = sensor_sample_fetch(adxl345_dev);
    if (rc != 0) {
        LOG_ERR("ADXL345: sensor_sample_fetch() failed: %d", rc);
        return rc;
    }

    /* Get acceleration data for all three axes */
    rc = sensor_channel_get(adxl345_dev, SENSOR_CHAN_ACCEL_X, &accel[0]);
    if (rc != 0) {
        LOG_ERR("ADXL345: sensor_channel_get(ACCEL_X) failed: %d", rc);
        return rc;
    }

    rc = sensor_channel_get(adxl345_dev, SENSOR_CHAN_ACCEL_Y, &accel[1]);
    if (rc != 0) {
        LOG_ERR("ADXL345: sensor_channel_get(ACCEL_Y) failed: %d", rc);
        return rc;
    }

    rc = sensor_channel_get(adxl345_dev, SENSOR_CHAN_ACCEL_Z, &accel[2]);
    if (rc != 0) {
        LOG_ERR("ADXL345: sensor_channel_get(ACCEL_Z) failed: %d", rc);
        return rc;
    }

    return 0;
}

/* ==================== Main Application ==================== */

int main(void)
{
    LOG_INF("==========================================");
    LOG_INF("Dual Sensor Test Application");
    LOG_INF("STM32 Nucleo WL55JC1 - Zephyr 4.3.0-rc1");
    LOG_INF("==========================================");
    LOG_INF("");

    /* Initialize sensors */
    const struct device *bme280 = init_bme280();
    const struct device *adxl345 = init_adxl345();

    if (bme280 == NULL || adxl345 == NULL) {
        LOG_ERR("Failed to initialize sensors. Halting.");
        return 1;
    }

    LOG_INF("");
    LOG_INF("Starting sensor measurements (every 1 second)...");
    LOG_INF("");

    /* Main reading loop */
    while (1) {
        struct sensor_q31_data temp_data = {0};
        struct sensor_q31_data press_data = {0};
        struct sensor_q31_data hum_data = {0};
        struct sensor_value accel[3] = {0};

        /* Read BME280 data */
        int rc_bme280 = read_bme280_data(&temp_data, &press_data, &hum_data);
        
        /* Read ADXL345 data */
        int rc_adxl345 = read_adxl345_data(accel);

        /* Print results if both readings were successful */
        if (rc_bme280 == 0 && rc_adxl345 == 0) {
            LOG_INF("[BME280] Temp: %s%d.%d Degree | Pressure: %s%d.%d Pa | Humidity: %s%d.%d %%",
                    PRIq_arg(temp_data.readings[0].temperature, 6, temp_data.shift),
                    PRIq_arg(press_data.readings[0].pressure, 6, press_data.shift),
                    PRIq_arg(hum_data.readings[0].humidity, 6, hum_data.shift));
            
            LOG_INF("[ADXL345] Accel X: %d.%06d m/s*s | Y: %d.%06d m/s*s | Z: %d.%06d m/s*s",
                    accel[0].val1, accel[0].val2,
                    accel[1].val1, accel[1].val2,
                    accel[2].val1, accel[2].val2);
            
            LOG_INF("---");
        } else {
            if (rc_bme280 != 0) {
                LOG_ERR("BME280 read failed with code %d", rc_bme280);
            }
            if (rc_adxl345 != 0) {
                LOG_ERR("ADXL345 read failed with code %d", rc_adxl345);
            }
        }

        /* Wait 1 second before next measurement */
        k_sleep(K_SECONDS(1));
    }

    return 0;
}
