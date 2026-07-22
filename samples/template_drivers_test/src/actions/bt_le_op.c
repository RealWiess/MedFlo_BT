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

#include "bt_le_op.h"
#include "ble_data_test_sample.h"
#include "ble_super_service.h"
#include <stdio.h>

extern void stop_periodic_timer(void);
extern void init_stop_adv_count(void);
extern void init_start_adv_count(void);
extern int checkDeviceLifeCycle(void);
extern void check_gpio_mfp(void);
EXTERN_C void init_gpio_led(void);
static struct bt_conn *slave_conn;
char name_mac[20];
uint8_t g_u8_gpio18_status;

/* Manufacturer Data 結構:
 * [0-1] Company ID (0xFFFF)
 * [2]   GPIO18 狀態
 * [3]   Status flags (BIT0=低電量, BIT1=感測器異常) */
struct medflo_manufacturer_data {
	uint16_t company_id;
	uint8_t  gpio_status;
	uint8_t  status_flags;
};
static struct medflo_manufacturer_data mfg_data = {
	.company_id = 0xFFFF,
	.gpio_status = 0,
	.status_flags = 0,
};

struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, name_mac, sizeof(name_mac)-1),
	BT_DATA(BT_DATA_MANUFACTURER_DATA, &mfg_data, sizeof(mfg_data)),
};

struct bt_le_conn_param app_update_cfg = {
	.interval_min = (40),   /* 40 × 1.25ms = 50ms */
	.interval_max = (40),
	.latency = (0),
	.timeout = (400),
};

void bt_update_gpio18_status(uint8_t val) {
    g_u8_gpio18_status = val;
    mfg_data.gpio_status = val;
}

void bt_refresh_advertising(void) {
    bt_le_adv_update_data(ad, ARRAY_SIZE(ad), NULL, 0);
}

void bt_set_battery_low(bool is_low)
{
	if (is_low) {
		mfg_data.status_flags |= MEDFLO_FLAG_BATTERY_LOW;
	} else {
		mfg_data.status_flags &= ~MEDFLO_FLAG_BATTERY_LOW;
	}
	bt_refresh_advertising();
}

void bt_set_sensor_alert(bool alert)
{
	if (alert) {
		mfg_data.status_flags |= MEDFLO_FLAG_SENSOR_ALERT;
	} else {
		mfg_data.status_flags &= ~MEDFLO_FLAG_SENSOR_ALERT;
	}
	bt_refresh_advertising();
}

void set_name_mac(void) {
    bt_addr_le_t addr;
    size_t count = 1;

    bt_id_get(&addr, &count);

    snprintf(name_mac, sizeof(name_mac),
             "MEDFLO-%02X%02X%02X%02X%02X%02X",
             addr.a.val[5], addr.a.val[4], addr.a.val[3],
             addr.a.val[2], addr.a.val[1], addr.a.val[0]);

    bt_set_name(name_mac);
}

void app_to_msg(uint8_t type, uint8_t event)
{
	struct app_msg msg = {0};
	msg.type = type;
	msg.value = event;
	send_msg(&msg, K_MSEC(100));
	printk("app_to_msg.event= %d\n",event);
}

void le_param_updated(struct bt_conn *conn, uint16_t interval,
			     uint16_t latency, uint16_t timeout)
{
	printk("LE conn param updated: int 0x%04x lat %d to %d\n", interval, latency, timeout);
}
 static void exchange_func(struct bt_conn *conn, uint8_t err,
			   struct bt_gatt_exchange_params *params)
 {
	uint16_t mtu;
	mtu = bt_gatt_get_mtu(conn);
	printk("Exchange %s mtu:%d\n", err == 0 ? "successful" : "failed",mtu);
 }

static struct bt_gatt_exchange_params exchange_params = {
	 .func = exchange_func,
 };

/* 上電前 5 秒用 100ms 快速廣播，之後切換為 500ms 省電 */
static struct k_timer slow_adv_timer;
static bool adv_is_slow = false;

static void switch_to_slow_adv(struct k_timer *timer)
{
	bt_le_adv_stop();
	bt_le_adv_start(APP_ADV_PARAM_SLOW, ad, ARRAY_SIZE(ad), NULL, 0);
	adv_is_slow = true;
	printk("Switched to slow advertising (500ms interval)\n");
}

static int start_adv(void)
{
	int err;
	set_name_mac();
	err = bt_le_adv_start(APP_ADV_PARAM_FAST, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return err;
	}
	printk("Advertising successfully started (100ms fast interval)\n");
	/* 上電 5 秒後切換為 500ms 慢速省電廣播 */
	if (!adv_is_slow) {
		k_timer_start(&slow_adv_timer, K_SECONDS(5), K_NO_WAIT);
	}
	return 0;
}

static void stop_adv(void)
{
	 bt_le_adv_stop();
	 printk("stop_adv\n");
}

void conn_update(void)
{
	uint8_t err;

	stop_adv();

	if (exchange_params.func)
	{
		bt_gatt_exchange_mtu(slave_conn, &exchange_params);
	}

	err = bt_conn_le_param_update(slave_conn, &app_update_cfg);
	if (err)
	{
		printk("Conn param update failed(err %d)\n", err);
		return;
	}
}

void connected(struct bt_conn *conn, uint8_t err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Connected: %s\n", addr);
	if (err)
	{
		printk("Connection failed (err 0x%02x)\n", err);
		return;
	}
	else
	{
		printk("Connected\n");
		slave_conn = bt_conn_ref(conn);

		app_to_msg(MSG_BLE_STATE, CONNECTED);

	}
}

void disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	if (conn !=slave_conn)
	{
		return;
	}

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	printk("Disconnected: %s (reason 0x%02x)\n", addr, reason);
	bt_conn_unref(conn);
	slave_conn = NULL;

	bt_data_trans_cancel();
	app_to_msg(MSG_BLE_STATE, START_ADV);
}

void system_ble_event_handle(uint32_t event)
{
	int ret;
	switch(event)
	{
		case START_ADV:
//			check_gpio_mfp();
			init_gpio_led();
			ret =checkDeviceLifeCycle();
			if(ret==0){
				printk("HANDLE START_ADV\n");
				start_adv();
			}
			else{
				 app_to_msg(MSG_BLE_STATE, STOP_ADV);
			}
			break;
		case STOP_ADV:
			printk("HANDLE STOP_ADV\n");
			stop_adv();

			break;
		case CONNECTED:
			conn_update();
			break;
		default:
		printk("unkown ble_event code\n");
		break;
	}
}

void system_input_event_handle(uint32_t event)
{
	switch(event)
	{
		//K1 start adv
		case 0x10:
			if(!slave_conn)
				start_adv();
			break;
		//K2 stop adv
		case 0x11:
				stop_adv();
			break;
		//K11 start notify
		case 0x12:
			if(slave_conn)
			{
				bt_data_submit_handle(slave_conn, SUPER_TEST_HDL);
			}
			break;
		//K3 stop notify
		case 0x13:
			bt_data_trans_cancel();
			break;
		//K6 manual disconnect
		case 0x24:
			if(slave_conn)
			{
				bt_conn_disconnect(slave_conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
			}
			break;
		default:
			printk("unkown input_event code\n");
			break;
	}
}

void key_event_handle_cb(struct device *dev, struct input_value *val)
{
	if(val->keypad.value)
		app_to_msg(MSG_KEY_INPUT, val->keypad.code);
}

void system_input_handle_init(void)
{
	struct device *keypad_dev =(struct device *)device_get_binding(CONFIG_KEYPAD_NAME);

	if (!keypad_dev) {
		printk("can not bind keypad dev!\n");
		return;
	}

	input_dev_enable(keypad_dev);
	input_dev_register_notify(keypad_dev, key_event_handle_cb);
	printk("system_input_handle_init finished\n");
}

static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
	.le_param_updated = le_param_updated,
};

void bt_le_op_init(void)
{
	/* register conn callbacks into bt stack */
	bt_conn_cb_register((struct bt_conn_cb *)&conn_callbacks);

	bt_data_trans_init();

	k_timer_init(&slow_adv_timer, switch_to_slow_adv, NULL);

	start_adv();
}

