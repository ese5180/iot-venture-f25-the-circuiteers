#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_REGISTER(lorawan_app, LOG_LEVEL_DBG);

/* Test Configuration */
#define TEST_INTERVAL_MS	5000  /* Run tests every 5 seconds */

/* Global variables */
static const struct device *lora_dev;

/* Test message for point-to-point communication */
static uint8_t test_message[] = "Hello from SX1276!";
static uint8_t rx_buffer[255];

/**
 * @brief Initialize LoRa device
 */
static int init_lora_device(void)
{
	LOG_INF("Initializing LoRa device...");
	
	/* Get the LoRa device */
	lora_dev = DEVICE_DT_GET(DT_ALIAS(lora0));
	if (!lora_dev) {
		LOG_ERR("Failed to get LoRa device!");
		return -ENODEV;
	}
	
	if (!device_is_ready(lora_dev)) {
		LOG_ERR("LoRa device not ready!");
		return -ENODEV;
	}
		
	LOG_INF("LoRa device is ready and accessible");
	return 0;
}

/**
 * @brief Configure LoRa modem for transmission
 */
static int configure_lora_tx(void)
{
	struct lora_modem_config config = {
		.frequency = 915000000,      /* 915 MHz */
		.bandwidth = BW_125_KHZ,     /* 125 kHz bandwidth */
		.datarate = SF_7,            /* Spreading Factor 7 */
		.preamble_len = 8,           /* 8 symbol preamble */
		.coding_rate = CR_4_5,       /* 4/5 coding rate */
		.iq_inverted = false,        /* Normal IQ */
		.public_network = false,     /* Private network */
		.tx_power = 14,              /* 14 dBm transmit power */
		.tx = true                   /* Configure for TX */
	};
	
	LOG_INF("Configuring LoRa modem for transmission...");
	
	int ret = lora_config(lora_dev, &config);
	if (ret < 0) {
		LOG_ERR("Failed to configure LoRa modem for TX: %d", ret);
		return ret;
	}
	
	/* Give some time for configuration to settle */
	k_sleep(K_MSEC(10));
	
	LOG_INF("LoRa modem configured for transmission");
	return 0;
}

/**
 * @brief Configure LoRa modem for reception
 */
static int configure_lora_rx(void)
{
	struct lora_modem_config config = {
		.frequency = 915000000,      /* 915 MHz */
		.bandwidth = BW_125_KHZ,     /* 125 kHz bandwidth */
		.datarate = SF_7,            /* Spreading Factor 7 */
		.preamble_len = 8,           /* 8 symbol preamble */
		.coding_rate = CR_4_5,       /* 4/5 coding rate */
		.iq_inverted = false,        /* Normal IQ */
		.public_network = false,     /* Private network */
		.tx_power = 14,              /* 14 dBm transmit power */
		.tx = false                  /* Configure for RX */
	};
	
	LOG_INF("Configuring LoRa modem for reception...");
	
	int ret = lora_config(lora_dev, &config);
	if (ret < 0) {
		LOG_ERR("Failed to configure LoRa modem for RX: %d", ret);
		return ret;
	}
	
	/* Give some time for configuration to settle */
	k_sleep(K_MSEC(10));
	
	LOG_INF("LoRa modem configured for reception");
	return 0;
}

/**
 * @brief Test LoRa transmission
 */
static int test_lora_transmission(void)
{
	int ret;
	
	LOG_INF("Testing LoRa transmission...");
	
	/* Configure for transmission */
	ret = configure_lora_tx();
	if (ret < 0) {
		return ret;
	}
	
	/* Send the message */
	ret = lora_send(lora_dev, test_message, sizeof(test_message) - 1); /* -1 to exclude null terminator */
	if (ret < 0) {
		LOG_ERR("LoRa transmission failed: %d", ret);
		return ret;
	}
	
	LOG_INF("LoRa transmission completed successfully");
	LOG_INF("   Sent: \"%s\" (%d bytes)", test_message, sizeof(test_message) - 1);
	return 0;
}

/**
 * @brief Test LoRa reception (non-blocking)
 */
static int test_lora_reception(void)
{
	int ret;
	uint8_t len = sizeof(rx_buffer);
	int16_t rssi;
	int8_t snr;
	
	LOG_INF("Testing LoRa reception (timeout: 2 seconds)...");
	
	/* Configure for reception */
	ret = configure_lora_rx();
	if (ret < 0) {
		return ret;
	}
	
	/* Clear the receive buffer */
	memset(rx_buffer, 0, sizeof(rx_buffer));
	
	ret = lora_recv(lora_dev, rx_buffer, len, K_MSEC(2000), &rssi, &snr);
	if (ret < 0) {
		if (ret == -EAGAIN) {
			LOG_INF("No LoRa message received (timeout)");
		} else {
			LOG_ERR("LoRa reception failed: %d", ret);
		}
		return ret;
	}
	
	LOG_INF("LoRa message received!");
	LOG_INF("   Data: \"%.*s\" (%d bytes)", ret, rx_buffer, ret);
	LOG_INF("   RSSI: %d dBm, SNR: %d dB", rssi, snr);
	return 0;
}



/**
 * @brief Main application entry point
 */
int main(void)
{
	int ret;

	LOG_INF("SX1276 LoRa Communication Test Starting...");
	LOG_INF("This test will verify LoRa driver communication with SX1276");
	LOG_INF("Using Nordic Connect SDK LoRa drivers\n");

	/* Initialize LoRa device */
	ret = init_lora_device();
	if (ret < 0) {
		LOG_ERR("Failed to initialize LoRa device: %d", ret);
		return ret;
	}

	LOG_INF("LoRa initialization complete");
	LOG_INF("Starting continuous message transmission every %d seconds...\n", TEST_INTERVAL_MS / 1000);

	/* Main loop - continuously send the same message */
	while (1) {
		ret = test_lora_transmission();
		if (ret < 0) {
			LOG_ERR("Transmission failed, retrying in %d seconds...", TEST_INTERVAL_MS / 1000);
		}
		
		/* Wait before next transmission */
		k_sleep(K_MSEC(TEST_INTERVAL_MS));
	}

	return 0;
}