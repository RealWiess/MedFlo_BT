/*
 * Copyright (c) 2022 Actions (Zhuhai) Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <device.h>
#include <drivers/input/input_dev.h>
#include "keypad_test.h"

void key_event_handle_cb(struct device *dev, struct input_value *val)
{
	printk("keypad.type:0x%x\n", val->keypad.type);
	printk("keypad.value:0x%x\n", val->keypad.value);
	printk("keypad.code:0x%x\n\n", val->keypad.code);
}

/*
 * GPIO:0/1/2/3/6/7/8/20/21/22/26
 * map to :KEY0/KEY1/KEY13/KEY14/KEY11/KEY7/KEY8/KEY3/KEY2/KEY9/KEY15
 */
void test_for_key(void)
{
	const struct deivce *keypad_dev = (struct deivce *)device_get_binding(CONFIG_KEYPAD_NAME);

	if (!keypad_dev) {
		printk("can not bind keypad dev!\n");
		return;
	}

	input_dev_enable(keypad_dev);
	input_dev_register_notify(keypad_dev, key_event_handle_cb);
}

