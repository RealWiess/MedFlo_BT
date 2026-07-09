/*
 * Copyright (c) 2018 Actions (Zhuhai) Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MSG_MANAGER_H__
#define __MSG_MANAGER_H__

#define MSG_CONTENT_SIZE 4

/* msg type */
enum {
	MSG_NULL,

	MSG_KEY_INPUT,
	MSG_LOW_POWER,

	MSG_BLE_STATE,
	MSG_AUDIO_INPUT,
	MSG_APP_TIMER,
	MSG_OTA_EVENT,
	MSG_ATVV_EVENT,
	MSG_HID_OUTPUT_EVENT,
};

struct app_msg;
typedef void (*MSG_CALLBAK)(struct app_msg *, int, void *);

/** message structure */
struct app_msg {
	uint8_t sender;
	uint8_t type;
	uint8_t cmd;
	uint8_t reserve;
	union {
		char content[MSG_CONTENT_SIZE];
		int value;
		void *ptr;
	};
	MSG_CALLBAK callback;
};

bool msg_manager_init(void);
bool send_msg(struct app_msg *msg, k_timeout_t timeout);
bool receive_msg(struct app_msg *msg, k_timeout_t timeout);

#endif
