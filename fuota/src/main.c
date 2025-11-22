/*
 * LoRaWAN FUOTA sample application
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



LOG_MODULE_REGISTER(lorawan_fuota, CONFIG_LORAWAN_SERVICES_LOG_LEVEL);

/* Customize based on device configuration */
#define LORAWAN_DEV_EUI  { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x07, 0x3C, 0xC0 }
#define LORAWAN_JOIN_EUI { 0x00, 0x80, 0xE1, 0x15, 0x06, 0x1D, 0x9F, 0x39 }
#define LORAWAN_APP_KEY  { 0x3D, 0xBF, 0x23, 0xF7, 0x33, 0xA4, 0x89, 0x19, 0xA4, 0x5A, 0x7F, 0xC6, 0x49, 0xFC, 0x64, 0xE9 }

#define DELAY K_SECONDS(1)

char data[] = {'u', 'p', 'd', 'a', 't', 'e', 'd', ' ', 'd', 'a', 't', 'a'};

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
		ret = lorawan_send(2, data, sizeof(data), LORAWAN_MSG_UNCONFIRMED);

		if (ret == 0) {
			LOG_INF("Updated sent!");
		} else {
			LOG_ERR("lorawan_send failed: %d", ret);
		}

		k_sleep(DELAY);
	}

	return 0;
}



