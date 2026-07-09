/*
 * Copyright (c) 2022 Actions (Zhuhai) Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <drivers/gpio.h>
#include "gpio_test.h"
#include "common.h"

#define GPIO_REG_BASE			0x40068000
#define GPIO_CTL0				0x4
#define GPIO_REG_CTL(base, pin)	((base) + GPIO_CTL0 + (pin) * 4)
#define GPIOB_PIN(pin)	        (pin - 32)

void test_for_gpio(void)
{
	const struct device* gpio_dev = device_get_binding(CONFIG_GPIO_A_NAME);
	if (!gpio_dev) {
		printk("gpio device bind error!\n");
		return;
	}

	/* config GPIO18 to normal gpio mode */
	gpio_pin_configure(gpio_dev, 20, GPIO_OUTPUT|GPIO_OUTPUT_INIT_HIGH);
	gpio_pin_configure(gpio_dev, 21, GPIO_OUTPUT|GPIO_OUTPUT_INIT_HIGH);

	printk("gpio-mfp:0x%x\n", sys_read32(GPIO20_CTL));
	printk("gpio-mfp:0x%x\n", sys_read32(GPIO21_CTL));
	gpio_pin_set(gpio_dev, 21, 0);
	gpio_pin_set(gpio_dev, 20, 0);

	const struct device* gpiob_dev = device_get_binding(CONFIG_GPIO_B_NAME);
	if (!gpiob_dev) {
		printk("gpiob device bind error!\n");
		return;
	}

	/* config GPIO42 to normal gpio mode */
	gpio_pin_configure(gpiob_dev, GPIOB_PIN(42), GPIO_OUTPUT|GPIO_OUTPUT_INIT_HIGH);
	printk("GPIO42_CTL: addr %x value 0x%x\n", GPIO_REG_CTL(GPIO_REG_BASE, 42), sys_read32(GPIO_REG_CTL(GPIO_REG_BASE, 42)));

	while(1){
		gpio_pin_set(gpio_dev, 21, 0);
		gpio_pin_set(gpio_dev, 20, 1);
//		gpio_pin_set(gpiob_dev, GPIOB_PIN(42), 1);
		k_sleep(K_MSEC(1000));

		gpio_pin_set(gpio_dev, 21, 1);
		gpio_pin_set(gpio_dev, 20, 0);
//		gpio_pin_set(gpiob_dev, GPIOB_PIN(42), 0);
		k_sleep(K_MSEC(1000));
	}
}
