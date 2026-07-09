/*
 * Copyright (c) 2018 Actions (Zhuhai) Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kernel.h>
#include <init.h>
#include <sys/printk.h>
#include <bluetooth/bluetooth.h>

#include "msg_manager.h"
#include "sys_wakelock.h"
#include "system_app_sm.h"
#include "system_app_input.h"
#include "system_app_audio.h"
#include "system_app_timer.h"
#include "system_app_led.h"
#include "ota.h"
#include "soc.h"
#include "soc_pm.h"
#include "system_app_batt.h"
#include "system_app_audio.h"
#include "system_app_ble.h"
#include "sys_monitor.h"
#include "sys_standby.h"
#include "version.h"

#include "sys_log.h"
#include "drivers/cfg_drv/pinctrl_tai.h"
LOG_MODULE_REGISTER(QD_TEST, CONFIG_LOG_DEFAULT_LEVEL);

struct qd_rotation_cnt{
	u16_t x_forward_cnt;
	u16_t x_back_cnt;
	u16_t y_forward_cnt;
	u16_t y_back_cnt;
	u16_t z_forward_cnt;
	u16_t z_back_cnt;
};

struct qd_rotation_cnt qd_cnt;

void qd_notify(struct device *dev, struct input_value *val)
{
	static u32_t notify_cnt = 0;
	static u32_t err_cnt;

	notify_cnt++;

	switch(val->keypad.code){
		case VKEY_QDX_F:
			qd_cnt.x_forward_cnt += val->keypad.value;
			break;

		case VKEY_QDX_R:
			qd_cnt.x_back_cnt += val->keypad.value;
			break;
		
		case VKEY_QDY_F:
			qd_cnt.y_forward_cnt += val->keypad.value;
			break;
		
		case VKEY_QDY_R:
			qd_cnt.y_back_cnt += val->keypad.value;
			break;
		
		case VKEY_QDZ_F:
			qd_cnt.z_forward_cnt += val->keypad.value;
			break;

		case VKEY_QDZ_R:
			qd_cnt.z_back_cnt += val->keypad.value;
			break;

		default:
			err_cnt++;
			break;
	}
}

void test_for_qd()
{
	const struct device *qd = NULL;

	/* get the qd device */
	qd = device_get_binding(CONFIG_QD_NAME);
	if(qd == NULL){
		return;
	}

	input_dev_register_notify(qd, qd_notify);

	input_dev_enable(qd);
}

