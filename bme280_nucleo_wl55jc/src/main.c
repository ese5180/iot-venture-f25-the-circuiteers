/*
 * Simple BME280 reader for Nucleo WL55JC1 using Zephyr sensor API.
 * Reads temperature, pressure and humidity and prints to console every second.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/sensor_data_types.h>
#include <zephyr/rtio/rtio.h>
#include <zephyr/dsp/print_format.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(bme280_nucleo, LOG_LEVEL_INF);

/* Get a device structure from a devicetree node with compatible "bosch,bme280" */
const struct device *const dev = DEVICE_DT_GET_ANY(bosch_bme280);

SENSOR_DT_READ_IODEV(iodev, DT_COMPAT_GET_ANY_STATUS_OKAY(bosch_bme280),
        {SENSOR_CHAN_AMBIENT_TEMP, 0},
        {SENSOR_CHAN_HUMIDITY, 0},
        {SENSOR_CHAN_PRESS, 0});

RTIO_DEFINE(ctx, 1, 1);

static const struct device *check_bme280_device(void)
{
    if (dev == NULL) {
        LOG_INF("Error: no device found. Ensure the BME280 is described in devicetree overlay and driver is enabled.");
        return NULL;
    }

    if (!device_is_ready(dev)) {
        LOG_INF("Error: Device '%s' is not ready; check driver logs.", dev->name);
        return NULL;
    }

    LOG_INF("Found device '%s', reading sensor data...", dev->name);
    return dev;
}

int main(void)
{
    const struct device *dev = check_bme280_device();

    if (dev == NULL) {
        return 0;
    }

    while (1) {
        uint8_t buf[128];
        int rc = sensor_read(&iodev, &ctx, buf, sizeof(buf));

        if (rc != 0) {
            LOG_ERR("sensor_read() failed: %d", rc);
            k_sleep(K_SECONDS(1));
            continue;
        }

        const struct sensor_decoder_api *decoder;
        rc = sensor_get_decoder(dev, &decoder);
        if (rc != 0) {
            LOG_ERR("sensor_get_decoder() failed: %d", rc);
            k_sleep(K_SECONDS(1));
            continue;
        }

        uint32_t temp_fit = 0;
        struct sensor_q31_data temp_data = {0};
        decoder->decode(buf,
                        (struct sensor_chan_spec){SENSOR_CHAN_AMBIENT_TEMP, 0},
                        &temp_fit, 1, &temp_data);

        uint32_t press_fit = 0;
        struct sensor_q31_data press_data = {0};
        decoder->decode(buf,
                        (struct sensor_chan_spec){SENSOR_CHAN_PRESS, 0},
                        &press_fit, 1, &press_data);

        uint32_t hum_fit = 0;
        struct sensor_q31_data hum_data = {0};
        decoder->decode(buf,
                        (struct sensor_chan_spec){SENSOR_CHAN_HUMIDITY, 0},
                        &hum_fit, 1, &hum_data);

        /* Print using LOG_INF so output goes to console */
        LOG_INF("Temp: %s%d.%d C; Pressure: %s%d.%d Pa; Humidity: %s%d.%d %%",
                PRIq_arg(temp_data.readings[0].temperature, 6, temp_data.shift),
                PRIq_arg(press_data.readings[0].pressure, 6, press_data.shift),
                PRIq_arg(hum_data.readings[0].humidity, 6, hum_data.shift));

        k_sleep(K_SECONDS(1));
    }

    return 0;
}
