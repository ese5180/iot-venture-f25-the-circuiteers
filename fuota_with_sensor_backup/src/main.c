/*
 * LoRaWAN FUOTA sample application with BME280 and ADXL345 sensors
 *
 * Copyright (c) 2022-2024 Libre Solar Technologies GmbH
 * Copyright (c) 2022-2024 tado GmbH
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/lorawan/lorawan.h>

#include <zephyr/dfu/mcuboot.h>
#include <zephyr/sys/reboot.h>

// testing printk
#include <zephyr/sys/printk.h>

// Sensor API
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/sensor_data_types.h>
#include <zephyr/rtio/rtio.h>
#include <zephyr/dsp/print_format.h>



LOG_MODULE_REGISTER(lorawan_fuota, CONFIG_LORAWAN_SERVICES_LOG_LEVEL);


// nucleo -1
/* Customize based on device configuration */
// #define LORAWAN_DEV_EUI  { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x07, 0x3C, 0xC0 }
// #define LORAWAN_JOIN_EUI { 0x00, 0x80, 0xE1, 0x15, 0x06, 0x1D, 0x9F, 0x39 }
// #define LORAWAN_APP_KEY  { 0x3D, 0xBF, 0x23, 0xF7, 0x33, 0xA4, 0x89, 0x19, 0xA4, 0x5A, 0x7F, 0xC6, 0x49, 0xFC, 0x64, 0xE9 }

// nucleo -2
#define LORAWAN_DEV_EUI  { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x07, 0x43, 0xCC }
#define LORAWAN_JOIN_EUI { 0x00, 0x80, 0xE1, 0x15, 0x06, 0x1D, 0xA7, 0x0F }
#define LORAWAN_APP_KEY  { 0xED, 0x5F, 0x2B, 0x36, 0x51, 0x6B, 0xC7, 0x8B, 0x65, 0xCA, 0x64, 0x4B, 0x3F, 0xAE, 0xE0, 0x9B }

#define DELAY K_SECONDS(2)

char data[] = {'d', 'a', 't', 'a', ' ', 's', 'e', 'n', 'd'};

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

/**
 * Pack sensor data into binary payload
 * Payload format (12 bytes):
 * - temp_x100 (2 bytes): Temperature * 100, range -327.68 ~ +327.67 °C
 * - hum_x100 (2 bytes): Humidity * 100, range 0 ~ 655.35 %
 * - press_x10 (2 bytes): Pressure * 10, e.g. 1013.2 hPa → 10132
 * - accel_x (2 bytes): X acceleration * 1000 (m/s²)
 * - accel_y (2 bytes): Y acceleration * 1000 (m/s²)
 * - accel_z (2 bytes): Z acceleration * 1000 (m/s²)
 * Returns the payload size
 */
static int pack_sensor_payload(struct sensor_q31_data *temp_data,
                               struct sensor_q31_data *hum_data,
                               struct sensor_q31_data *press_data,
                               struct sensor_value accel[3],
                               uint8_t *payload)
{
<<<<<<< HEAD:fuota_with_sensor/src/main.c
    /* Convert Q31 sensor values to fixed-point x1000 without float */
    /* Q31 means value is scaled by 2^31. */
    /* We want (value / 2^31) * 1000 => (value * 1000) / 2^31 */
    /* Use int64_t to prevent overflow during multiplication */
=======
    
    /* The raw sensor values are in Q31-like format with different shift values
     * Analysis from debug data:
     * temp_raw=800522, shift=16, Display: 24.43°C
     *   → 800522/2^16 = 12.21, display shows 24.43 (exactly 2x!)
     *   → So: actual_temp = temp_raw / 2^(shift-1)
     * 
     * hum_raw=39609, shift=21, Display: 38.68%
     *   → 39609/2^21 = 0.01887, but display shows 38.68
     *   → Factor needed: 38.68 / 0.01887 ≈ 2049 ≈ 2^11
     *   → So: actual_hum = hum_raw * 2^11 / 2^21 = hum_raw / 2^10
     * 
     * press_raw=25964, shift=23, Display: 101.42 Pa (actually hPa?)
     *   → 25964/2^23 = 0.00309, display shows 101.42
     *   → Factor needed: 101.42 / 0.00309 ≈ 32817 ≈ 2^15
     *   → So: actual_press = press_raw * 2^15 / 2^23 = press_raw / 2^8
     */
    
    /* Get raw Q31 values */
    int32_t temp_raw = temp_data->readings[0].temperature;
    int32_t hum_raw = hum_data->readings[0].humidity;
    int32_t press_raw = press_data->readings[0].pressure;
    
    /* Convert using the discovered relationships */
    float temp_c = (float)temp_raw / (1LL << (temp_data->shift - 1));    // 2x larger scale
    float humidity = (float)hum_raw / (1LL << (hum_data->shift - 11));   // Correct scale for %
    float pressure = (float)press_raw / (1LL << (press_data->shift - 15)); // Correct scale for Pa
>>>>>>> ff6ee73 (save local changes):fuota_with_sensor_backup/src/main.c
    
    int16_t temp_x1000 = (int16_t)(((int64_t)temp_data->readings[0].temperature * 1000) / 2147483648LL);
    int16_t hum_x1000 = (int16_t)(((int64_t)hum_data->readings[0].humidity * 1000) / 2147483648LL);
    int16_t press_x1000 = (int16_t)(((int64_t)press_data->readings[0].pressure * 1000) / 2147483648LL);

<<<<<<< HEAD:fuota_with_sensor/src/main.c
    /* Convert standard sensor values to fixed-point x1000 without float */
    /* sensor_value has val1 (int) and val2 (micro, /1,000,000) */
    /* val * 1000 = val1 * 1000 + val2 / 1000 */
    
    int16_t accel_x_1000 = (int16_t)(accel[0].val1 * 1000 + accel[0].val2 / 1000);
    int16_t accel_y_1000 = (int16_t)(accel[1].val1 * 1000 + accel[1].val2 / 1000);
    int16_t accel_z_1000 = (int16_t)(accel[2].val1 * 1000 + accel[2].val2 / 1000);
=======
    /* Convert to fixed-point integers for transmission
     * temp_c should be ~24.43 → temp_x100 = 2443
     * humidity should be ~38.68 → hum_x100 = 3868
     * pressure should be ~101.42 Pa → but typically we use hPa, so /100 first, then *10
     *   or just multiply the Pa value by 10: 101.42*10 = 1014.2 → press_x10 = 1014
     */
    int16_t temp_x100 = (int16_t)(temp_c * 100);        // temp in °C * 100
    uint16_t hum_x100 = (uint16_t)(humidity * 100);     // humidity in % * 100
    uint16_t press_x10 = (uint16_t)(pressure * 10);     // pressure in Pa * 10
    int16_t accel_x_1000 = (int16_t)(accel_x_f * 1000); // X acceleration * 1000
    int16_t accel_y_1000 = (int16_t)(accel_y_f * 1000); // Y acceleration * 1000
    int16_t accel_z_1000 = (int16_t)(accel_z_f * 1000); // Z acceleration * 1000

    /* Debug log the packed values */
    LOG_INF("DEBUG PACKED: temp_x100=%d, hum_x100=%u, press_x10=%u", temp_x100, hum_x100, press_x10);
>>>>>>> ff6ee73 (save local changes):fuota_with_sensor_backup/src/main.c

    /* Pack into binary payload (12 bytes) */
    payload[0] = (uint8_t)(temp_x100 >> 8);
    payload[1] = (uint8_t)(temp_x100 & 0xFF);
    payload[2] = (uint8_t)(hum_x100 >> 8);
    payload[3] = (uint8_t)(hum_x100 & 0xFF);
    payload[4] = (uint8_t)(press_x10 >> 8);
    payload[5] = (uint8_t)(press_x10 & 0xFF);
    payload[6] = (uint8_t)(accel_x_1000 >> 8);
    payload[7] = (uint8_t)(accel_x_1000 & 0xFF);
    payload[8] = (uint8_t)(accel_y_1000 >> 8);
    payload[9] = (uint8_t)(accel_y_1000 & 0xFF);
    payload[10] = (uint8_t)(accel_z_1000 >> 8);
    payload[11] = (uint8_t)(accel_z_1000 & 0xFF);

    return 12; /* 12 bytes total */
}

static void downlink_info(uint8_t port, uint8_t flags, int16_t rssi, int8_t snr, uint8_t len,
			  const uint8_t *data)
{
	LOG_INF("Received from port %d, flags %d, RSSI %ddB, SNR %ddBm", port, flags, rssi, snr);
	if (data) {
		LOG_HEXDUMP_INF(data, len, "Payload: ");
	}
}

static void datarate_changed(enum lorawan_datarate dr)
{
	uint8_t unused, max_size;

	lorawan_get_payload_sizes(&unused, &max_size);
	LOG_INF("New Datarate: DR %d, Max Payload %d", dr, max_size);
}

static void fuota_finished(void)
{
	LOG_INF("FUOTA finished. Reset device to apply firmware upgrade.");

	/*
	 * In an actual application the firmware should be rebooted here if
	 * no important tasks are pending
	 */

	int rc = boot_request_upgrade(BOOT_UPGRADE_PERMANENT);
    if (rc) {
        printk("boot_request_upgrade failed: %d\n", rc);
        return;
    }

    k_msleep(100);
    sys_reboot(SYS_REBOOT_COLD);
	
}

int main(void)
{

	printk("FUOTA app booting...\n");

	const struct device *lora_dev;
	struct lorawan_join_config join_cfg;
	uint8_t dev_eui[] = LORAWAN_DEV_EUI; 
	uint8_t join_eui[] = LORAWAN_JOIN_EUI;
	uint8_t app_key[] = LORAWAN_APP_KEY;
	int ret;

	struct lorawan_downlink_cb downlink_cb = {
		.port = LW_RECV_PORT_ANY,
		.cb = downlink_info
	};

	/* Initialize sensors */
	const struct device *bme280 = init_bme280();
	const struct device *adxl345 = init_adxl345();

	if (bme280 == NULL || adxl345 == NULL) {
		LOG_ERR("Failed to initialize sensors. Continuing with LoRaWAN setup.");
	}

	lora_dev = DEVICE_DT_GET(DT_ALIAS(lora0));
	if (!device_is_ready(lora_dev)) {
		LOG_ERR("%s: device not ready.", lora_dev->name);
		return -ENODEV;
	}

	ret = lorawan_start();
	if (ret < 0) {
		LOG_ERR("lorawan_start failed: %d", ret);
		return ret;
	}

	lorawan_register_downlink_callback(&downlink_cb);
	lorawan_register_dr_changed_callback(datarate_changed);

	join_cfg.mode = LORAWAN_ACT_OTAA;
	join_cfg.dev_eui = dev_eui;
	join_cfg.otaa.join_eui = join_eui;
	join_cfg.otaa.app_key = app_key;
	join_cfg.otaa.nwk_key = app_key;

	LOG_INF("Joining network over OTAA");
	ret = lorawan_join(&join_cfg);
	if (ret < 0) {
		LOG_ERR("lorawan_join_network failed: %d", ret);
		return ret;
	}

	lorawan_enable_adr(true);

	/*
	 * Clock synchronization is required to schedule the multicast session
	 * in class C mode. It can also be used independent of FUOTA.
	 */
	lorawan_clock_sync_run();

	/*
	 * The multicast session setup service is automatically started in the
	 * background. It is also responsible for switching to class C at a
	 * specified time.
	 */

	/*
	 * The fragmented data transport transfers the actual firmware image.
	 * It could also be used in a class A session, but would take very long
	 * in that case.
	 */
	lorawan_frag_transport_run(fuota_finished);

	/*
	 * Regular uplinks are required to open downlink slots in class A for
	 * FUOTA setup by the server.
	 */
	while (1) {
		/* Try to read and send sensor data */
		if (bme280 != NULL && adxl345 != NULL) {
			struct sensor_q31_data temp_data = {0};
			struct sensor_q31_data press_data = {0};
			struct sensor_q31_data hum_data = {0};
			struct sensor_value accel[3] = {0};

			/* Read BME280 data */
			int rc_bme280 = read_bme280_data(&temp_data, &press_data, &hum_data);
			
			/* Read ADXL345 data */
			int rc_adxl345 = read_adxl345_data(accel);

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

			/* Pack and send sensor data if both readings were successful */
			if (rc_bme280 == 0 && rc_adxl345 == 0) {
				uint8_t sensor_payload[12];
				int payload_size = pack_sensor_payload(&temp_data, &hum_data, &press_data,
														accel, sensor_payload);

				ret = lorawan_send(2, sensor_payload, payload_size, LORAWAN_MSG_UNCONFIRMED);

				if (ret == 0) {
					LOG_INF("Sensor data sent!");
				} else {
					LOG_ERR("lorawan_send (sensor) failed: %d", ret);
				}
			} else {
				if (rc_bme280 != 0) {
					LOG_ERR("BME280 read failed: %d", rc_bme280);
				}
				if (rc_adxl345 != 0) {
					LOG_ERR("ADXL345 read failed: %d", rc_adxl345);
				}
			}
		} else {
			/* Fallback to original data if sensors not available */
			ret = lorawan_send(2, data, sizeof(data), LORAWAN_MSG_UNCONFIRMED);

			if (ret == 0) {
				LOG_INF("Updated sent!");
			} else {
				LOG_ERR("lorawan_send failed: %d", ret);
			}
		}



		k_sleep(DELAY);
	}

	return 0;
}



