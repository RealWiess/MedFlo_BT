/*
 * Copyright (c) 2022 Actions (Zhuhai) Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <device.h>
#include <drivers/i2c.h>
#include <soc_irq.h>
#include "common.h"
#include "i2c_test.h"

#define TEST_DATA_SIZE 20
static uint8_t i2c_tx_buffer[TEST_DATA_SIZE]="0123456789abcdefghij";
static uint8_t i2c_rx_buffer[TEST_DATA_SIZE];

int test_for_i2c(void)
{
	uint8_t ret;
	//uint8_t val;

	const struct device *i2c = device_get_binding(CONFIG_I2C_0_NAME);
	if (!i2c) {
		printk("can not bind i2c device\n");
		return -1;
	}

	//ret = i2c_reg_write_byte(i2c, DEVICE_ADDR, 0x10, 0x37);
	ret = i2c_burst_write(i2c, DEVICE_ADDR, 0x10, i2c_tx_buffer, TEST_DATA_SIZE);
	if (ret) {
		printk("data write error\n");
		return -1;
	}

	k_sleep(K_MSEC(1000));
	//ret = i2c_reg_read_byte(i2c, DEVICE_ADDR, 0x10, &val);
	ret = i2c_burst_read(i2c, DEVICE_ADDR, 0x10, i2c_rx_buffer, TEST_DATA_SIZE);
	if (ret) {
		printk("data read error\n");
	} else {
		printk("data read success:%c\n", i2c_rx_buffer[0]);
	}

	return 0;
}

