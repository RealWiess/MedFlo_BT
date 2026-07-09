/*
 * Copyright (c) 2022 Actions (Zhuhai) Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <device.h>
#include <drivers/spi.h>
#include <soc_irq.h>
#include "common.h"
#include "spi_test.h"
#include <string.h>


static int start_spi_transfer(const struct device *dev)
{
	struct spi_config config = {
		.frequency = 24000000,
		.operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8),
		.slave = 0,
	};

	struct spi_buf_set tx_bufs;
	struct spi_buf_set rx_bufs;
	struct spi_buf tx_buf[1];
	struct spi_buf rx_buf[1];
	u8_t buf_tx[16];
	u8_t buf_rx[16] = {0};
	int ret;

	memset(buf_tx, 0x86, 16);

	printk("spi test:%s\n", dev->name);

	tx_buf[0].buf = buf_tx;
	tx_buf[0].len = 16;
	rx_buf[0].buf = buf_rx;
	rx_buf[0].len = 16;
	rx_bufs.buffers = rx_buf;
	rx_bufs.count = 1;
	tx_bufs.buffers = tx_buf;
	tx_bufs.count = 1;
	ret = spi_transceive(dev, &config, &tx_bufs, &rx_bufs);
	printk("ret=%d\n", ret);
	if (ret) {
		printk("spi test error\n");
	} else {
		printk("spi test pass\n");
	}
	printk("buf_rx : 0x%x 0x%x 0x%x 0x%x\n", buf_rx[0], buf_rx[1], buf_rx[14], buf_rx[15]);

	return 0;
}

int test_for_spi(void)
{
	const struct device *spix_dev;

	spix_dev = device_get_binding(CONFIG_SPI_2_NAME);
	if (!spix_dev) {
		printk("SPI master driver was not found!\n");
		return -1;
	}

	start_spi_transfer(spix_dev);

	return 0;
}

