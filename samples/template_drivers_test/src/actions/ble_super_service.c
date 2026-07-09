/*Copyright (c) 2018 Actions (Zhuhai) Technology
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include "errno.h"
#include <sys/printk.h>
#include <zephyr.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#include "ble_super_service.h"
#include "ble_data_test_sample.h"

/* Custom Service Variables */
static const struct bt_uuid_128 svc_uuid = BT_UUID_INIT_128(
	0xdd, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
0x00, 0x10, 0x00, 0x00, 0xf6, 0xfe, 0x00, 0x00);

static const struct bt_uuid_128 test_uuid = BT_UUID_INIT_128(
	0x89, 0x78, 0x61, 0x63, 0x74, 0x4c, 0x45, 0xb0,
0xd5, 0x4e, 0xf2, 0x2f, 0x02, 0x00, 0x5f, 0x00);

static uint8_t super_test;

static void super_test_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	super_test = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
	printk("super_test : %d", super_test);
}

static int ble_super_test_write_cb(struct bt_conn *conn,
				  const struct bt_gatt_attr *attr,
				  const void *buf, uint16_t len, uint16_t offset,
				  uint8_t flags)
{
	update_write_stats(len);
	return len;
}

/* demo Service Declaration */
BT_GATT_SERVICE_DEFINE(super_svc,
	/* demo Service Declaration */
	BT_GATT_PRIMARY_SERVICE((void*)&svc_uuid),

	BT_GATT_CHARACTERISTIC(&test_uuid.uuid,
				BT_GATT_CHRC_NOTIFY | BT_GATT_CHRC_WRITE,
				BT_GATT_PERM_WRITE, NULL, ble_super_test_write_cb, NULL),
	BT_GATT_CCC(super_test_ccc_cfg_changed,
			BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);

uint8_t ble_super_ccc_enabled(struct bt_conn *conn, uint8_t handle)
{
	switch (handle) {
	case SUPER_TEST_HDL:
		if (!super_test)
			return 0;
		break;
	}
	return 1;
}

void ble_super_send_notify(struct bt_conn *conn, uint8_t index, uint16_t len, uint8_t *p_value)
{
	bt_gatt_notify(conn, &attr_super_svc[index], p_value, len);
}
