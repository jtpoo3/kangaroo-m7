/* arch/arm/mach-msm/board-cp5_wl-keypad.c
 *
 * Copyright (C) 2008 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/gpio_event.h>
#include <linux/keyreset.h>
#include <asm/mach-types.h>
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <linux/delay.h>
#include "board-cp5_wl.h"

#undef MODULE_PARAM_PREFIX
#define MODULE_PARAM_PREFIX "board_cp5_wl."

static void config_gpio_table(uint32_t *table, int len)
{
	int n, rc;
	for (n = 0; n < len; n++) {
		rc = gpio_tlmm_config(table[n], GPIO_CFG_ENABLE);
		if (rc) {
			pr_err("[keypad]%s: gpio_tlmm_config(%#x)=%d\n",
				__func__, table[n], rc);
			break;
		}
	}
}

static struct gpio_event_direct_entry cp5_wl_keypad_input_map[] = {
	{
		.gpio = MSM_PWR_KEY_MSMz,
		.code = KEY_POWER,
	},
	{
		.gpio = MSM_VOL_UPz,
		.code = KEY_VOLUMEUP,
	},
	{
		.gpio = MSM_VOL_DOWNz,
		.code = KEY_VOLUMEDOWN,
	 },
};

static void cp5_wl_setup_input_gpio(void)
{
	uint32_t inputs_gpio_table[] = {
		GPIO_CFG(MSM_PWR_KEY_MSMz, 0, GPIO_CFG_INPUT,
			GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
		GPIO_CFG(MSM_VOL_UPz, 0, GPIO_CFG_INPUT,
			GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
		GPIO_CFG(MSM_VOL_DOWNz, 0, GPIO_CFG_INPUT,
			GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
	};

	config_gpio_table(inputs_gpio_table, ARRAY_SIZE(inputs_gpio_table));
}

uint32_t hw_clr_gpio_table[] = {
		GPIO_CFG(MSM_HW_RST_CLRz, 0, GPIO_CFG_INPUT,
			GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
		GPIO_CFG(MSM_HW_RST_CLRz, 0, GPIO_CFG_OUTPUT,
			GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
	};
static void cp5_wl_clear_hw_reset(void)
{
	printk(KERN_INFO "[KEY] %s ++++++\n", __func__);
	gpio_tlmm_config(hw_clr_gpio_table[1], GPIO_CFG_ENABLE);
	gpio_set_value(MSM_HW_RST_CLRz, 0);
	msleep(100);
	gpio_tlmm_config(hw_clr_gpio_table[0], GPIO_CFG_ENABLE);
	printk(KERN_INFO "[KEY] %s ------\n", __func__);
}

static struct gpio_event_input_info cp5_wl_keypad_input_info = {
	.info.func             = gpio_event_input_func,
	.flags                 = GPIOEDF_PRINT_KEYS,
	.type                  = EV_KEY,
#if BITS_PER_LONG != 64 && !defined(CONFIG_KTIME_SCALAR)
	.debounce_time.tv.nsec = 20 * NSEC_PER_MSEC,
# else
	.debounce_time.tv64    = 20 * NSEC_PER_MSEC,
# endif
	.keymap                = cp5_wl_keypad_input_map,
	.keymap_size           = ARRAY_SIZE(cp5_wl_keypad_input_map),
	.setup_input_gpio      = cp5_wl_setup_input_gpio,
	.clear_hw_reset	= cp5_wl_clear_hw_reset,
};

static struct gpio_event_info *cp5_wl_keypad_info[] = {
	&cp5_wl_keypad_input_info.info,
};

static struct gpio_event_platform_data cp5_wl_keypad_data = {
	.names = {
		"device-keypad",
		NULL,
	},
	.info = cp5_wl_keypad_info,
	.info_count = ARRAY_SIZE(cp5_wl_keypad_info),
};

static struct platform_device cp5_wl_keypad_input_device = {
	.name = GPIO_EVENT_DEV_NAME,
	.id = 0,
	.dev		= {
		.platform_data	= &cp5_wl_keypad_data,
	},
};
static struct keyreset_platform_data cp5_wl_reset_keys_pdata = {
	/*.keys_up = cp5_wl_reset_keys_up,*/
	.keys_down = {
		KEY_POWER,
		KEY_VOLUMEDOWN,
		KEY_VOLUMEUP,
		0
	},
};

struct platform_device cp5_wl_reset_keys_device = {
	.name = KEYRESET_NAME,
	.dev.platform_data = &cp5_wl_reset_keys_pdata,
};

int __init cp5_wl_init_keypad(void)
{
	printk(KERN_DEBUG "[KEY]%s\n", __func__);

	if (platform_device_register(&cp5_wl_reset_keys_device))
		printk(KERN_WARNING "%s: register reset key fail\n", __func__);

	return platform_device_register(&cp5_wl_keypad_input_device);
}
