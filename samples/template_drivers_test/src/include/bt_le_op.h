#ifndef BT_LE_OP_H
#define BT_LE_OP_H

#include <zephyr.h>
#include "msg_manager.h"
#include <device.h>
#include <drivers/input/input_dev.h>

/* Advertising interval: 500ms (0x320 * 0.625ms = 500ms)
 * 由 100ms 拉長至 500ms 大幅節省功耗 */
#define APP_ADV_INTERVAL  0x320

/* 不可連線廣播 (Non-connectable) — 僅廣播不提供連線，省電 */
#define APP_ADV_PARAM BT_LE_ADV_PARAM(BT_LE_ADV_OPT_NONE, \
				     APP_ADV_INTERVAL, \
				     APP_ADV_INTERVAL, NULL)

/* Status flags for Manufacturer Data */
#define MEDFLO_FLAG_BATTERY_LOW  BIT(0)  /* 低電量警告 */
#define MEDFLO_FLAG_SENSOR_ALERT BIT(1)  /* 感測器異常 (預留) */

void system_ble_event_handle(uint32_t event);
void system_input_event_handle(uint32_t key_event);
void system_input_handle_init(void);

void bt_le_op_init(void);
void app_to_msg(uint8_t type, uint8_t event);

void bt_update_gpio18_status(uint8_t val);
void bt_refresh_advertising(void);
void bt_set_battery_low(bool is_low);   /* 設定低電量旗標，會立即更新廣播 */
void bt_set_sensor_alert(bool alert);   /* 設定感測器異常旗標 (預留) */

enum
{
	START_ADV,
	STOP_ADV,
	CONNECTED,
};

#endif /* BT_LE_OP_H */
