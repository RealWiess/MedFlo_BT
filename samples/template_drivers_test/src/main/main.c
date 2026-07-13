/*
 * Copyright (c) 2018 Actions (Zhuhai) Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/printk.h>
#include <sys/byteorder.h>
#include <zephyr.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <settings/settings.h>
#include "sys_wakelock.h"
#include "sys_monitor.h"
#include "sys_standby.h"
#include "bt_le_op.h"
#include "msg_manager.h"
#include <device.h>
#include <drivers/input/input_dev.h>
#include "gpio_control.h"
#include <drivers/gpio.h>
#include <rtc_op.h>
#include <drivers/adc.h>

/* --- 電池監控 (ADC讀取IOVCC電壓) --- */
#define BATTERY_LOW_THRESHOLD_MV  2500
#define SARADC_IOVCC2VOL(x)       ((x) * 3600 / 65536)
#define SARADC_ID_IOVCC           (0)

bool g_battery_low = false;

static const struct device *batt_adc_dev;
static struct adc_channel_cfg batt_channel_cfg;

static const struct device *battery_monitor_init(void)
{
	batt_adc_dev = device_get_binding(CONFIG_SARADC_NAME);
	if (batt_adc_dev == NULL) {
		printk("BATTERY: ADC device not found\n");
		return NULL;
	}
	batt_channel_cfg.channel_id = SARADC_ID_IOVCC;
	batt_channel_cfg.gain       = ADC_GAIN_1;
	batt_channel_cfg.reference  = ADC_REF_INTERNAL;
	batt_channel_cfg.acquisition_time = ADC_ACQ_TIME_DEFAULT;
	if (adc_channel_setup(batt_adc_dev, &batt_channel_cfg) < 0) {
		printk("BATTERY: ADC channel setup failed\n");
		return NULL;
	}
	printk("BATTERY: monitor OK, threshold %dmV\n", BATTERY_LOW_THRESHOLD_MV);
	return batt_adc_dev;
}

static int battery_check_and_update(void)
{
	uint16_t sample_buffer[1];
	int voltage_mv;
	if (batt_adc_dev == NULL) return -1;
	const struct adc_sequence sequence = {
		.channels    = BIT(batt_channel_cfg.channel_id),
		.buffer      = sample_buffer,
		.buffer_size = sizeof(sample_buffer),
		.resolution  = 16,
	};
	if (adc_read(batt_adc_dev, &sequence) < 0) return -2;
	voltage_mv = SARADC_IOVCC2VOL(sample_buffer[0]);
	if (voltage_mv < BATTERY_LOW_THRESHOLD_MV) {
		printk("BATTERY: %dmV LOW!\n", voltage_mv);
		g_battery_low = true;
		bt_set_battery_low(true);
	} else {
		g_battery_low = false;
		bt_set_battery_low(false);
	}
	return voltage_mv;
}

extern uint32_t rtc_period;
extern uint32_t counter;
extern void turn_led_all_off(void);
extern void Load_Counter_First(void);
extern void update_counter(void);
extern void update_counter_in_nvram(void);
extern int checkDeviceLifeCycle(void);
extern void test_for_gpio(void);
struct k_timer periodic_timer;
struct k_timer led_periodic_timer;
struct k_timer battery_timer;

#define dwelltime 29000  //29Sec
#define sleeptime 1000000 //1Sec
#define BATTERY_CHECK_INTERVAL 30000  //30Sec
static void periodic_timer_expiry(struct k_timer *timer)
{
	app_to_msg(MSG_BLE_STATE, STOP_ADV);
	k_timer_stop(&led_periodic_timer);
//	printk("24500ms timeout out\r\n");
}
static void led_periodic_timer_expiry(struct k_timer *timer)
{
	display_led();
//	printk("timer100ms\r\n");
}
static void battery_timer_expiry(struct k_timer *timer)
{
	battery_check_and_update();
}

void stop_periodic_timer(void)
{
	k_timer_stop(&periodic_timer);
}

void stop_led_periodic_timer(void)
{
	k_timer_stop(&led_periodic_timer);
}
void init_stop_adv_count(void) {}
void init_start_adv_count(void) {}

void system_app_init(void)
{
	/* input manager initialization */
	system_input_handle_init();

	/* init message manager */
	msg_manager_init();

	/* system monitor initialization */
	sys_monitor_init();

	/* system standby initialization */
	sys_standby_init();

	gpio_control_init();

#ifdef CONFIG_STANDBY_POLL_DELAY_WORK
	/* system monitor start */
	sys_monitor_start(CONFIG_MONITOR_PERIOD);
#endif
}

void main(void)
{
	int err;
	int result = 0;
	struct app_msg msg = {0};

	printk("system app init\n");

//	watchdog_disable();
	system_app_init();
	Load_Counter_First();
//	test_for_gpio();
/*	if(err==0)
	{
		printk("===== RUN bt_le_op_init =====\n");
		err = bt_enable(NULL);
		if (err) {
			printk("Bluetooth init failed (err %d)\n", err);
			return;
		}
		 
		if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
			settings_load();
		}

		bt_le_op_init();

		k_timer_init(&periodic_timer, periodic_timer_expiry, NULL);
		k_timer_start(&periodic_timer, K_MSEC(4500), K_NO_WAIT);
		k_timer_init(&led_periodic_timer, led_periodic_timer_expiry, NULL);
		k_timer_start(&led_periodic_timer, K_MSEC(100), K_MSEC(100));
		turn_led_all_off();
		
	}
	else{
		turn_led_all_off();
		init_gpio_entrysleep();
		while(1){
			
		}
		app_to_msg(MSG_BLE_STATE, STOP_ADV);
	}*/
//	printk("===== RUN bt_le_op_init =====\n")
    printk("V02.00_B03_RTC_28_2_LedOk\r\n");
	if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
		settings_load();
	}

	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
		settings_load();
	}

	bt_id_create(NULL, NULL);

	bt_le_op_init();

	k_timer_init(&periodic_timer, periodic_timer_expiry, NULL);
//	k_timer_start(&periodic_timer, K_MSEC(25000), K_NO_WAIT);
	k_timer_start(&periodic_timer, K_MSEC(dwelltime), K_NO_WAIT);
	k_timer_init(&led_periodic_timer, led_periodic_timer_expiry, NULL);
	k_timer_start(&led_periodic_timer, K_MSEC(100), K_MSEC(100));
	turn_led_all_off();

	/* 電池監控: 每 30 秒檢查電壓 */
	if (battery_monitor_init() != NULL) {
		k_timer_init(&battery_timer, battery_timer_expiry, NULL);
		k_timer_start(&battery_timer, K_MSEC(BATTERY_CHECK_INTERVAL),
			      K_MSEC(BATTERY_CHECK_INTERVAL));
	}
	rtc_init_once();
	
	while (1)
	{
		if (receive_msg(&msg, K_FOREVER))
		{
			switch (msg.type)
			{
				case MSG_KEY_INPUT:
//					printk("MSG_KEY_INPUT %x\n", msg.value);
					system_input_event_handle(msg.value);
					break;
				case MSG_BLE_STATE:
					system_ble_event_handle(msg.value);
					if (msg.value == START_ADV) {
//						turn_green_led_on();
//						init_gpio_exitsleep();
						k_timer_start(&periodic_timer, K_MSEC(dwelltime), K_NO_WAIT);
						k_timer_start(&led_periodic_timer, K_MSEC(100), K_MSEC(100));
						
//						update_counter_in_nvram();

					}
					if (msg.value == STOP_ADV) {
						

						k_timer_stop(&periodic_timer);
						k_timer_stop(&led_periodic_timer);
						turn_led_all_off();
						update_counter();

						/* 不進睡眠，直接重新廣播 (連續廣播不中斷) */
						app_to_msg(MSG_BLE_STATE, START_ADV);
					}
					break;
				default:
					printk("error message type msg.type %d\n", msg.type);
					break;
			}

			if (msg.callback != NULL)
				msg.callback(&msg, result, NULL);
		}
	//	display_led();
	}
}
