/*
 * Copyright (c) 2015 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __INC_BOARD_H
#define __INC_BOARD_H

struct pwm_acts_cycle_config{
	unsigned int group;
	unsigned int cycle;
};

struct pwm_acts_pin_config{
	unsigned int pwm;
	unsigned int pin_num;
	unsigned int mode;
};

#endif /* __INC_BOARD_H */
