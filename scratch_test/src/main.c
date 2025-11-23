#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/dfu/mcuboot.h>

int main(void)
{
	printk("Hello World!\n", CONFIG_BOARD);
}