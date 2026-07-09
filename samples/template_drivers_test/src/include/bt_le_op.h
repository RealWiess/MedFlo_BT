#ifndef BT_LE_OP_H
#define BT_LE_OP_H

#include <zephyr.h>
#include "msg_manager.h"
#include <device.h>
#include <drivers/input/input_dev.h>

/* Advertising interval: 50ms */
#define APP_ADV_INTERVAL  0x50

#define APP_ADV_PARAM BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONNECTABLE, \
				     APP_ADV_INTERVAL, \
				     APP_ADV_INTERVAL, NULL)

void system_ble_event_handle(uint32_t event);
void system_input_event_handle(uint32_t key_event);
void system_input_handle_init(void);

void bt_le_op_init(void);
void app_to_msg(uint8_t type, uint8_t event);

void bt_update_gpio18_status(uint8_t val);
void bt_refresh_advertising(void);

enum
{
	START_ADV,
	STOP_ADV,
	CONNECTED,
};

#endif /* BT_LE_OP_H */
