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
#include <sys/byteorder.h>
#include <settings/settings.h>

#include "ble_super_service.h"
#include "ble_data_test_sample.h"

#include "soc.h"

#define ARRY_BUF_SIZE 48
static uint8_t notify_send_buf[ARRY_BUF_SIZE];

static struct k_delayed_work slave_notify_delaywork;
static void slave_notify_work_handle(struct k_work *work);
static K_DELAYED_WORK_DEFINE(slave_notify_delaywork, slave_notify_work_handle);

static struct bt_conn *notify_conn;
static uint16_t notify_index;
static bool writing;

struct k_work_q data_work_q;
K_THREAD_STACK_DEFINE(slave_data_stack, 512);

void update_write_stats(uint16_t len)
{
	static uint32_t curr_time;
	static uint32_t pre_time;
	static uint32_t write_total;

	write_total += len;

	curr_time = k_uptime_get_32();
	if ((curr_time - pre_time) >= 1000) {
		printk("rev write Rx: %d byte\n", write_total);
		write_total = 0;
		pre_time = curr_time;
	}
}

uint8_t curr_num = 1;
static void slave_notify_work_handle(struct k_work *work)
{
	static uint32_t notify_curr_time;
	static uint32_t notify_pre_time;
	static uint32_t notify_TxCount;
	uint16_t mtu;
	uint8_t i;

	if(notify_conn == NULL)
		return;

	if (ble_super_ccc_enabled(notify_conn, notify_index))
	{
		writing = true;

		for(i=0; i<1; i++)
		{
			mtu=1;
			notify_send_buf[0] = curr_num;
			ble_super_send_notify(notify_conn, notify_index, mtu, notify_send_buf);

			curr_num++;
			if (curr_num > 9) {
				curr_num = 1;
			}
			notify_TxCount += mtu;
			notify_curr_time = k_uptime_get_32();

			k_yield();
		}
		if ((notify_curr_time - notify_pre_time) >= 2000)
		{
			printk("notify Tx: %d byte\n", notify_TxCount);
			notify_TxCount = 0;
			notify_pre_time = notify_curr_time;
		}
	}

	k_delayed_work_submit_to_queue(&data_work_q, &slave_notify_delaywork, K_MSEC(2000));
}

void bt_data_trans_init()
{
	uint8_t i;

	for(i=0; i<ARRY_BUF_SIZE; i++)
	{
		notify_send_buf[i] = i + 1;
	}
	k_work_q_start(&data_work_q, slave_data_stack, K_THREAD_STACK_SIZEOF(slave_data_stack), 12);
	k_thread_name_set(&data_work_q.thread, "slave_tdata_stack");

	k_delayed_work_init(&slave_notify_delaywork, slave_notify_work_handle);
}

void bt_data_submit_handle(struct bt_conn *conn, uint16_t index)
{
	if(conn == NULL)
		return;

	if(!writing)
	{
		notify_conn = conn;
		notify_index = index;
		k_delayed_work_submit_to_queue(&data_work_q, &slave_notify_delaywork, K_MSEC(20));
	}
}

void bt_data_trans_cancel(void)
{
	notify_conn = NULL;
	writing = false;
	k_delayed_work_cancel(&slave_notify_delaywork);
}
