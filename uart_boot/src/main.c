/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/dfu/mcuboot.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/storage/flash_map.h>

/* 1000 msec = 1 sec */

/* STEP 4.4 - Change LED timing to create a new image for DFU*/
#define SLEEP_TIME_MS   5000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

void check_swap_state(void) {
	printk("=== Swap Configuration ===\n");
	
	/* Try to detect swap mechanism by checking for scratch area */
	const struct flash_area *fa_scratch;
	bool scratch_available = false;
	
	/* Try different possible scratch area IDs */
	for (int scratch_id = 3; scratch_id <= 5; scratch_id++) {
		if (flash_area_open(scratch_id, &fa_scratch) == 0) {
			printk("Scratch area found: ID %d (size: %zu bytes)\n", 
				   scratch_id, fa_scratch->fa_size);
			printk("Swap mechanism: SCRATCH-based (detected)\n");
			scratch_available = true;
			flash_area_close(fa_scratch);
			break;
		}
	}
	
	if (!scratch_available) {
		printk("No scratch area found - checking for other swap methods...\n");
		
		/* Check if we can detect move-based swapping by looking at slot sizes */
		const struct flash_area *fa_primary, *fa_secondary;
		if (flash_area_open(1, &fa_primary) == 0 && 
			flash_area_open(2, &fa_secondary) == 0) {
			
			if (fa_primary->fa_size == fa_secondary->fa_size) {
				printk("Equal slot sizes detected - MOVE-based swap possible\n");
				printk("Swap mechanism: MOVE-based (inferred)\n");
			} else {
				printk("Unequal slot sizes - swap mechanism unclear\n");
				printk("Swap mechanism: UNKNOWN or NOT CONFIGURED\n");
			}
			
			flash_area_close(fa_primary);
			flash_area_close(fa_secondary);
		} else {
			printk("Cannot access slots - swap mechanism detection failed\n");
			printk("Swap mechanism: NOT CONFIGURED - This is why swapping doesn't work!\n");
		}
	}
	
	/* Additional debugging - show compile-time config status */
	printk("Compile-time config check (application context):\n");
#ifdef CONFIG_BOOT_SWAP_USING_SCRATCH
	printk("- CONFIG_BOOT_SWAP_USING_SCRATCH: ENABLED\n");
#else
	printk("- CONFIG_BOOT_SWAP_USING_SCRATCH: NOT DEFINED (normal for app)\n");
#endif

#ifdef CONFIG_BOOT_SWAP_USING_MOVE  
	printk("- CONFIG_BOOT_SWAP_USING_MOVE: ENABLED\n");
#else
	printk("- CONFIG_BOOT_SWAP_USING_MOVE: NOT DEFINED (normal for app)\n");
#endif

	if (scratch_available) {
		printk("\nSWAP STATUS: HARDWARE READY FOR SWAPPING!\n");
		printk("Next steps to test swapping:\n");
		printk("1. Upload new image: mcumgr image upload <file>\n");
		printk("2. Mark for test: mcumgr image test <hash>\n");  
		printk("3. Reboot: mcumgr reset\n");
		printk("4. Check if swap occurred after reboot\n");
	}

	printk("==========================\n");
}

int main(void)
{
	int ret;

	printk("\n=== MCUboot Secondary Slot Boot Test ===\n");
	printk("Starting LED blink application for Nucleo WL55JC\n");
	
	/* Check current boot status */
	bool is_confirmed = boot_is_img_confirmed();
	printk("Current image confirmation status: %s\n", is_confirmed ? "CONFIRMED" : "NOT CONFIRMED");
	
	if (is_confirmed) {
		printk("Running from PRIMARY slot (confirmed image)\n");
		
		/* Check if secondary slot has an image before trying to boot from it */
		const struct flash_area *fa_secondary;
		bool can_boot_secondary = false;
		
		if (flash_area_open(2, &fa_secondary) == 0) {
			uint8_t buf[4];
			if (flash_area_read(fa_secondary, 0, buf, sizeof(buf)) == 0) {
				// Check for MCUboot magic bytes (0x96f3b83d in little endian)
				can_boot_secondary = (buf[0] == 0x3d && buf[1] == 0xb8 && 
									 buf[2] == 0xf3 && buf[3] == 0x96);
			}
			flash_area_close(fa_secondary);
		}
		
		if (can_boot_secondary) {
			printk("Secondary slot has valid image - requesting boot from secondary slot\n");
			printk("Note: Use MCUmgr to mark secondary slot for testing:\n");
			printk("  mcumgr image test <hash>\n");
			printk("  mcumgr reset\n");
			printk("Or manually reboot to stay in primary slot for now\n");
		} else {
			printk("WARNING: No valid image found in secondary slot\n");
			printk("Cannot boot from secondary slot - staying in primary slot\n");
			printk("Upload image using: mcumgr image upload <file>\n");
		}
	} else {
		printk("Running from SECONDARY slot (test image)\n");
		printk("Confirming secondary slot image to make it permanent...\n");
		boot_write_img_confirmed();
		printk("Secondary slot image is now confirmed and permanent\n");
	}
	
	/* Comprehensive MCUboot slot debugging */
	printk("\n=== MCUboot Slot Analysis ===\n");
	
	/* Check primary slot (slot 0) */
	const struct flash_area *fa_primary, *fa_secondary;
	uint8_t buf[32];
	bool slot0_has_image = false, slot1_has_image = false;
	
	/* Try to open flash areas - use hardcoded IDs as fallback */
	int ret_primary = flash_area_open(1, &fa_primary);  /* Primary slot is typically ID 1 */
	int ret_secondary = flash_area_open(2, &fa_secondary); /* Secondary slot is typically ID 2 */
	
	if (ret_primary == 0) {
		if (flash_area_read(fa_primary, 0, buf, sizeof(buf)) == 0) {
			// Check for MCUboot magic bytes (0x96f3b83d in little endian)
			slot0_has_image = (buf[0] == 0x3d && buf[1] == 0xb8 && 
							   buf[2] == 0xf3 && buf[3] == 0x96);
		}
		flash_area_close(fa_primary);
	} else {
		printk("Warning: Cannot access primary slot (ID 1)\n");
	}
	
	if (ret_secondary == 0) {
		if (flash_area_read(fa_secondary, 0, buf, sizeof(buf)) == 0) {
			// Check for MCUboot magic bytes
			slot1_has_image = (buf[0] == 0x3d && buf[1] == 0xb8 && 
							   buf[2] == 0xf3 && buf[3] == 0x96);
		}
		flash_area_close(fa_secondary);
	} else {
		printk("Warning: Cannot access secondary slot (ID 2)\n");
	}
	
	printk("SLOT 0 (Primary):   %s\n", slot0_has_image ? "HAS IMAGE" : "EMPTY");
	printk("SLOT 1 (Secondary): %s\n", slot1_has_image ? "HAS IMAGE" : "EMPTY");
	printk("Current confirmed:  %s\n", boot_is_img_confirmed() ? "YES" : "NO");
	
	/* Show flash area accessibility for debugging */
	printk("Flash area access: Primary=%s, Secondary=%s\n",
		   ret_primary == 0 ? "OK" : "FAIL",
		   ret_secondary == 0 ? "OK" : "FAIL");
	
	/* Check if there's a pending swap */
	if (slot1_has_image && slot0_has_image) {
		printk("ANALYSIS: Both slots have images\n");
		if (boot_is_img_confirmed()) {
			printk("- Primary image is confirmed (permanent)\n");
			printk("- Secondary slot has been marked as pending for next boot\n");
			printk("- REBOOT NOW to test secondary slot image\n");
		} else {
			printk("- Image NOT confirmed - currently testing secondary slot\n");
			printk("- Image has been confirmed to make it permanent\n");
		}
	} else if (!slot1_has_image) {
		printk("ANALYSIS: No image in secondary slot\n");
		printk("- Cannot boot from secondary slot - no image present\n");
		printk("- Upload image to slot1 using: mcumgr image upload <file>\n");
	} else {
		printk("ANALYSIS: Unusual state - check flash partitions\n");
	}
	printk("========================\n\n");
	
	/* Show all available flash areas for debugging */
	printk("=== Flash Area Inventory ===\n");
	for (int id = 0; id <= 10; id++) {
		const struct flash_area *fa;
		if (flash_area_open(id, &fa) == 0) {
			printk("Flash area ID %d: size=%zu bytes, offset=0x%lx\n", 
				   id, fa->fa_size, (unsigned long)fa->fa_off);
			flash_area_close(fa);
		}
	}
	printk("============================\n\n");
	
	/* Additional swap state analysis */
	check_swap_state();

	if (!device_is_ready(led.port)) {
		printk("Error: LED device not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Error: Failed to configure LED GPIO\n");
		return -1;
	}

	printk("LED configured successfully\n");

	/* Show instructions for testing secondary slot */
	printk("\n=== Secondary Slot Boot Instructions ===\n");
	printk("To test booting from secondary slot:\n");
	printk("1. Upload new image: mcumgr image upload <file>\n");
	printk("2. Mark for test: mcumgr image test <hash>\n");
	printk("3. Reboot device: mcumgr reset\n");
	printk("4. Device will boot from secondary slot\n");
	printk("5. Confirm if working: mcumgr image confirm\n");
	printk("==========================================\n\n");

	int blink_count = 0;
	while (1) {
		ret = gpio_pin_toggle_dt(&led);
		if (ret < 0) {
			printk("Error: Failed to toggle LED\n");
			return -1;
		}
		
		blink_count++;
		printk("LED toggled (blink %d)\n", blink_count);
		
		k_msleep(SLEEP_TIME_MS);
	}
}