/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef BLE_SUPER_SERVICE_H
#define BLE_SUPER_SERVICE_H
#ifdef __cplusplus
extern "C" {
#endif
#include <zephyr.h>

#define SUPER_START_HDL               0x0
#define SUPER_END_HDL                 (SUPER_MAX_HDL - 1)

enum {
	SUPER_SVC_HDL = SUPER_START_HDL,
	SUPER_TEST_CH_HDL,
	SUPER_TEST_HDL,
	SUPER_TEST_CH_CCC_HDL,
	SUPER_MAX_HDL
};

uint8_t ble_super_ccc_enabled(struct bt_conn *conn, uint8_t handle);
void ble_super_send_notify(struct bt_conn *conn, uint8_t index, uint16_t len, uint8_t *p_value);

#ifdef __cplusplus
}
#endif

#endif /* BLE_SUPER_SERVICE_H */
