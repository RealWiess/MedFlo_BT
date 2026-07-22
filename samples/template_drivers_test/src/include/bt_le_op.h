#ifndef BT_LE_OP_H
#define BT_LE_OP_H

#include <zephyr.h>
#include "msg_manager.h"
#include <device.h>
#include <drivers/input/input_dev.h>

/* Advertising interval: 100ms (0xA0 * 0.625ms = 100ms) — 上電前 5 秒使用 */
#define APP_ADV_INTERVAL_FAST  0xA0
/* Advertising interval: 500ms (0x320 * 0.625ms = 500ms) — 5 秒後切換省電 */
#define APP_ADV_INTERVAL_SLOW  0x320

#define APP_ADV_PARAM_FAST BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONNECTABLE, \
				     APP_ADV_INTERVAL_FAST, \
				     APP_ADV_INTERVAL_FAST, NULL)

#define APP_ADV_PARAM_SLOW BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONNECTABLE, \
				     APP_ADV_INTERVAL_SLOW, \
				     APP_ADV_INTERVAL_SLOW, NULL)

void system_ble_event_handle(uint32_t event);
void system_input_event_handle(uint32_t key_event);
void system_input_handle_init(void);

void bt_le_op_init(void);
void app_to_msg(uint8_t type, uint8_t event);

void bt_update_gpio18_status(uint8_t val);
void bt_refresh_advertising(void);
void bt_set_battery_low(bool is_low);
void bt_set_sensor_alert(bool alert);

/* Status flags for Manufacturer Data */
#define MEDFLO_FLAG_BATTERY_LOW  BIT(0)
#define MEDFLO_FLAG_SENSOR_ALERT BIT(1)

enum
{
	START_ADV,
	STOP_ADV,
	CONNECTED,
};

#endif /* BT_LE_OP_H */
