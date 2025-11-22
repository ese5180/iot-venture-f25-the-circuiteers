
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(gpio_test, LOG_LEVEL_INF);

/* GPIO configuration */
#define GPIO0_NODE DT_NODELABEL(gpio0)
#define GPIO1_NODE DT_NODELABEL(gpio1)

static const struct device *gpio0_dev = DEVICE_DT_GET(GPIO0_NODE);
static const struct device *gpio1_dev = DEVICE_DT_GET(GPIO1_NODE);

/* GPIO pins to control - using P1.04, P1.05, P1.06 on nRF7002DK */
#define TEST_PIN_1 4  /* P1.04 - Test pin 1 (GPIO1) */
#define TEST_PIN_2 5  /* P1.05 - Test pin 2 (GPIO1) */
#define TEST_PIN_3 6  /* P1.06 - Test pin 3 (GPIO1) */

/**
 * @brief Initialize GPIO pins as outputs
 */
static int init_gpio_pins(void)
{
	int ret;

	if (!device_is_ready(gpio1_dev)) {
		LOG_ERR("GPIO1 device not ready");
		return -ENODEV;
	}

	/* Configure P1.04, P1.05, P1.06 as outputs */
	ret = gpio_pin_configure(gpio1_dev, TEST_PIN_1, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		LOG_ERR("Failed to configure P1.04: %d", ret);
		return ret;
	}

	ret = gpio_pin_configure(gpio1_dev, TEST_PIN_2, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		LOG_ERR("Failed to configure P1.05: %d", ret);
		return ret;
	}

	ret = gpio_pin_configure(gpio1_dev, TEST_PIN_3, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		LOG_ERR("Failed to configure P1.06: %d", ret);
		return ret;
	}

	LOG_INF("GPIO pins configured successfully");
	return 0;
}

/**
 * @brief Toggle all GPIO pins (P1.04, P1.05, P1.06)
 */
static void toggle_gpio_pins(void)
{
	static bool state = false;
	
	state = !state;
	
	/* Toggle P1.04, P1.05, P1.06 to the same state */
	gpio_pin_set(gpio1_dev, TEST_PIN_1, state);  /* P1.04 */
	gpio_pin_set(gpio1_dev, TEST_PIN_2, state);  /* P1.05 */
	gpio_pin_set(gpio1_dev, TEST_PIN_3, state);  /* P1.06 */
	
	LOG_INF("GPIO pins P1.04, P1.05, P1.06 set to %s", state ? "HIGH" : "LOW");
}

/**
 * @brief Set specific GPIO pin high or low (P1.04, P1.05, P1.06)
 */
static void set_gpio_pin(int pin, bool high)
{
	gpio_pin_set(gpio1_dev, pin, high);
	LOG_INF("P1.%02d set to %s", pin, high ? "HIGH" : "LOW");
}

/**
 * @brief Main application entry point
 */
int main(void)
{
	int ret;
	int counter = 0;

	LOG_INF("GPIO Control Test Starting...");
	LOG_INF("This test will control GPIO pins HIGH/LOW");
	LOG_INF("Using Zephyr GPIO drivers\n");

	/* Initialize GPIO pins */
	ret = init_gpio_pins();
	if (ret < 0) {
		LOG_ERR("Failed to initialize GPIO pins: %d", ret);
		return ret;
	}

	LOG_INF("GPIO initialization complete");
	LOG_INF("Starting GPIO control test...\n");

	/* Main loop - control GPIO pins P1.04, P1.05, P1.06 */
	while (1) {
		/* Toggle all pins together */
		toggle_gpio_pins();
		
		/* Individual pin control demonstration */
		if (counter % 6 == 0) {
			set_gpio_pin(TEST_PIN_1, true);   /* P1.04 HIGH */
			set_gpio_pin(TEST_PIN_2, false);  /* P1.05 LOW */
			set_gpio_pin(TEST_PIN_3, false);  /* P1.06 LOW */
		} else if (counter % 6 == 1) {
			set_gpio_pin(TEST_PIN_1, false);  /* P1.04 LOW */
			set_gpio_pin(TEST_PIN_2, true);   /* P1.05 HIGH */
			set_gpio_pin(TEST_PIN_3, false);  /* P1.06 LOW */
		} else if (counter % 6 == 2) {
			set_gpio_pin(TEST_PIN_1, false);  /* P1.04 LOW */
			set_gpio_pin(TEST_PIN_2, false);  /* P1.05 LOW */
			set_gpio_pin(TEST_PIN_3, true);   /* P1.06 HIGH */
		} else if (counter % 6 == 3) {
			set_gpio_pin(TEST_PIN_1, true);   /* P1.04 HIGH */
			set_gpio_pin(TEST_PIN_2, true);   /* P1.05 HIGH */
			set_gpio_pin(TEST_PIN_3, false);  /* P1.06 LOW */
		} else if (counter % 6 == 4) {
			set_gpio_pin(TEST_PIN_1, false);  /* P1.04 LOW */
			set_gpio_pin(TEST_PIN_2, true);   /* P1.05 HIGH */
			set_gpio_pin(TEST_PIN_3, true);   /* P1.06 HIGH */
		} else {
			set_gpio_pin(TEST_PIN_1, true);   /* P1.04 HIGH */
			set_gpio_pin(TEST_PIN_2, false);  /* P1.05 LOW */
			set_gpio_pin(TEST_PIN_3, true);   /* P1.06 HIGH */
		}
		
		counter++;
		
		/* Wait 1 second before next operation */
		k_sleep(K_MSEC(1000));
	}

	return 0;
}