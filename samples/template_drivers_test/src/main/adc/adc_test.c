/*
 * Copyright (c) 2018 Actions (Zhuhai) Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <soc.h>
#include <sys/printk.h>
#include <drivers/adc.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(adc_test, CONFIG_LOG_DEFAULT_LEVEL);

/* adc channel id */
#define	SARADC_ID_IOVCC			(0)
#define SARADC_ID_SENSOR		(1)
#define	SARADC_ID_LRADC0		(2)
#define	SARADC_ID_LRADC1		(3)
#define	SARADC_ID_LRADC2		(4)
#define	SARADC_ID_LRADC3		(5)
#define	SARADC_ID_LRADC4		(6)
#define	SARADC_ID_LRADC5		(7)

/* adc value convert to voltage */
#define SARADC_LRADC2VOL(x) ((x) * 3600 / 65536)

/* adc handle */
struct adc_handle {
	const struct device *dev;
	struct adc_channel_cfg channel_config;
	uint8_t resolution;
};

static struct adc_handle saradc;

#define BUFFER_SIZE 1
static int saradc_read(uint8_t channel_id)
{
	struct adc_handle *adc = &saradc;
	uint16_t sample_buffer[BUFFER_SIZE];
	int retval = 0;

	adc->dev = device_get_binding(CONFIG_SARADC_NAME);
	if (adc->dev == NULL) {
		LOG_ERR("adc device not found");
		return -ENODEV;
	}

	adc->channel_config.channel_id = channel_id;
	retval = adc_channel_setup(adc->dev, &adc->channel_config);
	if (retval < 0) {
		LOG_ERR("adc channel setup failed, retval %d\n", retval);
		return -retval;
	}

	const struct adc_sequence sequence = {
		.channels	 = BIT(adc->channel_config.channel_id),
		.buffer		 = sample_buffer,
		.buffer_size = sizeof(sample_buffer),
		.resolution	 = adc->resolution,
	};

	retval = adc_read(adc->dev, &sequence);
	if (retval < 0) {
		LOG_ERR("adc read failed, retval %d\n", retval);
		return retval;
	}

	LOG_INF("adcval: 0x%x, voltage: %dmV", sample_buffer[0], SARADC_LRADC2VOL(sample_buffer[0]));
	return retval;
}

void test_for_saradc(void)
{
	/* read LRADC channel 0~1, gpio mfp must set to 0x18 */
	for(int i = SARADC_ID_LRADC0; i <= SARADC_ID_LRADC1; i++) {
		saradc_read(i);
		printk("\n");
	}
}
