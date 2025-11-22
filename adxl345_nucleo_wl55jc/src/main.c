/*
 * Simple ADXL345 reader for Nucleo WL55JC1 using Zephyr sensor API (SPI).
 * Reads acceleration (X, Y, Z) and prints to console every second.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/sensor_data_types.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(adxl345_nucleo, LOG_LEVEL_INF);

/* Get a device structure from a devicetree node with compatible "adi,adxl345" */
const struct device *const dev = DEVICE_DT_GET_ANY(adi_adxl345);

static const struct device *check_adxl345_device(void)
{
    if (dev == NULL) {
        LOG_INF("Error: no ADXL345 device found. Ensure it is described in devicetree overlay and driver is enabled.");
        return NULL;
    }

    if (!device_is_ready(dev)) {
        LOG_INF("Error: Device '%s' is not ready; check driver logs.", dev->name);
        return NULL;
    }

    LOG_INF("Found ADXL345 device '%s', reading acceleration data...", dev->name);
    return dev;
}

int main(void)
{
    const struct device *dev = check_adxl345_device();

    if (dev == NULL) {
        return 0;
    }

    while (1) {
        struct sensor_value accel[3];  /* X, Y, Z */
        
        /* Fetch sample from the sensor */
        int rc = sensor_sample_fetch(dev);
        if (rc != 0) {
            LOG_ERR("sensor_sample_fetch() failed: %d", rc);
            k_sleep(K_SECONDS(1));
            continue;
        }

        /* Get acceleration data for all three axes */
        rc = sensor_channel_get(dev, SENSOR_CHAN_ACCEL_X, &accel[0]);
        if (rc != 0) {
            LOG_ERR("sensor_channel_get(ACCEL_X) failed: %d", rc);
            k_sleep(K_SECONDS(1));
            continue;
        }

        rc = sensor_channel_get(dev, SENSOR_CHAN_ACCEL_Y, &accel[1]);
        if (rc != 0) {
            LOG_ERR("sensor_channel_get(ACCEL_Y) failed: %d", rc);
            k_sleep(K_SECONDS(1));
            continue;
        }

        rc = sensor_channel_get(dev, SENSOR_CHAN_ACCEL_Z, &accel[2]);
        if (rc != 0) {
            LOG_ERR("sensor_channel_get(ACCEL_Z) failed: %d", rc);
            k_sleep(K_SECONDS(1));
            continue;
        }

        /* Print acceleration data in m/s^2 */
        LOG_INF("Accel X: %d.%06d m/s^2, Y: %d.%06d m/s^2, Z: %d.%06d m/s^2",
                accel[0].val1, accel[0].val2,
                accel[1].val1, accel[1].val2,
                accel[2].val1, accel[2].val2);

        k_sleep(K_SECONDS(1));
    }

    return 0;
}
