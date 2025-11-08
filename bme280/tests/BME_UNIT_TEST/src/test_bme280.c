#include <zephyr/ztest.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>

ZTEST(dt_sanity, test_console_ready)
{
	const struct device *cons = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	zassert_true(cons != NULL, "console device NULL");
	zassert_true(device_is_ready(cons), "console not ready");
}

ZTEST(dt_sanity, test_soc_node_exists)
{
	zassert_true(DT_NODE_EXISTS(DT_PATH(soc)), "/soc missing in devicetree");
}

ZTEST(dt_sanity, test_uart0_ok)
{
	zassert_true(DT_NODE_HAS_STATUS(DT_NODELABEL(uart0), okay), "uart0 not okay");
	const struct device *uart0 = DEVICE_DT_GET(DT_NODELABEL(uart0));
	zassert_true(device_is_ready(uart0), "uart0 not ready");
}

ZTEST(dt_sanity, test_sys_clock_ticks)
{
	int64_t t0 = k_uptime_get();
	k_sleep(K_MSEC(5));
	int64_t t1 = k_uptime_get();
	zassert_true(t1 >= t0 + 4, "uptime did not advance as expected");
}

ZTEST_SUITE(dt_sanity, NULL, NULL, NULL, NULL, NULL);
